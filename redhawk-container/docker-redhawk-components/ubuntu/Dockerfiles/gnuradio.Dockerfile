FROM geontech/redhawk-ubuntu-development as builder

ARG gnu_asset

COPY ./tmpCustom/${gnu_asset} /root/${gnu_asset}
WORKDIR /root/${gnu_asset}
RUN apt-get update && \
    apt-get install -y rpm && \
    ln -s bash /bin/sh.bash && \
    mv /bin/sh.bash /bin/sh && \
    sed -i "s/^Requires/#Requires/g" *${gnu_asset}.spec && \
    sed -i "s/^BuildRequires/#BuildRequires/g" *${gnu_asset}.spec && \
    /bin/bash -lc "./build.sh rpm" && \
    mkdir /root/rpms && \
    find /root/rpmbuild/RPMS -name "*.rpm" -exec cp {} /root/rpms \;

FROM geontech/gnuradio-redhawk-runtime as runner
WORKDIR /root/rpms
#COPY ./tmpCustom/output /root/
COPY --from=builder /root/rpms /root/rpms
RUN apt-get update && \
    apt-get install -y alien && \
    alien /root/rpms/*.rpm && \
    dpkg -i /root/rpms/*.deb

# CMD ...run your sandboxed component
ENTRYPOINT ["/bin/bash", "-lc"]
