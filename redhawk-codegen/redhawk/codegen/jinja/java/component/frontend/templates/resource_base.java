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
//% set userclass = component.userclass.name
//% set classname = component.baseclass.name
//% set superClass = component.superclass.name
//% set artifactType = component.artifacttype
package ${component.package};

/*{% if component is device %}*/
import java.util.HashMap;
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
/*{% filter lines|unique|join('\n') %}*/
/*{%    for prop in component.properties %}*/
/*{%        if prop.CFType %}*/
import CF.${prop.CFType};
/*{%        endif %}*/
/*{%    endfor %}*/
/*{% endfilter %}*/

import org.ossie.component.*;
/*{% if component.properties %}*/
import org.ossie.properties.*;
/*{% endif %}*/
/*{% if component.hasbulkio %}*/

import bulkio.*;
/*{% endif %}*/
/*{% if component.hasfrontend %}*/
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import CF.DeviceManager;
import CF.DevicePackage.InvalidCapacity;

import frontend.*;
/*{% endif %}*/
/*{% if component.isafrontendtuner %}*/
import org.omg.CORBA.Any;
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

/**
 * This is the ${artifactType} code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general ${artifactType} housekeeping
 *
 * Source: ${component.profile.spd}
 *
 * @generated
 */
/*{% if component.hasfrontendprovides %}*/
/*{%     set implementedClasses = "" %}*/
/*{%     for port in component.ports if port is provides %}*/
/*{%         if port.javatype == "frontend.InAnalogTunerPort" %}*/
/*{%             set implementedClasses = implementedClasses + ",AnalogTunerDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InDigitalTunerPort" %}*/
/*{%             set implementedClasses = implementedClasses + ",DigitalTunerDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InFrontendTunerPort" %}*/
/*{%             set implementedClasses = implementedClasses + ",FrontendTunerDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InGPSPort" %}*/
/*{%             set implementedClasses = implementedClasses + ",GPSDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InNavDataPort" %}*/
/*{%             set implementedClasses = implementedClasses + ",NavDataDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InRFInfoPort" %}*/
/*{%             set implementedClasses = implementedClasses + ",RFInfoDelegate" %}*/
/*{%         endif %}*/
/*{%         if port.javatype == "frontend.InRFSourcePort" %}*/
/*{%             set implementedClasses = implementedClasses + ",RFSourceDelegate" %}*/
/*{%         endif %}*/
/*{%         if loop.last %}*/
public abstract class ${classname} extends ${superClass} implements Runnable ${implementedClasses}{
/*{%         endif %}*/
/*{%     endfor %}*/
/*{% else %}*/
public abstract class ${classname} extends ${superClass} implements Runnable {
/*{% endif %}*/
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(${classname}.class.getName());

