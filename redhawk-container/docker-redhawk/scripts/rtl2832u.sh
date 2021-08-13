#!/usr/bin/env bash
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of Docker REDHAWK.
#
# Docker REDHAWK is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# Docker REDHAWK is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
# Detect the script's location
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

filename=$(basename "$SOURCE")
source ${DIR}/image-name.sh
IMAGE_NAME=$(image_name "${filename%.*}")

# Try to detect the omniserver
OMNISERVER="$($DIR/omniserver-ip.sh)"

function print_status() {
	rtls=$(docker ps -a --filter="ancestor=${IMAGE_NAME}" --format="{{.Names}}")
	printf "%-40s %-20s\n" "Name" "Status"
	for rtl in $rtls; do
		status=$(docker inspect $rtl -f {{.State.Status}})
		printf "%-40s %-20s\n" $rtl $status
	done
}

function usage () {
	cat <<EOF

Usage: $0 start|stop NODE_NAME
	[-d|--domain  DOMAIN_NAME] Domain Name, default is REDHAWK_DEV
	[-o|--omni    OMNISERVER]  IP to the OmniServer (detected: ${OMNISERVER})
	[--rtlname    RTL_NAME]    RTL2832U Device name (optional)
	[--rtlvendor  RTL_VENDOR]  RTL Vendor Name (optional)
	[--rtlproduct RTL_PRODUCT] RTL Product Name (optional)
	[--rtlserial  RTL_SERIAL]  RTL Serial number (optional)
	[--rtlindex   RTL_INDEX]   RTL Device Index (optional)
	[   --args   "ARGS"]       Additional arguments passed to 'docker run'
	[-p|--print]               Print resolved settings

Examples:
	Start or stop a node:
		$0 start|stop DevMgr_MyRTL --domain REDHAWK_TEST2

	Status of all locally-running ${IMAGE_NAME} instances:
		$0

EOF
}

if [ -z ${1+x} ]; then
	print_status
	exit 0
fi

# Parse arguments
while [[ $# -gt 0 ]]; do
	key="$1"
	case $key in
		start|stop)
			if ! [ -z ${COMMAND+x} ]; then
				usage
				echo ERROR: The start and stop commands are mutually exclusive.
				exit 1
			fi
			COMMAND="$1"

			if [ -z ${2} ] || [[ ${2:0:1} == '-' ]]; then
				usage
				echo ERROR: The next argument after the command must be the name.
				exit 1
			else
				NODE_NAME="${2}"
				shift
			fi
			;;
		-d|--domain)
			DOMAIN_NAME="${2:?Missing DOMAIN_NAME Argument}"
			shift
			;;
		-o|--omni)
			OMNISERVER="${2:?Missing OMNISERVER Argument}"
			shift
			;;
		-h|--help)
			usage
			exit 0
			;;
		--rtlname)
			RTL_NAME="${2:?Missing RTL_NAME Argument}"
			shift
			;;
		--rtlvendor)
			RTL_VENDOR="${2:?Missing RTL_VENDOR Argument}"
			shift
			;;
		--rtlproduct)
			RTL_PRODUCT="${2:?Missing RTL_PRODUCT Argument}"
			shift
			;;
		--rtlserial)
			RTL_SERIAL="${2:?Missing RTL_SERIAL Argument}"
			shift
			;;
		--rtlindex)
			RTL_INDEX="${2:?Missing RTL_INDEX Argument}"
			shift
			;;
		--args)
			ARGUMENTS="${2:?Missing ARGUMENTS argument}"
			shift
			;;
		-p|--print)
			JUST_PRINT=YES
			;;
		*)
			echo ERROR: Undefined option: $1
			exit 1
			;;
	esac
	shift # past argument
done


if [ -z ${COMMAND+x} ]; then
	usage
	echo ERROR: No command specified \(start or stop\)
	exit 1
fi

# Enforce defaults
DOMAIN_NAME=${DOMAIN_NAME:-REDHAWK_DEV}
ARGUMENTS=${ARGUMENTS:-""}

if ! [ -z ${JUST_PRINT+x} ]; then
	cat <<EOF
Resolved Settings:
	COMMAND:      ${COMMAND}
	NODE_NAME:    ${NODE_NAME}
	DOMAIN_NAME:  ${DOMAIN_NAME}
	RTL_NAME:     ${RTL_NAME}
	RTL_VENDOR:   ${RTL_VENDOR:-Not Specified}
	RTL_PRODUCT:  ${RTL_PRODUCT:-Not Specified}
	RTL_SERIAL:   ${RTL_SERIAL:-Not Specified}
	RTL_INDEX:    ${RTL_INDEX:-Not Specified}
	OMNISERVER:   ${OMNISERVER}
EOF
	exit 0
fi

# Check if the image is installed yet, if not, build it.
$DIR/image-exists.sh ${IMAGE_NAME}
if [ $? -gt 0 ]; then
	echo "${IMAGE_NAME} was not built yet, building now"
	make -C $DIR/..  ${IMAGE_NAME} || { \
		echo Failed to build ${IMAGE_NAME}; exit 1;
	}
fi

# The container name will be the Node name with the domain name
CONTAINER_NAME=${NODE_NAME}-${DOMAIN_NAME}

# Handle the command
if [[ $COMMAND == "start" ]]; then
	if [[ $OMNISERVER == "" ]]; then
		usage
		echo ERROR: No omniserver running or OmniORB Server IP specified
		exit 1
	fi

	$DIR/container-running.sh ${CONTAINER_NAME}
	case $? in
		1)
			echo Starting...$(docker start ${CONTAINER_NAME})
			exit 0;
			;;
		0)
			echo ${IMAGE_NAME} ${CONTAINER_NAME} is already running
			exit 0;
			;;
		*)
			# Does not exist (expected), create it.
			echo Connecting to omniserver: $OMNISERVER

			docker run --rm -d \
			    -e NODENAME=${NODE_NAME} \
			    -e DOMAINNAME=${DOMAIN_NAME} \
			    -e OMNISERVICEIP=${OMNISERVER} \
			    -e RTL_NAME=${RTL_NAME:-} \
			    -e RTL_VENDOR=${RTL_VENDOR:-} \
			    -e RTL_PRODUCT=${RTL_PRODUCT:-} \
			    -e RTL_SERIAL=${RTL_SERIAL:-} \
			    -e RTL_INDEX=${RTL_INDEX:-} \
			    -v /dev/bus/usb:/dev/bus/usb \
			    --privileged \
			    --net host \
				--name ${CONTAINER_NAME} \
				${ARGUMENTS} \
				${IMAGE_NAME} &> /dev/null

			# Verify it is running
			sleep 1
			$DIR/container-running.sh ${CONTAINER_NAME}
			if [ $? -gt 0 ]; then
				echo Failed to start ${CONTAINER_NAME}
				docker stop ${CONTAINER_NAME} &> /dev/null
				exit 1
			else
				echo Started ${CONTAINER_NAME}
				exit 0
			fi
			;;
	esac
elif [[ $COMMAND == "stop" ]]; then
	$DIR/stop-container.sh ${CONTAINER_NAME}
fi
