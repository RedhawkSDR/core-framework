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

filename=$(basename "$SOURCE")
source ${DIR}/image-name.sh
IMAGE_NAME=$(image_name "${filename%.*}")

# Check if the image is installed yet, if not, build it.
$DIR/image-exists.sh ${IMAGE_NAME}
if [ $? -gt 0 ]; then
	echo "${IMAGE_NAME} was not built yet, building now"
	make -C $DIR/..  ${IMAGE_NAME} || { \
		echo Failed to build ${IMAGE_NAME}; exit 1;
	}
fi

# Prints the status for all containers inheriting from redhawk/development
function print_status() {
	docker ps -a \
		--filter="ancestor=${IMAGE_NAME}"\
		--format="table {{.Names}}\t{{.Mounts}}\t{{.Networks}}\t{{.Status}}"
}

# Try to detect the omniserver
OMNISERVER="$($DIR/omniserver-ip.sh)"

# Prints command usage information
function usage () {
	cat <<EOF

Usage: $0 create|delete options...
       $0 run (NAME, optional) options...
	[create|delete|run]             Create/Delete or Run with the volume(s) 
	                                specified
	[-s|--sdrroot   SDRROOT_VOLUME] SDRROOT Volume Name
	[-w|--workspace WORKSPACE]      Workspace (absolute path or named volume)
	[-o|--omni      OMNISERVER]     IP to the OmniServer 
	                                (detected: ${OMNISERVER})
	[-n|--name]                     Container name to use, default is:
	                IDE_${SDRROOT_VOLUME}_${WORKSPACE}
	[-p|--print]                    Just print resolved settings
	[-h|--help]                     This message.

Examples:
	Start an IDE with a workspace on your ~/redhawk_workspace path:
		$0 run \\
		    --sdrroot MY_SDRROOT \\
		    --workspace \${HOME}/redhawk_workspace

	Create a workspace and run with an existing SDRROOT:
		$0 create --workspace ${USER}_Workspace
		$0 run \\
		    --sdrroot MY_SDRROOT \\
		    --workspace ${USER}_Workspace

	Status of all locally-running ${IMAGE_NAME} instances:
		$0

SECURITY NOTE:
	The resulting container will mount and chown the workspace with your
	current user's ID.  By using this script you accept this potential
	security risk.

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
			if ! [ -z ${COMMAND+x} ]; then
				usage
				echo ERROR: The create, delete, and run commands are mutually exclusive.
				exit 1
			fi
			COMMAND="$1"
			;;
		run)
			if ! [ -z ${COMMAND+x} ]; then
				usage
				echo ERROR: The create, delete, and run commands are mutually exclusive.
				exit 1
			fi
			COMMAND="$1"

			if ! [ -z ${2} ] && ! [[ ${2:0:1} == '-' ]]; then
				CONTAINER_NAME="${2}"
				shift
			fi
			;;
		-s|--sdrroot)
			SDRROOT_VOLUME="${2:?Missing SDRROOT_VOLUME Argument}"
			shift
			;;
		-w|--workspace)
			WORKSPACE="${2:?Missing WORKSPACE Argument}"
			shift
			;;
		-o|--omni)
			OMNISERVER="${2:?Missing OMNISERVER Argument}"
			shift
			;;
		-n|--name)
			CONTAINER_NAME="${2:?Missing CONTAINER_NAME Argument}"
			shift
			;;
		-h|--help)
			usage
			exit 0
			;;
		-p|--print)
			JUST_PRINT=YES
			;;
		*)
			echo ERROR: Undefined option: $1
			exit 1
			;;
	esac
	shift # past argument
done

if ! [ -z ${JUST_PRINT+x} ]; then
	cat <<EOF
Resolved Settings:
	COMMAND:        ${COMMAND:-None Specified}
	SDRROOT_VOLUME: ${SDRROOT_VOLUME:-None Specified}
	WORKSPACE:      ${WORKSPACE:-None Specified}
	OMNISERVER:     ${OMNISERVER:-None Specified}
	CONTAINER_NAME: ${CONTAINER_NAME:-None Specified}
EOF
	exit 0
fi

if [ -z ${COMMAND+x} ]; then
	usage
	echo ERROR: Specify a command \(create, delete, run\)
	exit 1
fi

# Source the volume labels
source $DIR/volume-labels.sh

