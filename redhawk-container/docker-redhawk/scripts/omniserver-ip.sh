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
# Resolves what IP address is attached to the omni server container

# Detect the script's location
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

OMNISERVER_NAME=omniserver

GUESS=""
$DIR/container-running.sh ${OMNISERVER_NAME}
if [ $? -eq 0 ]; then
	GUESS="$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' ${OMNISERVER_NAME})"
	if [[ $GUESS == "" ]]; then
        HOST_IP="$(hostname -I 2> /dev/null)"
        if [ $? -eq 0 ]; then
            GUESS="$(echo ${HOST_IP} | grep -oP '^(\d{1,3}\.?){4}')"
        else
            for mac_en in {0..4}; do
                # Might be OS X
                GUESS="$(ipconfig getifaddr en${mac_en})"
                if [ $? -eq 0 ]; then
                    break;
                fi
            done
        fi
	fi
fi
echo $GUESS