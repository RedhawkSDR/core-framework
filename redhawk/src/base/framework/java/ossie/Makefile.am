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

include $(top_srcdir)/aminclude/jarfile.am

# NB: In order to create a large number of strongly-typed property classes that
#     differ only in the numeric type, common templates and type-specific sed
#     scripts are used to generate each instance. This allows us to make
#     changes in a single location if the base classes change, and reduces the
#     risk of a typo in a single class; however it somewhat complicates the
#     build of the jarfile.

# Numeric property classes
SIMPLE_PROPERTIES = src/org/ossie/properties/DoubleProperty.java \
		    src/org/ossie/properties/FloatProperty.java \
		    src/org/ossie/properties/LongLongProperty.java \
		    src/org/ossie/properties/LongProperty.java \
		    src/org/ossie/properties/OctetProperty.java \
		    src/org/ossie/properties/ShortProperty.java \
		    src/org/ossie/properties/ULongLongProperty.java \
		    src/org/ossie/properties/ULongProperty.java \
		    src/org/ossie/properties/UShortProperty.java

src/org/ossie/properties/%Property.java : src/org/ossie/properties/NumericProperty.template sed/%.sed
	$(AM_V_GEN)$(SED) -f sed/$*.sed $< > $@

# Empty rules for simple boolean and char properties, which are not generated
src/org/ossie/properties/BooleanProperty.java: ;
src/org/ossie/properties/BooleanSequenceProperty.java : ;
src/org/ossie/properties/CharProperty.java : ;
src/org/ossie/properties/CharSequenceProperty.java: ;

# Numeric sequence property classes
SEQUENCE_PROPERTIES = src/org/ossie/properties/DoubleSequenceProperty.java \
		      src/org/ossie/properties/FloatSequenceProperty.java \
		      src/org/ossie/properties/LongLongSequenceProperty.java \
		      src/org/ossie/properties/LongSequenceProperty.java \
		      src/org/ossie/properties/OctetSequenceProperty.java \
		      src/org/ossie/properties/ShortSequenceProperty.java \
		      src/org/ossie/properties/ULongLongSequenceProperty.java \
		      src/org/ossie/properties/ULongSequenceProperty.java \
		      src/org/ossie/properties/UShortSequenceProperty.java

src/org/ossie/properties/%SequenceProperty.java : src/org/ossie/properties/NumericSequenceProperty.template sed/%.sed
	$(AM_V_GEN)$(SED) -f sed/$*.sed $< > $@

# Complex simple property classes
COMPLEX_PROPERTIES = src/org/ossie/properties/ComplexBooleanProperty.java \
		     src/org/ossie/properties/ComplexCharProperty.java \
		     src/org/ossie/properties/ComplexDoubleProperty.java \
		     src/org/ossie/properties/ComplexFloatProperty.java \
		     src/org/ossie/properties/ComplexLongLongProperty.java \
		     src/org/ossie/properties/ComplexLongProperty.java \
		     src/org/ossie/properties/ComplexOctetProperty.java \
		     src/org/ossie/properties/ComplexShortProperty.java \
		     src/org/ossie/properties/ComplexULongLongProperty.java \
		     src/org/ossie/properties/ComplexULongProperty.java \
		     src/org/ossie/properties/ComplexUShortProperty.java

src/org/ossie/properties/Complex%Property.java : src/org/ossie/properties/ComplexProperty.template sed/%.sed
	$(AM_V_GEN)$(SED) -f sed/$*.sed $< > $@

# Complex sequence property classes
COMPLEX_SEQUENCES = src/org/ossie/properties/ComplexBooleanSequenceProperty.java \
		    src/org/ossie/properties/ComplexCharSequenceProperty.java \
		    src/org/ossie/properties/ComplexDoubleSequenceProperty.java \
		    src/org/ossie/properties/ComplexFloatSequenceProperty.java \
		    src/org/ossie/properties/ComplexLongLongSequenceProperty.java \
		    src/org/ossie/properties/ComplexLongSequenceProperty.java \
		    src/org/ossie/properties/ComplexOctetSequenceProperty.java \
		    src/org/ossie/properties/ComplexShortSequenceProperty.java \
		    src/org/ossie/properties/ComplexULongLongSequenceProperty.java \
		    src/org/ossie/properties/ComplexULongSequenceProperty.java \
		    src/org/ossie/properties/ComplexUShortSequenceProperty.java

src/org/ossie/properties/Complex%SequenceProperty.java : src/org/ossie/properties/ComplexSequenceProperty.template sed/%.sed 
	$(AM_V_GEN)$(SED) -f sed/$*.sed $< > $@