case ${COMMAND} in
	create)
		if ! [ -z ${WORKSPACE+x} ]; then
			if [ -d ${WORKSPACE} ]; then
				echo ERROR: Provide a name for a volume for the workspace, not a path.
				exit 1
			else
				$DIR/volume-manager.sh create workspace ${WORKSPACE}
			fi
		fi

		if ! [ -z ${SDRROOT_VOLUME+x} ]; then
			$DIR/volume-manager.sh create sdrroot ${SDRROOT_VOLUME}
		fi
		;;
	delete)
		if ! [ -z ${WORKSPACE+x} ]; then
			if [ -d ${WORKSPACE} ]; then
				echo ERROR: Provide a name for a volume for the workspace, not a path.
				exit 1
			else
				$DIR/volume-manager.sh delete ${WORKSPACE}
			fi
		fi

		if ! [ -z ${SDRROOT_VOLUME+x} ]; then
			$DIR/volume-manager.sh delete ${SDRROOT_VOLUME}
		fi
		;;
	run)
		# Enforce required options
		if [ -z ${WORKSPACE+x} ]; then
			usage
			echo ERROR: No workspace specified \(absolute path or Docker volume name\)
			exit 1
		else
			# Verify it's either a folder or volume name.
			$DIR/volume-exists.sh ${WORKSPACE} ${WORKSPACE_LABEL}
			if [ $? -eq 1 ] && [ ! -d "$WORKSPACE" ] ; then
				usage
				echo ERROR: ${WORKSPACE} Does not exist as a directory or Docker Volume.
				exit 1
			fi
		fi

		if [ -z ${SDRROOT_VOLUME+x} ]; then
			usage
			echo ERROR: No SDRROOT \(Docker\) Volume specified
			exit 1
		else
			$DIR/volume-exists.sh ${SDRROOT_VOLUME} ${SDRROOT_LABEL}
			if [ $? -eq 1 ]; then
				usage
				echo ERROR: ${SDRROOT_VOLUME} Does not exist with label: ${SDRROOT_LABEL}
				exit 1
			fi
		fi

		# Check if we know where the OmniORB server is.
		if [[ $OMNISERVER == "" ]]; then
			usage
			echo ERROR: No omniserver running or OmniORB Server IP specified
			exit 1
		fi

		# Default SDRROOT_VOLUME and volume command
		# Q) Why uuidgen for SDRROOT_VOLUME default?
		# A) Almost no way it'll ever be true, so sdrroot-cmd will be empty.
		SDRROOT_VOLUME=${SDRROOT_VOLUME:-$(uuidgen)}
		SDRROOT_CMD="$($DIR/sdrroot-cmd.sh $SDRROOT_VOLUME)"

		# Workspace command
		WORKSPACE_CMD="-v ${WORKSPACE}:/home/user/workspace"

		# Container name default is the IDE_${SDRROOT_VOLUME}_${WORKSPACE}
		CONTAINER_NAME=${CONTAINER_NAME:-IDE_${SDRROOT_VOLUME}_${WORKSPACE}}

		# And if it has any slashes, swap those for dashes :-)
		CONTAINER_NAME=${CONTAINER_NAME//\//-}

		# Check if such a workspace container exists
		$DIR/container-running.sh ${CONTAINER_NAME}
		case $? in
			0)
				echo IDE is already running for this workspace \(see ${CONTAINER_NAME}\)
				exit 0
				;;
			1)
				echo IDE for ${CONTAINER_NAME} has stopped.
				echo Use 'docker rm ${CONTAINER_NAME}' if you are finished with it.
				exit 1
				;;
			2)
				# Does not exist (good, let's make it)
				X11_UNIX=/tmp/.X11-unix
				docker run --rm -d \
					-e RHUSER_ID=$(id -u) \
					-e OMNISERVICEIP=${OMNISERVER} \
					-e DISPLAY=$DISPLAY \
					${SDRROOT_CMD} \
					${WORKSPACE_CMD} \
					-v $X11_UNIX:$X11_UNIX \
					--net host \
					--name ${CONTAINER_NAME} \
					${IMAGE_NAME} &> /dev/null
				;;
			*)
				echo ERROR: Unknown container state for ${CONTAINER_NAME}: $?
				exit 1
				;;
		esac
		;;
	*)
		echo ERROR: Unknown command: ${COMMAND}
		exit 1
esac
