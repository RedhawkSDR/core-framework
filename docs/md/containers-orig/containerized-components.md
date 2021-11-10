# Docker REDHAWK Components

REDHAWK core-framework [pull request #17](https://github.com/RedhawkSDR/core-framework/pull/17) adds support to the 3.0.0 baseline to run REDHAWK components on container orchestration clusters. This capability is provided by cluster-specific plugins. In order to use those cluster plugins, Docker images of REDHAWK components are needed. When using core-framework's Docker plugin, these images should be present on your machine; when using cluster technology plugins, they should be in the registry referenced in the configuration file ($OSSIEHOME/cluster.cfg).

The user needs to change the `code type` and `entrypoint` in order to correctly indicate to REDHAWK that they will be using a containerized version of the component. The `code type` should be changed to "Container" and the entrypoint to `<executable>::<container-name>`, where the "container-name" does not contain the registry portion or tag of the image name. The registry and tag are instead put into the configuration file `$OSSIEHOME/cluster.cfg`, and they are all concatenated together in the code.

These changes to the spd file can be made before compilation and install, or directly in `$SDRROOT`, eg:
```
sudo vi $SDRROOT/dom/components/rh/<asset>/<asset>.spd.xml
```

Images are built based on the [docker-redhawk](https://github.com/Geontech/docker-redhawk) project. These base images pull down and bake individual RH components from GitHub into a new image designed to run that selected Component.

# Pre-Built Images
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


## Custom Component Images

### GitHub Core Components

Custom component images are based off of images that contain REDHAWK.  Such an image can be built with `redhawkBuild.Dockerfile`, or downloaded from the [docker-redhawk](https://github.com/Geontech/docker-redhawk) project.

Custom component images are compatibile with the core-framework of the same minor release version.

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
A user-defined Component is a Component that the user made in REDHAWK themselves. These components can be made in any version of REDHAWK so long as the version of REDHAWK that is used for the base in the image matches where the Component was made. This is the only Dockerfile that is provided that requires `geontech/redhawk-development` and `geontech/redhawk-runtime`. The Dockerfile provided here is just an example and supports REDHAWK Components that were made on REDHAWK 2.2.8.

```bash
make custom CUSTOM=<custom>
```

