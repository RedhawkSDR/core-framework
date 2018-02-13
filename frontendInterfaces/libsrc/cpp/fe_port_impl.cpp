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
#include "fe_port_impl.h"


namespace frontend {


     FRONTEND::RFInfoPkt getRFInfoPkt(const RFInfoPkt &val) {
        FRONTEND::RFInfoPkt tmpVal;
        tmpVal.rf_flow_id = CORBA::string_dup(val.rf_flow_id.c_str());
        tmpVal.rf_center_freq = val.rf_center_freq;
        tmpVal.rf_bandwidth = val.rf_bandwidth;
        tmpVal.if_center_freq = val.if_center_freq;
        tmpVal.spectrum_inverted = val.spectrum_inverted;
        tmpVal.sensor.collector = CORBA::string_dup(val.sensor.collector.c_str());
        tmpVal.sensor.mission = CORBA::string_dup(val.sensor.mission.c_str());
        tmpVal.sensor.rx = CORBA::string_dup(val.sensor.rx.c_str());
        tmpVal.sensor.antenna.description = CORBA::string_dup(val.sensor.antenna.description.c_str());
        tmpVal.sensor.antenna.name = CORBA::string_dup(val.sensor.antenna.name.c_str());
        tmpVal.sensor.antenna.size = CORBA::string_dup(val.sensor.antenna.size.c_str());
        tmpVal.sensor.antenna.type = CORBA::string_dup(val.sensor.antenna.type.c_str());
        tmpVal.sensor.feed.name = CORBA::string_dup(val.sensor.feed.name.c_str());
        tmpVal.sensor.feed.polarization = CORBA::string_dup(val.sensor.feed.polarization.c_str());
        tmpVal.sensor.feed.freq_range.max_val = val.sensor.feed.freq_range.max_val;
        tmpVal.sensor.feed.freq_range.min_val = val.sensor.feed.freq_range.min_val;
        tmpVal.sensor.feed.freq_range.values.length(val.sensor.feed.freq_range.values.size());
        for (unsigned int i=0; i<val.sensor.feed.freq_range.values.size(); i++) {
            tmpVal.sensor.feed.freq_range.values[i] = val.sensor.feed.freq_range.values[i];
        }
        tmpVal.ext_path_delays.length(val.ext_path_delays.size());
        for (unsigned int i=0; i<val.ext_path_delays.size(); i++) {
            tmpVal.ext_path_delays[i].freq = val.ext_path_delays[i].freq;
            tmpVal.ext_path_delays[i].delay_ns = val.ext_path_delays[i].delay_ns;
        }
        tmpVal.capabilities.freq_range.min_val = val.capabilities.freq_range.min_val;
        tmpVal.capabilities.freq_range.max_val = val.capabilities.freq_range.max_val;
        tmpVal.capabilities.bw_range.min_val = val.capabilities.bw_range.min_val;
        tmpVal.capabilities.bw_range.max_val = val.capabilities.bw_range.max_val;
        tmpVal.additional_info = val.additional_info;
        return tmpVal;
    };


