/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK frontendInterfaces.
 *
 * REDHAWK frontendInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK frontendInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef FE_TUNER_PORT_H
#define FE_TUNER_PORT_H

#include "fe_port_impl.h"

#include <redhawk/FRONTEND/TunerControl.h>


namespace frontend {
    
    class tuner_delegation {
        public:
            virtual std::string getTunerType(const std::string& id) {
                throw FRONTEND::NotSupportedException("getTunerType not supported");
            }
            virtual bool getTunerDeviceControl(const std::string& id) {
                throw FRONTEND::NotSupportedException("getTunerDeviceControl not supported");
            }
            virtual std::string getTunerGroupId(const std::string& id) {
                throw FRONTEND::NotSupportedException("getTunerGroupId not supported");
            }
            virtual std::string getTunerRfFlowId(const std::string& id) {
                throw FRONTEND::NotSupportedException("getTunerRfFlowId not supported");
            }
            virtual CF::Properties* getTunerStatus(const std::string& id) = 0;
    };

    class analog_tuner_delegation : public virtual tuner_delegation {
        public:
            virtual void setTunerCenterFrequency(const std::string& id, double freq) {
                throw FRONTEND::NotSupportedException("setTunerCenterFrequency not supported");
            }
            virtual double getTunerCenterFrequency(const std::string& id) {
                throw FRONTEND::NotSupportedException("getTunerCenterFrequency not supported");
            }
            virtual void setTunerBandwidth(const std::string& id, double bw) {
                throw FRONTEND::NotSupportedException("setTunerBandwidth not supported");
            }
            virtual double getTunerBandwidth(const std::string& id) {
                throw FRONTEND::NotSupportedException("getTunerBandwidth not supported");
            }
            virtual void setTunerAgcEnable(const std::string& id, bool enable) {
                throw FRONTEND::NotSupportedException("setTunerAgcEnable not supported");
            }
            virtual bool getTunerAgcEnable(const std::string& id) {
                throw FRONTEND::NotSupportedException("getTunerAgcEnable not supported");
            }
            virtual void setTunerGain(const std::string& id, float gain) {
                throw FRONTEND::NotSupportedException("setTunerGain not supported");
            }
            virtual float getTunerGain(const std::string& id) {
                throw FRONTEND::NotSupportedException("getTunerGain not supported");
            }
            virtual void setTunerReferenceSource(const std::string& id, long source) {
                throw FRONTEND::NotSupportedException("setTunerReferenceSource not supported");
            }
            virtual long getTunerReferenceSource(const std::string& id) {
                throw FRONTEND::NotSupportedException("getTunerReferenceSource not supported");
            }
            virtual void setTunerEnable(const std::string& id, bool enable) {
                throw FRONTEND::NotSupportedException("setTunerEnable not supported");
            }
            virtual bool getTunerEnable(const std::string& id) {
                throw FRONTEND::NotSupportedException("getTunerEnable not supported");
            }
    };

    class digital_tuner_delegation : public virtual analog_tuner_delegation {
        public:
            virtual void setTunerOutputSampleRate(const std::string& id, double sr) {
                throw FRONTEND::NotSupportedException("setTunerOutputSampleRate not supported");
            }
            virtual double getTunerOutputSampleRate(const std::string& id) {
                throw FRONTEND::NotSupportedException("getTunerOutputSampleRate not supported");
            }
    };
    
    class analog_scanning_tuner_delegation : public virtual analog_tuner_delegation {
        public:
            virtual frontend::ScanStatus getScanStatus(const std::string& id) {
                throw FRONTEND::NotSupportedException("getScanStatus not supported");
            }
            virtual void setScanStartTime(const std::string& id, const BULKIO::PrecisionUTCTime& start_time) {
                throw FRONTEND::NotSupportedException("setScanStartTime not supported");
            }
            virtual void setScanStrategy(const std::string& id, const frontend::ScanStrategy* scan_strategy) {
                throw FRONTEND::NotSupportedException("setScanStrategy not supported");
            }
    };

