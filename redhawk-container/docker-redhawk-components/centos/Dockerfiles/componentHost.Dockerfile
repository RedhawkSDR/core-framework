FROM geontech/redhawk-development as builder

ARG repo_url=https://github.com/RedhawkSDR/core-framework.git
ARG branch_or_tag=2.2.8
RUN yum install -y epel-release && yum install -y git rpm-build automake gstreamer-python libuuid-devel boost-devel cppunit-devel autoconf automake libtool expat-devel gcc-c++ java-1.8.0-openjdk-devel python-devel python-matplotlib-qt4 numpy PyQt4 log4cxx log4cxx-devel omniORB omniORB-devel omniORB-doc omniORB-servers omniORB-utils python-jinja2 xsd libsqlite3x libsqlite3x-devel yaml-cpp-devel && \
    cd /root && git clone --depth=1 $repo_url && cd core-framework && yum clean all && rm -rf /var/cache/yum && \
    if [ $branch_or_tag != "master" ]; then "git checkout --track origin/$branch_or_tag"; fi

WORKDIR /root/core-framework/redhawk/src
RUN /bin/bash -lc "./build.sh"
    
FROM geontech/redhawk-runtime as runner
COPY --from=builder /root/core-framework /root/core-framework
WORKDIR /etc/redhawk/logging
COPY logrotate.redhawk /etc/redhawk/logging/
RUN cp /root/core-framework/redhawk/src/testing/sdr/dom/mgr/logging.properties /etc/redhawk/logging
WORKDIR /root/rpms
RUN yum install -y automake libtool gcc-c++ boost-devel libuuid-devel numactl-devel log4cxx-devel yaml-cpp-devel && \
    cd  /root/core-framework/redhawk/src/base/framework && \
    /bin/bash -lc "make install" && \
    cd /root/core-framework/redhawk/src/base/framework/idl && \
    /bin/bash -lc "make install" && \
    cd /root/core-framework/redhawk/src/control/sdr/ComponentHost && \
    /bin/bash -lc "make install" && \
    yum clean all && rm -rf /var/cache/yum && \
    rm -rf /root/core-framework && \
    chown root:redhawk /var/redhawk/sdr/dom/mgr/rh/ComponentHost/ComponentHost.spd.xml && \
    chmod 644 /var/redhawk/sdr/dom/mgr/rh/ComponentHost/ComponentHost.spd.xml

# CMD ...run your sandboxed component
ENTRYPOINT ["/bin/bash", "-lc"]
