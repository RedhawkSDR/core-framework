FROM rh.redhawkbuild:latest as builder

# A checkout of the redhawk core framework must be in the ./tmp directory
WORKDIR /root/core-framework/redhawk/src

FROM rh.redhawkbuild:latest as runner
WORKDIR /etc/redhawk/logging
COPY logrotate.redhawk /etc/redhawk/logging/
RUN cp /root/core-framework/redhawk/src/testing/sdr/dom/mgr/logging.properties /etc/redhawk/logging
WORKDIR /root/rpms
RUN cd  /root/core-framework/redhawk/src/base/framework && \
    /bin/bash -lc "make install" && \
    cd /root/core-framework/redhawk/src/base/framework/idl && \
    /bin/bash -lc "make install" && \
    cd /root/core-framework/redhawk/src/control/sdr/ComponentHost && \
    /bin/bash -lc "make install" && \
    yum clean all && rm -rf /var/cache/yum && \
    rm -rf /root/core-framework && \
#     chown root:redhawk /var/redhawk/sdr/dom/mgr/rh/ComponentHost/ComponentHost.spd.xml && \
    chmod 644 /var/redhawk/sdr/dom/mgr/rh/ComponentHost/ComponentHost.spd.xml

# CMD ...run your sandboxed component
ENTRYPOINT ["/bin/bash", "-lc"]
