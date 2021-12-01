#!/bin/sh

set -u
set -e
set -x

FIXME=modernize-use-noexcept
# FIXME=modernize-use-override

SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
RUN_CLANG_TIDY="run-clang-tidy -extra-arg=-isysroot -extra-arg=${SYSROOT} -extra-arg=-isystem -extra-arg=${SYSROOT}/usr/include/c++/v1"

${RUN_CLANG_TIDY} base/framework/*.cpp
#TODO ${RUN_CLANG_TIDY} -checks="-*,${FIXME}" base/framework/*.cpp -fix

