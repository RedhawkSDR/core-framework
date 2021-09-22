FROM @@@BASE_IMAGE@@@ as builder

ARG rh_asset

WORKDIR /root/
RUN apt-get update && \
    apt-get install -y git rpm && \
    ln -s bash /bin/sh.bash && \
    mv /bin/sh.bash /bin/sh && \
    mkdir /root/rpms && \
    git clone https://github.com/RedhawkSDR/${rh_asset}.git && \ 
    cd ./${rh_asset} && \
    sed -i "s/^Requires/#Requires/g" *${rh_asset}.spec && \
    sed -i "s/^BuildRequires/#BuildRequires/g" *${rh_asset}.spec && \
    rm -rf /root/${rh_asset}/java && \
    sed -i "s/java/python/g" *${rh_asset}.spec && \
    /bin/bash -lc "./build.sh rpm" && \
    find /root/rpmbuild/RPMS -name "*.rpm" -exec cp {} /root/rpms \; 

FROM @@@BASE_IMAGE@@@ as runner
WORKDIR /root/rpms
COPY --from=builder /root/rpms /root/rpms
RUN apt-get update && \
    apt-get install -y --no-install-recommends alien && \
    cd /root/rpms && \
    alien /root/rpms/*.rpm && \
    dpkg -i /root/rpms/*.deb 


ENTRYPOINT ["/bin/bash", "-lc"]
