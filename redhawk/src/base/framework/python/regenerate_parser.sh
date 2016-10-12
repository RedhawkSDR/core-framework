#!/usr/bin/env bash
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#


if test "x$GENERATE_DS" == "x"; then 
  # Assume it's on the path
  GENERATE_DS="generateDS.py"
fi

GENERATE_DS_FLAGS="-f --no-process-includes --silence -m"
echo "Generating DCD parser"
${GENERATE_DS} ${GENERATE_DS_FLAGS} -o ossie/parsers/dcd.py     ../../../xml/xsd/dcd.xsd
echo "Generating DMD parser"
${GENERATE_DS} ${GENERATE_DS_FLAGS} -o ossie/parsers/dmd.py     ../../../xml/xsd/dmd.xsd
echo "Generating DPD parser"
${GENERATE_DS} ${GENERATE_DS_FLAGS} -o ossie/parsers/dpd.py     ../../../xml/xsd/dpd.xsd
echo "Generating PRF parser"
${GENERATE_DS} ${GENERATE_DS_FLAGS} -o ossie/parsers/prf.py     ../../../xml/xsd/prf.xsd
echo "Generating PROFILE parser"
${GENERATE_DS} ${GENERATE_DS_FLAGS} -o ossie/parsers/profile.py ../../../xml/xsd/profile.xsd
echo "Generating SAD parser"
${GENERATE_DS} ${GENERATE_DS_FLAGS} -o ossie/parsers/sad.py     ../../../xml/xsd/sad.xsd
echo "Generating SCD parser"
${GENERATE_DS} ${GENERATE_DS_FLAGS} -o ossie/parsers/scd.py     ../../../xml/xsd/scd.xsd
echo "Generating SPD parser"
${GENERATE_DS} ${GENERATE_DS_FLAGS} -o ossie/parsers/spd.py     ../../../xml/xsd/spd.xsd
