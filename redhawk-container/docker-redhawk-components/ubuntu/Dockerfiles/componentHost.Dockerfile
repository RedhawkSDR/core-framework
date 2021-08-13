FROM geontech/redhawk-ubuntu-development as builder

ARG repo_url=https://github.com/RedhawkSDR/core-framework.git
ARG branch_or_tag=2.2.8
RUN apt-get install -y git rpm libyaml-cpp-dev && \
    ln -s bash /bin/sh.bash && \
    mv /bin/sh.bash /bin/sh && \
    cd /root && git clone --depth=1 $repo_url && cd /root/core-framework && \
    if [ $branch_or_tag != "master" ]; then "git checkout --track origin/$branch_or_tag"; fi && \
    cd /root/core-framework/redhawk/src && \
    export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64/ && \
    sed -i "s/\$(BOOST_SYSTEM_LIB)/$\(BOOST_SYSTEM_LIB\) $\(BOOST_FILESYSTEM_LIB\) $\(OMNICOS_LIBS\) $\(CPPUNIT_LIBS\)/g" /root/core-framework/redhawk/src/control/sdr/ComponentHost/Makefile.am && \
    /bin/bash -lc "./build.sh distclean && ./build.sh" && \
    apt-get clean autoclean && apt-get autoremove --yes
    
FROM geontech/redhawk-ubuntu-runtime as runner
WORKDIR /root/rpms
COPY --from=builder /root/core-framework /root/core-framework
RUN apt-get update && \
    apt-get install -y alien automake libtool libboost-filesystem-dev liblog4cxx-dev libboost-thread-dev libboost-regex-dev libcppunit-dev libyaml-cpp-dev && \
    cd /root/core-framework/redhawk/src/control/sdr/ComponentHost && \
    /bin/bash -lc "make install" && \
    rm -rf /root/core-framework && apt-get clean autoclean && apt-get autoremove --yes

# CMD ...run your sandboxed component
ENTRYPOINT ["/bin/bash", "-lc"]
