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

CONTAINER_NAME=webserver

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

Usage: $0 start|stop [-o|--omni OMNISERVER]
	start|stop       Start or stop the ${CONTAINER_NAME}
	-p|--port        The port for the server to use (default is 8080)
	--rest-python    Path to alternate REST-Python instance to use
	--args "ARGS"    Additional arguments passed to 'docker run'

Examples:
	Status of ${CONTAINER_NAME}:
		$0

	Start the ${CONTAINER_NAME}:
		$0 start

EOF
}

function print_status () {
	$DIR/container-running.sh ${CONTAINER_NAME}
	case $? in
	2)
		echo ${CONTAINER_NAME} does not exist.
		;;
	1)
		echo ${CONTAINER_NAME} is not running.
		;;
	*)
		IP=""
		if [ $? -eq 0 ]; then
			IP="$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' ${CONTAINER_NAME})"
			if [[ $IP == "" ]]; then
				HOST_IP="$(hostname -I 2> /dev/null)"
				if [ $? -eq 0 ]; then
					IP="$(echo ${HOST_IP} | grep -oP '^(\d{1,3}\.?){4}')"
				else
					for mac_en in {0..4}; do
						# Might be OS X
						IP="$(ipconfig getifaddr en${mac_en})"
						if [ $? -eq 0 ]; then
							break;
						fi
					done
				fi
			fi
		fi
		echo ${CONTAINER_NAME} is at: http://${IP}:8080
		;;
	esac
}

# Parameter checks
if [ -z ${1+x} ]; then
	print_status
	exit 0;
fi

# Try to detect the omniserver
OMNISERVER="$($DIR/omniserver-ip.sh)"

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
		-o|--omni)
			OMNISERVER="${2:?Missing OMNISERVER Argument}"
			shift
			;;
		-p|--port)
			REST_PYTHON_PORT="${2:?Missing REST_PYTHON_PORT Argument}"
			shift
			;;
		--rest-python)
			REST_PYTHON_VOLUME="-v ${2:?Missing REST_PYTHON_VOLUME Argument}:/opt/rest-python"
			shift
			;;
		--args)
			ARGUMENTS="${2:?Missing ARGUMENTS argument}"
			shift
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

REST_PYTHON_VOLUME=${REST_PYTHON_VOLUME:-}
REST_PYTHON_PORT=${REST_PYTHON_PORT:-8080}
ARGUMENTS=${ARGUMENTS:-""}


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
	if [[ $OMNISERVER == "" ]]; then
		usage
		echo ERROR: No omniserver running or OmniORB Server IP specified
		exit 1
	fi

	$DIR/container-running.sh ${CONTAINER_NAME}
	if [ $? -eq 0 ]; then
		echo A ${CONTAINER_NAME} is already running.
		exit 0
	else
		echo Starting ${CONTAINER_NAME} ...

		# If mac, switch to port exposure vs. --network host
		NETWORK_ARGS="-e REST_PYTHON_PORT=${REST_PYTHON_PORT} --network host"
		if [ "$(uname -s)" == "Darwin" ]; then
			NETWORK_ARGS="-p ${REST_PYTHON_PORT}:8080"
		fi
		docker run --rm -d \
			-e OMNISERVICEIP=${OMNISERVER} \
			${NETWORK_ARGS} \
			${REST_PYTHON_VOLUME} \
			${ARGUMENTS} \
			--name ${CONTAINER_NAME} \
			${IMAGE_NAME} &> /dev/null
		
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