     FRONTEND::RFInfoPkt* returnRFInfoPkt(const RFInfoPkt &val) {
        FRONTEND::RFInfoPkt* tmpVal = new FRONTEND::RFInfoPkt();
        tmpVal->rf_flow_id = CORBA::string_dup(val.rf_flow_id.c_str());
        tmpVal->rf_center_freq = val.rf_center_freq;
        tmpVal->rf_bandwidth = val.rf_bandwidth;
        tmpVal->if_center_freq = val.if_center_freq;
        tmpVal->spectrum_inverted = val.spectrum_inverted;
        tmpVal->sensor.collector = CORBA::string_dup(val.sensor.collector.c_str());
        tmpVal->sensor.mission = CORBA::string_dup(val.sensor.mission.c_str());
        tmpVal->sensor.rx = CORBA::string_dup(val.sensor.rx.c_str());
        tmpVal->sensor.antenna.description = CORBA::string_dup(val.sensor.antenna.description.c_str());
        tmpVal->sensor.antenna.name = CORBA::string_dup(val.sensor.antenna.name.c_str());
        tmpVal->sensor.antenna.size = CORBA::string_dup(val.sensor.antenna.size.c_str());
        tmpVal->sensor.antenna.type = CORBA::string_dup(val.sensor.antenna.type.c_str());
        tmpVal->sensor.feed.name = CORBA::string_dup(val.sensor.feed.name.c_str());
        tmpVal->sensor.feed.polarization = CORBA::string_dup(val.sensor.feed.polarization.c_str());
        tmpVal->sensor.feed.freq_range.max_val = val.sensor.feed.freq_range.max_val;
        tmpVal->sensor.feed.freq_range.min_val = val.sensor.feed.freq_range.min_val;
        tmpVal->sensor.feed.freq_range.values.length(val.sensor.feed.freq_range.values.size());
        for (unsigned int i=0; i<val.sensor.feed.freq_range.values.size(); i++) {
            tmpVal->sensor.feed.freq_range.values[i] = val.sensor.feed.freq_range.values[i];
        }
        tmpVal->ext_path_delays.length(val.ext_path_delays.size());
        for (unsigned int i=0; i<val.ext_path_delays.size(); i++) {
            tmpVal->ext_path_delays[i].freq = val.ext_path_delays[i].freq;
            tmpVal->ext_path_delays[i].delay_ns = val.ext_path_delays[i].delay_ns;
        }
        tmpVal->capabilities.freq_range.min_val = val.capabilities.freq_range.min_val;
        tmpVal->capabilities.freq_range.max_val = val.capabilities.freq_range.max_val;
        tmpVal->capabilities.bw_range.min_val = val.capabilities.bw_range.min_val;
        tmpVal->capabilities.bw_range.max_val = val.capabilities.bw_range.max_val;
        tmpVal->additional_info = val.additional_info;
        return tmpVal;
    };
     RFInfoPkt returnRFInfoPkt(const FRONTEND::RFInfoPkt &tmpVal) {
        RFInfoPkt val;
        val.rf_flow_id = ossie::corba::returnString(tmpVal.rf_flow_id);
        val.rf_center_freq = tmpVal.rf_center_freq;
        val.rf_bandwidth = tmpVal.rf_bandwidth;
        val.if_center_freq = tmpVal.if_center_freq;
        val.spectrum_inverted = tmpVal.spectrum_inverted;
        val.sensor.collector = ossie::corba::returnString(tmpVal.sensor.collector);
        val.sensor.mission = ossie::corba::returnString(tmpVal.sensor.mission);
        val.sensor.rx = ossie::corba::returnString(tmpVal.sensor.rx);
        val.sensor.antenna.description = ossie::corba::returnString(tmpVal.sensor.antenna.description);
        val.sensor.antenna.name = ossie::corba::returnString(tmpVal.sensor.antenna.name);
        val.sensor.antenna.size = ossie::corba::returnString(tmpVal.sensor.antenna.size);
        val.sensor.antenna.type = ossie::corba::returnString(tmpVal.sensor.antenna.type);
        val.sensor.feed.name = ossie::corba::returnString(tmpVal.sensor.feed.name);
        val.sensor.feed.polarization = ossie::corba::returnString(tmpVal.sensor.feed.polarization);
        val.sensor.feed.freq_range.max_val = tmpVal.sensor.feed.freq_range.max_val;
        val.sensor.feed.freq_range.min_val = tmpVal.sensor.feed.freq_range.min_val;
        val.sensor.feed.freq_range.values.resize(tmpVal.sensor.feed.freq_range.values.length());
        for (unsigned int i=0; i<val.sensor.feed.freq_range.values.size(); i++) {
            val.sensor.feed.freq_range.values[i] = tmpVal.sensor.feed.freq_range.values[i];
        }
        val.ext_path_delays.resize(tmpVal.ext_path_delays.length());
        for (unsigned int i=0; i<val.ext_path_delays.size(); i++) {
            val.ext_path_delays[i].freq = tmpVal.ext_path_delays[i].freq;
            val.ext_path_delays[i].delay_ns = tmpVal.ext_path_delays[i].delay_ns;
        }
        val.capabilities.freq_range.min_val = tmpVal.capabilities.freq_range.min_val;
        val.capabilities.freq_range.max_val = tmpVal.capabilities.freq_range.max_val;
        val.capabilities.bw_range.min_val = tmpVal.capabilities.bw_range.min_val;
        val.capabilities.bw_range.max_val = tmpVal.capabilities.bw_range.max_val;
        val.additional_info = tmpVal.additional_info;
        return val;
    };
    
