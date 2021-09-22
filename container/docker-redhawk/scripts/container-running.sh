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
# Returns :
#   2 if the container doesn't exist
#   1 if the container is stopped
#   0 if the container is running
RESULT="$(docker inspect -f {{.State.Running}} $1 2> /dev/null)"
if [ $? -eq 1 ]; then
	# Error, not found
	exit 2
else
	if [[ $RESULT == "true" ]]; then
		# Ok, running
		exit 0
	else
		# Error, stopped
		exit 1
	fi
fi