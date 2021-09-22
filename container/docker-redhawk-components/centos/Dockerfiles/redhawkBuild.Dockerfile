ARG docker_repo
FROM $docker_repo

ARG arch=x86_64
ARG configure_options=""
ARG cxxflags=""
ARG dist=el7
ARG num_threads=8
ARG redhawk_deps_ver=3.0.0

ENV OSSIEHOME=/usr/local/redhawk/core \
    SDRROOT=/usr/local/redhawk/sdr \
    PYTHONPATH=/usr/local/redhawk/core/lib64/python:/usr/local/redhawk/core/lib/python \
    PATH=/usr/local/redhawk/core/bin:$PATH \
    LD_LIBRARY_PATH=/usr/local/redhawk/core/lib64:/usr/local/redhawk/core/lib \
    CXXFLAGS=$cxxflags \
    OMNIEVENTS_LOGDIR=/var/log/omniEvents \
    JACORB_HOME=/usr/share/java/jacorb \
    JAVA_HOME_TMP=/etc/alternatives/java_sdk_11

# Some downstream actions will fail if build and install are done by root.
RUN useradd redhawk && \
    mkdir -p          /src  /usr/local/redhawk/core  /var/redhawk/sdr && \
    chown -R redhawk. /src  /usr/local/redhawk/core  /var/redhawk/sdr

# The Docker build context must contain:
# - tmp/core-framework  # a checkout of https://github.com/RedhawkSDR/core-framework.git
# - tmp/redhawk-dependencies-<version>-<dist>-<arch>.tar.gz  # download from https://github.com/RedhawkSDR/
ADD --chown=redhawk:redhawk tmp/core-framework /src/core-framework
ADD --chown=redhawk:redhawk tmp/redhawk-dependencies-$redhawk_deps_ver-$dist-$arch.tar.gz /src

# Configure the local files as a yum repo.  todo:  gpgcheck=1
RUN dname=redhawk-dependencies-$redhawk_deps_ver-$dist-$arch && \
    repo="[redhawk-deps]\nname=redhawk-deps\nbaseurl=file:///src/$dname/\nenabled=1\n\ngpgcheck=0" && \
    printf $repo >> /etc/yum.repos.d/redhawk-deps.repo

RUN yum install -y autoconf \
                   autoconf-archive \
                   automake \
                   boost-devel \
                   cppcheck \
                   cppunit-devel \
                   createrepo \
                   devtoolset-9-gcc-c++ \
                   devtoolset-9-make \
                   elfutils-libelf-devel \
                   expat-devel \
                   git \
                   gstreamer-python \
                   jacorb \
                   java-1.8.0-openjdk-devel \
                   java-11-openjdk-devel \
                   junit4 \
                   lapack-devel \
                   libomniEvents2-devel \
                   libtool \
                   libuuid-devel \
                   log4cxx-devel \
                   numactl \
                   numactl-devel \
                   numpy \
                   numpy-f2py \
                   octave-devel \
                   omniORB-devel \
                   omniORBpy-devel \
                   python3-pip \
                   python3-psutil \
                   python3-pylint \
                   python3-setuptools \
                   python36-coverage \
                   python36-devel \
                   python36-jinja2 \
                   python36-lxml \
                   python36-nose \
                   python36-pyqt5-sip \
                   python36-qt5 \
                   python36-qt5-base \
                   python36-qt5-devel \
                   rpmdevtools \
                   s3cmd \
                   scipy \
                   sqlite-devel \
                   sudo \
                   supervisor \
                   valgrind \
                   wget \
                   xsd \
                   yum-utils \
                   zlib-devel && \
    yum clean all

# Use the Centos `alternatives` mechanism to set the active `java` and `javac`.
RUN path_java=$(alternatives --display java | grep -e '^\/.*java-11-openjdk' | cut -d ' ' -f 1) && \
    alternatives --set java $path_java && \
    path_javac=$(alternatives --display javac | grep -e '^\/.*java-11-openjdk' | cut -d ' ' -f 1) && \
    alternatives --set javac $path_javac

# Add a config file related to JacORB to the JDK.
RUN printf "%s\n" "org.omg.CORBA.ORBClass=org.jacorb.orb.ORB" >>$JAVA_HOME_TMP/lib/orb.properties && \
    printf "%s\n" "org.omg.CORBA.ORBSingletonClass=org.jacorb.orb.ORBSingleton" >>$JAVA_HOME_TMP/lib/orb.properties && \
    printf "%s\n" "jacorb.config.dir=/etc" >>$JAVA_HOME_TMP/lib/orb.properties && \
    chmod a+r $JAVA_HOME_TMP/lib/orb.properties
ENV JAVA_HOME_TMP=

# Build and install Redhawk.
USER redhawk
WORKDIR /src/core-framework/redhawk/src
RUN /bin/bash -lc "./reconf && \
    . tools/redhawk-devtoolset-enable.sh && \
    ./configure $configure_options && \
    make -j${num_threads}"
USER root
RUN make install -j${num_threads}

USER redhawk
WORKDIR /src/core-framework/bulkioInterfaces
RUN /bin/bash -lc "./reconf && \
    . tools/redhawk-devtoolset-enable.sh && \
    ./configure $configure_options && \
    make -j${num_threads}"
USER root
RUN make install -j${num_threads}

USER redhawk
WORKDIR /src/core-framework/burstioInterfaces
RUN /bin/bash -lc "./reconf && \
    . tools/redhawk-devtoolset-enable.sh && \
    ./configure $configure_options && \
    make -j${num_threads}"
USER root
RUN make install -j${num_threads}

USER redhawk
WORKDIR /src/core-framework/frontendInterfaces
RUN /bin/bash -lc "./reconf && \
    . tools/redhawk-devtoolset-enable.sh && \
    ./configure $configure_options && \
    make -j${num_threads}"
USER root
RUN make install -j${num_threads}

USER redhawk
WORKDIR /src/core-framework/GPP/cpp
RUN /bin/bash -lc "./reconf && \
    . tools/redhawk-devtoolset-enable.sh && \
    ./configure $configure_options && \
    make -j${num_threads}"
USER root
RUN make install -j${num_threads}

WORKDIR /src/core-framework/redhawk-codegen
RUN python3 setup.py install --home=$OSSIEHOME

ENTRYPOINT ["/bin/bash", "-lc"]

