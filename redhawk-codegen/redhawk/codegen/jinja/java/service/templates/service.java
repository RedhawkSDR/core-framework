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
//% set interface = component.interface
//% set operations = component.operations
//% set attributes = component.attributes
//% set mainclass = component.mainclass
//% set namespace = component.namespace
//% set imports = component.imports
//% set userclass = component.userclass.name

package ${component.package};

import java.lang.reflect.InvocationTargetException;
import java.util.Map;
import java.util.Properties;
import org.apache.log4j.Logger;

import org.ossie.component.Service;
import org.ossie.component.RHLogger;
import CF.InvalidObjectReference;

import org.omg.PortableServer.POA;
import org.omg.PortableServer.Servant;

import ${imports}${namespace}.${interface}Operations;
import ${imports}${namespace}.${interface}POATie;

/**
 * @generated
 */

public class ${userclass} extends Service implements ${interface}Operations
{
    public final static Logger logger = Logger.getLogger(${userclass}.class.getName());

    public ${userclass}(Map<String, String> execparams)
    {
        setLogger( logger, ${userclass}.class.getName() );
        if (execparams.containsKey("SERVICE_NAME")) {
            this.serviceName = execparams.get("SERVICE_NAME");
            _baseLog = RHLogger.getLogger(this.serviceName);
        }
    }

    public void terminateService()
    {
    }

/*{% for function in operations %}*/
    public ${function.returns} ${function.name} (${function.arglist}) ${"throws " + function.throws if function.throws}
    {
        // TODO
/*{% if function.returns != 'void' %}*/
        return ${java.defaultValue(function.returns)};
/*{% endif %}*/
    }

/*{% endfor %}*/
/*{% for attr in attributes %}*/
    // ${attr.name} getter
    public ${attr.type} ${attr.name} ()
    {
        // TODO
        return ${java.defaultValue(attr.type)};
    }
/*{% if attr.readonly != 1 %}*/

    // ${attr.name} setter
    public void ${attr.name} (${attr.type} new_${attr.name})
    {
        // TODO
    }
/*{% endif %}*/

/*{% endfor %}*/
    protected Servant newServant(final POA poa)
    {
        return new ${interface}POATie(this, poa);
    }

    public static void main (String[] args)
    {
        final Properties orbProps = new Properties();
        try {
            Service.start_service(${userclass}.class, args, orbProps);
        } catch (InvalidObjectReference e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();  
        } catch (NoSuchMethodException e) {
            logger.error("Service must immplement constructor: ${userclass}(Map<String,String> execparams)");
        }
    }
}

