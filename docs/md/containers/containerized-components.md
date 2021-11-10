# Docker REDHAWK Components

This page covers how to get Docker images that contain REDHAWK components.
These can be:

- downloaded as a pre-built image
- built
    - from REDHAWK Core Assets
    - from custom created components

Component images are compatibile with the core-framework of the same minor release version.


## Pre-Built Images
Prebuilt versions of several REDHAWK Component images are available on DockerHub.
* [HardLimit](https://hub.docker.com/r/geontech/rh.hardlimit)
* [SigGen](https://hub.docker.com/r/geontech/rh.siggen)
* [agc](https://hub.docker.com/r/geontech/rh.agc)
* [AmFmPmBasebandDemod](https://hub.docker.com/r/geontech/rh.amfmpmbasebanddemod)
* [ArbitraryRateResampler](https://hub.docker.com/r/geontech/rh.arbitraryrateresampler)
* [autocorrelate](https://hub.docker.com/r/geontech/rh.autocorrelate)
* [fastfilter](https://hub.docker.com/r/geontech/rh.fastfilter)
* [fcalc](https://hub.docker.com/r/geontech/rh.fcalc)
* [FileWriter](https://hub.docker.com/r/geontech/rh.filewriter)
* [psd](https://hub.docker.com/r/geontech/rh.psd)
* [psk_soft](https://hub.docker.com/r/geontech/rh.psk_soft)
* [RBDSDecoder](https://hub.docker.com/r/geontech/rh.rbdsdecoder)
* [SinkSDDS](https://hub.docker.com/r/geontech/rh.sinksdds)
* [sinksocket](https://hub.docker.com/r/geontech/rh.sinksocket)
* [sourcesocket](https://hub.docker.com/r/geontech/rh.sourcesocket)
* [TuneFilterDecimate](https://hub.docker.com/r/geontech/rh.tunefilterdecimate)
* [ComponentHost](https://hub.docker.com/r/geontech/rh.componenthost) (Includes DataConverter and SourceSDDS by default)

These images were built based on the [docker-redhawk-components](https://github.com/Geontech/docker-redhawk-components) project.
These base images pull down and bake individual RH components from GitHub into a new image.

## Custom Component Images

A Docker image with REDHAWK installed is required to support containerized component development and deployment.
This image must contain the REDHAWK libraries and the appropriate REDHAWK environment variables such as `$SDRROOT`.
Install the image in a location that makes it available for both containerizing components and orchestration software.

### Prepare the Base Image

To build a base image with a particular version of REDHAWK, modify the following commands:
```sh
cd core-framework/container/docker-redhawk-components/tmp
git clone <git-repo-url>/redhawk/core-framework.git
git checkout <branch-or-tag>
wget https://github.com/RedhawkSDR/redhawk/releases/download/<version>/redhawk-yum-<version>-<dist>-<arch>.tar.gz
docker build --build-arg docker_repo=<url> \
    [--build-arg <key>=<value>] \
    --tag my-base-image
    redhawkBuild.Dockerfile
```
The available `--build-arg`s can be seen in the `ARG` lines at the top of the Dockerfile.  
Note that `ARG docker_repo` does not have a default value, so the user must supply an url.  
This builds REDHAWK from source.

See the Installation section of the Manual for more details about downloading the REDHAWK yum archive.  For preparing a base image, the archive only needs to be downloaded.  It does not need to be setup as a yum repository.

####  Verify the Image is Built
```
> docker image list
REPOSITORY            TAG                  IMAGE ID       CREATED          SIZE
my-base-image         latest               bef41b7af47a   45 seconds ago   3.19GB
```

### REDHAWK Asset from GitHub
REDHAWK Asset components can be found at `https://github.com/RedhawkSDR/<asset>`.  These are the core assets that come installed with REDHAWK.  The Dockerfiles found in `core-framework/container/Dockerfiles` call out to this repo depending on the below build argument that specifies which REDHAWK Component to pull down into the image at image build time.

First, to build a desired component into a Docker image, cd into `core-framework/container/Dockerfiles` and run the following command, replacing `<asset>` with the properly capitalized name of the desired REDHAWK component as seen listed in `/var/redhawk/sdr/dom/components/`:

```bash
make rhAsset ASSET=<asset>
```

For example, to build an image for the HardLimit component installed in `/var/redhawk/sdr/dom/components/rh/HardLimit`, run the following command:

```bash
make rhAsset ASSET=HardLimit
```

This will create an image called `rh.hardlimit` (in lowercase). Check this by running `docker image ls`.

### Component Host and Shared Library from GitHub
Shared Library type components all require a Component Host to run on. Therefore, a Component Host image is needed for the components to be baked into. The following command will build the Component Host image. Building the ComponentHost image does not load libraries needed for shared library components.

```bash
cd Dockerfiles
make componentHost
```

The componentHost's Dockerfile has default arguments set for the repo URL and branch of that repo to use as the source for core-framework to pull down and build ComponentHost from. You can override those values like so:

```bash
make componentHost repo_url='https://<username>:<password>@host/of/core-framework' branch_or_tag=3.0.0
```

Now that a base Component Host image with no Shared Library components has been built, the user can now extend this docker image and add Shared Library components. Note that the Shared Library components must be loaded onto the rh.componentHost image made in the previous step instead of being their own separate images. This is because the Shared Library components are launched onto a Component Host namespace and need to be where the Component Host is to do so.

There are two Makefile targets to include a Shared Library component into a Component Host image:
- sharedRhAsset
- sharedCustom

Use `sharedRhAsset` to add a SharedLibrary from GitHub at `https://github.com/RedhawkSDR/<asset>` to your Component Host image.  
Use `sharedCustom` to add a custom-made SharedLibrary to your ComponentHost image.

Use the syntax below to add a pre-existing REDHAWK asset into your ComponentHost image where `<asset>` is the properly capitalized name of the desired SharedLibrary Component.
```bash
make sharedRhAsset ASSET=<asset>
```
for example:
```bash
make sharedRhAsset ASSET=SourceSDDS
```
Use the syntax below to add your custom REDHAWK SharedLibrary Component into your ComponentHost image.

Prior to running this command, copy your Component's directory to `Dockerfiles/rhSharedLibrary/components/` because the Dockerfile attempts to copy your component from this path into the image.

```bash
make sharedCustom CUSTOM=<custom>
```

To add multiple SharedLibrary type components into your ComponentHost, use space-separated syntax. Also be sure to include the runtime dependencies for each SharedLibrary identified in their spd.xml files.

This example adds DataConverter and SourceSDDS, with dependencies, to a ComponentHost image:
```bash
make sharedRhAsset ASSET="dsp fftlib DataConverter SourceSDDS"
```

You can verify that your Components have been installed by running a container and verifying that the Components are in $SDRROOT:
```bash
docker container run --rm -it rh.componenthost /bin/bash
ls -l /var/redhawk/sdr/dom/components/rh/
```

### User Defined REDHAWK Component
A user-defined component is a component that the user made in REDHAWK.  Use these steps:

1. Use the IDE to create PRF, SCD, and SPD files.
1. Outside the IDE, run `redhawk-codegen` on the SPD file.
1. Modify the code for your functionality.
1. Build the component.
1. Modify `custom.Dockerfile` to use the Docker image created in the section, above.

    ```bash
    cd core-framework/container/docker-redhawk-components/Dockerfiles
    vi custom.Dockerfile
    ```

1. Create the image:

    ```bash
    cd core-framework/container/docker-redhawk-components
    make custom CUSTOM=<path-to-SPD-file>
    ```
