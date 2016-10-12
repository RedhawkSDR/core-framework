/*
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
 */

package BasicService_java.java;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Map;
import java.util.Properties;
import org.apache.log4j.Logger;

import org.ossie.component.Service;
import org.ossie.properties.AnyUtils;
import org.ossie.properties.IProperty;

import CF.InvalidObjectReference;

import org.omg.PortableServer.POA;
import org.omg.PortableServer.Servant;

import CF.DataType;
import CF.PropertySetOperations;
import CF.PropertySetPOATie;

/**
 * @generated
 */

public class BasicService_java extends Service implements PropertySetOperations
{
    public final static Logger logger = Logger.getLogger(BasicService_java.class.getName());
    private Map<String, String> params;
    
    public BasicService_java(Map<String, String> execparams)
    {
        params = execparams;
    }

    public void terminateService()
    {
    }


    public void initializeProperties (CF.DataType[] configProperties) throws CF.PropertySetPackage.InvalidConfiguration, CF.PropertySetPackage.PartialConfiguration
    {
    }

    public void configure (CF.DataType[] configProperties) throws CF.PropertySetPackage.InvalidConfiguration, CF.PropertySetPackage.PartialConfiguration
    {
        // TODO
    }

    public void query (CF.PropertiesHolder configProperties) throws CF.UnknownProperties
    {
        // for queries of zero length, return all id/value pairs in propertySet
        if (configProperties.value.length == 0) {
            final ArrayList<DataType> props = new ArrayList<DataType>(this.params.size());
            for (String currKey : this.params.keySet()) {
                props.add(new DataType(currKey, AnyUtils.stringToAny(this.params.get(currKey), "string")));
            }
            
            // Add other prop types
            props.add(new DataType("PARAM1", AnyUtils.stringToAny("ABCD", "string")));
            props.add(new DataType("PARAM2", AnyUtils.stringToAny("42", "long" )));
            props.add(new DataType("PARAM3", AnyUtils.stringToAny("3.1459", "float")));
            props.add(new DataType("PARAM4", AnyUtils.stringToAny("False", "boolean")));
            props.add(new DataType("PARAM5", AnyUtils.stringToAny("Hello World", "string")));
            
            configProperties.value = props.toArray(new DataType[props.size()]);
            return;
        }
    }

    /**
     * {@inheritDoc}
     */
    public String registerPropertyListener(final org.omg.CORBA.Object listener, String[] prop_ids, float interval)
        throws CF.UnknownProperties, CF.InvalidObjectReference
    {
        throw new CF.UnknownProperties();
    }


    public void unregisterPropertyListener(final String reg_id)
        throws CF.InvalidIdentifier
    {
        throw new CF.InvalidIdentifier();
    }


    protected Servant newServant(final POA poa)
    {
        return new PropertySetPOATie(this, poa);
    }

    public static void main (String[] args)
    {
        final Properties orbProps = new Properties();
        try {
            Service.start_service(BasicService_java.class, args, orbProps);
        } catch (InvalidObjectReference e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();  
        } catch (NoSuchMethodException e) {
            logger.error("Service must immplement constructor: BasicService_java(Map<String,String> execparams)");
        }
    }
}


