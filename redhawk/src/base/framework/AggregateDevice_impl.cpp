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


#include "ossie/AggregateDevice_impl.h"

AggregateDevice_impl::AggregateDevice_impl()
{
    _devices = new CF::DeviceSequence;
    _devices->length(0);
}

AggregateDevice_impl::~AggregateDevice_impl()
{
    delete _devices;
}

void AggregateDevice_impl::addDevice(CF::Device_ptr associatedDevice)
{
    unsigned int devSeqLength = _devices->length();

    for (unsigned int i = 0; i < devSeqLength; i++) {
      std::string a_id = ossie::corba::returnString(associatedDevice->identifier());
      std::string dev_id = ossie::corba::returnString((*_devices)[i]->identifier() );
      if (!strcmp(a_id.c_str(), dev_id.c_str()) ) {
            return;
        }
    }
    _devices->length(devSeqLength + 1);
    (*_devices)[devSeqLength] = CF::Device::_duplicate(associatedDevice);
}

void AggregateDevice_impl::removeDevice(CF::Device_ptr associatedDevice)
{
    unsigned int devSeqLength = _devices->length();

    for (unsigned int i = 0; i < devSeqLength; i++) {
      std::string a_id = ossie::corba::returnString(associatedDevice->identifier());
      std::string dev_id = ossie::corba::returnString((*_devices)[i]->identifier() );
       if (!strcmp(a_id.c_str(), dev_id.c_str()) ) {
            for (unsigned int j = i + 1; j < devSeqLength; j++) {
                (*_devices)[j-1] = (*_devices)[j];
            }
            _devices->length(devSeqLength - 1);
        }
    }

    return;
}

CF::DeviceSequence* AggregateDevice_impl::devices()
{
    CF::DeviceSequence_var result = new CF::DeviceSequence(*_devices);
    return result._retn();
}
