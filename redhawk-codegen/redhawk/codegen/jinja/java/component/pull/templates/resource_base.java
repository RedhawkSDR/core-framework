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

/*{% if component.hasmultioutport %}*/
import java.util.List;
/*{% endif %}*/
import java.util.Properties;

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

/**
 * This is the ${artifactType} code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general ${artifactType} housekeeping
 *
 * Source: ${component.profile.spd}
 *
 * @generated
 */
public abstract class ${classname} extends ${superClass} {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(${classname}.class.getName());

/*{% import "base/properties.java" as properties with context %}*/
/*{% for prop in component.properties %}*/
    ${properties.create(prop)|indent(4)}
/*{% endfor %}*/
/*{% for port in component.ports if port is provides %}*/
/*{%   if loop.first %}*/
    // Provides/inputs
/*{%   endif %}*/
    /**
     * @generated
     */
    public ${port.javatype} ${port.javaname};

/*{% endfor %}*/
/*{% for port in component.ports if port is uses %}*/
/*{%   if loop.first %}*/
    // Uses/outputs
/*{%   endif %}*/
    /**
     * @generated
     */
    public ${port.javatype} ${port.javaname};

/*{% endfor %}*/
    /**
     * @generated
     */
    public ${classname}()
    {
        super();
/*{% for prop in component.properties %}*/
/*{%   if loop.first %}*/

        // Properties
/*{%   endif %}*/
        addProperty(${prop.javaname});
/*{% endfor %}*/
/*{% for port in component.ports if port is provides %}*/
/*{%   if loop.first %}*/

        // Provides/inputs
/*{%   endif %}*/
        this.${port.javaname} = new ${port.constructor};
        this.addPort("${port.name}", this.${port.javaname});
/*{% endfor %}*/
/*{% for port in component.ports if port is uses %}*/
/*{%   if loop.first %}*/

        // Uses/outputs
/*{%   endif %}*/
        this.${port.javaname} = new ${port.constructor};
        this.addPort("${port.name}", this.${port.javaname});
/*{% endfor %}*/
/*{% if component.hasmultioutport %}*/

        this.connectionTable.addChangeListener(new PropertyListener<List<connection_descriptor_struct>>() {
            public void valueChanged (List<connection_descriptor_struct> oldValue, List<connection_descriptor_struct> newValue)
            {
                ${classname}.this.connectionTableChanged(oldValue, newValue);
            }
        });
/*{% endif %}*/
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
    public void start() throws CF.ResourcePackage.StartError
    {
/*{% for port in component.ports if port.start %}*/
        this.${port.javaname}.${port.start};
/*{% endfor %}*/
        super.start();
    }

    public void stop() throws CF.ResourcePackage.StopError
    {
/*{% for port in component.ports if port.stop %}*/
        this.${port.javaname}.${port.stop};
/*{% endfor %}*/
        super.stop();
    }
/*{% if component.hasmultioutport %}*/

    protected void connectionTableChanged (List<connection_descriptor_struct> oldValue, List<connection_descriptor_struct> newValue)
    {
/*{% for port in component.ports if port.multiout %}*/
        this.${port.javaname}.updateConnectionFilter(newValue);
/*{% endfor %}*/
    }
/*{% endif %}*/

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
}
