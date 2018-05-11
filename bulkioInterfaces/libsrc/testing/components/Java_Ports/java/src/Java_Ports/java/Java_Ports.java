/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package Java_Ports.java;


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
import org.ossie.properties.AnyUtils;
import BULKIO.StreamSRI;
import BULKIO.PrecisionUTCTime;
import BULKIO.SDDSStreamDefinition;
import BULKIO.dataSDDSPackage.AttachError;
import BULKIO.dataSDDSPackage.DetachError;
import BULKIO.dataSDDSPackage.StreamInputError;
import org.ossie.events.*;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 *
 * Source: Java_Ports.spd.xml
 * Generated on: Mon Mar 11 15:04:23 EDT 2013
 * Redhawk IDE
 * Version:N.1.8.3
 * Build id: v201302111216

 * @generated
 */
public class Java_Ports extends Resource implements Runnable,  bulkio.InSDDSPort.Callback  {

    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(Java_Ports.class.getName());

    public bulkio.InCharPort   port_dataCharIn;
    public bulkio.InOctetPort  port_dataOctetIn;
    public bulkio.InShortPort  port_dataShortIn;
    public bulkio.InUShortPort  port_dataUShortIn;
    public bulkio.InLongPort  port_dataLongIn;
    public bulkio.InULongPort  port_dataULongIn;
    public bulkio.InLongLongPort  port_dataLongLongIn;
    public bulkio.InULongLongPort  port_dataULongLongIn;
    public bulkio.InDoublePort  port_dataDoubleIn;
    public bulkio.InFloatPort  port_dataFloatIn;
    public bulkio.InFilePort  port_dataFileIn;
    public bulkio.InXMLPort  port_dataXMLIn;
    public bulkio.InSDDSPort  port_dataSDDSIn;

    public bulkio.OutCharPort   port_dataCharOut;
    public bulkio.OutOctetPort  port_dataOctetOut;
    public bulkio.OutShortPort  port_dataShortOut;
    public bulkio.OutUShortPort  port_dataUShortOut;
    public bulkio.OutLongPort  port_dataLongOut;
    public bulkio.OutULongPort  port_dataULongOut;
    public bulkio.OutLongLongPort  port_dataLongLongOut;
    public bulkio.OutULongLongPort  port_dataULongLongOut;
    public bulkio.OutDoublePort  port_dataDoubleOut;
    public bulkio.OutFloatPort  port_dataFloatOut;
    public bulkio.OutFilePort  port_dataFileOut;
    public bulkio.OutXMLPort  port_dataXMLOut;
    public bulkio.OutSDDSPort  port_dataSDDSOut;

