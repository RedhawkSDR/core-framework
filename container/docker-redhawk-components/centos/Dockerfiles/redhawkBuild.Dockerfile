ARG docker_repo
FROM $docker_repo

ARG arch=x86_64
ARG cxxflags=""
ARG dist=el7
ARG redhawk_deps_ver=3.0.0
ARG target_env=${arch}-redhat-linux-gnu

ENV CXXFLAGS=$cxxflags \
    JACORB_HOME=/usr/share/java/jacorb \
    JAVA_HOME_TMP=/etc/alternatives/java_sdk_11 \
    LD_LIBRARY_PATH=/usr/local/redhawk/core/lib64:/usr/local/redhawk/core/lib \
    OMNIEVENTS_LOGDIR=/var/log/omniEvents \
    OSSIEHOME=/usr/local/redhawk/core \
    PATH=/usr/local/redhawk/core/bin:$PATH \
    PYTHONPATH=/usr/local/redhawk/core/lib64/python:/usr/local/redhawk/core/lib/python \
    SDRROOT=/var/redhawk/sdr

# Docker build context requires:
#   - tmp/core-framework (Local checkout of https://github.com/RedhawkSDR/core-framework.git)
#   - tmp/redhawk-dependencies-<version>-<dist>-<arch>.tar.gz (Available for download from https://github.com/RedhawkSDR/)
COPY tmp/core-framework /src/core-framework
ADD  tmp/redhawk-dependencies-$redhawk_deps_ver-$dist-$arch.tar.gz /src

# Configure redhawk-dependencies-<version>-<dist>-<arch>.tar.gz file as a yum repository
RUN dname=redhawk-dependencies-$redhawk_deps_ver-$dist-$arch && \
    repo="[redhawk-deps]\nname=redhawk-deps\nbaseurl=file:///src/$dname/\nenabled=1\n\ngpgcheck=0" && \
    printf $repo >> /etc/yum.repos.d/redhawk-deps.repo; \
    yum install -y autoconf \
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
                   python3-setuptools \
                   python36-coverage \
                   python36-devel \
                   python36-jinja2 \
                   python36-lxml \
                   python36-nose \
                   python36-numpy \
                   python36-pylint \
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

# Use the CentOS `alternatives` mechanism to set the active `java` and `javac` to Java 11
RUN path_java=$(alternatives --display java | grep -e '^\/.*java-11-openjdk' | cut -d ' ' -f 1) && \
    alternatives --set java $path_java && \
    path_javac=$(alternatives --display javac | grep -e '^\/.*java-11-openjdk' | cut -d ' ' -f 1) && \
    alternatives --set javac $path_javac; \
# Add a config file related to JacORB to the Java Development Kit
    printf "%s\n" "org.omg.CORBA.ORBClass=org.jacorb.orb.ORB"                   >> $JAVA_HOME_TMP/lib/orb.properties && \
    printf "%s\n" "org.omg.CORBA.ORBSingletonClass=org.jacorb.orb.ORBSingleton" >> $JAVA_HOME_TMP/lib/orb.properties && \
    printf "%s\n" "jacorb.config.dir=/etc"                                      >> $JAVA_HOME_TMP/lib/orb.properties && \
    chmod a+r $JAVA_HOME_TMP/lib/orb.properties; \
# Add omniORB configuration file
    printf "%s\n" "InitRef = NameService=corbaname::127.0.0.1:2809"             >  /etc/omniORB.cfg && \
    printf "%s\n" "supportBootstrapAgent = 1"                                   >> /etc/omniORB.cfg && \
    printf "%s\n" "InitRef = EventService=corbaloc::127.0.0.1:11169/omniEvents" >> /etc/omniORB.cfg
ENV JAVA_HOME_TMP=

