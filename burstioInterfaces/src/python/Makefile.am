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

.NOTPARALLEL:

idl_srcdir = $(top_srcdir)/src/idl

# Map external IDL modules to Python packages
CF_MODULES = cf PortTypes QueryablePort LogInterfaces
BULKIO_MODULES = bulkioDataTypes bio_runtimeStats
PYTHON_EXTERNS = $(CF_MODULES:%=%:ossie.cf) $(BULKIO_MODULES:%=%:bulkio.bulkioInterfaces)

IDLFLAGS = $(PYTHON_EXTERNS:%=-Wbextern=%) -I $(idl_srcdir) -I $(OSSIE_IDLDIR) -I $(BULKIO_IDLDIR)

redhawk/burstioInterfaces/%_idl.py : $(idl_srcdir)/redhawk/BURSTIO/%.idl
	$(IDL) -bpython -Wbpackage=redhawk.burstioInterfaces $(IDLFLAGS) $<

EXTRA_DIST = setup.py \
	     redhawk/burstio/__init__.py \
	     redhawk/burstio/executor.py \
	     redhawk/burstio/inports.py \
	     redhawk/burstio/outports.py \
	     redhawk/burstio/statistics.py \
	     redhawk/burstio/traits.py \
	     redhawk/burstio/usesport.py \
	     redhawk/burstio/utils.py

BUILT_SOURCES = redhawk/burstioInterfaces/burstioDataTypes_idl.py \
		redhawk/burstioInterfaces/burstio_burstByte_idl.py \
		redhawk/burstioInterfaces/burstio_burstDouble_idl.py \
		redhawk/burstioInterfaces/burstio_burstFloat_idl.py \
		redhawk/burstioInterfaces/burstio_burstLong_idl.py \
		redhawk/burstioInterfaces/burstio_burstLongLong_idl.py \
		redhawk/burstioInterfaces/burstio_burstShort_idl.py \
		redhawk/burstioInterfaces/burstio_burstUbyte_idl.py \
		redhawk/burstioInterfaces/burstio_burstUlong_idl.py \
		redhawk/burstioInterfaces/burstio_burstUlongLong_idl.py \
		redhawk/burstioInterfaces/burstio_burstUshort_idl.py


CLEANFILES = $(BUILT_SOURCES)

# Create a symbolic link back to the library source, if necessary
.PHONY: source-link
source-link:
	test -e redhawk/burstio || ln -s $(abs_srcdir)/redhawk/burstio redhawk/burstio

# Remove the generated IDL skeletons and stubs, and if the library source is a
# symbolic link (i.e., the build is out-of-place), remove the link
clean-local:
	rm -rf redhawk/burstioInterfaces
	test ! -h redhawk/burstio || rm redhawk/burstio

install-exec-hook: source-link
	python setup.py install -f --$(PYTHON_INSTALL_SCHEME)=$(DESTDIR)$(prefix)
