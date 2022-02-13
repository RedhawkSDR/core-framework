FROM @@@BASE_IMAGE@@@ as builder

ARG shared_asset

COPY ./rhSharedLibrary/components/ /root/components
WORKDIR /root/rpms
RUN yum install -y git rpm-build automake && \
    yum install -y gstreamer-python libuuid-devel boost-devel cppunit-devel autoconf automake libtool expat-devel gcc-c++ java-1.8.0-openjdk-devel python-devel python-matplotlib-qt4 numpy PyQt4 log4cxx log4cxx-devel omniORB omniORB-devel omniORB-doc omniORB-servers omniORB-utils python-jinja2 xsd libsqlite3x libsqlite3x-devel && \
    for i in ${shared_asset}; do pushd /root/components/${i} && /bin/bash -lc "./build.sh rpm" && find /root/rpmbuild/RPMS -name "*.rpm" -exec cp {} /root/rpms \; && popd && /bin/bash -lc "yum install -y *rpm"; done
    
FROM rh.componenthost as runner
WORKDIR /root/rpms
RUN rm -rf /root/rpms/*
COPY --from=builder /root/rpms /root/rpms
RUN yum install -y git rpm-build automake && \
    yum localinstall -y /root/rpms/*

# CMD ...run your sandboxed component
ENTRYPOINT ["/bin/bash", "-lc"]
