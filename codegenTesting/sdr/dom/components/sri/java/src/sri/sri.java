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
package sri;

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
import bulkio.*;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 *
 * Source: sri.spd.xml
 *
 * @generated
 */
public class sri extends sri_base {
    public sri()
    {
        super();
        this.mqd.addChangeListener(new PropertyListener<Short>(){ 
                                       public void valueChanged(Short old_value, Short new_value){
                                           mqdChanged(old_value, new_value);
                                       }
        });
    }

    protected int serviceFunction() 
    {
            //begin-user-code
            InShortPort.Packet short_data = this.port_dataShortIn.getPacket(0);
            if (short_data != null) {
                if (short_data.sriChanged()) {
                    this.port_dataShortOut.pushSRI(short_data.getSRI());
                }

                if (short_data.inputQueueFlushed()) {
                    short [] data = new short[1];
                    data[0] = 0;
                                
                    this.port_dataShortOut.pushPacket(data, short_data.getTime(), short_data.getEndOfStream(), short_data.getStreamID());
                } else {
                        
                    this.port_dataShortOut.pushPacket(short_data.getData(), short_data.getTime(), short_data.getEndOfStream(), short_data.getStreamID());
                }
            }
            //end-user-code
            return NOOP;
    }

    public void mqdChanged(Short old_value, Short new_value)
    {
        this.port_dataShortIn.setMaxQueueDepth(new_value);
        mqd.setValue((short) this.port_dataShortIn.getMaxQueueDepth());
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