    class digital_scanning_tuner_delegation : public virtual digital_tuner_delegation {
        public:
            virtual frontend::ScanStatus getScanStatus(const std::string& id) {
                throw FRONTEND::NotSupportedException("getScanStatus not supported");
            }
            virtual void setScanStartTime(const std::string& id, const BULKIO::PrecisionUTCTime& start_time) {
                throw FRONTEND::NotSupportedException("setScanStartTime not supported");
            }
            virtual void setScanStrategy(const std::string& id, const frontend::ScanStrategy* scan_strategy) {
                throw FRONTEND::NotSupportedException("setScanStrategy not supported");
            }
    };

    class InFrontendTunerPort : public virtual POA_FRONTEND::FrontendTuner, public Port_Provides_base_impl
    {
        public:
            InFrontendTunerPort(std::string port_name, tuner_delegation *_parent): 
            Port_Provides_base_impl(port_name)
            {
                parent = _parent;
            };
            ~InFrontendTunerPort() {};
            char* getTunerType(const char* id) {
                boost::mutex::scoped_lock lock(portAccess);
                std::string _id(id);
                return (CORBA::string_dup(this->parent->getTunerType(_id).c_str()));
            };
            CORBA::Boolean getTunerDeviceControl(const char* id) {
                boost::mutex::scoped_lock lock(portAccess);
                std::string _id(id);
                return (this->parent->getTunerDeviceControl(_id));
            };
            char* getTunerGroupId(const char* id) {
                boost::mutex::scoped_lock lock(portAccess);
                std::string _id(id);
                return (CORBA::string_dup(this->parent->getTunerGroupId(_id).c_str()));
            };
            char* getTunerRfFlowId(const char* id) {
                boost::mutex::scoped_lock lock(portAccess);
                std::string _id(id);
                return (CORBA::string_dup(this->parent->getTunerRfFlowId(_id).c_str()));
            };
            CF::Properties* getTunerStatus(const char* id) {
                boost::mutex::scoped_lock lock(portAccess);
                std::string _id(id);
                return (this->parent->getTunerStatus(_id));
            };
            std::string getRepid() const {
                return "IDL:FRONTEND/FrontendTuner:1.0";
            };
        protected:
            boost::mutex portAccess;
        private:
            tuner_delegation *parent;
    };
    