    /**
     * Return values for service function.
     */
    public final static int FINISH = -1;
    public final static int NOOP   = 0;
    public final static int NORMAL = 1;
/*{% if component.hasfrontend %}*/
    public final static boolean BOOLEAN_VALUE_HERE=false;
    public final static double  DOUBLE_VALUE_HERE=0.0;
/*{% endif %}*/

/*{% import "base/properties.java" as properties with context %}*/
/*{% from "frontend_properties.java" import frontendstructsequence with context %}*/
/*{% for prop in component.properties %}*/
/*{%     if ((not component.isafrontendtuner) or
             (component.isafrontendtuner and 
              prop.name != "device_kind" and 
              prop.name != "device_model" and 
              prop.name != "frontend_tuner_status" and 
              prop.name != "frontend_tuner_allocation" and 
              prop.name != "frontend_listener_allocation")) %}*/
/*{%         if prop is simple %}*/
    ${properties.simple(prop)|indent(4)}
/*{%         elif prop is simplesequence %}*/
    ${properties.simplesequence(prop)|indent(4)}
/*{%         elif prop is struct %}*/
    ${properties.struct(prop)|indent(4)}
/*{%         elif prop is structsequence %}*/
    ${properties.structsequence(prop)|indent(4)}
/*{%         endif %}*/
/*{%    endif %}*/
/*{%    if (component.isafrontendtuner and
             prop.name == "frontend_tuner_status") %}*/
    ${properties.header(prop)}
    ${frontendstructsequence(prop)|indent(4)}
/*{%    endif %}*/
/*{% endfor %}*/
    // Provides/inputs
/*{% for port in component.ports if port is provides %}*/
    /**
     * @generated
     */
    public ${port.javatype} ${port.javaname};

/*{% endfor %}*/
    // Uses/outputs
/*{% for port in component.ports if port is uses %}*/
    /**
     * @generated
     */
    public ${port.javatype} ${port.javaname};

/*{% endfor %}*/
    /**
     * @generated
     */
/*{% if component.isafrontendtuner %}*/
    public ${classname}(DeviceManager devMgr, String compId, String label, String softwareProfile, ORB orb, POA poa) throws InvalidObjectReference, ServantNotActive, WrongPolicy, CF.DevicePackage.InvalidCapacity {
        super(devMgr, compId, label, softwareProfile, orb, poa);
/*{% else %}*/
    public ${classname}() {
        super();
/*{% endif %}*/
/*{% if component is device %}*/
        this.usageState = UsageType.IDLE;
        this.operationState = OperationalType.ENABLED;
        this.adminState = AdminType.UNLOCKED;
        this.callbacks = new HashMap<String, AllocCapacity>();
/*{% endif %}*/
/*{% for prop in component.properties if prop is structsequence %}*/
        ${properties.structsequence_init(prop)|indent(8)}
/*{% endfor %}*/
/*{% for prop in component.properties %}*/
        addProperty(${prop.javaname});
/*{% endfor %}*/

        // Provides/input
/*{% for port in component.ports if port is provides %}*/
        this.${port.javaname} = new ${port.constructor};
        this.addPort("${port.name}", this.${port.javaname});
/*{% endfor %}*/

        // Uses/output
/*{% for port in component.ports if port is uses %}*/
        this.${port.javaname} = new ${port.constructor};
        this.addPort("${port.name}", this.${port.javaname});
/*{% endfor %}*/

/*{% for port in component.ports if port is provides %}*/
/*{%     if port.javatype == "frontend.InFrontendTunerPort" %}*/
    /*#- add FEI FrontendTuner callback functions #*/
    // Set callbacks for ${port.javatype}
    ${port.javaname}.setDelegate(this);
/*{%-     endif %}*/
/*{%     if port.javatype == "frontend.InAnalogTunerPort" %}*/
    /*#- add FEI AnalogTuner callback functions #*/
    ${port.javaname}.setDelegate(this);
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InDigitalTunerPort" %}*/
    /*#- add FEI DigitalTuner callback functions #*/
    ${port.javaname}.setDelegate(this);
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InGPSPort" %}*/
    // Set callbacks for ${port.javatype}
    ${port.javaname}.setDelegate(this);
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InNavDataPort" %}*/
    // Set callbacks for ${port.javatype}
    ${port.javaname}.setDelegate(this);
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InRFInfoPort" %}*/
    // Set callbacks for ${port.javatype}
    ${port.javaname}.setDelegate(this);
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InRFSourcePort" %}*/
    // Set callbacks for ${port.javatype}
    ${port.javaname}.setDelegate(this);
/*{%     endif %}*/
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
    public void run() 
    {
        while(this.started())
        {
            int state = this.serviceFunction();
            if (state == NOOP) {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    break;
                }
            } else if (state == FINISH) {
                return;
            }
        }
    }

    protected abstract int serviceFunction();

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
/*{% if component.isafrontendtuner %}*/
            ${superClass.split("<")[0]}.start_${artifactType}(${userclass}.class, args, orbProps);
/*{% else %}*/
            ${superClass}.start_${artifactType}(${userclass}.class, args, orbProps);
/*{% endif %}*/
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

/*{% set foundInFrontendInterface = False %}*/
/*{% set foundInAnalogInterface = False %}*/
/*{% set foundInDigitalInterface = False %}*/
/*{% set foundInGPSPort = False %}*/
/*{% set foundInNavDataPort = False %}*/
/*{% set foundInRFInfoPort = False %}*/
/*{% set foundInRFSourcePort = False %}*/
/*{% for port in component.ports if port is provides %}*/
/*{%     if port.javatype == "frontend.InDigitalTunerPort" or
            port.javatype == "frontend.InAnalogTunerPort" or
            port.javatype == "frontend.InFrontendTunerPort" %}*/
    /*#- add FEI FrontendTuner callback functions #*/
/*{%         if foundInFrontendInterface == False %}*/
    public String fe_getTunerType(String id) throws FRONTEND.FrontendException{
            int tuner_id = getTunerMapping(id);
            if (tuner_id < 0){
                throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
            }
            return tunerChannels.get(tuner_id).frontend_status.tuner_type.getValue();
    }

    public boolean fe_getTunerDeviceControl(String id) throws FRONTEND.FrontendException{
            int tuner_id = getTunerMapping(id);
            if (tuner_id < 0){
                throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
            }
            if(id == tunerChannels.get(tuner_id).control_allocation_id){
                return true;
            }
            return false;
    }

    public String fe_getTunerGroupId(String id) throws FRONTEND.FrontendException{
        int tuner_id = getTunerMapping(id);
        if (tuner_id < 0){
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        return tunerChannels.get(tuner_id).frontend_status.group_id.getValue();
    }

    public String fe_getTunerRfFlowId(String id) throws FRONTEND.FrontendException{
        int tuner_id = getTunerMapping(id);
        if (tuner_id < 0){
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        return tunerChannels.get(tuner_id).frontend_status.rf_flow_id.getValue();
    }

    public CF.DataType[] fe_getTunerStatus(String id) throws FRONTEND.FrontendException{
        int tuner_id = getTunerMapping(id);
        if (tuner_id < 0){
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        Any prop;
        prop = tunerChannels.get(tuner_id).frontend_status.toAny();
        CF.DataType[] tmpVal = (CF.DataType[]) AnyUtils.convertAny(prop);
        return tmpVal;
    }
/*{%             set foundInFrontendInterface = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InDigitalTunerPort" or
            port.javatype == "frontend.InAnalogTunerPort" %}*/
    /*#- add FEI AnalogTuner callback functions #*/
/*{%         if foundInAnalogInterface == False %}*/
    public double fe_getTunerCenterFrequency(String id) throws FRONTEND.FrontendException{
        int tuner_id = getTunerMapping(id);
        if (tuner_id < 0){
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        return tunerChannels.get(tuner_id).frontend_status.center_frequency.getValue();
    }

    public void fe_setTunerCenterFrequency(String id, double freq) throws FRONTEND.FrontendException{
        int tuner_id = getTunerMapping(id);
        if (tuner_id < 0){
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        if(id != tunerChannels.get(tuner_id).control_allocation_id){
                throw new FRONTEND.FrontendException("ERROR: ID: " + id + " DOES NOT HAVE AUTHORIZATION TO MODIFY TUNER!");
        }

        try {
                // If the freq has changed (change in stream) or the tuner is disabled, then set it as disabled
               boolean isTunerEnabled = tunerChannels.get(tuner_id).frontend_status.enabled.getValue();
                if (!isTunerEnabled || tunerChannels.get(tuner_id).frontend_status.center_frequency.getValue() != freq)
                        enableTuner(tuner_id, false); // TODO: is this a generic thing we can assume is desired when changing any tuner device?
                {
                    synchronized(tunerChannels.get(tuner_id).lock){
                        if (!_valid_center_frequency(freq,tuner_id)){
                            throw new FRONTEND.BadParameterException("INVALID FREQUENCY");
                        }
                        try {
                             _dev_set_center_frequency(freq,tuner_id);
                        }
                        catch(Exception e){
                            throw new FRONTEND.FrontendException("WARNING: failed when configuring device hardware");
                        };
                        try {
                            tunerChannels.get(tuner_id).frontend_status.center_frequency.setValue(_dev_get_center_frequency(tuner_id));
                        }
                        catch(Exception e){
                            throw new FRONTEND.FrontendException("WARNING: failed when querying device hardware");
                        };
                    }
                }
                if (isTunerEnabled){
                    enableTuner(tuner_id, true);
                }
        } catch (Exception e) {
            throw new FRONTEND.FrontendException("WARNING: " + e.getMessage());
        }
    }

    public double fe_getTunerBandwidth(String id) throws FRONTEND.FrontendException{
        int tuner_id = getTunerMapping(id);
        if (tuner_id < 0){
                throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        return tunerChannels.get(tuner_id).frontend_status.bandwidth.getValue();
    }

    public void fe_setTunerBandwidth(String id,double bw) throws FRONTEND.FrontendException, FRONTEND.BadParameterException{
        int tuner_id = getTunerMapping(id);
        if (tuner_id < 0){
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        if(id != tunerChannels.get(tuner_id).control_allocation_id)
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " DOES NOT HAVE AUTHORIZATION TO MODIFY TUNER!");

        try {
                synchronized(tunerChannels.get(tuner_id).lock){
                    if (!_valid_bandwidth(bw,tuner_id)){
                        throw new FRONTEND.BadParameterException("INVALID BANDWIDTH");
                    }
                    try {
                        _dev_set_bandwidth(bw,tuner_id);
                    }catch(Exception e){
                        throw new FRONTEND.FrontendException("WARNING: failed when configuring device hardware");
                    };
                    try {
                            tunerChannels.get(tuner_id).frontend_status.bandwidth.setValue(_dev_get_bandwidth(tuner_id));
                    }catch(Exception e){
                        throw new FRONTEND.FrontendException("WARNING: failed when querying device hardware");
                    };
                }
        } catch (Exception e) {
            throw new FRONTEND.FrontendException("WARNING: " + e.getMessage());
        }
    }

    public boolean fe_getTunerAgcEnable(String id) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("getTunerAgcEnable(String id) IS NOT CURRENTLY SUPPORTED");
    }

    public void fe_setTunerAgcEnable(String id,boolean enable) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("setTunerAgcEnable(String id, boolean enable) IS NOT CURRENTLY SUPPORTED");
    }

    public float fe_getTunerGain(String id) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("getTunerGain(String id) IS NOT CURRENTLY SUPPORTED");
    }

    public void fe_setTunerGain(String id,float gain) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("setTunerGain(String id, float gain) IS NOT CURRENTLY SUPPORTED");
    }

    public int fe_getTunerReferenceSource(String id) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("getTunerReferenceSource(String id) IS NOT CURRENTLY SUPPORTED");
    }

    public void fe_setTunerReferenceSource(String id,int source) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("setTunerReferenceSource(String id, int source) IS NOT CURRENTLY SUPPORTED");
    }

    public boolean fe_getTunerEnable(String id) throws FRONTEND.FrontendException{
        int tuner_id = getTunerMapping(id);
        if (tuner_id < 0){
                throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        return tunerChannels.get(tuner_id).frontend_status.enabled.getValue();
    }

    public void fe_setTunerEnable(String id,boolean enable) throws FRONTEND.FrontendException{
        int tuner_id = getTunerMapping(id);
        if (tuner_id < 0){
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        if(id != tunerChannels.get(tuner_id).control_allocation_id){
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " DOES NOT HAVE AUTHORIZATION TO MODIFY TUNER!");
        }
        try {
            enableTuner(tuner_id, enable);
        } catch (Exception e) {
            throw new FRONTEND.FrontendException("WARNING: Exception Caught during enableTuner: " + e.getMessage());
        }
    }

/*{%             set foundInAnalogInterface = True %}*/
/*{%         endif %}*/
/*{% endif %}*/
/*{%     if port.javatype == "frontend.InDigitalTunerPort" %}*/
    /*#- add FEI DigitalTuner callback functions #*/
/*{%         if foundInDigitalInterface == False %}*/
    public double fe_getTunerOutputSampleRate(String id) throws FRONTEND.FrontendException{
        int tuner_id = getTunerMapping(id);
        if (tuner_id < 0){
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        return tunerChannels.get(tuner_id).frontend_status.sample_rate.getValue();
    }

    public void fe_setTunerOutputSampleRate(String id,double sr) throws FRONTEND.FrontendException, FRONTEND.BadParameterException{
        int tuner_id = getTunerMapping(id);
        if (tuner_id < 0){
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        if(id != tunerChannels.get(tuner_id).control_allocation_id){
            throw new FRONTEND.FrontendException("ERROR: ID: " + id + " DOES NOT HAVE AUTHORIZATION TO MODIFY TUNER!");
        }

        try {
            synchronized(tunerChannels.get(tuner_id).lock){
                if (!_valid_sample_rate(sr,tuner_id)){
                    throw new FRONTEND.BadParameterException("INVALID SAMPLE RATE");
                }
                try {
                    _dev_set_sample_rate(sr,tuner_id);
                } catch(Exception e){
                    throw new FRONTEND.FrontendException("WARNING: failed when configuring device hardware");
                };
                try {
                    tunerChannels.get(tuner_id).frontend_status.sample_rate.setValue(_dev_get_sample_rate(tuner_id));
                } catch(Exception e){
                    throw new FRONTEND.FrontendException("WARNING: failed when querying device hardware");
                };
            }    
        } catch (Exception e) {
                throw new FRONTEND.FrontendException("WARNING: " + e.getMessage());
        }
}
/*{%             set foundInDigitalInterface = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InGPSPort" %}*/
/*{%         if foundInGPSPort == False %}*/
    public FRONTEND.GPSInfo fe_getGPSInfo() throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("gps_info() IS NOT CURRENTLY SUPPORTED");
    }
    public void fe_setGPSInfo(FRONTEND.GPSInfo data) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("gps_info(FRONTEND.GPSInfo data) IS NOT CURRENTLY SUPPORTED");
    }
    public FRONTEND.GpsTimePos fe_getGpsTimePos() throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("gps_time_pos() IS NOT CURRENTLY SUPPORTED");
    }
    public void fe_setGpsTimePos(FRONTEND.GpsTimePos data) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("gps_time_pos(FRONTEND.GpsTimePos data) IS NOT CURRENTLY SUPPORTED");
    }
/*{%             set foundInGPSPort = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InNavDataPort" %}*/
/*{%         if foundInNavDataPort == False %}*/
    public FRONTEND.NavigationPacket fe_getNavPkt() throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("nav_packet() IS NOT CURRENTLY SUPPORTED");
    }
    public void fe_setNavPkt(FRONTEND.NavigationPacket data) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("nav_packet(FRONTEND.NavigationPacket data) IS NOT CURRENTLY SUPPORTED");
    }
