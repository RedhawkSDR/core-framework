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

function usage () {
	cat <<EOF

Usage: $0 create|delete -s|--sdrroot VOLUME_NAME

EOF
}

source $DIR/volume-labels.sh

function print_status () {
	cat <<EOF

All SDRROOT volumes:
VOLUME NAME
$(docker volume ls --format="{{.Name}}" --filter="label=${SDRROOT_LABEL}")

Volumes mounted to domains:
$(docker ps -a --filter="ancestor=redhawk/domain" --filter="label=${SDRROOT_LABEL}" --format="table {{.Names}}\t{{.Mounts}}")

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
		create|delete)
			COMMAND="$1"
			;;
		-h|--help)
			usage
			exit 0
			;;
		*)
			VOLUME_NAME="$1"
			;;
	esac
	shift # past argument
done

if [ -z ${COMMAND+x} ]; then
	usage
	echo ERROR: You must a command, start or stop
	exit 1
fi
if [ -z ${VOLUME_NAME+x} ]; then
	usage
	echo ERROR: You must provide a \(unique\) volume name
	exit 1
fi

$DIR/volume-manager.sh $COMMAND sdrroot $VOLUME_NAME