        /**
     * @generated
     */
    public PropertyEventSupplier port_propEvent;
        /**
     * @generated
     */
    public Java_Ports()
    {
        super();

        // Provides/input
        this.port_dataCharIn = new bulkio.InCharPort("dataCharIn");
        this.addPort("dataCharIn", this.port_dataCharIn);
        this.port_dataOctetIn = new bulkio.InOctetPort("dataOctetIn");
        this.addPort("dataOctetIn", this.port_dataOctetIn);
        this.port_dataShortIn = new bulkio.InShortPort("dataShortIn");
        this.addPort("dataShortIn", this.port_dataShortIn);
        this.port_dataUShortIn = new bulkio.InUShortPort("dataUShortIn");
        this.addPort("dataUShortIn", this.port_dataUShortIn);
        this.port_dataLongIn = new bulkio.InLongPort("dataLongIn");
        this.addPort("dataLongIn", this.port_dataLongIn);
        this.port_dataULongIn = new bulkio.InULongPort("dataULongIn");
        this.addPort("dataULongIn", this.port_dataULongIn);
        this.port_dataLongLongIn = new bulkio.InLongLongPort("dataLongLongIn");
        this.addPort("dataLongLongIn", this.port_dataLongLongIn);
        this.port_dataULongLongIn = new bulkio.InULongLongPort("dataULongLongIn");
        this.addPort("dataULongLongIn", this.port_dataULongLongIn);
        this.port_dataFloatIn = new bulkio.InFloatPort("dataFloatIn");
        this.addPort("dataFloatIn", this.port_dataFloatIn);
        this.port_dataDoubleIn = new bulkio.InDoublePort("dataDoubleIn");
        this.addPort("dataDoubleIn", this.port_dataDoubleIn);
        this.port_dataFileIn = new bulkio.InFilePort("dataFileIn");
        this.addPort("dataFileIn", this.port_dataFileIn);
        this.port_dataXMLIn = new bulkio.InXMLPort("dataXMLIn");
        this.addPort("dataXMLIn", this.port_dataXMLIn);
        this.port_dataSDDSIn = new bulkio.InSDDSPort("dataSDDSIn", this);
        this.addPort("dataSDDSIn", this.port_dataSDDSIn);

        // Uses/output
        this.port_dataCharOut = new bulkio.OutCharPort("dataCharOut");
        this.addPort("dataCharOut", this.port_dataCharOut);
        this.port_dataOctetOut = new bulkio.OutOctetPort("dataOctetOut");
        this.addPort("dataOctetOut", this.port_dataOctetOut);
        this.port_dataShortOut = new bulkio.OutShortPort("dataShortOut");
        this.addPort("dataShortOut", this.port_dataShortOut);
        this.port_dataUShortOut = new bulkio.OutUShortPort("dataUShortOut");
        this.addPort("dataUShortOut", this.port_dataUShortOut);
        this.port_dataLongOut = new bulkio.OutLongPort("dataLongOut");
        this.addPort("dataLongOut", this.port_dataLongOut);
        this.port_dataULongOut = new bulkio.OutULongPort("dataULongOut");
        this.addPort("dataULongOut", this.port_dataULongOut);
        this.port_dataLongLongOut = new bulkio.OutLongLongPort("dataLongLongOut");
        this.addPort("dataLongLongOut", this.port_dataLongLongOut);
        this.port_dataULongLongOut = new bulkio.OutULongLongPort("dataULongLongOut");
        this.addPort("dataULongLongOut", this.port_dataULongLongOut);
        this.port_dataFloatOut = new bulkio.OutFloatPort("dataFloatOut");
        this.addPort("dataFloatOut", this.port_dataFloatOut);
        this.port_dataDoubleOut = new bulkio.OutDoublePort("dataDoubleOut");
        this.addPort("dataDoubleOut", this.port_dataDoubleOut);
        this.port_dataFileOut = new bulkio.OutFilePort("dataFileOut");
        this.addPort("dataFileOut", this.port_dataFileOut);
        this.port_dataXMLOut = new bulkio.OutXMLPort("dataXMLOut");
        this.addPort("dataXMLOut", this.port_dataXMLOut);
        this.port_propEvent = new PropertyEventSupplier("propEvent");
        this.addPort("propEvent", this.port_propEvent);
        this.port_dataSDDSOut = new bulkio.OutSDDSPort("dataSDDSOut");
        this.addPort("dataSDDSOut", this.port_dataSDDSOut);
    }


    /**
     * @generated
     */
    public CF.Resource setup(final String compId, final String compName, final ORB orb, final POA poa) throws ServantNotActive, WrongPolicy
    {
            CF.Resource retval = super.setup(compId, compName, orb, poa);
        this.registerPropertyChangePort(this.port_propEvent);
            return retval;
    }

        /**
     * This method is used to handle the attach call and return an attachId for the connection.
     * The value of the attachId should be unique across attach calls within this component instance.
     *
     * @param stream the new stream definition
     * @param userId the userId for the stream
     * @return an id for this attach call that is unique across all calls within this component instance.
         * @throws AttachError
         * @throws StreamInputError
     * @generated
     */
    public String attach(SDDSStreamDefinition stream, String userId) throws AttachError, StreamInputError {

        String attachID = java.util.UUID.randomUUID().toString();
        logger.debug( " ATTACH: " + attachID + " STREAM ID/ADDR/PORT:" + stream.id  + "/" + stream.multicastAddress + "/" + stream.port );
        try{
            port_dataSDDSOut.attach( stream, userId );
        }catch (DetachError e){
            e.printStackTrace();
        }
        return attachID;
    }


