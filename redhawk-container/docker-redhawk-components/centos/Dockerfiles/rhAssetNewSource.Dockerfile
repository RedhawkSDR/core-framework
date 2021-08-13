FROM geontech/redhawk-development as builder

ARG rh_asset

COPY ./rhSharedLibrary/root/core-framework/ /root/core-framework
WORKDIR /root/core-framework/redhawk/src
RUN yum install -y git rpm-build automake gstreamer-python libuuid-devel boost-devel cppunit-devel autoconf automake libtool expat-devel gcc-c++ java-1.8.0-openjdk-devel python-devel python-matplotlib-qt4 numpy PyQt4 log4cxx log4cxx-devel omniORB omniORB-devel omniORB-doc omniORB-servers omniORB-utils python-jinja2 xsd libsqlite3x libsqlite3x-devel && \
    /bin/bash -lc "./build.sh"
ENV LD_LIBRARY_PATH=/usr/local/redhawk/core/lib:$LD_LIBRARY_PATH
WORKDIR /root/
RUN git clone https://github.com/RedhawkSDR/${rh_asset}.git
WORKDIR /root/${rh_asset}
RUN /bin/bash -lc "./build.sh rpm" && \
    mkdir /root/rpms && \
    find /root/rpmbuild/RPMS -name "*.rpm" -exec cp {} /root/rpms \;

FROM geontech/redhawk-runtime as runner
WORKDIR /root/rpms
ENV LD_LIBRARY_PATH=/usr/local/redhawk/core/lib:$LD_LIBRARY_PATH
COPY --from=builder /root/core-framework /root/core-framework
COPY --from=builder /root/rpms /root/rpms
RUN cp /root/core-framework/redhawk/src/etc/profile.d/redhawk.sh /etc/profile.d/redhawk.sh && \
    cp /root/core-framework/redhawk/src/etc/profile.d/redhawk.csh /etc/profile.d/redhawk.csh && \
    yum install -y automake libtool gcc-c++ boost-devel libuuid-devel numactl-devel log4cxx-devel && \
    cd /root/rpms && yum install -y /root/rpms/*rpm && \
    cd  /root/core-framework/redhawk/src/base/framework && \
    /bin/bash -lc "make install" && \
    cd /root/core-framework/redhawk/src/base/framework/idl && \
    /bin/bash -lc "make install" && \
    cd /root/core-framework/redhawk/src/control/sdr/ComponentHost && \
    /bin/bash -lc "make install"

# CMD ...run your sandboxed component
ENTRYPOINT ["/bin/bash", "-lc"]
