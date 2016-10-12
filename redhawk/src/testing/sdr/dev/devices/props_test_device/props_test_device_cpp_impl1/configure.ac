#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

AC_INIT(props_test_device, 0.5.0)
AM_INIT_AUTOMAKE(nostdinc)

AC_PREFIX_DEFAULT(${OSSIEHOME})

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_CORBA_ORB
OSSIE_CHECK_OSSIE
OSSIE_SDRROOT_AS_PREFIX
AC_PREFIX_DEFAULT(${SDRROOT})
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
PKG_CHECK_MODULES(OSSIE, ossie >= 1.7.0)
CXXFLAGS="$CXXFLAGS $OSSIE_CFLAGS"
LIBS="$LIBS $OSSIE_LIBS"
PKG_CHECK_MODULES([OMNIORB], [omniORB4 >= 4.0.0])

OSSIE_ENABLE_LOG4CXX

CPPFLAGS="${CPPFLAGS} -UPACKAGE_NAME -UPACKAGE_BUGREPORT -UPACKAGE_TARNAME -UPACKAGE_STRING -UPACKAGE_VERSION"

AX_BOOST_BASE([1.41])
AX_BOOST_THREAD
AC_CONFIG_FILES(Makefile)

AC_OUTPUT