# Regular source files
ossie_jar_SOURCE = src/org/ossie/component/AllocCapacity.java \
		   src/org/ossie/component/Logging.java \
		   src/org/ossie/component/Device.java \
		   src/org/ossie/component/ProvidesPort.java \
		   src/org/ossie/component/ProcessThread.java \
		   src/org/ossie/component/QueryableUsesPort.java \
		   src/org/ossie/component/PropertyChangeRec.java \
		   src/org/ossie/component/Resource.java \
		   src/org/ossie/component/Component.java \
		   src/org/ossie/component/ThreadedComponent.java \
		   src/org/ossie/component/ThreadedDevice.java \
		   src/org/ossie/component/ThreadedResource.java \
		   src/org/ossie/component/Service.java \
		   src/org/ossie/component/UsesPort.java \
           src/org/ossie/component/PortBase.java \
		   src/org/ossie/logging/logging.java \
		   src/org/ossie/logging/RH_LogEventAppender.java \
		   src/org/ossie/events/Consumer_i.java \
		   src/org/ossie/events/EventCallback.java \
		   src/org/ossie/events/MessageConsumerPort.java \
		   src/org/ossie/events/MessageSupplierPort.java \
		   src/org/ossie/events/PropertyEventSupplier.java \
		   src/org/ossie/events/SupplierAdmin_i.java \
		   src/org/ossie/events/Manager.java \
		   src/org/ossie/events/MessageAdapter.java \
		   src/org/ossie/events/MessageListener.java \
		   src/org/ossie/events/Publisher.java \
		   src/org/ossie/events/Subscriber.java \
		   src/org/ossie/corba/utils.java \
		   src/org/ossie/properties/AbstractSequenceProperty.java \
		   src/org/ossie/properties/AbstractSimpleProperty.java \
		   src/org/ossie/properties/Action.java \
		   src/org/ossie/properties/Allocator.java \
		   src/org/ossie/properties/AnyUtils.java \
		   src/org/ossie/properties/BooleanProperty.java \
		   src/org/ossie/properties/BooleanSequenceProperty.java \
		   src/org/ossie/properties/CharProperty.java \
		   src/org/ossie/properties/CharSequenceProperty.java \
		   src/org/ossie/properties/ComplexArrayUtils.java \
		   src/org/ossie/properties/ComplexUtils.java \
		   src/org/ossie/properties/IProperty.java \
		   src/org/ossie/properties/Kind.java \
		   src/org/ossie/properties/LegacyProperty.java \
		   src/org/ossie/properties/Mode.java \
		   src/org/ossie/properties/NumericProperty.java \
		   src/org/ossie/properties/NumericSequenceProperty.java \
		   src/org/ossie/properties/PrimitiveArrayUtils.java \
		   src/org/ossie/properties/Property.java \
		   src/org/ossie/properties/PropertyListener.java \
		   src/org/ossie/properties/StringProperty.java \
		   src/org/ossie/properties/StringSequenceProperty.java \
		   src/org/ossie/properties/StructDef.java \
		   src/org/ossie/properties/StructProperty.java \
		   src/org/ossie/properties/StructSequenceProperty.java \
		   src/org/ossie/properties/UnsignedUtils.java \
                   src/org/ossie/redhawk/ApplicationContainer.java \
                   src/org/ossie/redhawk/DomainManagerContainer.java \
                   src/org/ossie/redhawk/NetworkContainer.java \
                   src/org/ossie/redhawk/DeviceManagerContainer.java

# Deprecated generic property classes, replaced by strongly-typed classes but
# maintained for source compatibility.
ossie_jar_SOURCE += src/org/ossie/properties/SimpleProperty.java \
		src/org/ossie/properties/SimpleSequenceProperty.java

ossie_jar_SOURCE += $(SIMPLE_PROPERTIES)
ossie_jar_SOURCE += $(SEQUENCE_PROPERTIES)
ossie_jar_SOURCE += $(COMPLEX_PROPERTIES)
ossie_jar_SOURCE += $(COMPLEX_SEQUENCES)

# Build older, deprecated property interfaces in a separate convenience (not
# installed) jar so that we can suppress the warnings due to deprecating an
# entire class hierarchy--at least some versions of javac issue a deprecation
# warning even if the class extending a deprecated class is itself deprecated.
noinst_java_JARFILES = ossie-legacy.jar

ossie_legacy_jar_SOURCE = src/org/ossie/BooleanSimplePropertyContainer.java \
		src/org/ossie/CharSimplePropertyContainer.java \
		src/org/ossie/DoubleSimplePropertyContainer.java \
		src/org/ossie/FloatSimplePropertyContainer.java \
		src/org/ossie/IntegerSimplePropertyContainer.java \
		src/org/ossie/LongSimplePropertyContainer.java \
		src/org/ossie/OctetSimplePropertyContainer.java \
		src/org/ossie/PropertyContainer.java \
		src/org/ossie/ShortSimplePropertyContainer.java \
		src/org/ossie/SimplePropertyContainer.java \
		src/org/ossie/StringSimplePropertyContainer.java

ossie_legacy_jar_CLASSPATH = ../CFInterfaces.jar
ossie_legacy_jar_JAVACFLAGS = -g -Xlint:-deprecation

# Additional Java compiler flags
ossie_jar_CLASSPATH = ../apache-commons-lang-2.4.jar:../CFInterfaces.jar:../log4j-1.2.15.jar
ossie_jar_JAVACFLAGS = -g -Xlint:deprecation -Xlint:unchecked
ossie_jar_JARADD = -C src . ossie-legacy.jar

java_JARFILES = ossie.jar

ossie.jar : ../CFInterfaces.jar ossie-legacy.jar

CLEANFILES = $(SIMPLE_PROPERTIES) $(SEQUENCE_PROPERTIES) $(COMPLEX_PROPERTIES) $(COMPLEX_SEQUENCES)
