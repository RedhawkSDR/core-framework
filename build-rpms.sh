#!/bin/bash
#
# Build RPMs for all REDHAWK 4.0 projects inside a container.
#
# Usage (from host):
#   docker run --rm \
#     -v /path/to/4.0:/src:ro \
#     -v /path/to/output:/rpms \
#     rh-4-0-0-el:java21 \
#     /src/build-rpms.sh
#
# Or run directly inside the container:
#   /src/build-rpms.sh
#
# RPMs are written to /rpms (mount a host directory there).
# Projects are built in dependency order; each is installed before the next.
#
set -e

SRC=${SRC:-/src}
RPMOUT=${RPMOUT:-/rpms}
TOPDIR=/root/rpmbuild

# ── helpers ──────────────────────────────────────────────────────────────────

log() { echo "==== $* ===="; }

make_tarball() {
    local name=$1 version=$2 srcdir=$3
    local tarname="${name}-${version}"
    local tarball="${TOPDIR}/SOURCES/${tarname}.tar.gz"
    log "Tarball: ${tarname}"
    mkdir -p "${TOPDIR}/SOURCES"
    tar czf "$tarball" \
        --transform="s|^${srcdir#/}|${tarname}|" \
        --exclude='.git' --exclude='autom4te.cache' \
        --exclude='*.o' --exclude='*.lo' --exclude='*.la' \
        --exclude='.libs' --exclude='.deps' \
        "$srcdir"
}

build_rpm() {
    local specfile=$1
    log "RPM: $(basename $specfile)"
    rpmbuild --define "_topdir ${TOPDIR}" \
             --define "_ossiehome /usr/local/redhawk/core" \
             --define "_sdrroot /var/redhawk/sdr" \
             --nodeps \
             -ba "$specfile"
}

