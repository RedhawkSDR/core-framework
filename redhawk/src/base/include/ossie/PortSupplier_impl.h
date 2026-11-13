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


#ifndef PORTSUPPLIER_IMPL_H
#define PORTSUPPLIER_IMPL_H

#include <map>
#include <string>

#include "CF/cf.h"
#include "Port_impl.h"
#include "debug.h"
#include "ossie/Autocomplete.h"

/*
The port supplier provides specialized functionality to manage Ports
*/

class PortSupplier_impl
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
        : public virtual POA_CF::PortSupplier
#endif
{
    ENABLE_LOGGING;

public:
    PortSupplier_impl ();

    // Return an object reference for the named port.
    CORBA::Object* getPort (const char*);

    void setLogger(rh_logger::LoggerPtr logptr);

protected:
    typedef std::map<std::string, PortBase*> PortServantMap;
    PortServantMap _portServants;

    void addPort (const std::string& name, PortBase* servant);
    void addPort (const std::string& name, const std::string& description, PortBase* servant);
    void releasePorts ();

    void startPorts ();
    void stopPorts ();

    // Legacy interface; new components should use the above methods
    typedef std::map<std::string, Port_Uses_base_impl *>       RH_UsesPortMap;
    typedef std::map<std::string, Port_Provides_base_impl *>   RH_ProvidesPortMap;

    RH_UsesPortMap  outPorts;
    std::map<std::string, CF::Port_var> outPorts_var;
    RH_ProvidesPortMap inPorts;

    void registerInPort(Port_Provides_base_impl *port);
    void registerOutPort(Port_Uses_base_impl *port, CF::Port_ptr ref);

    void releaseInPorts();
    void releaseOutPorts();
    void deactivateOutPorts();
    void deactivateInPorts();

private:
    void insertPort (const std::string& name, PortBase* servant);
    void deactivatePort (PortBase* servant);

    rh_logger::LoggerPtr _portsupplierLog;

};

#endif
