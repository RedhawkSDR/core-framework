FROM rh.redhawkbuild:latest as builder

ARG custom_asset

COPY ./tmpCustom/${custom_asset} /root/${custom_asset}
WORKDIR /root/${custom_asset}
RUN yum install -y rpm-build git && \
    source /usr/local/redhawk/core/bin/redhawk-devtoolset-enable.sh && \
    /bin/bash -lc "./build.sh rpm" && \
    mkdir /root/rpms && \
    find /root/rpmbuild/RPMS -name "*.rpm" -exec cp {} /root/rpms \;

FROM rh.redhawkbuild:latest as runner
WORKDIR /root/rpms
COPY --from=builder /root/rpms /root/rpms
RUN yum install -y /root/rpms/*.rpm

# CMD ...run your sandboxed component
ENTRYPOINT ["/bin/bash", "-lc"]
