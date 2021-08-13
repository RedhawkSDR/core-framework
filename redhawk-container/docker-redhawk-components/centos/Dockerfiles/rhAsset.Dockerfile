FROM geontech/redhawk-development as builder

ARG rh_asset

RUN yum install -y git rpm-build automake log4cxx && \
    git clone https://github.com/RedhawkSDR/${rh_asset}.git 
WORKDIR /root/${rh_asset} 
RUN /bin/bash -lc "./build.sh rpm" && \
    mkdir /root/rpms && \
    find /root/rpmbuild/RPMS -name "*.rpm" -exec cp {} /root/rpms \; 

FROM geontech/redhawk-runtime as runner
COPY --from=builder /root/rpms /root/rpms
RUN cd /root/rpms && yum install -y /root/rpms/*rpm


ENTRYPOINT ["/bin/bash", "-lc"]
