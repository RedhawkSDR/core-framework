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

if [ -z ${1+x} ]; then
	echo You must provide a container name.
	exit 1
fi

# Detect the script's location
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

CONTAINER_NAME=$1
DELAY=${2:-5}

$DIR/container-running.sh ${CONTAINER_NAME}
case $? in
	2)
		echo ${CONTAINER_NAME} does not exist
		exit 1
		;;
	1)
		echo ${CONTAINER_NAME} is already stopped
		exit 0
		;;
	*)
		echo Stopping ${CONTAINER_NAME} ...
		# docker stop --time $DELAY ${CONTAINER_NAME} &> /dev/null
		docker kill --signal SIGINT ${CONTAINER_NAME} &> /dev/null

		# Verify it stopped
		sleep $DELAY
		$DIR/container-running.sh ${CONTAINER_NAME}
		if [ $? -eq 0 ]; then
			echo Failed to stop ${CONTAINER_NAME}
			exit 1
		else
			echo Stopped ${CONTAINER_NAME}
			exit 0
		fi
		;;
esac