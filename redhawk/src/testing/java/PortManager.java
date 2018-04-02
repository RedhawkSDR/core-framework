/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
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

import java.util.ArrayList;
import java.util.List;

import org.ossie.component.StartablePort;

/**
 * Test helper class to manage a set of ports as they might be used in a
 * component, but for use in unit test fixtures.
 */
 public class PortManager {

    public org.omg.CORBA.Object addPort(Object port) throws org.omg.CORBA.UserException
    {
        _ports.add(port);

        org.omg.CORBA.ORB orb = org.ossie.corba.utils.Orb();
        if (port instanceof org.omg.PortableServer.Servant) {
            return activatePort((org.omg.PortableServer.Servant) port);
        } else if (port instanceof omnijni.Servant) {
            return ((omnijni.Servant) port)._this_object(orb);
        } else {
            return null;
        }
    }

    private org.omg.CORBA.Object activatePort(org.omg.PortableServer.Servant servant) throws org.omg.CORBA.UserException
    {
        org.ossie.corba.utils.RootPOA().activate_object(servant);
        return servant._this_object();
    }

    public void start()
    {
        startPorts();
    }

    public void stop()
    {
        stopPorts();
    }

    public void releaseObject()
    {
        releasePorts();
        _ports.clear();
    }

    private void startPorts()
    {
        for (Object port : _ports) {
            if (port instanceof StartablePort) {
                ((StartablePort) port).startPort();
            }
        }
    }

    private void stopPorts()
    {
        for (Object port : _ports) {
            if (port instanceof StartablePort) {
                ((StartablePort) port).stopPort();
            }
        }
    }

    private void releasePorts()
    {
        for (Object port : _ports) {
            if (port instanceof org.omg.PortableServer.Servant) {
                deactivatePort((org.omg.PortableServer.Servant) port);
            } else if (port instanceof omnijni.Servant) {
                ((omnijni.Servant) port)._deactivate();
            }
        }
        _ports.clear();
    }

    private void deactivatePort(org.omg.PortableServer.Servant servant)
    {
        try {
            org.omg.PortableServer.POA poa = org.ossie.corba.utils.RootPOA();
            poa.deactivate_object(poa.servant_to_id(servant));
        } catch (Exception exc) {
            // Ignore errors
        }
    }

    private List<Object> _ports = new ArrayList<>();
}