/*{%             set foundInNavDataPort = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InRFInfoPort" %}*/
/*{%         if foundInRFInfoPort == False %}*/
    public String fe_getRFFlowId() throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("rf_flow_id() IS NOT CURRENTLY SUPPORTED");
    }
    public void fe_setRFFlowId(String data) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("rf_flow_id(String data) IS NOT CURRENTLY SUPPORTED");
    }
    public FRONTEND.RFInfoPkt fe_getRFInfoPkt() throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("rfinfo_pkt() IS NOT CURRENTLY SUPPORTED");
    }
    public void fe_setRFInfoPkt(FRONTEND.RFInfoPkt data) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("rfinfo_pkt(FRONTEND.RFInfoPkt data) IS NOT CURRENTLY SUPPORTED");
    }
/*{%             set foundInRFInfoPort = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InRFSourcePort" %}*/
/*{%         if foundInRFSourcePort == False %}*/
    public FRONTEND.RFInfoPkt[] fe_getAvailableRFInputs() throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("available_rf_inputs() IS NOT CURRENTLY SUPPORTED");
    }
    public void fe_setAvailableRFInputs(FRONTEND.RFInfoPkt[] data) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("available_rf_inputs(FRONTEND.RFInfoPkt[] data) IS NOT CURRENTLY SUPPORTED");
    }
    public FRONTEND.RFInfoPkt fe_getCurrentRFInput() throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("current_rf_input() IS NOT CURRENTLY SUPPORTED");
    }
    public void fe_setCurrentRFInput(FRONTEND.RFInfoPkt data) throws FRONTEND.NotSupportedException{
        throw new FRONTEND.NotSupportedException("current_rf_input(FRONTEND.RFInfoPkt data) IS NOT CURRENTLY SUPPORTED");
    }
/*{%             set foundInRFSourcePort = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{% endfor %}*/
/*{% block extensions %}*/
/*# Allow for child class extensions #*/
/*{% endblock %}*/

}