    class InAnalogTunerPort : public virtual POA_FRONTEND::AnalogTuner, public InFrontendTunerPort
    {
        public:
            typedef InFrontendTunerPort super;
            InAnalogTunerPort(std::string port_name, analog_tuner_delegation *_parent):super(port_name, _parent)
            {
                parent = _parent;
            };
            ~InAnalogTunerPort() {};
            void setTunerCenterFrequency(const char* id, CORBA::Double freq) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                this->parent->setTunerCenterFrequency(_id, freq);
            };
            CORBA::Double getTunerCenterFrequency(const char* id) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                return (this->parent->getTunerCenterFrequency(_id));
            };
            void setTunerBandwidth(const char* id, CORBA::Double bw) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                this->parent->setTunerBandwidth(_id, bw);
            };
            CORBA::Double getTunerBandwidth(const char* id) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                return (this->parent->getTunerBandwidth(_id));
            };
            void setTunerAgcEnable(const char* id, CORBA::Boolean enable) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                this->parent->setTunerAgcEnable(_id, enable);
            };
            CORBA::Boolean getTunerAgcEnable(const char* id) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                return (this->parent->getTunerAgcEnable(_id));
            };
            void setTunerGain(const char* id, CORBA::Float gain) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                this->parent->setTunerGain(_id, gain);
            };
            CORBA::Float getTunerGain(const char* id) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                return (this->parent->getTunerGain(_id));
            };
            void setTunerReferenceSource(const char* id, CORBA::Long source) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                this->parent->setTunerReferenceSource(_id, source);
            };
            CORBA::Long getTunerReferenceSource(const char* id) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                return (this->parent->getTunerReferenceSource(_id));
            };
            void setTunerEnable(const char* id, CORBA::Boolean enable) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                this->parent->setTunerEnable(_id, enable);
            };
            CORBA::Boolean getTunerEnable(const char* id) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                return (this->parent->getTunerEnable(_id));
            };
            std::string getRepid() const {
                return "IDL:FRONTEND/AnalogTuner:1.0";
            };
        private:
            analog_tuner_delegation *parent;
    };
    
    class InDigitalTunerPort : public virtual POA_FRONTEND::DigitalTuner, public InAnalogTunerPort
    {
        public:
            typedef InAnalogTunerPort super;
            InDigitalTunerPort(std::string port_name, digital_tuner_delegation *_parent):super(port_name, _parent)
            {
                parent = _parent;
            };
            ~InDigitalTunerPort() {};
            void setTunerOutputSampleRate(const char* id, CORBA::Double sr) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                this->parent->setTunerOutputSampleRate(_id, sr);
            };
            CORBA::Double getTunerOutputSampleRate(const char* id) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                return (this->parent->getTunerOutputSampleRate(_id));
            };
            std::string getRepid() const {
                return "IDL:FRONTEND/DigitalTuner:1.0";
            };
        private:
            digital_tuner_delegation *parent;
    };

    class InAnalogScanningTunerPort : public virtual POA_FRONTEND::AnalogScanningTuner, public InAnalogTunerPort
    {
        public:
            typedef InAnalogTunerPort super;
            InAnalogScanningTunerPort(std::string port_name, analog_scanning_tuner_delegation *_parent):super(port_name, _parent)
            {
                parent = _parent;
            };
            ~InAnalogScanningTunerPort() {};
            FRONTEND::ScanningTuner::ScanStatus* getScanStatus(const char* id) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                return (returnScanStatus(this->parent->getScanStatus(_id)));
            };
            void setScanStartTime(const char* id, const BULKIO::PrecisionUTCTime& start_time) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                this->parent->setScanStartTime(_id, start_time);
            };
            void setScanStrategy(const char* id, const FRONTEND::ScanningTuner::ScanStrategy& scan_strategy) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                std::unique_ptr<frontend::ScanStrategy> _strat(returnScanStrategy(scan_strategy));
                this->parent->setScanStrategy(_id, _strat.get());
            };
            std::string getRepid() const {
                return "IDL:FRONTEND/AnalogScanningTuner:1.0";
            };
        private:
            analog_scanning_tuner_delegation *parent;
    };

    class InDigitalScanningTunerPort : public virtual POA_FRONTEND::DigitalScanningTuner, public InDigitalTunerPort
    {
        public:
            typedef InDigitalTunerPort super;
            InDigitalScanningTunerPort(std::string port_name, digital_scanning_tuner_delegation *_parent):super(port_name, _parent)
            {
                parent = _parent;
            };
            ~InDigitalScanningTunerPort() {};
            FRONTEND::ScanningTuner::ScanStatus* getScanStatus(const char* id) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                return (returnScanStatus(this->parent->getScanStatus(_id)));
            };
            void setScanStartTime(const char* id, const BULKIO::PrecisionUTCTime& start_time) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                this->parent->setScanStartTime(_id, start_time);
            };
            void setScanStrategy(const char* id, const FRONTEND::ScanningTuner::ScanStrategy& scan_strategy) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::string _id(id);
                std::unique_ptr<frontend::ScanStrategy> _strat(returnScanStrategy(scan_strategy));
                this->parent->setScanStrategy(_id, _strat.get());
            };
            std::string getRepid() const {
                return "IDL:FRONTEND/DigitalScanningTuner:1.0";
            };
        private:
            digital_scanning_tuner_delegation *parent;
    };

    template<typename PortType_var, typename PortType>
    class OutFrontendTunerPortT : public OutFrontendPort<PortType_var, PortType>
    {
        public:
            OutFrontendTunerPortT(std::string port_name) : OutFrontendPort<PortType_var, PortType>(port_name)
            {};
            ~OutFrontendTunerPortT(){};

            std::vector<std::string> getConnectionIds() {
                std::vector<std::string> retval;
                for (unsigned int i = 0; i < this->outConnections.size(); i++) {
                    retval.push_back(this->outConnections[i].second);
                }
                return retval;
            }

            void __evaluateRequestBasedOnConnections(const std::string &__connection_id__, bool returnValue, bool inOut, bool out) {
                if (__connection_id__.empty() and (this->outConnections.size() > 1)) {
                    if (out or inOut or returnValue) {
                        throw redhawk::PortCallError("Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.",
                                getConnectionIds());
                    }
                }
                if (this->outConnections.empty()) {
                    if (out or inOut or returnValue) {
                        throw redhawk::PortCallError("No connections available.", std::vector<std::string>());
                    } else {
                        if (not __connection_id__.empty()) {
                            std::ostringstream eout;
                            eout<<"The requested connection id ("<<__connection_id__<<") does not exist.";
                            throw redhawk::PortCallError(eout.str(), getConnectionIds());
                        }
                    }
                }
                if ((not __connection_id__.empty()) and (not this->outConnections.empty())) {
                    bool foundConnection = false;
                    typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (i->second == __connection_id__) {
                            foundConnection = true;
                            break;
                        }
                    }
                    if (not foundConnection) {
                        std::ostringstream eout;
                        eout<<"The requested connection id ("<<__connection_id__<<") does not exist.";
                        throw redhawk::PortCallError(eout.str(), getConnectionIds());
                    }
                }
            }

            std::string getTunerType(const std::string &id, const std::string __connection_id__ = "") {
                CORBA::String_var retval = "";
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerType(id.c_str());
                    }
                }
                std::string str_retval = ossie::corba::returnString(retval);
                return str_retval;
            };
            bool getTunerDeviceControl(const std::string &id, const std::string __connection_id__ = "") {
                CORBA::Boolean retval = false;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerDeviceControl(id.c_str());
                    }
                }
                return retval;
            };
            std::string getTunerGroupId(const std::string &id, const std::string __connection_id__ = "") {
                CORBA::String_var retval = "";
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerGroupId(id.c_str());
                    }
                }
                std::string str_retval = ossie::corba::returnString(retval);
                return str_retval;
            };
            std::string getTunerRfFlowId(const std::string &id, const std::string __connection_id__ = "") {
                CORBA::String_var retval = "";
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerRfFlowId(id.c_str());
                    }
                }
                std::string str_retval = ossie::corba::returnString(retval);
                return str_retval;
            };
            CF::Properties* getTunerStatus(const std::string &id, const std::string __connection_id__ = "") {
                CF::Properties_var retval = new CF::Properties();
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerStatus(id.c_str());
                    }
                }
                return retval._retn();
            };
    };
 
    template<typename PortType_var, typename PortType>
    class OutAnalogTunerPortT : public OutFrontendTunerPortT<PortType_var, PortType>
    {
        public:
            OutAnalogTunerPortT(std::string port_name) : OutFrontendTunerPortT<PortType_var, PortType>(port_name)
            {};
            ~OutAnalogTunerPortT(){};

            void setTunerCenterFrequency(const std::string &id, double freq, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        ((*i).first)->setTunerCenterFrequency(id.c_str(), freq);
                    }
                }
                return;
            };
            double getTunerCenterFrequency(const std::string &id, const std::string __connection_id__ = "") {
                CORBA::Double retval = 0;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerCenterFrequency(id.c_str());
                    }
                }
                return retval;
            };
            void setTunerBandwidth(const std::string &id, double bw, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        ((*i).first)->setTunerBandwidth(id.c_str(), bw);
                    }
                }
                return;
            };
            double getTunerBandwidth(const std::string &id, const std::string __connection_id__ = "") {
                CORBA::Double retval = 0;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerBandwidth(id.c_str());
                    }
                }
                return retval;
            };
            void setTunerAgcEnable(const std::string &id, bool enable, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        ((*i).first)->setTunerAgcEnable(id.c_str(), enable);
                    }
                }
                return;
            };
            bool getTunerAgcEnable(const std::string &id, const std::string __connection_id__ = "") {
                CORBA::Boolean retval = false;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerAgcEnable(id.c_str());
                    }
                }
                return retval;
            };
            void setTunerGain(const std::string &id, float gain, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        ((*i).first)->setTunerGain(id.c_str(), gain);
                    }
                }
                return;
            };
            float getTunerGain(const std::string &id, const std::string __connection_id__ = "") {
                CORBA::Float retval = 0;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerGain(id.c_str());
                    }
                }
                return retval;
            };
            void setTunerReferenceSource(const std::string &id, int source, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        ((*i).first)->setTunerReferenceSource(id.c_str(), source);
                    }
                }
                return;
            };
            int getTunerReferenceSource(const std::string &id, const std::string __connection_id__ = "") {
                CORBA::Long retval = 0;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerReferenceSource(id.c_str());
                    }
                }
                return retval;
            };
            void setTunerEnable(const std::string &id, bool enable, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        ((*i).first)->setTunerEnable(id.c_str(), enable);
                    }
                }
                return;
            };
            bool getTunerEnable(const std::string &id, const std::string __connection_id__ = "") {
                CORBA::Boolean retval = false;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerEnable(id.c_str());
                    }
                }
                return retval;
            };
    };
    
    template<typename PortType_var, typename PortType>
    class OutDigitalTunerPortT : public OutAnalogTunerPortT<PortType_var, PortType>
    {
        public:
            OutDigitalTunerPortT(std::string port_name) : OutAnalogTunerPortT<PortType_var, PortType>(port_name)
            {};
            ~OutDigitalTunerPortT(){};

            void setTunerOutputSampleRate(const std::string &id, double sr, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        ((*i).first)->setTunerOutputSampleRate(id.c_str(), sr);
                    }
                }
                return;
            };
            double getTunerOutputSampleRate(const std::string &id, const std::string __connection_id__ = "") {
                CORBA::Double retval = 0;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getTunerOutputSampleRate(id.c_str());
                    }
                }
                return retval;
            };
    };
    
    // ----------------------------------------------------------------------------------------
    // OutFrontendTunerPort declaration
    // ----------------------------------------------------------------------------------------
    class OutFrontendTunerPort : public OutFrontendTunerPortT<FRONTEND::FrontendTuner_var,FRONTEND::FrontendTuner> {
        public:
            OutFrontendTunerPort(std::string port_name) : OutFrontendTunerPortT<FRONTEND::FrontendTuner_var,FRONTEND::FrontendTuner>(port_name)
            {};
    };

    // ----------------------------------------------------------------------------------------
    // OutAnalogTunerPort declaration
    // ----------------------------------------------------------------------------------------
    class OutAnalogTunerPort : public OutAnalogTunerPortT<FRONTEND::AnalogTuner_var,FRONTEND::AnalogTuner> {
        public:
            OutAnalogTunerPort(std::string port_name) : OutAnalogTunerPortT<FRONTEND::AnalogTuner_var,FRONTEND::AnalogTuner>(port_name)
            {};
    };

    // ----------------------------------------------------------------------------------------
    // OutDigitalTunerPort declaration
    // ----------------------------------------------------------------------------------------
    class OutDigitalTunerPort : public OutDigitalTunerPortT<FRONTEND::DigitalTuner_var,FRONTEND::DigitalTuner> {
        public:
            OutDigitalTunerPort(std::string port_name) : OutDigitalTunerPortT<FRONTEND::DigitalTuner_var,FRONTEND::DigitalTuner>(port_name)
            {};
    };

    template<typename PortType_var, typename PortType>
    class OutDigitalScanningTunerPortT : public OutDigitalTunerPortT<PortType_var, PortType>
    {
        public:
            OutDigitalScanningTunerPortT(std::string port_name) : OutDigitalTunerPortT<PortType_var, PortType>(port_name)
            {};
            ~OutDigitalScanningTunerPortT(){};

            void setScanStartTime(const std::string &id, const BULKIO::PrecisionUTCTime& start_time, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        ((*i).first)->setScanStartTime(id.c_str(), start_time);
                    }
                }
                return;
            };
            void setScanStrategy(const std::string &id, const FRONTEND::ScanningTuner::ScanStrategy& scan_strategy, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        ((*i).first)->setScanStrategy(id.c_str(), scan_strategy);
                    }
                }
                return;
            };
            FRONTEND::ScanningTuner::ScanStatus* getScanStatus(const std::string &id, const std::string __connection_id__ = "") {
                FRONTEND::ScanningTuner::ScanStatus_var retval;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);
                OutFrontendTunerPortT<PortType_var, PortType>::__evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        retval = ((*i).first)->getScanStatus(id.c_str());
                    }
                }
                return retval._retn();
            };
    };
    // ----------------------------------------------------------------------------------------
    // OutDigitalTunerPort declaration
    // ----------------------------------------------------------------------------------------
    class OutDigitalScanningTunerPort : public OutDigitalScanningTunerPortT<FRONTEND::DigitalScanningTuner_var,FRONTEND::DigitalScanningTuner> {
        public:
            OutDigitalScanningTunerPort(std::string port_name) : OutDigitalScanningTunerPortT<FRONTEND::DigitalScanningTuner_var,FRONTEND::DigitalScanningTuner>(port_name)
            {};
    };

} // end of frontend namespace


#endif
