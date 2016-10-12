#{#
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
#}
#% set className = component.baseclass.name
#% set superclass = component.superclasses[0].name
#% set artifactType = component.artifacttype
#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: ${component.profile.spd}
from ossie.cf import CF, CF__POA
from ossie.utils import uuid

#{% if not component.isafrontendtuner %}
    #{% for parent in component.superclasses %}
from ${parent.package} import ${parent.name}
    #{% endfor %}
#{% endif %}
#{% if component.properties|test('simple') is sometimes(true) or component.structdefs %}
from ossie.properties import simple_property
#{% endif %}
#{% if component.properties|test('simplesequence') is sometimes(true) %}
from ossie.properties import simpleseq_property
#{% endif %}
#{% if component.structdefs %}
from ossie.properties import struct_property
#{% endif %}
#{% if component.properties|test('structsequence') is sometimes(true) %}
from ossie.properties import structseq_property
#{% endif %}

import Queue, copy, time, threading
#{% filter lines|unique|join('\n') %}
#{% for portgen in component.portgenerators %}
#{%   if loop.first %}
from ossie.resource import usesport, providesport
#{%     if component.hasbulkio %}
import bulkio
#{%     endif %}
#{%     if component.hasfrontend %}
import frontend 
#{%     endif %}
#{%   endif %}
#{%   for statement in portgen.imports() %}
${statement}
#{%   endfor %}
#{% endfor %}
#{% endfilter %}

#{%     if component.hasfrontend %}
BOOLEAN_VALUE_HERE=False
DOUBLE_VALUE_HERE=0.0
#{%     endif %}

NOOP = -1
NORMAL = 0
FINISH = 1
class ProcessThread(threading.Thread):
    def __init__(self, target, pause=0.0125):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.target = target
        self.pause = pause
        self.stop_signal = threading.Event()

    def stop(self):
        self.stop_signal.set()

    def updatePause(self, pause):
        self.pause = pause

    def run(self):
        state = NORMAL
        while (state != FINISH) and (not self.stop_signal.isSet()):
            state = self.target()
            delay = 1e-6
            if (state == NOOP):
                # If there was no data to process sleep to avoid spinning
                delay = self.pause
            time.sleep(delay)

#{% if component.isafrontendtuner %}
class ${className}(${component.superclasses|join(', ', attribute='name')}):
#{% else %}
class ${className}(${component.poaclass}, ${component.superclasses|join(', ', attribute='name')}):
#{% endif %}
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

#{% if component is device %}
        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            ${superclass}.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
#{%   if component is aggregatedevice %}
            AggregateDevice.__init__(self)
#{%   endif %}
#{% else %}
        def __init__(self, identifier, execparams):
            loggerName = (execparams['NAME_BINDING'].replace('/', '.')).rsplit("_", 1)[0]
            Resource.__init__(self, identifier, execparams, loggerName=loggerName)
#{% endif %}
            self.threadControlLock = threading.RLock()
            self.process_thread = None
            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 ${artifactType}s.  This variable may be removed
            # in future releases
            self.auto_start = False

        def initialize(self):
            ${superclass}.initialize(self)
            
            # Instantiate the default implementations for all ports on this ${artifactType}
#{% for port in component.ports %}
            self.${port.pyname} = ${port.constructor}
#{% endfor %}

#{% for port in component.ports if port is provides %}
#{%     if port.constructor.find("frontend.InDigitalTunerPort") != -1 or
            port.constructor.find("frontend.InAnalogTunerPort") != -1 or
            port.constructor.find("frontend.InFrontendTunerPort") != -1 %}
            #{# add FEI FrontendTuner callback functions -#}
            # Set callbacks for ${port.pyname}
            ${port.pyname}.setTunerTypeGetterCB(${className}.fe_getTunerType);
            ${port.pyname}.setTunerDeviceControlGetterCB(${className}.fe_getTunerDeviceControl);
            ${port.pyname}.setTunerGroupIdGetterCB(${className}.fe_getTunerGroupId);
            ${port.pyname}.setTunerRfFlowIdGetterCB(${className}.fe_getTunerRfFlowId);
            ${port.pyname}.setTunerStatusGetterCB(${className}.fe_getTunerStatus);
