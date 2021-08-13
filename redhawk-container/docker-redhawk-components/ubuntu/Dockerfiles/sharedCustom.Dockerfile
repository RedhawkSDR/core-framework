FROM geontech/redhawk-ubuntu-development as builder

ARG shared_asset

COPY ./rhSharedLibrary/components/ /root/components
RUN apt-get update && \
    apt-get install -y git rpm alien && \
    ln -s bash /bin/sh.bash && \
    mv /bin/sh.bash /bin/sh 
RUN mkdir /root/rpms && \
    for i in ${shared_asset}; do echo ${i} && pushd /root/components/${i} && sed -i "s/^Requires/#Requires/g" *.spec && sed -i "s/^BuildRequires/#BuildRequires/g" *.spec && /bin/bash -lc "./build.sh rpm" && find /root/rpmbuild/RPMS -name "*.rpm" -exec cp {} /root/rpms \; && pushd /root/rpms && /bin/bash -lc "alien *.rpm" && dpkg -i *.deb && popd && popd; done
    
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
