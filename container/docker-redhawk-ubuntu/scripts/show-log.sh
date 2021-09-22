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

# Pulls everything that inherits from redhawk/runtime
function print_status() {
	docker ps -a \
		--filter="ancestor=redhawk/runtime" \
		--filter="status=running" \
		--format="table {{.Image}}\t{{.Names}}\t{{.Status}}"
}

function usage () {
	cat <<EOF

Usage: $0 CONTAINER_NAME [options]
	[CONTAINER_NAME]    The Docker Container name to view
	[-l|--log LOG_NAME] The log file name(s) to display.
	                    The default is to show all found in 
	                     /var/log/supervisord/... .log
	[-n      NUM_LINES] The number of lines of the log to display, default 50

Examples:
	List all running containers:
		$0

	Join a Domain container as the "redhawk" user
		$0 REDHAWK_TEST2 redhawk

EOF
}

# Param check, help, etc.
if [ -z ${1+x} ]; then
	print_status
	exit 0;
fi

# Parse arguments
LOG_NAME=""
while [[ $# -gt 0 ]]; do
	key="$1"
	case $key in
		-l|--log)
			LOG_NAME="${2:?Missing LOG_NAME Argument} ${LOG_NAME}"
			shift
			;;
		-n)
			NUM_LINES="${2:?Missing NUM_LINES Argument}"
			shift
			;;
		-h|--help)
			usage
			exit 0
			;;
		*)
			if ! [ -z ${CONTAINER_NAME+x} ]; then
				usage
				echo ERROR: CONTAINER_NAME is already set.
				exit 1
			fi
			CONTAINER_NAME="$1"
			;;
	esac
	shift # past argument
done

if [ -z ${CONTAINER_NAME+x} ]; then
	usage
	echo ERROR: Must specify a container name.
	exit 1
fi

# Defaults...
NUM_LINES="${NUM_LINES:-50}"

# Check for the container
$DIR/container-running.sh ${CONTAINER_NAME}
case $? in
2)
	echo ${CONTAINER_NAME} does not exist
	exit 1
	;;
1)
	echo ${CONTAINER_NAME} is not running
	exit 0
	;;
*)
	echo Displaying log... Type \"CTRL+C\" when finished.
	if [ -z ${LOG_NAME} ]; then
		LOG_NAME='/var/log/supervisord/*.log'
	fi
	docker exec -it ${CONTAINER_NAME} bash -c "cat ${LOG_NAME} | tail -fn ${NUM_LINES} ${LOG_NAME}"
	;;
esac