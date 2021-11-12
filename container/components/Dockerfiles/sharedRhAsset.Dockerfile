FROM @@@BASE_IMAGE@@@ as builder

ARG shared_asset

RUN yum install -y git rpm-build automake && \
    source /usr/local/redhawk/core/bin/redhawk-devtoolset-enable.sh && \
    mkdir /root/rpms && \
    for i in ${shared_asset}; do pushd /root/ && git clone https://github.com/RedhawkSDR/${i}.git && pushd ${i} && /bin/bash -lc "./build.sh rpm" && find /root/rpmbuild/RPMS -name "*.rpm" -exec cp {} /root/rpms \; && popd && popd && /bin/bash -lc "yum install -y *rpm"; done && \
    ls /root/rpms

FROM rh.componenthost as runner
WORKDIR /root/rpms
RUN rm -rf /root/rpms/*
COPY --from=builder /root/rpms /root/rpms
RUN yum install -y /root/rpms/*.rpm
    
# CMD ...run your sandboxed component
ENTRYPOINT ["/bin/bash", "-lc"]
