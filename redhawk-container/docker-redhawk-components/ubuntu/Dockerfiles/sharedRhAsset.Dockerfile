FROM geontech/redhawk-ubuntu-development as builder

ARG shared_asset

RUN apt-get update && \
    apt-get install -y git rpm && \
    ln -s bash /bin/sh.bash && \
    mv /bin/sh.bash /bin/sh 
RUN for i in ${shared_asset}; do git clone https://github.com/RedhawkSDR/${i}.git && pushd ${i} && sed -i "s/^Requires/#Requires/g" *${i}.spec && sed -i "s/^BuildRequires/#BuildRequires/g" *${i}.spec && /bin/bash -lc "./build.sh rpm" && popd; done && \
    mkdir /root/rpms && \
    find /root/rpmbuild/RPMS -name "*.rpm" -exec cp {} /root/rpms \;
    
FROM rh.componenthost as runner
WORKDIR /root/rpms
RUN rm -rf /root/rpms/*
COPY --from=builder /root/rpms /root/rpms
RUN apt-get update && \
    apt-get install -y alien && \
    cd /root/rpms/ && \
    alien /root/rpms/*.rpm && \
    dpkg -i /root/rpms/*.deb

# CMD ...run your sandboxed component
ENTRYPOINT ["/bin/bash", "-lc"]

# Example docker build . -t tmp_comphost --build-arg shared_asset="fftlib dsp DataConverter"
# Example docker build . -t tmp_comphost --build-arg shared_asset="SourceSDDS"
