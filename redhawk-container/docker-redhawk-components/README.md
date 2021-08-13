# Docker REDHAWK Components

REDHAWK core-framework pull [request #17](https://github.com/RedhawkSDR/core-framework/pull/17) adds support to the latest 2.2.8 baseline to run redhawk components on container orchestration clusters. This capability is provided by cluster-specific plugins. In order to use those cluster plugins, Docker images of REDHAWK components are needed. When using core-framework's Docker plugin, these images should be present on your machine; when using cluster technology plugins, they should be in the registry referenced in the configuration file ($OSSIEHOME/cluster.cfg).

The user needs to change the `code type` and `entrypoint` in order to correctly indicate to REDHAWK that they will be using a containerized version of the component. The `code type` should be changed to "Container" and the entrypoint to "<executable>::<container-name>", where the "container-name" does not contain the registry portion or tag of the image name. The registry and tag are instead put into the configuration file at `$OSSIEHOME/cluster.cfg` so that they all can be concatenated together later in the code.

These changes to the spd file detailed above can be made to the spd file before compilation and install, or, if it is a build-in REDHAWK component, can be changed directly in SDRROOT (`sudo vi $SDRROOT/dom/components/rh/[ASSET]/[ASSET].spd.xml`).

Images are built based on [docker-redhawk](https://github.com/Geontech/docker-redhawk) and [docker-redhawk-ubuntu](https://github.com/Geontech/docker-redhawk-ubuntu) projects. These base images pull down and bake individual RH components from GitHub into a new image designed to run that selected Component.

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

Read on if you'd like to build your own REDHAWK Component images.

## Building Component Images

### GitHub Core Components

Component images can be based off of either centos or ubuntu images that contain REDHAWK, provided by the [docker-redhawk](https://github.com/Geontech/docker-redhawk) and [docker-redhawk-ubuntu](https://github.com/Geontech/docker-redhawk-ubuntu) projects, respectively. Centos-based images have a smaller footprint than ubuntu components, and core-framework of version 2.2.X is compatible with Component images of the same minor release (2.2.X).

If you want to build a centos-based Component, go to ./centos and if you want a ubuntu build go to ./ubuntu. Please *NOTE* that if you want to use a GNURadio docker container that this must be built on a Ubuntu docker container at this time.

### REDHAWK Asset from GitHub
REDHAWK Asset components can be found at `https://github.com/RedhawkSDR/[ASSET].git`. These are the core components that come installed with REDHAWK. The Dockerfiles found in `centos/Dockerfiles` and `ubuntu/Dockerfiles` call out to this repo depending on the below build argument that specifies which REDHAWK Component to pull down into the image at image build time.

First, to build a desired component into a Docker image, cd into `centos` or `ubuntu` and then run the following command, replacing [ASSET] with the *properly capitalized* name of the desired REDHAWK component as seen listed in `/var/redhawk/sdr/dom/components/`:

```bash
make rhAsset ASSET=[ASSET]
```

For example, to build an image for the HardLimit component installed in `/var/redhawk/sdr/dom/components/rh/HardLimit`, run the following command:

```bash
make rhAsset ASSET=HardLimit
```

This will create an image called `rh.hardlimit` (in all lowercase). Check this by running `docker image ls`.

### Java
Because of the nature of the Ubuntu-based builds, they are currently unable to compile Java components. If you need a Java implementation of a Component, build it with `docker-redhawk` instead of `docker-ubuntu-redhawk.`

### Component Host and Shared Library from GitHub
Shared Library type components all require a Component Host to run on. Therefore, a Component Host image is needed for the components to be baked into. The following command run from inside the `ubuntu` directory will build the Component Host image (although the centos 7 ComponentHost dockerfile builds, it does not currently work at runtime). Building the ComponentHost image *does not*, however, load any specific libraries needed for specific shared library components.

```bash
cd ./ubuntu
make componentHost
```

The componentHost's Dockerfile has default arguments set for the repo URL and branch of that repo to use as the source for core-framework to pull down and build ComponentHost from. You can override those values like so:

```bash
make componentHost repo_url='https://<YourUsername>:<YourPassword>@host/of/core-framework.git' branch_or_tag=2.2.9
```

Now that a base Component Host image with no Shared Library components loaded into it has been built, the user can now extend this docker image and add Shared Library components. Note that the Shared Library components must be loaded onto the rh.componentHost image made in the previous step instead of being their own separate images. This is because the Shared Library components are launched onto a Component Host namespace and need to be where the Component Host is to do so.

There are two Makefile targets to include a Shared Library component into a Component Host image:
1. sharedRhAsset
2. sharedCustom

Use `sharedRhAsset` if you want to add a SharedLibrary available on GitHub at https://github.com/RedhawkSDR/[ASSET].git into your Component Host image. Use `sharedCustom` if you have a custom-made SharedLibrary type component you want to put into your ComponentHost image.

Use the syntax below to add a pre-existing REDHAWK asset into your ComponentHost image where `ASSET` is the properly capitalized name of the desired SharedLibrary Component.
```bash
make sharedRhAsset ASSET=[ASSET]
```
```bash
make sharedRhAsset ASSET=SourceSDDS
```
Use the syntax below to add your custom REDHAWK SharedLibrary Component into your ComponentHost image. **Prior to running this command**, copy your Component's directory to `./ubuntu/Dockerfiles/rhSharedLibrary/components` because the Dockerfile attempts to copy your component from this path into the image.

```bash
make sharedCustom CUSTOM=[CUSTOM]
```

To add *multiple* SharedLibrary type components into your ComponentHost, space separate your SharedLibraries. Also be sure to include the runtime dependencies for each SharedLibrary identified in their spd.xml files:

Add DataConverter and SourceSDDS, with dependencies, to empty ComponentHost image:
```bash
make sharedRhAsset ASSET="dsp fftlib DataConverter SourceSDDS"
```

You can verify that your Components have been installed by running a container and verifying that the Components are in $SDRROOT:
```bash
docker container run --rm -it rh.componenthost /bin/bash
ls -l /var/redhawk/sdr/dom/components/rh/
```

### GNURadio Component
Building a GNURadio Component is a multi-step process and is only supported on Ubuntu-based builds:
1. Install package dependencies
2. Install [gr-redhawk-integration](https://github.com/Geontech/gr-redhawk_integration.git)
3. Modify your GNURadio Component's flowgraph (*.grc) file to become compatible with [gr-component_converter.](https://github.com/Geontech/gr-component_converter)
4. Convert your flowgraph file into a REDHAWK Component using gr-component_coverter
5. Dockerize the Component
6. Install Component to your SDRROOT

#### Install the package dependencies

```bash
sudo yum install -y gnuradio gnuradio-devel
```

#### Install gr-redhawk-integration

```bash
sudo yum group install -y "Development Tools" && sudo yum install -y cmake cppunit cppunit-devel
git clone https://github.com/Geontech/gr-redhawk_integration.git
cd gr-redhawk-integration
```
Then follow the build directions for "Source or Package Manager Installations" from [gr-redhawk-integration](https://github.com/Geontech/gr-redhawk_integration.git)

### Modify your GNURadio Component's Flowgraph
You can find sample *.grc files in the [gr-component_converter repo.](https://github.com/Geontech/gr-component_converter/tree/master/test)

Either grab a sample or grab your own ready *.grc file and move it into this project at `ubuntu/Dockerfiles/tmp-custom/<yourfile>.grc`

Open your *.grc file in gnuradio-companion and modify it to use `rh_source_bulkio` and `rh_sink_bulkio`
```bash
PYTHONPATH=/usr/local/lib64/python2.7/site-packages/ gnuradio-companion
```
 Then File > Open and browse to your path where the grc file lives and open it `ubuntu/Dockerfiles/tmp-custom/double_ref.grc`
 
 Your flowgraph should resemble the following picture below when configured correctly (double_ref flowgraph used as an example):
 ![Double Ref Example](Double_ref_Example.png)

#### Convert your flowgraph into a REDHAWK Component
GNURadio components need to be converted to REDHAWK Components and then are able to be dockerized like any other REDHAWK component. First, ensure the `*.grc` flowgraph is in `./ubuntu/Dockerfiles/tmp-custom`. The Makefile in this project run the conversion step for you, assuming your *.grc has been put in the correct path first.

#### Dockerize 
Then run the following makefile command to dockerize the GNU script:

```bash
make gnuradio GRC=[GRC]
```
Where GRC is the name of your *.grc file without the .grc suffix.

#### Install to SDRROOT
Once it is dockerized, go to `./ubuntu/Dockerfiles/tmp-custom/[GRC]` and run `./build.sh install`. This will install the component to the native host's SDRROOT. Now edit `$SDRROOT/dom/components/[GRC]/[GRC].spd.xml` to use the code type `Container` rather than `Executable`.

### User Defined REDHAWK Component
A user-defined Component is a Component that the user made in REDHAWK themselves. These components can be made in any version of REDHAWK so long as the version of REDHAWK that is used for the base in the image matches where the Component was made. This is the only Dockerfile that is provided that requires `geontech/redhawk-development` and `geontech/redhawk-runtime`. The Dockerfile provided here is just an example and supports REDHAWK Components that were made on REDHAWK 2.2.8.

```bash
make custom CUSTOM=[CUSTOM]
```

Also, please note that the SharedLibrary Custom components need to be made in REDHAWK 2.2.1 if you want to use them with the Component Host container provided, as the pre-built ComponetHost image is based on Docker REDHAWK Ubuntu 2.2.1. If a SharedLibrary component needs a different version of REDHAWK then the Component Host needs to be rebuilt with that REDHAWK version as its base.