     FRONTEND::GPSInfo* returnGPSInfo(const frontend::GPSInfo &val) {
        FRONTEND::GPSInfo* tmpVal = new FRONTEND::GPSInfo();
        tmpVal->source_id = CORBA::string_dup(val.source_id.c_str());
        tmpVal->rf_flow_id = CORBA::string_dup(val.rf_flow_id.c_str());
        tmpVal->mode = CORBA::string_dup(val.mode.c_str());
        tmpVal->fom = val.fom;
        tmpVal->tfom = val.tfom;
        tmpVal->datumID = val.datumID;
        tmpVal->time_offset = val.time_offset;
        tmpVal->freq_offset = val.freq_offset;
        tmpVal->time_variance = val.time_variance;
        tmpVal->freq_variance = val.freq_variance;
        tmpVal->satellite_count = val.satellite_count;
        tmpVal->snr = val.snr;
        tmpVal->status_message = CORBA::string_dup(val.status_message.c_str());
        tmpVal->timestamp = val.timestamp;
        tmpVal->additional_info = val.additional_info;
        return tmpVal;
    };
     frontend::GPSInfo returnGPSInfo(const FRONTEND::GPSInfo &tmpVal) {
        frontend::GPSInfo val;
        val.source_id = ossie::corba::returnString(tmpVal.source_id);
        val.rf_flow_id = ossie::corba::returnString(tmpVal.rf_flow_id);
        val.mode = ossie::corba::returnString(tmpVal.mode);
        val.fom = tmpVal.fom;
        val.tfom = tmpVal.tfom;
        val.datumID = tmpVal.datumID;
        val.time_offset = tmpVal.time_offset;
        val.freq_offset = tmpVal.freq_offset;
        val.time_variance = tmpVal.time_variance;
        val.freq_variance = tmpVal.freq_variance;
        val.satellite_count = tmpVal.satellite_count;
        val.snr = tmpVal.snr;
        val.status_message = ossie::corba::returnString(tmpVal.status_message);
        val.timestamp = tmpVal.timestamp;
        val.additional_info = tmpVal.additional_info;
        return val;
    };
    
     FRONTEND::GpsTimePos* returnGpsTimePos(const frontend::GpsTimePos &val) {
        FRONTEND::GpsTimePos* tmpVal = new FRONTEND::GpsTimePos();
        tmpVal->position.valid = val.position.valid;
        tmpVal->position.datum = CORBA::string_dup(val.position.datum.c_str());
        tmpVal->position.lat = val.position.lat;
        tmpVal->position.lon = val.position.lon;
        tmpVal->position.alt = val.position.alt;
        tmpVal->timestamp = val.timestamp;
        return tmpVal;
    };
     frontend::GpsTimePos returnGpsTimePos(const FRONTEND::GpsTimePos &tmpVal) {
        frontend::GpsTimePos val;
        val.position.valid = tmpVal.position.valid;
        val.position.datum = ossie::corba::returnString(tmpVal.position.datum);
        val.position.lat = tmpVal.position.lat;
        val.position.lon = tmpVal.position.lon;
        val.position.alt = tmpVal.position.alt;
        val.timestamp = tmpVal.timestamp;
        return val;
    };