install_rpms() {
    local pattern=$1
    log "Installing: $pattern"
    rpm -Uvh --nodeps --force ${TOPDIR}/RPMS/*/${pattern} 2>&1 || true
}

# ── setup ────────────────────────────────────────────────────────────────────

mkdir -p "$TOPDIR"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
mkdir -p "$RPMOUT"

VERSION=4.0.4
LOG4CXX_VERSION=0.11.0

# ── 0. log4cxx ───────────────────────────────────────────────────────────────
# REDHAWK links against log4cxx 0.11 (soname liblog4cxx.so.11). Build and install
# it as an RPM first so redhawk links against it and the REDHAWK packages have a
# real provider for their auto-generated soname dependency. Requires network to
# fetch the upstream release tarball.

log "Building log4cxx ${LOG4CXX_VERSION}"
curl -fsSL -o "${TOPDIR}/SOURCES/v${LOG4CXX_VERSION}.tar.gz" \
    "https://github.com/apache/logging-log4cxx/archive/refs/tags/v${LOG4CXX_VERSION}.tar.gz"
cp "${SRC}/redhawk/src/releng/log4cxx.spec" "${TOPDIR}/SPECS/"
build_rpm "${TOPDIR}/SPECS/log4cxx.spec"
install_rpms "log4cxx-*.rpm"

# ── 1. redhawk core ─────────────────────────────────────────────────────────

log "Building redhawk core"
make_tarball redhawk "$VERSION" "${SRC}/redhawk/src"

# The spec lives inside the source tree; copy it to SPECS
cp "${SRC}/redhawk/src/releng/redhawk.spec" "${TOPDIR}/SPECS/"

# Patch the spec inline:
#  - remove --disable-java, fix --without-tests
#  - update JDK version
#  - uncomment Java JAR/JNI file listings (they were commented out when Java was disabled)
#  - add new JARs that didn't exist in the original spec
sed -i \
    -e 's/%configure --with-sdr=%{_sdrroot} --with-pyscheme=home --without-tests/%configure --with-sdr=%{_sdrroot} --with-pyscheme=home --disable-tests/' \
    -e 's/java-1.8.0-openjdk-devel/java-21-openjdk-devel/' \
    -e 's|^# %{_prefix}/lib/CFInterfaces.jar|%{_prefix}/lib/CFInterfaces.jar|' \
    -e 's|^# %{_prefix}/lib/apache-commons-lang-2.4.jar|%{_prefix}/lib/commons-lang3-3.12.0.jar|' \
    -e 's|^# %{_prefix}/lib/reload4j-1.2.19.jar|%{_prefix}/lib/reload4j-1.2.19.jar|' \
    -e 's|^# %{_prefix}/lib/reload4j.jar|%{_prefix}/lib/ossie.jar|' \
    -e 's|^# %{_prefix}/lib/ossie.jar|%{_prefix}/lib/slf4j-api-1.7.36.jar\n%{_prefix}/lib/slf4j-reload4j-1.7.36.jar\n%{_prefix}/lib/jboss-rmi-api_1.0_spec-1.0.6.Final.jar|' \
    -e 's|^# %{_libdir}/libomnijni.so.\*|%{_libdir}/libomnijni.so.*|' \
    -e 's|^# %{_libdir}/libomnijni.so$|%{_libdir}/libomnijni.so|' \
    -e 's|^# %{_libdir}/libossiecfjni.so.\*|%{_libdir}/libossiecfjni.so.*|' \
    -e 's|^# %{_libdir}/libomnijni.\*a|%{_libdir}/libomnijni.*a|' \
    -e 's|^# %{_libdir}/libossiecfjni.\*a|%{_libdir}/libossiecfjni.*a|' \
    -e 's|^# %{_libdir}/libossiecfjni.so$|%{_libdir}/libossiecfjni.so|' \
    "${TOPDIR}/SPECS/redhawk.spec"

# The spec uses %setup -q which expects the tarball to unpack to redhawk-<ver>/
# but the source tree is redhawk/src/. The tarball transform above handles this,
# but the spec also cd's into src/. We need the tarball root to contain src/.
# Re-make tarball with correct structure:
rm -f "${TOPDIR}/SOURCES/redhawk-${VERSION}.tar.gz"
tmpdir=$(mktemp -d)
cp -a "${SRC}/redhawk/src" "${tmpdir}/redhawk-${VERSION}"
tar czf "${TOPDIR}/SOURCES/redhawk-${VERSION}.tar.gz" \
    --exclude='.git' --exclude='autom4te.cache' \
    --exclude='*.o' --exclude='*.lo' --exclude='*.la' \
    --exclude='.libs' --exclude='.deps' \
    -C "$tmpdir" "redhawk-${VERSION}"
rm -rf "$tmpdir"

# The spec cd's into src/ but our tarball root IS the src/ contents.
# Patch the spec to not cd into src/
sed -i 's|^cd src$|# cd src (tarball is already rooted at src/)|' "${TOPDIR}/SPECS/redhawk.spec"

build_rpm "${TOPDIR}/SPECS/redhawk.spec"
install_rpms "redhawk-*.rpm"

# ── 2. bulkioInterfaces ─────────────────────────────────────────────────────

log "Building bulkioInterfaces"
tmpdir=$(mktemp -d)
cp -a "${SRC}/bulkioInterfaces" "${tmpdir}/bulkioInterfaces-${VERSION}"
tar czf "${TOPDIR}/SOURCES/bulkioInterfaces-${VERSION}.tar.gz" \
    --exclude='.git' --exclude='autom4te.cache' \
    --exclude='*.o' --exclude='*.lo' --exclude='*.la' \
    --exclude='.libs' --exclude='.deps' \
    -C "$tmpdir" "bulkioInterfaces-${VERSION}"
rm -rf "$tmpdir"

cp "${SRC}/bulkioInterfaces/bulkioInterfaces.spec" "${TOPDIR}/SPECS/"
sed -i 's/%configure --disable-java/%configure/' "${TOPDIR}/SPECS/bulkioInterfaces.spec"
awk '
    /^%files$/ && !done { in_files=1 }
    /^%defattr/ && in_files && !done {
        print
        print "%{_prefix}/lib/BULKIOInterfaces.jar"
        print "%{_prefix}/lib/bulkio.jar"
        print "%{_libdir}/libbulkiojni.*"
        in_files=0; done=1
        next
    }
    { print }
' "${TOPDIR}/SPECS/bulkioInterfaces.spec" > "${TOPDIR}/SPECS/bulkioInterfaces.spec.new" && \
    mv "${TOPDIR}/SPECS/bulkioInterfaces.spec.new" "${TOPDIR}/SPECS/bulkioInterfaces.spec"

build_rpm "${TOPDIR}/SPECS/bulkioInterfaces.spec"
install_rpms "bulkioInterfaces-*.rpm"

# ── 3. burstioInterfaces ────────────────────────────────────────────────────

log "Building burstioInterfaces"
tmpdir=$(mktemp -d)
cp -a "${SRC}/burstioInterfaces" "${tmpdir}/burstioInterfaces-${VERSION}"
tar czf "${TOPDIR}/SOURCES/burstioInterfaces-${VERSION}.tar.gz" \
    --exclude='.git' --exclude='autom4te.cache' \
    --exclude='*.o' --exclude='*.lo' --exclude='*.la' \
    --exclude='.libs' --exclude='.deps' \
    -C "$tmpdir" "burstioInterfaces-${VERSION}"
rm -rf "$tmpdir"

cp "${SRC}/burstioInterfaces/burstioInterfaces.spec" "${TOPDIR}/SPECS/"
sed -i 's/%configure --disable-java/%configure/' "${TOPDIR}/SPECS/burstioInterfaces.spec"
# Append Java files after the first %defattr line following %files
awk '
    /^%files$/ && !done { in_files=1 }
    /^%defattr/ && in_files && !done {
        print
        print "%{_prefix}/lib/BURSTIOInterfaces.jar"
        print "%{_prefix}/lib/burstio.jar"
        print "%{_libdir}/libburstiojni.*"
        in_files=0; done=1
        next
    }
    { print }
' "${TOPDIR}/SPECS/burstioInterfaces.spec" > "${TOPDIR}/SPECS/burstioInterfaces.spec.new" && \
    mv "${TOPDIR}/SPECS/burstioInterfaces.spec.new" "${TOPDIR}/SPECS/burstioInterfaces.spec"

build_rpm "${TOPDIR}/SPECS/burstioInterfaces.spec"
install_rpms "burstioInterfaces-*.rpm"

# ── 4. frontendInterfaces ───────────────────────────────────────────────────

log "Building frontendInterfaces"
tmpdir=$(mktemp -d)
cp -a "${SRC}/frontendInterfaces" "${tmpdir}/frontendInterfaces-${VERSION}"
tar czf "${TOPDIR}/SOURCES/frontendInterfaces-${VERSION}.tar.gz" \
    --exclude='.git' --exclude='autom4te.cache' \
    --exclude='*.o' --exclude='*.lo' --exclude='*.la' \
    --exclude='.libs' --exclude='.deps' \
    -C "$tmpdir" "frontendInterfaces-${VERSION}"
rm -rf "$tmpdir"

cp "${SRC}/frontendInterfaces/frontendInterfaces.spec" "${TOPDIR}/SPECS/"
sed -i 's/%configure --disable-java/%configure/' "${TOPDIR}/SPECS/frontendInterfaces.spec"
awk '
    /^%files$/ && !done { in_files=1 }
    /^%defattr/ && in_files && !done {
        print
        print "%{_prefix}/lib/FRONTENDInterfaces.jar"
        print "%{_prefix}/lib/frontend.jar"
        in_files=0; done=1
        next
    }
    { print }
' "${TOPDIR}/SPECS/frontendInterfaces.spec" > "${TOPDIR}/SPECS/frontendInterfaces.spec.new" && \
    mv "${TOPDIR}/SPECS/frontendInterfaces.spec.new" "${TOPDIR}/SPECS/frontendInterfaces.spec"

build_rpm "${TOPDIR}/SPECS/frontendInterfaces.spec"
install_rpms "frontendInterfaces-*.rpm"

# ── 5. redhawk-codegen ──────────────────────────────────────────────────────

log "Building redhawk-codegen"
tmpdir=$(mktemp -d)
cp -a "${SRC}/redhawk-codegen" "${tmpdir}/redhawk-codegen-${VERSION}"
tar czf "${TOPDIR}/SOURCES/redhawk-codegen-${VERSION}.tar.gz" \
    --exclude='.git' --exclude='autom4te.cache' \
    --exclude='__pycache__' \
    -C "$tmpdir" "redhawk-codegen-${VERSION}"
rm -rf "$tmpdir"

cp "${SRC}/redhawk-codegen/redhawk-codegen.spec" "${TOPDIR}/SPECS/"

build_rpm "${TOPDIR}/SPECS/redhawk-codegen.spec"
install_rpms "redhawk-codegen-*.rpm"

# ── 6. GPP ───────────────────────────────────────────────────────────────────

log "Building GPP"
tmpdir=$(mktemp -d)
cp -a "${SRC}/GPP" "${tmpdir}/GPP-${VERSION}"
tar czf "${TOPDIR}/SOURCES/GPP-${VERSION}.tar.gz" \
    --exclude='.git' --exclude='autom4te.cache' \
    --exclude='*.o' --exclude='*.lo' --exclude='*.la' \
    --exclude='.libs' --exclude='.deps' \
    -C "$tmpdir" "GPP-${VERSION}"
rm -rf "$tmpdir"

cp "${SRC}/GPP/GPP.spec" "${TOPDIR}/SPECS/"

build_rpm "${TOPDIR}/SPECS/GPP.spec"

# ── collect output ──────────────────────────────────────────────────────────

log "Collecting RPMs"
# Binary RPMs only -- src RPMs are not used downstream and would otherwise ride
# along in an image layer.
find "${TOPDIR}/RPMS" -name "*.rpm" -exec cp {} "$RPMOUT/" \;

log "Done"
echo "RPMs written to ${RPMOUT}:"
ls -1 "${RPMOUT}/"*.rpm 2>/dev/null