#{%-     endif %}
#{%     if port.constructor.find("frontend.InDigitalTunerPort") != -1 or
            port.constructor.find("frontend.InAnalogTunerPort") != -1 %}
            #{# add FEI AnalogTuner callback functions #}

            ${port.pyname}.setTunerCenterFrequencyGetterCB(${className}.fe_getTunerCenterFrequency);
            ${port.pyname}.setTunerCenterFrequencySetterCB(${className}.fe_setTunerCenterFrequency);
            ${port.pyname}.setTunerBandwidthGetterCB(${className}.fe_getTunerBandwidth);
            ${port.pyname}.setTunerBandwidthSetterCB(${className}.fe_setTunerBandwidth);
            ${port.pyname}.setTunerAgcEnableGetterCB(${className}.fe_getTunerAgcEnable);
            ${port.pyname}.setTunerAgcEnableSetterCB(${className}.fe_setTunerAgcEnable);
            ${port.pyname}.setTunerGainGetterCB(${className}.fe_getTunerGain);
            ${port.pyname}.setTunerGainSetterCB(${className}.fe_setTunerGain);
            ${port.pyname}.setTunerReferenceSourceGetterCB(${className}.fe_getTunerReferenceSource);
            ${port.pyname}.setTunerReferenceSourceSetterCB(${className}.fe_setTunerReferenceSource);
            ${port.pyname}.setTunerEnableGetterCB(${className}.fe_getTunerEnable);
            ${port.pyname}.setTunerEnableSetterCB(${className}.fe_setTunerEnable);
#{%     endif %}
#{%     if port.constructor.find("frontend.InDigitalTunerPort") != -1 %}
            #{# add FEI DigitalTuner callback functions -#} 
            ${port.pyname}.setTunerOutputSampleRateGetterCB(${className}.fe_getTunerOutputSampleRate);
            ${port.pyname}.setTunerOutputSampleRateSetterCB(${className}.fe_setTunerOutputSampleRate);
#{%     endif %}
#{%     if port.constructor.find("frontend.InGPSPort") != -1 %}
            # Set callbacks for ${port.pyname}
            ${port.pyname}.setGPSInfoGetterCB(${className}.fe_getGPSInfo);
            ${port.pyname}.setGPSInfoSetterCB(${className}.fe_setGPSInfo);
            ${port.pyname}.setGpsTimePosGetterCB(${className}.fe_getGpsTimePos);
            ${port.pyname}.setGpsTimePosSetterCB(${className}.fe_setGpsTimePos);
#{%     endif %}
#{%     if port.constructor.find("frontend.InNavDataPort") != -1 %}
            # Set callbacks for ${port.pyname}
            ${port.pyname}.setNavPktGetterCB(${className}.fe_getNavPkt);
            ${port.pyname}.setNavPktSetterCB(${className}.fe_setNavPkt);
#{%     endif %}
#{%     if port.constructor.find("frontend.InRFInfoPort") != -1%}
            # Set callbacks for ${port.pyname}
            ${port.pyname}.setRFFlowIdGetterCB(${className}.fe_getRFFlowId);
            ${port.pyname}.setRFFlowIdSetterCB(${className}.fe_setRFFlowId);
            ${port.pyname}.setRFInfoPktGetterCB(${className}.fe_getRFInfoPkt);
            ${port.pyname}.setRFInfoPktSetterCB(${className}.fe_setRFInfoPkt);
#{%     endif %}
#{%     if port.constructor.find("frontend.InRFSourcePort") != -1 %}
            # Set callbacks for ${port.pyname}
            ${port.pyname}.setAvailableRFInputsGetterCB(${className}.fe_getAvailableRFInputs);
            ${port.pyname}.setAvailableRFInputsSetterCB(${className}.fe_setAvailableRFInputs);
            ${port.pyname}.setCurrentRFInputGetterCB(${className}.fe_getCurrentRFInput);
            ${port.pyname}.setCurrentRFInputSetterCB(${className}.fe_setCurrentRFInput);
