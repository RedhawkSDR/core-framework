/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK codegenTesting.
 *
 * REDHAWK codegenTesting is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package event_props.java;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Properties;
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import CF.PropertiesHolder;
import CF.ResourceHelper;
import CF.UnknownProperties;
import CF.LifeCyclePackage.InitializeError;
import CF.LifeCyclePackage.ReleaseError;
import CF.InvalidObjectReference;
import CF.PropertySetPackage.InvalidConfiguration;
import CF.PropertySetPackage.PartialConfiguration;
import CF.ResourcePackage.StartError;
import CF.ResourcePackage.StopError;
import CF.DataType;
import org.omg.CORBA.UserException;
import org.omg.CosNaming.NameComponent;
import org.apache.log4j.Logger;
import org.ossie.component.*;
import org.ossie.properties.*;
import org.ossie.events.PropertyEventSupplier;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 *
 * Source: event_props.spd.xml
 *
 * @generated
 */
public class event_props extends event_props_base {
    public event_props()
    {
        super();
        this.propToSend.addChangeListener(new PropertyListener<String>(){
                                       public void valueChanged(String old_value, String new_value){
                                           propToSendChanged(old_value, new_value);
                                       }
        });
    }

    /**
     * @generated
     */
    public CF.Resource setup(final String compId, final String compName, final ORB orb, final POA poa) throws ServantNotActive, WrongPolicy
    {
    	CF.Resource retval = super.setup(compId, compName, orb, poa);
    	
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventShortSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventStringSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventBoolSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventUlongSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventFloatSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventOctetSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventCharSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventUshortSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventDoubleSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventLongSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventLonglongSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventUlonglongSimple);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventStringSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventBoolSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventUlongSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventFloatSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventOctetSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventCharSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventUshortSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventDoubleSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventLongSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventLonglongSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventUlonglongSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventShortSeq);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventStruct);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.eventStructSeq);
        
        this.registerPropertyChangePort(this.port_propEvent);
        
        //begin-user-code
        //end-user-code
        
    	return retval;
    }


    protected int serviceFunction() 
    {
        //begin-user-code
        //end-user-code
        
            //begin-user-code
            // Process data here
            try {
                logger.debug("run() example log message");
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }
            return NOOP;     
        
        //begin-user-code
        //end-user-code
    }

    private void propToSendChanged(String old_value, String new_value) {
        port_propEvent.sendPropertyEvent(new_value);
        eventSent.setValue(true);
    }
        
    /**
     * Set additional options for ORB startup. For example:
     *
     *   orbProps.put("com.sun.CORBA.giop.ORBFragmentSize", Integer.toString(fragSize));
     *
     * @param orbProps
     */
    public static void configureOrb(final Properties orbProps) {
    }

}
