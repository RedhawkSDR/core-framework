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

rpm: clean
	@rpmbuild -bb simpleseqdev_python_impl1.spec

xmldir = $(prefix)/dev/devices/simpleseqdev/
dist_xml_DATA = ../simpleseqdev.prf.xml ../simpleseqdev.scd.xml ../simpleseqdev.spd.xml

devdir = $(prefix)/dev/devices/simpleseqdev/simpleseqdev_python_impl1/
dist_dev_SCRIPTS = simpleseqdev_python_impl1_base.py simpleseqdev_python_impl1.py
