# Docker REDHAWK Components
This page covers how to build Docker images that contain REDHAWK components.

Component images are compatible with the core-framework of the same minor release version.

## Custom Component Images
A Docker image with REDHAWK installed is used as a base image into which to add a REDHAWK component.
This base image must contain the REDHAWK libraries and the appropriate REDHAWK environment variables such as `$SDRROOT`.

### Prepare the Base Image
To build a base image with a particular version of REDHAWK, modify the following commands:
```sh
cd core-framework/container/components/tmp
git clone <git-repo-url>/redhawk/core-framework.git
git checkout <branch-or-tag>
wget https://github.com/RedhawkSDR/redhawk/releases/download/<version>/redhawk-yum-<version>-<dist>-<arch>.tar.gz
cd ../Dockerfiles
docker build \
    --build-arg docker_repo=<url> \
    [--build-arg <key>=<value>] \
    --tag my-base-image \
    -f redhawkBuild.Dockerfile \
    .
```
The available `--build-arg`s can be seen in the `ARG` lines at the top of the Dockerfile.  
Note that `ARG docker_repo` does not have a default value, so the user must supply an url.  
This builds REDHAWK from source.

See the Installation section of the Manual for details about downloading the REDHAWK yum archive.  For preparing a base image, the archive only needs to be downloaded.  It does not need to be setup as a yum repository.

####  Verify the Image is Built
```
> docker image list
REPOSITORY            TAG                  IMAGE ID       CREATED          SIZE
my-base-image         latest               bef41b7af47a   45 seconds ago   3.19GB
```

### User Defined REDHAWK Component
A user-defined component is a component that the user made in REDHAWK.  Use these steps:

1. Use the IDE to create PRF, SCD, and SPD files.
1. Outside the IDE, run `redhawk-codegen` on the SPD file.
1. Modify the code for your functionality.
1. Build the component.
1. Modify `custom.Dockerfile` to use the Docker image created in the section, above by changing `@@@BASE_IMAGE@@@` to the name of the image you created.

    ```bash
    cd core-framework/container/components/Dockerfiles
    vi custom.Dockerfile
    ```

1. Create the image:

    ```bash
    cd core-framework/container/components
    make custom CUSTOM=<path-to-root-component-build-directory>
    ```
