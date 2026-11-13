# Stage 1 of the GitLab pipeline: build RPMs for every core-framework project.
#
# Produces the binary (and source) RPMs under /rpms by running build-rpms.sh,
# which builds and installs each project in dependency order:
#   redhawk -> bulkioInterfaces -> burstioInterfaces -> frontendInterfaces
#           -> redhawk-codegen -> GPP
#
# The resulting /rpms directory is exported from the pipeline as job artifacts
# and consumed by rck9.rpm-install.Dockerfile (stage 2).
#
# Build context must be the core-framework source tree.
#
#   docker build -f rck9.rpm-builder.Dockerfile -t rh-core-rpm-builder:4.0.4 .

ARG BASE_IMAGE=rh-4-0-4-deps-el9:latest
FROM ${BASE_IMAGE}

COPY . /src
RUN SRC=/src RPMOUT=/rpms /src/build-rpms.sh

# /rpms now holds the built RPMs; stage 2 (or CI) copies them out.
WORKDIR /rpms
