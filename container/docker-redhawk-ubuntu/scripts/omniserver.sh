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

CONTAINER_NAME=omniserver

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

function usage () {
	cat <<EOF

Usage: $0 start|stop 
	start|stop       Start or stop the omniserver

Examples:
	Status of omniserver:
		$0

	Start the omniserver:
		$0 start

EOF
}

function print_status () {
	$DIR/container-running.sh ${CONTAINER_NAME}
	case $? in
	2)
		echo omniserver does not exist.
		;;
	1)
		echo omniserver is not running.
		;;
	*)
		IP=$($DIR/omniserver-ip.sh)
		echo omniserver IP address: ${IP}
		;;
	esac
}

# Parameter checks
if [ -z ${1+x} ]; then
	print_status
	exit 0;
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
			;;
		-h|--help)
			usage
			exit 0
			;;
		*)
			echo ERROR: Undefined option: $1
			exit 1
			;;
	esac
	shift # past argument
done

# Enforce required and defaults
if [ -z ${COMMAND+x} ]; then
	usage
	echo ERROR: No command specified \(start or stop\)
	exit 1
fi


# Check if the image is installed yet, if not, build it.
$DIR/image-exists.sh ${IMAGE_NAME}
if [ $? -gt 0 ]; then
	echo ${IMAGE_NAME} was not built yet, building now
	make -C $DIR/.. ${IMAGE_NAME} || { \
		echo Failed to build ${IMAGE_NAME}; exit 1;
	}
fi

# Check the container and the command
if [[ $COMMAND == "start" ]]; then
	$DIR/container-running.sh ${CONTAINER_NAME}
	if [ $? -eq 0 ]; then
		echo A ${CONTAINER_NAME} is already running.
		exit 0
	else
		echo Starting ${CONTAINER_NAME} ...
		docker run --rm -d \
			--network host \
			--name ${CONTAINER_NAME} ${IMAGE_NAME} &> /dev/null
		
		# Verify it's running
		$DIR/container-running.sh ${CONTAINER_NAME}
		if [ $? -gt 0 ]; then
			echo Failed to start ${IMAGE_NAME}
			docker stop ${CONTAINER_NAME} &> /dev/null
			exit 1
		else
			echo Started ${CONTAINER_NAME}
			print_status
			exit 0
		fi
	fi
elif [[ $COMMAND == "stop" ]]; then
	$DIR/stop-container.sh ${CONTAINER_NAME}
else
	echo Unknown command $COMMAND
	exit 1
fi
