# Canonical REDHAWK core-framework build/runtime dependency image (Rocky 9).
# Includes the C++/Python toolchain, omniORB, Java 21 + JacORB 3.9, and
# test/packaging tooling. log4cxx is NOT installed here -- it is built as a
# pinned RPM by the core-framework pipeline (build-rpms.sh). This is the base
# for both pipeline stages (rck9.rpm-builder.Dockerfile, rck9.rpm-install.Dockerfile).
#
# Build and tag:
#   docker build -f rck9.deps.Dockerfile \
#     -t rh-4-0-4-deps-el9:latest .
FROM rockylinux/rockylinux:9

# JAVA_VERSION=21 is read by configure.ac to set
# javac --release.
ENV JACORB_HOME=/usr/share/java/jacorb \
    JAVA_HOME=/usr/lib/jvm/java-21-openjdk \
    JAVA_VERSION=21 \
    LD_LIBRARY_PATH=/usr/local/redhawk/core/lib64:/usr/local/redhawk/core/lib:/usr/local/lib64 \
    OMNIEVENTS_LOGDIR=/var/log/omniEvents \
    OSSIEHOME=/usr/local/redhawk/core \
    PATH=/usr/lib/jvm/java-21-openjdk/bin:/usr/share/java/jacorb/bin:/usr/local/redhawk/core/bin:$PATH \
    PYTHONPATH=/usr/local/redhawk/core/lib64/python:/usr/local/redhawk/core/lib/python \
    SDRROOT=/var/redhawk/sdr \
    USER=hello

RUN dnf install -y epel-release

RUN dnf upgrade -y --refresh
RUN dnf config-manager --set-enabled crb
RUN dnf install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-9.noarch.rpm https://dl.fedoraproject.org/pub/epel/epel-next-release-latest-9.noarch.rpm

RUN dnf install -y pip
RUN dnf install -y omniORB-devel omniORB-servers omniORB-utils omniORBpy-devel python3-omniORB.x86_64
RUN dnf install -y libtool

RUN dnf install -y cppunit-devel

RUN dnf install -y gcc-c++
RUN dnf install -y diffutils-3.7-12.el9.x86_64
RUN dnf install -y file
RUN dnf install -y boost boost-devel boost-filesystem boost-system boost-serialization boost-regex boost-thread boost-date-time
RUN dnf install -y expat-devel xsd

RUN dnf install -y git cmake dnf libapreq2-devel zip sqlite-devel

RUN dnf install -y libuuid-devel libcurl-devel jsoncpp-devel

# Build deps for the log4cxx RPM (built by the core-framework pipeline, not here)
RUN dnf install -y apr-devel apr-util-devel

RUN pip3 install psutil

RUN dnf install -y python3-pyyaml

# testing support
RUN dnf install -y procps-ng valgrind
RUN pip install xmlrunner lxml nose
RUN dnf install -y python3-jinja2 python3-qt5-devel
RUN dnf install -y fftw-devel python3-scipy

# additional support
RUN dnf install -y rpm-build uhd-devel createrepo_c sudo

# Java 21 + JacORB 3.9 (required by REDHAWK 4.0 Java framework / omnijni)
RUN dnf install -y java-21-openjdk-devel unzip
RUN curl -fsSL -o /tmp/jacorb-3.9-binary.zip https://www.jacorb.org/releases/3.9/jacorb-3.9-binary.zip && \
    unzip -q /tmp/jacorb-3.9-binary.zip -d /usr/share/java && \
    ln -sf jacorb-3.9 /usr/share/java/jacorb && \
    ln -sf jacorb-3.9.jar          /usr/share/java/jacorb/lib/jacorb.jar && \
    ln -sf jacorb-omgapi-3.9.jar   /usr/share/java/jacorb/lib/jacorb-omgapi.jar && \
    ln -sf jacorb-services-3.9.jar /usr/share/java/jacorb/lib/jacorb-services.jar && \
    rm -f /tmp/jacorb-3.9-binary.zip && \
    test -x /usr/share/java/jacorb/bin/idl && \
    test -f /usr/share/java/jacorb/lib/jacorb.jar

ENV PKG_CONFIG_PATH=/usr/local/share/pkgconfig