#{%     endif %}
#{% endfor %}

        def start(self):
            self.threadControlLock.acquire()
            try:
                ${superclass}.start(self)
                if self.process_thread == None:
                    self.process_thread = ProcessThread(target=self.process, pause=self.PAUSE)
                    self.process_thread.start()
            finally:
                self.threadControlLock.release()

        def process(self):
            """The process method should process a single "chunk" of data and then return.  This method will be called
            from the processing thread again, and again, and again until it returns FINISH or stop() is called on the
            ${artifactType}.  If no work is performed, then return NOOP"""
            raise NotImplementedError

        def stop(self):
            self.threadControlLock.acquire()
            try:
                process_thread = self.process_thread
                self.process_thread = None

                if process_thread != None:
                    process_thread.stop()
                    process_thread.join(self.TIMEOUT)
                    if process_thread.isAlive():
                        raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")
                ${superclass}.stop(self)
            finally:
                self.threadControlLock.release()

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            self.threadControlLock.acquire()
            try:
                ${superclass}.releaseObject(self)
            finally:
                self.threadControlLock.release()

#{% set foundInFrontendInterface = False %}
#{% set foundInAnalogInterface = False %}
#{% set foundInDigitalInterface = False %}
#{% set foundInGPSPort = False %}
#{% set foundInNavDataPort = False %}
#{% set foundInRFInfoPort = False %}
#{% set foundInRFSourcePort = False %}
#{% for port in component.ports if port is provides %}
#{%     if port.cpptype == "frontend.InDigitalTunerPort" or
            port.cpptype == "frontend.InAnalogTunerPort" or
            port.cpptype == "frontend.InFrontendTunerPort" %}
    #{#- add FEI FrontendTuner callback functions #}
#{%         if foundInFrontendInterface == False %}
        def fe_getTunerType(self, id):
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0)
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"));
            return str(tunerChannels[tuner_id].frontend_status.tuner_type);
        }

        def fe_getTunerDeviceControl(self, id):
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0)
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"));
            if(str(id) == tunerChannels[tuner_id].control_allocation_id)
                return True;
            return False;
        }

        def fe_getTunerGroupId(self, id):
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0)
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"));
            return str(tunerChannels[tuner_id].frontend_status.group_id);
        }

        def fe_getTunerRfFlowId(self, id):
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0)
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
            return str(tunerChannels[tuner_id].frontend_status.rf_flow_id)
        }
        def fe_getTunerStatus(self, id):
            tmpVal= CF.Properties();
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0)
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
            CORBA.Any prop;
#            prop <<= *(static_cast<frontend_tuner_status_struct_struct*>(tunerChannels[tuner_id].frontend_status));
#            prop >>= tmpVal;

#            CF::Properties_var tmp = new CF::Properties(*tmpVal);
#            return tmp._retn();
             return tmpVal
        }
#{%             set foundInFrontendInterface = True %}
#{%         endif %}
#{%     endif %}
#{%     if port.cpptype == "frontend.InDigitalTunerPort" or
            port.cpptype == "frontend.InAnalogTunerPort" %}
    #{#- add FEI AnalogTuner callback functions #}