# Build and install Redhawk RPMs
WORKDIR /src/core-framework/redhawk
ARG spec_file=src/releng/redhawk.spec
ARG build_dir=/src/core-framework/redhawk/src
RUN /bin/bash -lc  'name=`rpm --query --queryformat "%{name}\n" --specfile "${spec_file}"| head -n 1` && \
                    version=`rpm --query --queryformat "%{version}\n" --specfile "${spec_file}" | head -n 1`; \
                    tar --transform "s,^,$name-$version/,S" -czf ${build_dir}/$name-$version.tar.gz *; \
                    tar tf ${build_dir}/$name-$version.tar.gz; \
                    mkdir -p /root/rpmbuild/SOURCES ${build_dir}/rpms; \
                    mv ${build_dir}/$name-$version.tar.gz /root/rpmbuild/SOURCES; \
                    yum-builddep -y ${spec_file}; \
                    scl enable devtoolset-9 "rpmbuild --target=${target_env} -bb ${spec_file}"; \
                    mv /root/rpmbuild/RPMS/${arch}/* ${build_dir}/rpms; \
                    yum install -y ${build_dir}/rpms/*; \
                    yum clean all -y; \
# Remove Redhawk RPMs and rpmbuild directory
                    rm -rf ${build_dir}/rpms; \
                    rm -rf /root/rpmbuild/'

# Build and install Bulkio Interfaces RPMs
WORKDIR /src/core-framework/bulkioInterfaces
ARG spec_file=bulkioInterfaces.spec
ARG build_dir=/src/core-framework/bulkioInterfaces
RUN /bin/bash -lc  'name=`rpm --query --queryformat "%{name}\n" --specfile "${spec_file}"| head -n 1` && \
                    version=`rpm --query --queryformat "%{version}\n" --specfile "${spec_file}" | head -n 1`; \
                    tar --transform "s,^,$name-$version/,S" -czf ${build_dir}/$name-$version.tar.gz *; \
                    tar tf ${build_dir}/$name-$version.tar.gz; \
                    mkdir -p /root/rpmbuild/SOURCES ${build_dir}/rpms; \
                    mv ${build_dir}/$name-$version.tar.gz /root/rpmbuild/SOURCES; \
                    yum-builddep -y ${spec_file}; \
                    scl enable devtoolset-9 "rpmbuild --target=${target_env} -bb ${spec_file}"; \
                    mv /root/rpmbuild/RPMS/${arch}/* ${build_dir}/rpms; \
                    yum install -y ${build_dir}/rpms/*; \
                    yum clean all -y; \
# Remove Bulkio Interfaces RPMs and rpmbuild directory
                    rm -rf ${build_dir}/rpms; \
                    rm -rf /root/rpmbuild/'

# Build and install Burstio Interfaces RPMs
WORKDIR /src/core-framework/burstioInterfaces
ARG spec_file=burstioInterfaces.spec
ARG build_dir=/src/core-framework/burstioInterfaces
RUN /bin/bash -lc  'name=`rpm --query --queryformat "%{name}\n" --specfile "${spec_file}"| head -n 1` && \
                    version=`rpm --query --queryformat "%{version}\n" --specfile "${spec_file}" | head -n 1`; \
                    tar --transform "s,^,$name-$version/,S" -czf ${build_dir}/$name-$version.tar.gz *; \
                    tar tf ${build_dir}/$name-$version.tar.gz; \
                    mkdir -p /root/rpmbuild/SOURCES ${build_dir}/rpms; \
                    mv ${build_dir}/$name-$version.tar.gz /root/rpmbuild/SOURCES; \
                    yum-builddep -y ${spec_file}; \
                    scl enable devtoolset-9 "rpmbuild --target=${target_env} -bb ${spec_file}"; \
                    mv /root/rpmbuild/RPMS/${arch}/* ${build_dir}/rpms; \
                    yum install -y ${build_dir}/rpms/*; \
                    yum clean all -y; \
# Remove Burstio Interfaces RPMs and rpmbuild directory
                    rm -rf ${build_dir}/rpms; \
                    rm -rf /root/rpmbuild/'

