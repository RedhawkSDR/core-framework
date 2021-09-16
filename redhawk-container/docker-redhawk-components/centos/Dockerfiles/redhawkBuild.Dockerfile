FROM @@@BASE_IMAGE@@@ as builder

RUN yum install -y autoconf autoconf-archive automake boost-devel cppcheck cppunit-devel createrepo devtoolset-9-gcc++ devtoolset-9 elfutils-libelf-devel \
                   expat-devel git gstreamer-python java-1.8.0-openjdk-devel junit4 lapack-devel libomniEvents2-devel libtool \
                   libuuid-devel log4cxx-devel numactl numactl-devel numpy numpy-f2py octave-devel omniORB-devel omniORBpy-devel \
                   pythonoverage python-devel python-jinja2 python-lxml python-nose python-setuptools python2-pip python2-psutil \
                   python2-pylint rpmdevtools s3cmd scipy sqlite-devel sudo supervisor valgrind wget xsd yum-utils zlib-devel

# A checkout of the redhawk core framework must be in the ./tmp directory
#  The redhawk core framework source can be found at: https://github.com/RedhawkSDR/core-framework.git
ADD ./tmp /root
RUN cd /root/core-framework

ENV OSSIEHOME /usr/local/redhawk/core
ENV SDRROOT /usr/local/redhawk/sdr

WORKDIR /root/core-framework/redhawk/src
RUN /bin/bash -lc "./reconf" && \
    source /root/core-framework/redhawk/src/tools/redhawk-devtoolset-enable.sh && \
    /bin/bash -lc "./configure --disable-java" && \
    /bin/bash -lc "make -j8" && \
    /bin/bash -lc "make install -j8"

# CMD ...navigate the container
ENTRYPOINT ["/bin/bash", "-lc"]