#{%         if foundInAnalogInterface == False %}
        def fe_getTunerCenterFrequency(self, id):
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0)
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
            return tunerChannels[tuner_id].frontend_status.center_frequency;
        }

        def fe_setTunerCenterFrequency(self, id, freq):
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0)
            raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
                if(str(id) != tunerChannels[tuner_id].control_allocation_id)
                    raise FRONTEND.FrontendException("ERROR:: ID: " + str(id) + " DOES NOT HAVE AUTHORIZATION TO MODIFY TUNER!")

                try:
                    # If the freq has changed (change in stream) or the tuner is disabled, then set it as disabled
                    isTunerEnabled = tunerChannels[tuner_id].frontend_status.enabled;
                    if (!isTunerEnabled || tunerChannels[tuner_id].frontend_status.center_frequency != freq)
                        enableTuner(tuner_id, false); #TODO: is this a generic thing we can assume is desired when changing any tuner device?
                    {
                        tunerChannels[tuner_id].lock.acquire()
                        try:
                            if (!_valid_center_frequency(freq,tuner_id)){
                                raise FRONTEND.BadParameterException("INVALID FREQUENCY");
                            }
                            try:
                                _dev_set_center_frequency(freq,tuner_id);
                            except:
                                raise FRONTEND.FrontendException("WARNING: failed when configuring device hardware");
                            try:
                                tunerChannels[tuner_id].frontend_status.center_frequency = _dev_get_center_frequency(tuner_id);
                            except:
                                raise FRONTEND.FrontendException("WARNING: failed when querying device hardware");
                        finally:
                            tunerChannels[tuner_id].lock.release()
                    }
                    if (isTunerEnabled)
                        enableTuner(tuner_id, true);
                } except Exception e) {
                    raise FRONTEND.FrontendException(("WARNING: " + str(e.what())))
                }
        }

        def fe_getTunerBandwidth(self, id){
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0)
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
            return tunerChannels[tuner_id].frontend_status.bandwidth;
        }

        def fe_setTunerBandwidth(self, id, bw){
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0)
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
            if(str(id) != tunerChannels[tuner_id].control_allocation_id)
                raise FRONTEND.FrontendException(("ERROR:: ID: " + str(id) + " DOES NOT HAVE AUTHORIZATION TO MODIFY TUNER!"))

            try:
                tunerChannels[tuner_id].lock.acquire()
                try:
                    if (!_valid_bandwidth(bw,tuner_id)):
                        raise FRONTEND.BadParameterException("INVALID BANDWIDTH");
                    try:
                        _dev_set_bandwidth(bw,tuner_id);
                    except:
                        raise FRONTEND.FrontendException("WARNING: failed when configuring device hardware");
                    try:
                        tunerChannels[tuner_id].frontend_status.bandwidth = _dev_get_bandwidth(tuner_id);
                    except:
                       raise FRONTEND.FrontendException("WARNING: failed when querying device hardware");
                finally:
                    tunerChannels[tuner_id].lock.release()
            except Exception, e:
                raise FRONTEND.FrontendException("WARNING: " + str(e))
        }

        def fe_getTunerAgcEnable(self, id){
            raise FRONTEND.NotSupportedException("getTunerAgcEnable(self,id) IS NOT CURRENTLY SUPPORTED");
        }

        def fe_setTunerAgcEnable(self, id, enable){
            raise FRONTEND.NotSupportedException("setTunerAgcEnable(self, id, enable) IS NOT CURRENTLY SUPPORTED");
        }

        def fe_getTunerGain(self, id){
            raise FRONTEND.NotSupportedException("getTunerGain(self, id) IS NOT CURRENTLY SUPPORTED");
        }

        def fe_setTunerGain(self, id, gain){
            raise FRONTEND.NotSupportedException("setTunerGain(self, id, gain) IS NOT CURRENTLY SUPPORTED");
        }

        def fe_getTunerReferenceSource(self, id){
            raise FRONTEND.NotSupportedException("getTunerReferenceSource(self, id) IS NOT CURRENTLY SUPPORTED");
        }
        def fe_getTunerAgcEnable(self, id){
            raise FRONTEND.NotSupportedException("getTunerAgcEnable(self, id) IS NOT CURRENTLY SUPPORTED");
        }

        def fe_setTunerAgcEnable(self, id, enable){
            raise FRONTEND.NotSupportedException("setTunerAgcEnable(self, id, enable) IS NOT CURRENTLY SUPPORTED");
        }

        def fe_getTunerGain(self, id){
            raise FRONTEND.NotSupportedException("getTunerGain(self, id) IS NOT CURRENTLY SUPPORTED");
        }

        def fe_setTunerGain(self, id, gain){
            raise FRONTEND.NotSupportedException("setTunerGain(self, id, gain) IS NOT CURRENTLY SUPPORTED");
        }

        def fe_getTunerReferenceSource(self, id){
            raise FRONTEND.NotSupportedException("getTunerReferenceSource(self, id) IS NOT CURRENTLY SUPPORTED");
        }

        def fe_setTunerReferenceSource(self, id, source){
            raise FRONTEND.NotSupportedException("setTunerReferenceSource(self, id, source) IS NOT CURRENTLY SUPPORTED");
        }

        def fe_getTunerEnable(self, id){
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0):
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
            return tunerChannels[tuner_id].frontend_status.enabled;
        }

        def fe_setTunerEnable(self, id, enable){
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0):
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
            if(str(id) != tunerChannels[tuner_id].control_allocation_id)
                raise FRONTEND.FrontendException(("ERROR:: ID: " + str(id) + " DOES NOT HAVE AUTHORIZATION TO MODIFY TUNER!"))
            try:
                enableTuner(tuner_id, enable);
            except Exception, e:
                raise FRONTEND.FrontendException("WARNING: Exception Caught during enableTuner: " + str(e))
        }