    FRONTEND::NavigationPacket* returnNavigationPacket(const frontend::NavigationPacket &val) {
        FRONTEND::NavigationPacket* tmpVal = new FRONTEND::NavigationPacket();
        tmpVal->source_id = CORBA::string_dup(val.source_id.c_str());
        tmpVal->rf_flow_id = CORBA::string_dup(val.rf_flow_id.c_str());
        tmpVal->position.valid = val.position.valid;
        tmpVal->position.datum = CORBA::string_dup(val.position.datum.c_str());
        tmpVal->position.lat = val.position.lat;
        tmpVal->position.lon = val.position.lon;
        tmpVal->position.alt = val.position.alt;
        tmpVal->cposition.valid = val.cposition.valid;
        tmpVal->cposition.datum = CORBA::string_dup(val.cposition.datum.c_str());
        tmpVal->cposition.x = val.cposition.x;
        tmpVal->cposition.y = val.cposition.y;
        tmpVal->cposition.z = val.cposition.z;
        tmpVal->velocity.valid = val.velocity.valid;
        tmpVal->velocity.datum = CORBA::string_dup(val.velocity.datum.c_str());
        tmpVal->velocity.coordinate_system = CORBA::string_dup(val.velocity.coordinate_system.c_str());
        tmpVal->velocity.x = val.velocity.x;
        tmpVal->velocity.y = val.velocity.y;
        tmpVal->velocity.z = val.velocity.z;
        tmpVal->acceleration.valid = val.acceleration.valid;
        tmpVal->acceleration.datum = CORBA::string_dup(val.acceleration.datum.c_str());
        tmpVal->acceleration.coordinate_system = CORBA::string_dup(val.acceleration.coordinate_system.c_str());
        tmpVal->acceleration.x = val.acceleration.x;
        tmpVal->acceleration.y = val.acceleration.y;
        tmpVal->acceleration.z = val.acceleration.z;
        tmpVal->attitude.valid = val.attitude.valid;
        tmpVal->attitude.pitch = val.attitude.pitch;
        tmpVal->attitude.yaw = val.attitude.yaw;
        tmpVal->attitude.roll = val.attitude.roll;
        tmpVal->timestamp = val.timestamp;
        tmpVal->additional_info = val.additional_info;
        return tmpVal;
    };
     frontend::NavigationPacket returnNavigationPacket(const FRONTEND::NavigationPacket &tmpVal) {
        frontend::NavigationPacket val;
        val.source_id = ossie::corba::returnString(tmpVal.source_id);
        val.rf_flow_id = ossie::corba::returnString(tmpVal.rf_flow_id);
        val.position.valid = tmpVal.position.valid;
        val.position.datum = ossie::corba::returnString(tmpVal.position.datum);
        val.position.lat = tmpVal.position.lat;
        val.position.lon = tmpVal.position.lon;
        val.position.alt = tmpVal.position.alt;
        val.cposition.valid = tmpVal.cposition.valid;
        val.cposition.datum = ossie::corba::returnString(tmpVal.cposition.datum);
        val.cposition.x = tmpVal.cposition.x;
        val.cposition.y = tmpVal.cposition.y;
        val.cposition.z = tmpVal.cposition.z;
        val.velocity.valid = tmpVal.velocity.valid;
        val.velocity.datum = ossie::corba::returnString(tmpVal.velocity.datum);
        val.velocity.coordinate_system = ossie::corba::returnString(tmpVal.velocity.coordinate_system);
        val.velocity.x = tmpVal.velocity.x;
        val.velocity.y = tmpVal.velocity.y;
        val.velocity.z = tmpVal.velocity.z;
        val.acceleration.valid = tmpVal.acceleration.valid;
        val.acceleration.datum = ossie::corba::returnString(tmpVal.acceleration.datum);
        val.acceleration.coordinate_system = ossie::corba::returnString(tmpVal.acceleration.coordinate_system);
        val.acceleration.x = tmpVal.acceleration.x;
        val.acceleration.y = tmpVal.acceleration.y;
        val.acceleration.z = tmpVal.acceleration.z;
        val.attitude.valid = tmpVal.attitude.valid;
        val.attitude.pitch = tmpVal.attitude.pitch;
        val.attitude.yaw = tmpVal.attitude.yaw;
        val.attitude.roll = tmpVal.attitude.roll;
        val.timestamp = tmpVal.timestamp;
        val.additional_info = tmpVal.additional_info;
        return val;
    };

      void  copyRFInfoPktSequence(const RFInfoPktSequence &src, FRONTEND::RFInfoPktSequence &dest) {
        RFInfoPktSequence::const_iterator i=src.begin();
        for( ; i != src.end(); i++ ) {
            ossie::corba::push_back( dest, getRFInfoPkt(*i) );
        }
    }

     void copyRFInfoPktSequence(const FRONTEND::RFInfoPktSequence &src, RFInfoPktSequence &dest ) {
        for( unsigned int i=0 ; i != src.length(); i++ ) {
            dest.push_back( returnRFInfoPkt( src[i] ));
        }
    }

