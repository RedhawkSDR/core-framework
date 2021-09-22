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
	gpss=$(docker ps -a --filter="ancestor=${IMAGE_NAME}" --format="{{.Names}}")
	printf "%-40s %-20s\n" "Name" "Status"
	for gps in $gpss; do
		status=$(docker inspect $gps -f {{.State.Status}})
		printf "%-40s %-20s\n" $gps $status
	done
}

function usage () {
	cat <<EOF

Usage: $0 start|stop NODE_NAME (options)
	[-s|--serialport SERIAL_PORT] Serial Port for the GPS, default is /dev/ttyUSB0
	[-d|--domain     DOMAIN_NAME] Domain Name, default is REDHAWK_DEV
	[-o|--omni       OMNISERVER]  IP to the OmniServer (detected: ${OMNISERVER})
	[-p|--print]                  Just print resolved settings

Examples:
	Start or stop a node:
		$0 start|stop DevMgr_MyGPS

		# Results in a container: DevMgr_MyGPS-REDHAWK_DEV

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
		-s|--serialport)
			SERIAL_PORT="${2:?Missing SERIAL_PORT Argument}"
			shift
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
SERIAL_PORT=${SERIAL_PORT:-/dev/ttyUSB0}
DOMAIN_NAME=${DOMAIN_NAME:-REDHAWK_DEV}

if ! [ -z ${JUST_PRINT+x} ]; then
	cat <<EOF
Resolved Settings:
	COMMAND:      ${COMMAND}
	SERIAL_PORT:  ${SERIAL_PORT}
	NODE_NAME:    ${NODE_NAME}
	DOMAIN_NAME:  ${DOMAIN_NAME}
	OMNISERVER:   ${OMNISERVER:-None Specified}
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
			    -e GPS_PORT=${SERIAL_PORT} \
			    -e NODENAME=${NODE_NAME} \
			    -e DOMAINNAME=${DOMAIN_NAME} \
			    -e OMNISERVICEIP=${OMNISERVER} \
			    -v ${SERIAL_PORT}:${SERIAL_PORT} \
			    --privileged \
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