#{%             set foundInAnalogInterface = True %}
#{%         endif %}
#{% endif %}
#{%     if port.cpptype == "frontend.InDigitalTunerPort" %}
    #{#- add FEI DigitalTuner callback functions #}
#{%         if foundInDigitalInterface == False %}
        def fe_getTunerOutputSampleRate(self, id){
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0):
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
            return tunerChannels[tuner_id].frontend_status.sample_rate;
        }
        def fe_setTunerOutputSampleRate(self, id, sr){
            tuner_id = getTunerMapping(id);
            if (tuner_id < 0):
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
            if(str(id) != tunerChannels[tuner_id].control_allocation_id)
                raise FRONTEND.FrontendException(("ERROR:: ID: " + str(id) + " DOES NOT HAVE AUTHORIZATION TO MODIFY TUNER!"))

            try:
                tunerChannels[tuner_id].lock.acquire();
                try:
                    if (!_valid_sample_rate(sr,tuner_id)):
                        raise FRONTEND.BadParameterException("INVALID SAMPLE RATE");

                    try:
                        _dev_set_sample_rate(sr,tuner_id);
                    except: 
                        raise FRONTEND.FrontendException("WARNING: failed when configuring device hardware");
                    try:
                        tunerChannels[tuner_id].frontend_status.sample_rate = _dev_get_sample_rate(tuner_id);
                    except: 
                        raise FRONTEND.FrontendException("WARNING: failed when querying device hardware");
                finally:
                    tunerChannels[tuner_id].lock.release();
            except Exception, e: 
                raise FRONTEND.FrontendException("WARNING: " + str(e))
        }
#{%             set foundInDigitalInterface = True %}
#{%         endif %}
#{%     endif %}
#{%     if port.cpptype == "frontend.InGPSPort" %}
#{%         if foundInGPSPort == False %}
        def fe_getGPSInfo(self){
            raise FRONTEND.NotSupportedException("gps_info(self) IS NOT CURRENTLY SUPPORTED");
        }
        def fe_setGPSInfo(self,data){
            raise FRONTEND.NotSupportedException("gps_info(self,data) IS NOT CURRENTLY SUPPORTED");
        }
        def fe_getGpsTimePos(self){
            raise FRONTEND.NotSupportedException("gps_time_pos(self) IS NOT CURRENTLY SUPPORTED");
        }
        def fe_setGpsTimePos(self,data){
            raise FRONTEND.NotSupportedException("gps_time_pos(self,data) IS NOT CURRENTLY SUPPORTED");
        }
#{%             set foundInGPSPort = True %}
#{%         endif %}
#{%     endif %}
#{%     if port.cpptype == "frontend.InNavDataPort" %}
#{%         if foundInNavDataPort == False %}
        def fe_getNavPkt(self){
            raise FRONTEND.NotSupportedException("nav_packet(self) IS NOT CURRENTLY SUPPORTED");
        }
        def fe_setNavPkt(self,data){
            raise FRONTEND.NotSupportedException("nav_packet(self,data) IS NOT CURRENTLY SUPPORTED");
        }