    /**
     * This method is used to handle the detach call.
         *
         * @param attachId the attachId from a previous call to attach
         * @throws DetachError
         * @throws StreamInputError
         * @generated
         */
    public void detach(String attachId) throws DetachError, StreamInputError {
        logger.debug( "DETACH ATTACH: " + attachId  );
    }


        /**
     *
     * Main processing thread
     *
     * <!-- begin-user-doc -->
     *
     * General functionality:
     *
     *    This function is running as a separate thread from the component's main thread.
     *
     *    The IDE uses JMerge during the generation (and re-generation) process.  To keep
     *    customizations to this file from being over-written during subsequent generations,
     *    put your customization in between the following tags:
     *      - //begin-user-code
     *      - //end-user-code
     *    or, alternatively, set the @generated flag located before the code you wish to
     *    modify, in the following way:
     *      - "@generated NOT"
     *
     * StreamSRI:
     *    To create a StreamSRI object, use the following code:
     *        this.stream_id = "stream";
     *                   StreamSRI sri = new StreamSRI();
     *                   sri.mode = 0;
     *                   sri.xdelta = 0.0;
     *                   sri.ydelta = 1.0;
     *                   sri.subsize = 0;
     *                   sri.xunits = 1; // TIME_S
     *                   sri.streamID = (this.stream_id.getValue() != null) ? this.stream_id.getValue() : "";
     *
     * PrecisionUTCTime:
     *    To create a PrecisionUTCTime object, use the following code:
     *                   long tmp_time = System.currentTimeMillis();
     *                   double wsec = tmp_time / 1000;
     *                   double fsec = tmp_time % 1000;
     *                   PrecisionUTCTime tstamp = new PrecisionUTCTime(BULKIO.TCM_CPU.value, (short)1, (short)0, wsec, fsec);
     *
     * Ports:
     *
     *    Each port instance is accessed through members of the following form: this.port_<PORT NAME>
     *
     *    Data is obtained in the run function through the getPacket call (BULKIO only) on a
     *    provides port member instance. The getPacket function call is non-blocking; it takes
     *    one argument which is the time to wait on new data. If you pass 0, it will return
     *    immediately if no data available (won't wait).
     *
     *    To send data, call the appropriate function in the port directly. In the case of BULKIO,
     *    convenience functions have been added in the port classes that aid in output.
     *
     *    Interactions with non-BULKIO ports are left up to the component developer's discretion.
     *
     * Properties:
     *
     *    Properties are accessed through members of the same name with helper functions. If the
     *    property name is baudRate, then reading the value is achieved by: this.baudRate.getValue();
     *    and writing a new value is achieved by: this.baudRate.setValue(new_value);
     *
     * Example:
     *
     *    This example assumes that the component has two ports:
     *        - A provides (input) port of type BULKIO::dataShort called dataShort_in
     *        - A uses (output) port of type BULKIO::dataFloat called dataFloat_out
     *    The mapping between the port and the class is found the class of the same name.
     *    This example also makes use of the following Properties:
     *        - A float value called amplitude with a default value of 2.0
     *        - A boolean called increaseAmplitude with a default value of true
     *
     *    BULKIO_dataShortInPort.Packet<short[]> data = this.port_dataShort_in.getPacket(125);
     *
     *    if (data != null) {
     *        float[] outData = new float[data.getData().length];
     *        for (int i = 0; i < data.getData().length; i++) {
     *            if (this.increaseAmplitude.getValue()) {
     *                outData[i] = (float)data.getData()[i] * this.amplitude.getValue();
     *            } else {
     *                outData[i] = (float)data.getData()[i];
     *            }
     *        }
     *
     *        // NOTE: You must make at least one valid pushSRI call
     *        if (data.sriChanged()) {
     *            this.port_dataFloat_out.pushSRI(data.getSRI());
     *        }
     *        this.port_dataFloat_out.pushPacket(outData, data.getTime(), data.getEndOfStream(), data.getStreamID());
     *    }
     *
     * <!-- end-user-doc -->
     *
     * @generated
     */
    public void run()
    {
        //begin-user-code
        //end-user-code

        while(this.started())
        {
            //begin-user-code
            // Process data here
            try {
                int serviced = 0;
                serviced += SF(port_dataFloatIn,port_dataFloatOut, "FLOAT" );
                serviced += SF(port_dataDoubleIn,port_dataDoubleOut, "DOUBLE");
                serviced += SF(port_dataCharIn,port_dataCharOut, "CHAR");
                serviced += SF(port_dataOctetIn,port_dataOctetOut, "OCTET");
                serviced += SF(port_dataShortIn,port_dataShortOut, "SHORT");
                serviced += SF(port_dataUShortIn,port_dataUShortOut, "USHORT");
                serviced += SF(port_dataLongIn,port_dataLongOut, "LONG");
                serviced += SF(port_dataULongIn,port_dataULongOut, "ULONG");
                serviced += SF(port_dataLongLongIn,port_dataLongLongOut, "LONGLONG");
                serviced += SF(port_dataULongLongIn,port_dataULongLongOut, "ULONGLONG");
                serviced += SF(port_dataFileIn,port_dataFileOut, "FILE");
                serviced += SF(port_dataXMLIn,port_dataXMLOut, "XML");

                if (serviced == 0) {
                    Thread.sleep(1000);
                }
            } catch (InterruptedException e) {
                break;
            }

            //end-user-code
        }

        //begin-user-code
        //end-user-code
    }


