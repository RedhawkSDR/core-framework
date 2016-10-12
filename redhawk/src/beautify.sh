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

svnversion | grep "M"
if [ $? == 0 ]; then 
  echo "Beautify cannot be run on a modified working copy."
  echo "Revert all changes before beautification 'svn revert -R .'"
  exit 1
fi 
rm MD5SUMS
make clean
make all
find ./framework -maxdepth 1 -name '*.o' -exec strip -s '{}' ';'
find ./framework -maxdepth 1 -name '*.o' -exec md5sum '{}' >> MD5SUMS ';'

find ./framework -maxdepth 1 -name '*.c' -exec astyle --options=astylerc '{}' ';'
find ./framework -maxdepth 1 -name '*.cpp' -exec astyle --options=astylerc '{}' ';'
find ./include/ossie -maxdepth 1 -name '*.h' -exec astyle --options=astylerc '{}' ';'
make clean
make all
find ./framework -maxdepth 1 -name '*.o' -exec strip -s '{}' ';'
md5sum -c MD5SUMS
