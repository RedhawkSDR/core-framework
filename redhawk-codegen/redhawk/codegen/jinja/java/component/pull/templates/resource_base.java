/*#
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 #*/
/*{% block license %}*/
/*# Allow child templates to include license #*/
/*{% endblock %}*/
//% set userclass = component.userclass.name
//% set classname = component.baseclass.name
//% set superClass = component.superclass.name
//% set artifactType = component.artifacttype
package ${component.package};

//% set listImported = false
/*{% if component.hasmultioutport %}*/
import java.util.List;
//% set listImported = true
/*{% endif %}*/

/*{% for prop in component.properties %}*/
/*{%   if prop is struct %}*/
/*{%     for p in prop.fields %}*/
/*{%       if p is simplesequence and not listImported %}*/
import java.util.List;
//%          set listImported = true
/*{%       endif %}*/
/*{%     endfor %}*/
/*{%   elif prop is structsequence %}*/
/*{%     for p in prop.structdef.fields %}*/
/*{%       if p is simplesequence and not listImported %}*/
import java.util.List;
//%          set listImported = true
/*{%       endif %}*/
/*{%     endfor %}*/
/*{%   endif %}*/
/*{% endfor %}*/
import java.util.Properties;
import org.ossie.component.RHLogger;

import org.apache.log4j.Logger;

/*{% if component.events %}*/
import org.omg.CORBA.ORB;
/*{% endif %}*/
/*{% if component is not device %}*/
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.NotFound;
/*{% endif %}*/
/*{% if component.events %}*/
import org.omg.PortableServer.POA;
/*{% endif %}*/
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

/*{% if component is device %}*/
import CF.DevicePackage.AdminType;
import CF.DevicePackage.OperationalType;
import CF.DevicePackage.UsageType;
/*{% endif %}*/
import CF.InvalidObjectReference;

import org.ossie.component.*;
/*{% if component.properties %}*/
import org.ossie.properties.*;
/*{% endif %}*/
/*{% filter lines|unique|join('\n') %}*/
/*{%   for portgen in component.portgenerators %}*/
/*{%     if loop.first %}*/

/*{%     endif %}*/
/*{%     if portgen.package %}*/
import ${portgen.package}.${portgen.className()};
/*{%     endif %}*/

/*{%   endfor %}*/
/*{% endfilter %}*/
/*{% if component.hasmultioutport %}*/
import bulkio.connection_descriptor_struct;
/*{% endif %}*/

/*{% block frontendimports %}*/
/*{% if component.hasfrontendprovides %}*/
import frontend.*;
/*{% endif %}*/
/*{% endblock %}*/

/*{% block baseadditionalimports %}*/
/*# Allow for child class imports #*/
/*{% endblock %}*/

/**
 * This is the ${artifactType} code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general ${artifactType} housekeeping
 *
 * Source: ${component.profile.spd}
 *
 * @generated
 */