# Build and install Frontend Interfaces RPMs
WORKDIR /src/core-framework/frontendInterfaces
ARG spec_file=frontendInterfaces.spec
ARG build_dir=/src/core-framework/frontendInterfaces
RUN /bin/bash -lc  'name=`rpm --query --queryformat "%{name}\n" --specfile "${spec_file}"| head -n 1` && \
                    version=`rpm --query --queryformat "%{version}\n" --specfile "${spec_file}" | head -n 1`; \
                    tar --transform "s,^,$name-$version/,S" -czf ${build_dir}/$name-$version.tar.gz *; \
                    tar tf ${build_dir}/$name-$version.tar.gz; \
                    mkdir -p /root/rpmbuild/SOURCES ${build_dir}/rpms; \
                    mv ${build_dir}/$name-$version.tar.gz /root/rpmbuild/SOURCES; \
                    yum-builddep -y ${spec_file}; \
                    scl enable devtoolset-9 "rpmbuild --target=${target_env} -bb ${spec_file}"; \
                    mv /root/rpmbuild/RPMS/${arch}/* ${build_dir}/rpms; \
                    yum install -y ${build_dir}/rpms/*; \
                    yum clean all -y; \
# Remove Frontend Interfaces RPMs and rpmbuild directory
                    rm -rf ${build_dir}/rpms; \
                    rm -rf /root/rpmbuild/'

# Build and install GPP RPMs
WORKDIR /src/core-framework/GPP
ARG spec_file=GPP.spec
ARG build_dir=/src/core-framework/GPP
RUN /bin/bash -lc  'name=`rpm --query --queryformat "%{name}\n" --specfile "${spec_file}"| head -n 1` && \
                    version=`rpm --query --queryformat "%{version}\n" --specfile "${spec_file}" | head -n 1`; \
                    tar --transform "s,^,$name-$version/,S" -czf ${build_dir}/$name-$version.tar.gz *; \
                    tar tf ${build_dir}/$name-$version.tar.gz; \
                    mkdir -p /root/rpmbuild/SOURCES ${build_dir}/rpms; \
                    mv ${build_dir}/$name-$version.tar.gz /root/rpmbuild/SOURCES; \
                    yum-builddep -y ${spec_file}; \
                    scl enable devtoolset-9 "rpmbuild --target=${target_env} -bb ${spec_file}"; \
                    mv /root/rpmbuild/RPMS/${arch}/* ${build_dir}/rpms; \
                    yum install -y ${build_dir}/rpms/*; \
                    yum clean all -y; \
# Remove GPP RPMs and rpmbuild directory
                    rm -rf ${build_dir}/rpms; \
                    rm -rf /root/rpmbuild/'

# Build and install redhawk-codegen RPMs
WORKDIR /src/core-framework/redhawk-codegen
ARG spec_file=redhawk-codegen.spec
ARG build_dir=/src/core-framework/redhawk-codegen
RUN /bin/bash -lc  'name=`rpm --query --queryformat "%{name}\n" --specfile "${spec_file}"| head -n 1` && \
                    version=`rpm --query --queryformat "%{version}\n" --specfile "${spec_file}" | head -n 1`; \
                    tar --transform "s,^,$name-$version/,S" -czf ${build_dir}/$name-$version.tar.gz *; \
                    tar tf ${build_dir}/$name-$version.tar.gz; \
                    mkdir -p /root/rpmbuild/SOURCES ${build_dir}/rpms; \
                    mv ${build_dir}/$name-$version.tar.gz /root/rpmbuild/SOURCES; \
                    yum-builddep -y ${spec_file}; \
                    scl enable devtoolset-9 "rpmbuild --target=${target_env} -bb ${spec_file}"; \
                    mv /root/rpmbuild/RPMS/noarch/* ${build_dir}/rpms; \
                    python3 setup.py install --home=$OSSIEHOME; \
                    yum install -y ${build_dir}/rpms/*; \
                    yum clean all -y; \
# Remove redhawk-codegen RPMs and rpmbuild directory
                    rm -rf ${build_dir}/rpms; \
                    rm -rf /root/rpmbuild/'

WORKDIR /src/core-framework