    FRONTEND::ScanningTuner::ScanStatus* returnScanStatus(const frontend::ScanStatus &val) {
        FRONTEND::ScanningTuner::ScanStatus* tmpVal = new FRONTEND::ScanningTuner::ScanStatus();
        ScanStrategy* _scan_strategy = val.strategy.get();
        if (dynamic_cast<ManualStrategy*>(_scan_strategy) != NULL) {
            tmpVal->strategy.scan_mode = FRONTEND::ScanningTuner::MANUAL_SCAN;
            ManualStrategy* _strat = dynamic_cast<ManualStrategy*>(_scan_strategy);
            tmpVal->strategy.scan_definition.center_frequency(_strat->center_frequency);
            switch(_strat->control_mode) {
                case frontend::TIME_BASED:
                    tmpVal->strategy.control_mode = FRONTEND::ScanningTuner::TIME_BASED;
                    break;
                case frontend::SAMPLE_BASED:
                    tmpVal->strategy.control_mode = FRONTEND::ScanningTuner::SAMPLE_BASED;
                    break;
            }
            tmpVal->strategy.control_value = _strat->control_value;
        } else if (dynamic_cast<SpanStrategy*>(_scan_strategy) != NULL) {
            tmpVal->strategy.scan_mode = FRONTEND::ScanningTuner::SPAN_SCAN;
            SpanStrategy* _strat = dynamic_cast<SpanStrategy*>(_scan_strategy);
            FRONTEND::ScanningTuner::ScanSpanRanges _tmp;
            _tmp.length(_strat->freq_scan_list.size());
            for (unsigned int i=0; i<_strat->freq_scan_list.size(); i++) {
                _tmp[i].begin_frequency = _strat->freq_scan_list[i].begin_frequency;
                _tmp[i].end_frequency = _strat->freq_scan_list[i].end_frequency;
                _tmp[i].step = _strat->freq_scan_list[i].step;
            }
            tmpVal->strategy.scan_definition.freq_scan_list(_tmp);
            switch(_strat->control_mode) {
                case frontend::TIME_BASED:
                    tmpVal->strategy.control_mode = FRONTEND::ScanningTuner::TIME_BASED;
                    break;
                case frontend::SAMPLE_BASED:
                    tmpVal->strategy.control_mode = FRONTEND::ScanningTuner::SAMPLE_BASED;
                    break;
            }
            tmpVal->strategy.control_value = _strat->control_value;
        } else if (dynamic_cast<DiscreteStrategy*>(_scan_strategy) != NULL) {
            tmpVal->strategy.scan_mode = FRONTEND::ScanningTuner::DISCRETE_SCAN;
            DiscreteStrategy* _strat = dynamic_cast<DiscreteStrategy*>(_scan_strategy);
            FRONTEND::ScanningTuner::Frequencies _tmp;
            _tmp.length(_strat->discrete_freq_list.size());
            for (unsigned int i=0; i<_strat->discrete_freq_list.size(); i++) {
                _tmp[i] = _strat->discrete_freq_list[i];
            }
            tmpVal->strategy.scan_definition.discrete_freq_list(_tmp);
            switch(_strat->control_mode) {
                case frontend::TIME_BASED:
                    tmpVal->strategy.control_mode = FRONTEND::ScanningTuner::TIME_BASED;
                    break;
                case frontend::SAMPLE_BASED:
                    tmpVal->strategy.control_mode = FRONTEND::ScanningTuner::SAMPLE_BASED;
                    break;
            }
            tmpVal->strategy.control_value = _strat->control_value;
        }
        tmpVal->start_time = val.start_time;
        tmpVal->center_tune_frequencies.length(val.center_tune_frequencies.size());
        for (unsigned int i=0; i<val.center_tune_frequencies.size(); i++) {
            tmpVal->center_tune_frequencies[i] = val.center_tune_frequencies[i];
        }
        tmpVal->started = val.started;
        return tmpVal;
    };
    frontend::ScanStatus returnScanStatus(const FRONTEND::ScanningTuner::ScanStatus &tmpVal) {
        switch (tmpVal.strategy.scan_mode) {
            case FRONTEND::ScanningTuner::MANUAL_SCAN:
                break;
            case FRONTEND::ScanningTuner::SPAN_SCAN:
                {
                    ScanSpanRanges freqs;
                    FRONTEND::ScanningTuner::ScanSpanRanges _tmp(tmpVal.strategy.scan_definition.freq_scan_list());
                    freqs.resize(_tmp.length());
                    for (unsigned int i=0; i<_tmp.length(); i++) {
                        freqs[i].begin_frequency = _tmp[i].begin_frequency;
                        freqs[i].end_frequency = _tmp[i].end_frequency;
                        freqs[i].step = _tmp[i].step;
                    }
                    SpanStrategy* _strat = new SpanStrategy(freqs);
                    switch(tmpVal.strategy.control_mode) {
                        case frontend::TIME_BASED:
                            _strat->control_mode = frontend::TIME_BASED;
                            break;
                        case frontend::SAMPLE_BASED:
                            _strat->control_mode = frontend::SAMPLE_BASED;
                            break;
                    }
                    _strat->control_value = tmpVal.strategy.control_value;
                    ScanStatus retval(_strat);
                    retval.start_time = tmpVal.start_time;
                    retval.started = tmpVal.started;
                    for (unsigned int i=0; i<tmpVal.center_tune_frequencies.length(); i++)
                        retval.center_tune_frequencies.push_back(tmpVal.center_tune_frequencies[i]);
                    return retval;
                }
            case FRONTEND::ScanningTuner::DISCRETE_SCAN:
                {
                    Frequencies freqs;
                    FRONTEND::ScanningTuner::Frequencies _tmp(tmpVal.strategy.scan_definition.discrete_freq_list());
                    for (unsigned int i=0; i<_tmp.length(); i++) {
                        freqs.push_back(_tmp[i]);
                    }
                    DiscreteStrategy* _strat = new DiscreteStrategy(freqs);
                    switch(tmpVal.strategy.control_mode) {
                        case frontend::TIME_BASED:
                            _strat->control_mode = frontend::TIME_BASED;
                            break;
                        case frontend::SAMPLE_BASED:
                            _strat->control_mode = frontend::SAMPLE_BASED;
                            break;
                    }
                    _strat->control_value = tmpVal.strategy.control_value;
                    ScanStatus retval(_strat);
                    retval.start_time = tmpVal.start_time;
                    retval.started = tmpVal.started;
                    for (unsigned int i=0; i<tmpVal.center_tune_frequencies.length(); i++)
                        retval.center_tune_frequencies.push_back(tmpVal.center_tune_frequencies[i]);
                    return retval;
                }
        }
        ManualStrategy* _strat = new ManualStrategy(tmpVal.strategy.scan_definition.center_frequency());
        switch(tmpVal.strategy.control_mode) {
            case frontend::TIME_BASED:
                _strat->control_mode = frontend::TIME_BASED;
                break;
            case frontend::SAMPLE_BASED:
                _strat->control_mode = frontend::SAMPLE_BASED;
                break;
        }
        _strat->control_value = tmpVal.strategy.control_value;
        ScanStatus retval(_strat);
        retval.start_time = tmpVal.start_time;
        retval.started = tmpVal.started;
        for (unsigned int i=0; i<tmpVal.center_tune_frequencies.length(); i++)
            retval.center_tune_frequencies.push_back(tmpVal.center_tune_frequencies[i]);
        return retval;
    };
    FRONTEND::ScanningTuner::ScanStrategy* returnScanStrategy(const frontend::ScanStrategy &val) {
        FRONTEND::ScanningTuner::ScanStrategy* tmpVal = new FRONTEND::ScanningTuner::ScanStrategy();
        if (dynamic_cast<const ManualStrategy*>(&val) != NULL) {
            tmpVal->scan_mode = FRONTEND::ScanningTuner::MANUAL_SCAN;
            const ManualStrategy* _strat = dynamic_cast<const ManualStrategy*>(&val);
            tmpVal->scan_definition.center_frequency(_strat->center_frequency);
            switch(_strat->control_mode) {
                case frontend::TIME_BASED:
                    tmpVal->control_mode = FRONTEND::ScanningTuner::TIME_BASED;
                    break;
                case frontend::SAMPLE_BASED:
                    tmpVal->control_mode = FRONTEND::ScanningTuner::SAMPLE_BASED;
                    break;
            }
            tmpVal->control_value = _strat->control_value;
        } else if (dynamic_cast<const SpanStrategy*>(&val) != NULL) {
            tmpVal->scan_mode = FRONTEND::ScanningTuner::SPAN_SCAN;
            const SpanStrategy* _strat = dynamic_cast<const SpanStrategy*>(&val);
            FRONTEND::ScanningTuner::ScanSpanRanges _tmp;
            _tmp.length(_strat->freq_scan_list.size());
            for (unsigned int i=0; i<_strat->freq_scan_list.size(); i++) {
                _tmp[i].begin_frequency = _strat->freq_scan_list[i].begin_frequency;
                _tmp[i].end_frequency = _strat->freq_scan_list[i].end_frequency;
                _tmp[i].step = _strat->freq_scan_list[i].step;
            }
            tmpVal->scan_definition.freq_scan_list(_tmp);
            switch(_strat->control_mode) {
                case frontend::TIME_BASED:
                    tmpVal->control_mode = FRONTEND::ScanningTuner::TIME_BASED;
                    break;
                case frontend::SAMPLE_BASED:
                    tmpVal->control_mode = FRONTEND::ScanningTuner::SAMPLE_BASED;
                    break;
            }
            tmpVal->control_value = _strat->control_value;
        } else if (dynamic_cast<const DiscreteStrategy*>(&val) != NULL) {
            tmpVal->scan_mode = FRONTEND::ScanningTuner::DISCRETE_SCAN;
            const DiscreteStrategy* _strat = dynamic_cast<const DiscreteStrategy*>(&val);
            FRONTEND::ScanningTuner::Frequencies _tmp;
            _tmp.length(_strat->discrete_freq_list.size());
            for (unsigned int i=0; i<_strat->discrete_freq_list.size(); i++) {
                _tmp[i] = _strat->discrete_freq_list[i];
            }
            tmpVal->scan_definition.discrete_freq_list(_tmp);
            switch(_strat->control_mode) {
                case frontend::TIME_BASED:
                    tmpVal->control_mode = FRONTEND::ScanningTuner::TIME_BASED;
                    break;
                case frontend::SAMPLE_BASED:
                    tmpVal->control_mode = FRONTEND::ScanningTuner::SAMPLE_BASED;
                    break;
            }
            tmpVal->control_value = _strat->control_value;
        }
        return tmpVal;
    };
    frontend::ScanStrategy* returnScanStrategy(const FRONTEND::ScanningTuner::ScanStrategy &tmpVal) {
        switch (tmpVal.scan_mode) {
            case FRONTEND::ScanningTuner::MANUAL_SCAN:
                break;
            case FRONTEND::ScanningTuner::SPAN_SCAN:
                {
                    ScanSpanRanges freqs;
                    FRONTEND::ScanningTuner::ScanSpanRanges _tmp(tmpVal.scan_definition.freq_scan_list());
                    freqs.resize(_tmp.length());
                    for (unsigned int i=0; i<_tmp.length(); i++) {
                        freqs[i].begin_frequency = _tmp[i].begin_frequency;
                        freqs[i].end_frequency = _tmp[i].end_frequency;
                        freqs[i].step = _tmp[i].step;
                    }
                    SpanStrategy* retval = new SpanStrategy(freqs);
                    switch(tmpVal.control_mode) {
                        case frontend::TIME_BASED:
                            retval->control_mode = frontend::TIME_BASED;
                            break;
                        case frontend::SAMPLE_BASED:
                            retval->control_mode = frontend::SAMPLE_BASED;
                            break;
                    }
                    retval->control_value = tmpVal.control_value;
                    return retval;
                }
            case FRONTEND::ScanningTuner::DISCRETE_SCAN:
                {
                    Frequencies freqs;
                    FRONTEND::ScanningTuner::Frequencies _tmp(tmpVal.scan_definition.discrete_freq_list());
                    for (unsigned int i=0; i<_tmp.length(); i++) {
                        freqs.push_back(_tmp[i]);
                    }
                    DiscreteStrategy* retval = new DiscreteStrategy(freqs);
                    switch(tmpVal.control_mode) {
                        case frontend::TIME_BASED:
                            retval->control_mode = frontend::TIME_BASED;
                            break;
                        case frontend::SAMPLE_BASED:
                            retval->control_mode = frontend::SAMPLE_BASED;
                            break;
                    }
                    retval->control_value = tmpVal.control_value;
                    return retval;
                }
        }
        ManualStrategy* retval = new ManualStrategy(tmpVal.scan_definition.center_frequency());
        switch(tmpVal.control_mode) {
            case frontend::TIME_BASED:
                retval->control_mode = frontend::TIME_BASED;
                break;
            case frontend::SAMPLE_BASED:
                retval->control_mode = frontend::SAMPLE_BASED;
                break;
        }
        retval->control_value = tmpVal.control_value;
        return retval;
    };
} // end of frontend namespace

