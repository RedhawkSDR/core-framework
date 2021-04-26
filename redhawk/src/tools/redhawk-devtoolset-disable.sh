#!/bin/bash
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
# along with this program.  If not, see http://www.gnu.org/licenses/

# Remove devtoolset from env var paths.

# Guard against executing, which fails to set the env vars in the calling shell.
if [[ "${BASH_SOURCE[0]}" -ef "$0" ]]; then
    echo "This script should be sourced, not executed."
    exit 1
fi

function remove_devtoolset() {
    IFS_=$IFS
    IFS=:
    read -a orig_array <<< "$2"
    IFS=$IFS_
    new_array=()
    for element in "${orig_array[@]}"
    do
        [[ "$element" =~ ^/opt/rh/devtoolset ]] || new_array+=($element)
    done
    new_array=$(IFS=: ; echo "${new_array[*]}")
    export $1=$new_array
}

remove_devtoolset PATH $PATH
remove_devtoolset MANPATH $MANPATH
remove_devtoolset INFOPATH $INFOPATH
remove_devtoolset PCP_DIR $PCP_DIR
remove_devtoolset LD_LIBRARY_PATH $LD_LIBRARY_PATH
remove_devtoolset PKG_CONFIG_PATH $PKG_CONFIG_PATH
