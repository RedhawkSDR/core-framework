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

# vim: noet: softtabstop=0

# Because omniidl is definitely not capable
.NOTPARALLEL:

all-local:
	python setup.py build
if INSTALL_PYTHON_BACKPORTS
	cd backports; python setup-backports.py build
endif

clean-local:
	python setup.py clean
	rm -rf build

ossie/cf/%_idl.py : ../../idl/ossie/CF/%.idl
	$(IDL) -bpython -Wbpackage=ossie.cf -Wbextern=CosNaming -Wbextern=CosEventComm -Wbextern=CosEventChannelAdmin -I $(IDLDIR) -I $(OMNICOS_IDLDIR) -I ../../idl $<

EXTRA_DIST = setup.py

BUILT_SOURCES = \
 ossie/cf/cf_idl.py \
 ossie/cf/PortTypes_idl.py \
 ossie/cf/StandardEvent_idl.py \
 ossie/cf/ExtendedEvent_idl.py \
 ossie/cf/AggregateDevices_idl.py \
 ossie/cf/QueryablePort_idl.py \
 ossie/cf/WellKnownProperties_idl.py \
 ossie/cf/sandbox_idl.py

CLEANFILES = $(BUILT_SOURCES)

install-exec-hook:
	python setup.py install -f --$(PYTHON_INSTALL_SCHEME)=$(DESTDIR)$(prefix)
if INSTALL_PYTHON_BACKPORTS
	cd backports; python setup-backports.py install --$(PYTHON_INSTALL_SCHEME)=$(DESTDIR)$(prefix)
endif
