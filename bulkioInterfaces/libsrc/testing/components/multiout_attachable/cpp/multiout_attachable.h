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
#ifndef MULTIOUT_ATTACHABLE_CPP_IMPL_H
#define MULTIOUT_ATTACHABLE_CPP_IMPL_H

#include "multiout_attachable_base.h"

class multiout_attachable_i : public multiout_attachable_base, 
                                  public bulkio::InSDDSPort::Callback,
                                  public bulkio::InVITA49Port::Callback
    
{
    typedef const std::vector<SDDSStreamDefinition_struct> SddsStreamDefs;
    typedef const std::vector<VITA49StreamDefinition_struct> Vita49StreamDefs;

    ENABLE_LOGGING
    public:
        multiout_attachable_i(const char *uuid, const char *label);
        ~multiout_attachable_i();
        int serviceFunction();

	    virtual char* attach(const BULKIO::SDDSStreamDefinition& stream, const char* userid);
	    
        virtual char* attach(const BULKIO::VITA49StreamDefinition& stream, const char* userid);

        // Applicable for both SDDS and VITA callback interface
	    virtual void detach(const char* attachId);

	    void vita49StreamDefChanged(Vita49StreamDefs *oldValue, Vita49StreamDefs *newValue);
	    void sddsStreamDefChanged(SddsStreamDefs *oldValue, SddsStreamDefs *newValue);

	    void newSriCallback(const BULKIO::StreamSRI& sri);
	    void sriChangeCallback(const BULKIO::StreamSRI& sri);
};

#endif // MULTIOUT_ATTACHABLE_CPP_IMPL_H
