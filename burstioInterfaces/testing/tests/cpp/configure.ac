/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

dnl Process this file with autoconf to produce a configure script.
AC_INIT(Burstio_Test,0.1)
AC_CONFIG_SRCDIR([Burstio.cpp])
AM_INIT_AUTOMAKE
AM_PATH_CPPUNIT(1.9.6)
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PREFIX_DEFAULT(${OSSIEHOME})
AC_CORBA_ORB

OSSIE_CHECK_OSSIE
OSSIE_OSSIEHOME_AS_PREFIX
PKG_CHECK_MODULES([RH_DEPS], [ossie >= 1.7 omniORB4 >= 4.0.0])
PKG_CHECK_MODULES([BULKIO], [ bulkio >= 1.0 bulkioInterfaces >= 1.9 ])

AX_BOOST_BASE([1.41])
AX_BOOST_THREAD
AX_BOOST_SYSTEM

# set PKG_CONFIG_PATH to look at local xxx.pc files
export PKG_CONFIG_PATH="../../../..:../../..":${PKG_CONFIG_PATH}
PKG_CHECK_MODULES([BIO], [ burstio >= 1.0 burstioInterfaces >= 1.8 ])

AC_OUTPUT(Makefile)
