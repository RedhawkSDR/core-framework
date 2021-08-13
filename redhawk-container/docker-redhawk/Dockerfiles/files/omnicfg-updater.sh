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
set -e

# If OMNISERVICEIP hasn't been set, use our IP so the service is exposed
if [ "${OMNISERVICEIP}" == "127.0.0.1" ]; then
	export OMNISERVICEIP=`hostname -I | grep -oP "^(\d{1,3}\.?){4}"`
fi

echo Setting OmniORB Service IP to: $OMNISERVICEIP
sed -i "s/127\.0\.0\.1/$OMNISERVICEIP/g" /etc/omniORB.cfg

# If OMNISERVICEPORTS is set, patch in the related command
if [ ! -z ${OMNISERVICEPORTS} ] && [ ! $(grep "giop:tcp::" /etc/omniORB.cfg) ]; then
    echo Setting OmniORB Port Range to: $OMNISERVICEPORTS
    echo "endPoint default = giop:tcp::${OMNISERVICEPORTS}" >> /etc/omniORB.cfg
fi
