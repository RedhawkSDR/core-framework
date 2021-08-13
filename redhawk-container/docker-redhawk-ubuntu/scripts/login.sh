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

Usage: $0 [CONTAINER_NAME] [USER]
	[CONTAINER_NAME]   The Docker Container name to log into
	[USER]             User name (must be valid for the container)

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
else
	if [[ $1 == "-h" ]] || [[ $1 == "--help" ]]; then
		usage
		exit 0
	fi
fi

CONTAINER_NAME=$1
USER_NAME=${2:-root}

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
	echo Joining... Type \"exit\" when finished.
	docker exec -u ${USER_NAME} -it ${CONTAINER_NAME} bash
	;;
esac