/*{% block classdeclaration %}*/
/*{% if component.hasfrontendprovides %}*/
/*{%     set implementedClasses = "" %}*/
/*{%     for port in component.ports if port is provides %}*/
/*{%         if port.javatype == "frontend.InAnalogScanningTunerPort" and "AnalogScanningTunerDelegate" not in implementedClasses %}*/
/*{%             set implementedClasses = implementedClasses + ",AnalogScanningTunerDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InDigitalScanningTunerPort" and "DigitalScanningTunerDelegate" not in implementedClasses %}*/
/*{%             set implementedClasses = implementedClasses + ",DigitalScanningTunerDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InAnalogTunerPort" and "AnalogTunerDelegate" not in implementedClasses %}*/
/*{%             set implementedClasses = implementedClasses + ",AnalogTunerDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InDigitalTunerPort" and "DigitalTunerDelegate" not in implementedClasses %}*/
/*{%             set implementedClasses = implementedClasses + ",DigitalTunerDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InFrontendTunerPort" and "FrontendTunerDelegate" not in implementedClasses %}*/
/*{%             set implementedClasses = implementedClasses + ",FrontendTunerDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InGPSPort" and "GPSDelegate" not in implementedClasses %}*/
/*{%             set implementedClasses = implementedClasses + ",GPSDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InNavDataPort" and "NavDataDelegate" not in implementedClasses %}*/
/*{%             set implementedClasses = implementedClasses + ",NavDataDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InRFInfoPort" and "RFInfoDelegate" not in implementedClasses %}*/
/*{%             set implementedClasses = implementedClasses + ",RFInfoDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InRFSourcePort" and "RFSourceDelegate" not in implementedClasses %}*/
/*{%             set implementedClasses = implementedClasses + ",RFSourceDelegate" %}*/
/*{%         endif %}*/
/*{%         if loop.last %}*/
/*{%             set implementedClasses = implementedClasses[1:] %}*/
public abstract class ${classname} extends ${superClass} implements ${implementedClasses}
{
/*{%         endif %}*/
/*{%     endfor %}*/
/*{% else %}*/
public abstract class ${classname} extends ${superClass} 
{
/*{% endif %}*/
/*{% endblock %}*/
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(${classname}.class.getName());

/*{% import "base/properties.java" as properties with context %}*/
/*{% for prop in component.properties if prop is enumerated %}*/
/*{%   if loop.first %}*/
    /**
     * Enumerated values for properties
     */
    public static class enums {
/*{%   endif %}*/
        ${properties.enumvalues(prop)|indent(8)}
/*{%   if loop.last %}*/
    }
/*{%   endif %}*/

/*{% endfor %}*/
/*{% for prop in component.properties %}*/
/*{%   if not prop.inherited %}*/
    ${properties.create(prop)|indent(4)}
//%    endif
/*{% endfor %}*/
/*{% for port in component.ports if port is provides %}*/
/*{%   if loop.first %}*/
    // Provides/inputs
/*{%   endif %}*/
    /**
     * ${port.description|default("If the meaning of this port isn't clear, a description should be added.", true)}
     *
     * @generated
     */
    public ${port.javatype} ${port.javaname};

/*{% endfor %}*/
/*{% for port in component.ports if port is uses %}*/
/*{%   if loop.first %}*/
    // Uses/outputs
/*{%   endif %}*/
    /**
     * ${port.description|default("If the meaning of this port isn't clear, a description should be added.", true)}
     *
     * @generated
     */
    public ${port.javatype} ${port.javaname};

/*{% endfor %}*/
    /**
     * @generated
     */
    public ${classname}()
    {
/*{% if component.hastunerstatusstructure %}*/
        super(frontend_tuner_status_struct_struct.class);
/*{% else %}*/
        super();
/*{% endif %}*/

        setLogger( logger, ${classname}.class.getName() );

/*{% for prop in component.properties %}*/
/*{%   if loop.first %}*/

        // Properties
/*{%   endif %}*/
/*{%   if not prop.inherited %}*/
        addProperty(${prop.javaname});

/*{%   elif prop.javavalue and prop.javavalue != "null" %}*/
        ${prop.javaname}.setValue(${prop.javavalue});
/*{%   endif %}*/
/*{% endfor %}*/
/*{% for port in component.ports if port is provides %}*/
/*{%   if loop.first %}*/

        // Provides/inputs
/*{%   endif %}*/
        this.${port.javaname} = new ${port.constructor};
/*{%   if port.hasDescription %}*/
        this.addPort("${port.name}", "${port.description}", this.${port.javaname}); 
/*{%   else %}*/
        this.addPort("${port.name}", this.${port.javaname});
/*{%   endif %}*/
/*{% endfor %}*/
/*{% for port in component.ports if port is uses %}*/
/*{%   if loop.first %}*/

        // Uses/outputs
/*{%   endif %}*/
        this.${port.javaname} = new ${port.constructor};
/*{%   if port.hasDescription %}*/
        this.addPort("${port.name}", "${port.description}", this.${port.javaname}); 
/*{%   else %}*/
        this.addPort("${port.name}", this.${port.javaname});
/*{%   endif %}*/
/*{% endfor %}*/
/*{% if component.hasmultioutport %}*/
/*{% if 'FrontendTuner' in component.implements %}*/
        this.listeners = new HashMap<String, String>();
/*{% endif %}*/

        this.connectionTable.addChangeListener(new PropertyListener<List<connection_descriptor_struct>>() {
            public void valueChanged (List<connection_descriptor_struct> oldValue, List<connection_descriptor_struct> newValue)
            {
                ${classname}.this.connectionTableChanged(oldValue, newValue);
            }
        });
/*{% endif %}*/
    }

/*{% block getTunerStatus %}*/
/*{% if 'FrontendTuner' in component.implements %}*/
    public CF.DataType[] getTunerStatus(final String allocation_id) throws FRONTEND.FrontendException
    {
        return new CF.DataType[0];
    }
/*{% endif %}*/
/*{% endblock %}*/

    protected void setupPortLoggers() {
/*{% for port in component.ports %}*/
        ${port.javaname}.setLogger(this._baseLog.getChildLogger("${port.name}", "ports"));
/*{% endfor %}*/
    }

/*{% if component.events %}*/
    /**
     * @generated
     */
    public CF.Resource setup(final String compId, final String compName, final ORB orb, final POA poa) throws ServantNotActive, WrongPolicy
    {
    	CF.Resource retval = super.setup(compId, compName, orb, poa);
    	
/*{% for prop in component.events %}*/
        this.port_propEvent.registerProperty(this.compId, this.compName, this.${prop.javaname});
/*{% endfor %}*/
        
        this.registerPropertyChangePort(this.port_propEvent);
        
    	return retval;
    }

/*{% endif %}*/
/*{% if component.hasmultioutport %}*/

    protected void connectionTableChanged (List<connection_descriptor_struct> oldValue, List<connection_descriptor_struct> newValue)
    {
/*{% for port in component.ports if port.multiout %}*/
        this.${port.javaname}.updateConnectionFilter(newValue);
/*{% endfor %}*/
    }
/*{% endif %}*/

/*{% block extensions %}*/
/*# Allow for child class extensions #*/
/*{% endblock %}*/

/*{% block resourcemain %}*/
    /**
     * The main function of your ${artifactType}.  If no args are provided, then the
     * CORBA object is not bound to an SCA Domain or NamingService and can
     * be run as a standard Java application.
     * 
     * @param args
     * @generated
     */
    public static void main(String[] args) 
    {
        final Properties orbProps = new Properties();
        ${userclass}.configureOrb(orbProps);

        try {
            ${superClass}.start_${artifactType}(${userclass}.class, args, orbProps);
        } catch (InvalidObjectReference e) {
            e.printStackTrace();
/*{% if component is not device %}*/
        } catch (NotFound e) {
            e.printStackTrace();
        } catch (CannotProceed e) {
            e.printStackTrace();
        } catch (InvalidName e) {
            e.printStackTrace();
/*{% endif %}*/
        } catch (ServantNotActive e) {
            e.printStackTrace();
        } catch (WrongPolicy e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
    }
/*{% endblock %}*/
}
