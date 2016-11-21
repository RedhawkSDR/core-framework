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
package bulkio_ports;

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
 * Source: bulkio_ports.spd.xml
 *
 * @generated
 */
public class bulkio_ports extends bulkio_ports_base {
    public bulkio_ports()
    {
        super();
        this.mqd.addChangeListener(new PropertyListener<Short>(){
                                       public void valueChanged(Short old_value, Short new_value){
                                           mqdChanged(old_value, new_value);
                                       }
        });
    }

    private void mqdChanged(Short old_value, Short new_value){
    	this.port_dataShortIn.setMaxQueueDepth(new_value);
    	mqd.setValue((short)this.port_dataShortIn.getMaxQueueDepth());
    }

    protected int serviceFunction() 
    {
        //begin-user-code
        //end-user-code
        
            //begin-user-code
            InCharPort.Packet char_data = this.port_dataCharIn.getPacket(0);
            if (char_data != null) {
                this.port_dataCharOut.pushPacket(char_data.getData(), char_data.getTime(), char_data.getEndOfStream(), char_data.getStreamID());
            }
        	
            InDoublePort.Packet double_data = this.port_dataDoubleIn.getPacket(0);
            if (double_data != null) {
                this.port_dataDoubleOut.pushPacket(double_data.getData(), double_data.getTime(), double_data.getEndOfStream(), double_data.getStreamID());
            }
        	
            InFloatPort.Packet float_data = this.port_dataFloatIn.getPacket(0);
            if (float_data != null) {
            	this.port_dataFloatOut.pushPacket(float_data.getData(), float_data.getTime(), float_data.getEndOfStream(), float_data.getStreamID());
            }
        	
            InLongPort.Packet long_data = this.port_dataLongIn.getPacket(0);
            if (long_data != null) {
            	this.port_dataLongOut.pushPacket(long_data.getData(), long_data.getTime(), long_data.getEndOfStream(), long_data.getStreamID());
            }
        	
            InOctetPort.Packet octet_data = this.port_dataOctetIn.getPacket(0);
            if (octet_data != null) {
            	this.port_dataOctetOut.pushPacket(octet_data.getData(), octet_data.getTime(), octet_data.getEndOfStream(), octet_data.getStreamID());
            }
        	
            InShortPort.Packet short_data = this.port_dataShortIn.getPacket(0);
            if (short_data != null) {
            	this.port_dataShortOut.pushPacket(short_data.getData(), short_data.getTime(), short_data.getEndOfStream(), short_data.getStreamID());
            }
        	
            InULongPort.Packet ulong_data = this.port_dataUlongIn.getPacket(0);
            if (ulong_data != null) {
            	this.port_dataUlongOut.pushPacket(ulong_data.getData(), ulong_data.getTime(), ulong_data.getEndOfStream(), ulong_data.getStreamID());
            }
        	
            InLongLongPort.Packet longlong_data = this.port_dataLongLongIn.getPacket(0);
            if (longlong_data != null) {
            	this.port_dataLongLongOut.pushPacket(longlong_data.getData(), longlong_data.getTime(), longlong_data.getEndOfStream(), longlong_data.getStreamID());
            }
        	
            InULongLongPort.Packet ulonglong_data = this.port_dataUlongLongIn.getPacket(0);
            if (ulonglong_data != null) {
            	this.port_dataUlongLongOut.pushPacket(ulonglong_data.getData(), ulonglong_data.getTime(), ulonglong_data.getEndOfStream(), ulonglong_data.getStreamID());
            }
        	
            InUShortPort.Packet ushort_data = this.port_dataUshortIn.getPacket(0);
            if (ushort_data != null) {
            	this.port_dataUshortOut.pushPacket(ushort_data.getData(), ushort_data.getTime(), ushort_data.getEndOfStream(), ushort_data.getStreamID());
            }
        	
            InXMLPort.Packet xml_data = this.port_dataXMLIn.getPacket(0);
            if (xml_data != null) {
            	this.port_dataXMLOut.pushPacket(xml_data.getData(), xml_data.getEndOfStream(), xml_data.getStreamID());
            }
        	
            InFilePort.Packet file_data = this.port_dataFileIn.getPacket(0);
            if (file_data != null) {
            	this.port_dataFileOut.pushPacket(file_data.getData(), file_data.getTime(), file_data.getEndOfStream(), file_data.getStreamID());
            }
            return NORMAL;
            //end-user-code
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
