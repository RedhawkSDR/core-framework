# Stage 2 of the GitLab pipeline: assemble a local yum repo and install REDHAWK.
#
# Expects the RPMs built in stage 1 (rck9.rpm-builder.Dockerfile) to be present
# in the build context under ./rpms/ (delivered as GitLab job artifacts).
#
# Layout produced in the image:
#   /repo/<version>/                 createrepo yum repository (RPMs + repodata/)
#   /repo/rh-core-<version>.tar.gz   gzip tarball of the <version>/ repo dir
#   /etc/yum.repos.d/redhawk-local.repo   points at file:///repo/<version>
#
# The REDHAWK packages are installed from that local repo via dnf.
#
#   docker build -f rck9.rpm-install.Dockerfile -t rh-core:4.0.4 .

ARG BASE_IMAGE=rh-4-0-4-deps-el9:latest
FROM ${BASE_IMAGE}

ARG VERSION=4.0.4

# RPMs from stage 1 (CI artifacts). Binary RPMs only go into the repo.
COPY rpms/ /tmp/rpms/

# 1. Build the local yum repository at /repo/<version>.
RUN mkdir -p /repo/${VERSION} && \
    find /tmp/rpms -name '*.rpm' ! -name '*.src.rpm' -exec cp -n {} /repo/${VERSION}/ \; && \
    createrepo_c /repo/${VERSION} && \
    rm -rf /tmp/rpms

# 2. Tarball of the repo directory (contains <version>/ with repodata).
RUN tar czf /repo/rh-core-${VERSION}.tar.gz -C /repo ${VERSION}

# 3. Register the local repo (RPMs are unsigned -> gpgcheck=0).
RUN printf '[redhawk-local]\nname=REDHAWK %s (local)\nbaseurl=file:///repo/%s\nenabled=1\ngpgcheck=0\n' \
      "${VERSION}" "${VERSION}" > /etc/yum.repos.d/redhawk-local.repo

# 4. Install the full REDHAWK stack from the local repo. dnf resolves
#    inter-package deps from the repo and external deps from the base image's
#    repos (AppStream/CRB/EPEL). This project pre-dates containerization, so a
#    complete install (including -devel and qt-tools) is expected.
ARG RH_PACKAGES="redhawk redhawk-devel redhawk-qt-tools redhawk-sdrroot-dom-mgr redhawk-sdrroot-dom-profile redhawk-sdrroot-dev-mgr bulkioInterfaces burstioInterfaces frontendInterfaces redhawk-codegen GPP"
RUN dnf -y makecache && \
    dnf -y install ${RH_PACKAGES} && \
    dnf clean all

WORKDIR /
