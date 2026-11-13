# Multi-stage image for REDHAWK core-framework, built through the rfnext
# ci-support docker.yml template. The template selects a stage with TARGETS:
#   TARGETS=builder  -> the RPM builder stage (produces /rpms)
#   (default)        -> the installed rh-core image (local yum repo + snapshot
#                       tarball + full install)
#
# BASE_IMAGE / VERSION are passed as BUILD_ARGS from .gitlab-ci.images.yml.
# Local build example:
#   docker build -f rck9.Dockerfile --target runtime \
#     --build-arg BASE_IMAGE=rh-4-0-4-deps-el9:latest \
#     --build-arg VERSION=4.0.4 -t rh-core:4.0.4 .

ARG BASE_IMAGE=rh-4-0-4-deps-el9:latest

# ---- builder: compile every core-framework project into RPMs ----
FROM ${BASE_IMAGE} AS builder
COPY . /src
RUN SRC=/src RPMOUT=/rpms /src/build-rpms.sh
WORKDIR /rpms

# ---- runtime: assemble the local yum repo, snapshot it, install REDHAWK ----
FROM ${BASE_IMAGE} AS runtime
ARG VERSION=4.0.4

# RPMs come straight from the builder stage (no CI artifact round-trip).
COPY --from=builder /rpms /tmp/rpms

# 1. local yum repository at /repo/<version>
RUN mkdir -p /repo/${VERSION} && \
    find /tmp/rpms -name '*.rpm' ! -name '*.src.rpm' -exec cp -n {} /repo/${VERSION}/ \; && \
    createrepo_c /repo/${VERSION} && \
    rm -rf /tmp/rpms

# 2. snapshot tarball of the repo
RUN tar czf /repo/rh-core-${VERSION}.tar.gz -C /repo ${VERSION}

# 3. register the local repo (unsigned RPMs -> gpgcheck=0)
RUN printf '[redhawk-local]\nname=REDHAWK %s (local)\nbaseurl=file:///repo/%s\nenabled=1\ngpgcheck=0\n' \
      "${VERSION}" "${VERSION}" > /etc/yum.repos.d/redhawk-local.repo

# 4. install the full REDHAWK stack from the local repo
ARG RH_PACKAGES="redhawk redhawk-devel redhawk-qt-tools redhawk-sdrroot-dom-mgr redhawk-sdrroot-dom-profile redhawk-sdrroot-dev-mgr bulkioInterfaces burstioInterfaces frontendInterfaces redhawk-codegen GPP"
RUN dnf -y makecache && \
    dnf -y install ${RH_PACKAGES} && \
    dnf clean all

WORKDIR /