#{%             set foundInNavDataPort = True %}
#{%         endif %}
#{%     endif %}
#{%     if port.cpptype == "frontend.InRFInfoPort" %}
#{%         if foundInRFInfoPort == False %}
        def fe_getRFFlowId(self){
            raise FRONTEND.NotSupportedException("rf_flow_id(self) IS NOT CURRENTLY SUPPORTED");
        }
        def fe_setRFFlowId(self,data){
            raise FRONTEND.NotSupportedException("rf_flow_id(self,data) IS NOT CURRENTLY SUPPORTED");
        }
        def fe_getRFInfoPkt(self){
            raise FRONTEND.NotSupportedException("rfinfo_pkt(self) IS NOT CURRENTLY SUPPORTED");
        }
        def fe_setRFInfoPkt(self,data){
            raise FRONTEND.NotSupportedException("rfinfo_pkt(self,data) IS NOT CURRENTLY SUPPORTED");
        }
#{%             set foundInRFInfoPort = True %}
#{%         endif %}
#{%     endif %}
#{%     if port.cpptype == "frontend.InRFSourcePort" %}
#{%         if foundInRFSourcePort == False %}
        def fe_getAvailableRFInputs(self){
            raise FRONTEND.NotSupportedException("available_rf_inputs(self) IS NOT CURRENTLY SUPPORTED");
        }
        def fe_setAvailableRFInputs(self,data){
            raise FRONTEND.NotSupportedException("available_rf_inputs(self,data) IS NOT CURRENTLY SUPPORTED");
        }
        def fe_getCurrentRFInput(self){
            raise FRONTEND.NotSupportedException("current_rf_input(self) IS NOT CURRENTLY SUPPORTED");
        }
        def fe_setCurrentRFInput(self, data){
            raise FRONTEND.NotSupportedException("current_rf_input(self,data) IS NOT CURRENTLY SUPPORTED");
        }
#{%             set foundInRFSourcePort = True %}
#{%         endif %}
#{%     endif %}
#{% endfor %}

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

#{% for portgen in component.portgenerators if portgen.hasImplementation() %}
        # '${portgen.namespace}/${portgen.interface}' port
        class ${portgen.templateClass()}(${portgen.poaClass()}):
            """This class is a port template for the ${portgen.className()} port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend
            from this class instead of the base CORBA class ${portgen.poaClass()}.
            """
            pass

#{% endfor %}
#{% for port in component.ports %}
#{%   filter codealign %}
        ${port.pyname} = ${port.generator.direction}port(name="${port.name}",
                                                         repid="${port.repid}",
                                                         type_="${port.types[0]}")
#{%   endfilter %}

#{% endfor %}
        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
#{% import "base/properties.py" as properties with context %}
#{% for prop in component.properties if prop is simple %}
        ${properties.simple(prop)|indent(8)}
#{% endfor %}
#{% for prop in component.properties if prop is simplesequence %}
        ${properties.simplesequence(prop)|indent(8)}
#{% endfor %}
#{% for prop in component.properties if prop is struct %}
        ${properties.structdef(prop)|indent(8)}

        ${properties.struct(prop)|indent(8)}
#{% endfor %}
#{% for prop in component.properties if prop is structsequence %}
        ${properties.structdef(prop.structdef,False)|indent(8)}

        ${properties.structsequence(prop)|indent(8)}
#{% endfor %}
#{% for portgen in component.portgenerators if portgen is provides and portgen.hasImplementation() %}

#{%   if loop.first %}
'''provides port(s)'''

#{%   endif %}
#{% include portgen.implementation() %}
#{% endfor %}
#{% for portgen in component.portgenerators if portgen is uses and portgen.hasImplementation() %}

#{%   if loop.first %}
'''uses port(s)'''

#{%   endif %}
#{% include portgen.implementation() %}
#{% endfor %}
