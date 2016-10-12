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

# Mimic automake silent rules
OSSIE_V_pysetup = $(ossie__v_pysetup_$(V))
ossie__v_pysetup_ = $(ossie__v_pysetup__$(AM_DEFAULT_VERBOSITY))
ossie__v_pysetup_0 = --quiet
ossie__v_pysetup__0 = $(ossie__v_pysetup_0)

SUBDIRS = sdr/dev/devices/ExecutableDevice \
          sdr/dev/devices/BasicTestDevice_cpp/BasicTestDevice_cpp_impl1 \
          sdr/dev/devices/BasicDevWithExecParam_cpp/BasicDevWithExecParam_cpp_impl1 \
          sdr/dev/devices/CppTestDevice/cpp \
          sdr/dev/devices/SimpleDevice/SimpleDevice_cpp_impl1 \
          sdr/dev/devices/props_test_device/props_test_device_cpp_impl1 \
          sdr/dev/devices/issue_111_cpp/cpp \
          sdr/dev/devices/PersonaDevice/cpp \
          sdr/dev/devices/ProgrammableDevice/cpp \
          sdr/dev/devices/cpp_dev/cpp \
          sdr/dev/devices/GPP/cpp \
          sdr/dev/devices/java_dev/java \
          sdr/dom/deps/cpp_dep1/cpp \
          sdr/dom/deps/cpp_dep2/cpp \
          sdr/dom/components/TestCppProps \
          sdr/dom/components/linkedLibraryTest \
          sdr/dom/components/TestCppsoftpkgDeps \
          sdr/dom/components/BasicAC/BasicAC_cpp_impl1 \
          sdr/dom/components/SimpleComponent/SimpleComponent_cpp_impl1 \
          sdr/dom/components/MessageReceiverCpp \
          sdr/dom/components/MessageSenderCpp \
          sdr/dom/components/PropertyChangeEventsCpp \
          sdr/dom/components/ticket2093/cpp \
          sdr/dom/components/CommandWrapperEmptyDir \
          sdr/dom/components/TestCppPropsRange/cpp \
          sdr/dom/components/TestComplexProps/cpp \
          sdr/dom/components/TestAllPropTypes/cpp \
          sdr/dom/components/CppCallbacks/cpp \
          sdr/dom/components/prop_trigger_timing/cpp \
          sdr/dom/components/cpp_comp/cpp \
          sdr/dom/components/java_comp/java \
          sdr/dom/components/TestLoggingAPI/cpp \
          sdr/dom/components/ECM_CPP/cpp \
          sdr/dom/components/C1/cpp \
          sdr/dom/components/C2/cpp \
          sdr/dom/components/PropertyChange_C1/cpp \
          sdr/dom/components/Property_CPP/cpp \
          sdr/dom/components/commandline_prop/cpp \
          sdr/dom/components/nocommandline_prop/cpp \
          sdr/dom/components/foo/bar/comp/cpp \
          sdr/dom/components/TestCppOptionalProps/cpp \
          sdr/dom/components/cpp_with_deps/cpp

if HAVE_JAVASUPPORT
SUBDIRS += sdr/dev/devices/BasicTestDevice_java/java \
           sdr/dev/devices/JavaTestDevice/java \
           sdr/dev/devices/issue_111_java/java \
           sdr/dev/services/BasicService_java/java \
           sdr/dom/components/BasicAC/basicac_java_impl1 \
           sdr/dom/components/EventSend/EventSend_java_impl1 \
           sdr/dom/components/EventReceive/EventReceive_java_impl1 \
           sdr/dom/components/javaDep/javaDep \
           sdr/dom/components/java_comp/java \
           sdr/dom/components/HardLimit/HardLimit_java_impl1 \
           sdr/dom/components/PropertyChangeEventsJava/PropertyChangeEventsJava_java_impl1 \
           sdr/dom/components/TestJavaProps \
           sdr/dom/components/TestLegacyJavaProps \
           sdr/dom/components/TestJavaPropsRange/java \
           sdr/dom/components/TestComplexProps/java \
           sdr/dom/components/TestAllPropTypes/java \
           sdr/dom/components/TestLoggingAPI/java \
           sdr/dom/components/ECM_JAVA/java \
           sdr/dom/components/PropertyChange_J1/java \
           sdr/dom/components/Property_JAVA/java \
           sdr/dom/components/foo/bar/jv/java \
           sdr/dom/components/TestJavaOptionalProps/java
endif

all-local:
	python setup.py $(OSSIE_V_pysetup) build

clean-local:
	python setup.py clean
	rm -rf build

EXTRA_DIST = setup.py

PYTHON_INSTALL_FILES=".pythonInstallFiles"

install-exec-hook:
	python setup.py install -f --$(PYTHON_INSTALL_SCHEME)=$(DESTDIR)$(prefix) --record $(PYTHON_INSTALL_FILES)

uninstall:
	rm `cat $(PYTHON_INSTALL_FILES)`
