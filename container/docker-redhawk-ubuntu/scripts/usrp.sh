#!/bin/bash
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
	usrps=$(docker ps -a --filter="ancestor=${IMAGE_NAME}" --format="{{.Names}}")
	printf "%-40s %-20s\n" "Name" "Domain" "Status"
	for usrp in $usrps; do
		status=$(docker inspect $usrp -f {{.State.Status}})
		printf "%-40s %-20s\n" $usrp $status
	done
}

function usage () {
	cat <<EOF

Usage: $0 start|stop NODE_NAME
	[-d|--domain  DOMAIN_NAME] Domain Name, default is REDHAWK_DEV
	[-o|--omni    OMNISERVER]  IP to the OmniServer (detected: ${OMNISERVER})
	[--usrptype   USRP_TYPE]   USRP type according to UHD (b200, etc., optional)
	[--usrpserial USRP_SERIAL] USRP Serial number for UHD (optional)
	[--usrpname   USRP_NAME]   USRP name according to UHD (optional)
	[--usrpip     USRP_IP]     USRP IP Address (for networked devices, optional)
	[--list-usb]               Print list of possible USB USRPs
	[-p|--print]               Print resolved settings

Examples:
	Start or stop a node:
		$0 start|stop DevMgr_MyUSRP --domain REDHAWK_TEST2

		# Results in a container: DevMgr_MyUSRP-REDHAWK_TEST2

	Status of all locally-running ${IMAGE_NAME} instances:
		$0

EOF
}

function list_usb_devices () {
	EVID=2500

	printf '\n'
	echo The following USB devices have Ettus\'s Vendor ID \(${EVID}\):
	
	mapfile -t devs < <(lsusb -d ${EVID}:)
	count=${#devs[@]}

	for (( c=0; c<$count; c++)); do
		if [[ "${devs[$c]}" =~ ([0-9]{3})[[:space:]]Device[[:space:]]([0-9]{3}) ]]; then
			vid=${BASH_REMATCH[1]}
			pid=${BASH_REMATCH[2]}
		fi
		usb_path=/dev/bus/usb/${vid}/${pid}
		printf '\t%-20s -> %-40s\n' $usb_path "${devs[$c]}"
	done
	printf '\n'
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
		--usrptype)
			USRP_TYPE="${2:?Missing USRP_TYPE Argument}"
			shift
			;;
		--usrpname)
			USRP_NAME="${2:?Missing USRP_NAME Argument}"
			shift
			;;
		--usrpserial)
			USRP_SERIAL="${2:?Missing USRP_SERIAL Argument}"
			shift
			;;
		--usrpip)
			USRP_IP="${2:?Missing USRP_IP Argument}"
			shift
			;;
		--list-usb)
			list_usb_devices
			exit 0
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

if ! [ -z ${JUST_PRINT+x} ]; then
	cat <<EOF
Resolved Settings:
	COMMAND:      ${COMMAND}
	NODE_NAME:    ${NODE_NAME}
	DOMAIN_NAME:  ${DOMAIN_NAME}
	USRP_NAME:    ${USRP_NAME:-Not Specified}
	USRP_TYPE:    ${USRP_TYPE:-Not Specified}
	USRP_SERIAL:  ${USRP_SERIAL:-Not Specified}
	USRP_IP:      ${USRP_IP:-Not Specified}
	OMNISERVER:   ${OMNISERVER:-Not Specified}
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

# The container name will be the node name
CONTAINER_NAME=${NODE_NAME}-${DOMAIN_NAME}

# Handle the command
if [[ $COMMAND == "start" ]]; then
	if [[ $OMNISERVER == "" ]]; then
		usage
		echo ERROR: No omniserver running or OmniORB Server IP specified
		exit 1
	fi

	# Verify at least one of the usrp options has been set
	if [ -z ${USRP_NAME+x} ] && [ -z ${USRP_TYPE+x} ] && [ -z ${USRP_SERIAL+x} ] && [ -z ${USRP_IP+x} ] ]; then
		usage
		echo ERROR: At least one of the --usrp... options must be specified to find the device
		exit 1
	fi

	# If usrptype is b100, b200, the USB option must be set.
	USB_DEVIC=""
	case ${USRP_TYPE} in
		b100) ;&
		b200)
			USB_DEVICE="-v /dev/bus/usb:/dev/bus/usb --privileged"
			;;
		e100) ;&
		e300)
			echo You should consider running REDHAWK on the device instead
			echo using meta-redhawk-sdr, for example.
			;&
		*)
			# Assuming the rest are networkable
			if [ -z ${USRP_IP+x} ]; then
				echo ERROR: Networked USRPs must be specified with an IP address
				exit 1
			fi
			;;
	esac

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
			    -e USRP_NAME=${USRP_NAME} \
			    -e USRP_TYPE=${USRP_TYPE:-} \
			    -e USRP_IP=${USRP_IP:-} \
			    -e USRP_SERIAL=${USRP_SERIAL:-} \
			    ${USB_DEVICE} \
			    --net host \
				--name ${CONTAINER_NAME} \
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