    int SF( bulkio.InFloatPort inPort, bulkio.OutFloatPort outPort, String portType ) {

       bulkio.InFloatPort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

       if ( pkt != null ) {

	   if (pkt.sriChanged ) {
	       outPort.pushSRI( pkt.SRI );
	   }

	   logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length );
	   outPort.pushPacket( pkt.dataBuffer, pkt.T, pkt.EOS, pkt.streamID );
           return 1;
       }
       return 0;
   }

   int SF( bulkio.InDoublePort inPort, bulkio.OutDoublePort outPort, String portType ) {

       bulkio.InDoublePort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

       if ( pkt != null ) {

	   if (pkt.sriChanged ) {
	       outPort.pushSRI( pkt.SRI );
	   }

	   logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length );
	   outPort.pushPacket( pkt.dataBuffer, pkt.T, pkt.EOS, pkt.streamID );
           return 1;
       }
       return 0;
   }

    public int SF( bulkio.InCharPort inPort, bulkio.OutCharPort outPort, String portType ) {

        bulkio.InCharPort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

	if ( pkt != null ) {
	    if (pkt.sriChanged ) {
		outPort.pushSRI( pkt.SRI );
	    }

	    logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length );

	    outPort.pushPacket( pkt.dataBuffer, pkt.T, pkt.EOS, pkt.streamID );
            return 1;
       }
       return 0;
    }

    public int SF( bulkio.InShortPort inPort, bulkio.OutShortPort outPort, String portType ) {

        bulkio.InShortPort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

	if ( pkt != null ) {
	    if (pkt.sriChanged ) {
		outPort.pushSRI( pkt.SRI );
	    }

	    logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length );

	    outPort.pushPacket( pkt.dataBuffer, pkt.T, pkt.EOS, pkt.streamID );
            return 1;
       }
       return 0;
    }


    public int SF( bulkio.InLongPort inPort, bulkio.OutLongPort outPort, String portType ) {

        bulkio.InLongPort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

	if ( pkt != null ) {
	    if (pkt.sriChanged ) {
		outPort.pushSRI( pkt.SRI );
	    }

	    logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length );

	    outPort.pushPacket( pkt.dataBuffer, pkt.T, pkt.EOS, pkt.streamID );
            return 1;
       }
       return 0;
    }

    public int SF( bulkio.InLongLongPort inPort, bulkio.OutLongLongPort outPort, String portType ) {

        bulkio.InLongLongPort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

	if ( pkt != null ) {
	    if (pkt.sriChanged ) {
		outPort.pushSRI( pkt.SRI );
	    }

	    logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length );

	    outPort.pushPacket( pkt.dataBuffer, pkt.T, pkt.EOS, pkt.streamID );
            return 1;
       }
       return 0;
    }



    public int SF( bulkio.InOctetPort inPort, bulkio.OutOctetPort outPort, String portType ) {

        bulkio.InOctetPort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

	if ( pkt != null ) {
	    if (pkt.sriChanged ) {
		outPort.pushSRI( pkt.SRI );
	    }

	    logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length );

	    outPort.pushPacket( pkt.dataBuffer, pkt.T, pkt.EOS, pkt.streamID );
            return 1;
       }
       return 0;
    }

    public int SF( bulkio.InUShortPort inPort, bulkio.OutUShortPort outPort, String portType ) {

        bulkio.InUShortPort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

	if ( pkt != null ) {
	    if (pkt.sriChanged ) {
		outPort.pushSRI( pkt.SRI );
	    }

	    logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length );

	    outPort.pushPacket( pkt.dataBuffer, pkt.T, pkt.EOS, pkt.streamID );
            return 1;
       }
       return 0;
    }


    public int SF( bulkio.InULongPort inPort, bulkio.OutULongPort outPort, String portType ) {

        bulkio.InULongPort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

	if ( pkt != null ) {
	    if (pkt.sriChanged ) {
		outPort.pushSRI( pkt.SRI );
	    }

	    logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length );

	    outPort.pushPacket( pkt.dataBuffer, pkt.T, pkt.EOS, pkt.streamID );
            return 1;
       }
       return 0;
    }

    public int SF( bulkio.InULongLongPort inPort, bulkio.OutULongLongPort outPort, String portType ) {

        bulkio.InULongLongPort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

	if ( pkt != null ) {
	    if (pkt.sriChanged ) {
		outPort.pushSRI( pkt.SRI );
	    }

	    logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length );

	    outPort.pushPacket( pkt.dataBuffer, pkt.T, pkt.EOS, pkt.streamID );
            return 1;
       }
       return 0;
    }




    public int SF( bulkio.InFilePort inPort, bulkio.OutFilePort outPort, String portType ) {

        bulkio.InFilePort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

	if ( pkt != null ) {
	    if (pkt.sriChanged ) {
		outPort.pushSRI( pkt.SRI );
	    }

	    logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length() );

	    outPort.pushPacket( pkt.dataBuffer, pkt.T, pkt.EOS, pkt.streamID );
            return 1;
       }
       return 0;
    }

    public int SF( bulkio.InXMLPort inPort, bulkio.OutXMLPort outPort, String portType ) {

        bulkio.InXMLPort.Packet pkt = inPort.getPacket( bulkio.Const.NON_BLOCKING );

	if ( pkt != null ) {
	    if (pkt.sriChanged ) {
		outPort.pushSRI( pkt.SRI );
	    }

	    logger.debug( "SF  TYPE:" + portType + " DATALEN:" + pkt.dataBuffer.length() );

	    outPort.pushPacket( pkt.dataBuffer, pkt.EOS, pkt.streamID );
            return 1;
       }
       return 0;
    }

    public int SF( bulkio.InSDDSPort inPort, bulkio.OutSDDSPort outPort, String portType ) {
        return 0;
    }




    /**
     * The main function of your component.  If no args are provided, then the
     * CORBA object is not bound to an SCA Domain or NamingService and can
     * be run as a standard Java application.
     *
     * @param args
     * @generated
     */
    public static void main(String[] args)
    {
        final Properties orbProps = new Properties();

        //begin-user-code
        // TODO You may add extra startup code here, for example:
        // orbProps.put("com.sun.CORBA.giop.ORBFragmentSize", Integer.toString(fragSize));
        //end-user-code

        try {
            Resource.start_component(Java_Ports.class, args, orbProps);
        } catch (InvalidObjectReference e) {
            e.printStackTrace();
        } catch (NotFound e) {
            e.printStackTrace();
        } catch (CannotProceed e) {
            e.printStackTrace();
        } catch (InvalidName e) {
            e.printStackTrace();
        } catch (ServantNotActive e) {
            e.printStackTrace();
        } catch (WrongPolicy e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }

        //begin-user-code
        // TODO You may add extra shutdown code here
        //end-user-code
    }
}
