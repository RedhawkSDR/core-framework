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

package stubs;

import java.util.ArrayList;
import java.util.List;

import org.omg.PortableServer.Servant;

public abstract class Stub<E> implements BULKIO.updateSRIOperations,
                                         BULKIO.ProvidesPortStatisticsProviderOperations {
    public List<BULKIO.StreamSRI> H = new ArrayList<BULKIO.StreamSRI>();
    public List<Packet<E>> packets = new ArrayList<Packet<E>>();

    public void pushSRI(BULKIO.StreamSRI H)
    {
        this.H.add(H);
    }

    public BULKIO.StreamSRI[] activeSRIs()
    {
        return new BULKIO.StreamSRI[0];
    }

    public BULKIO.PortUsageType state()
    {
        return BULKIO.PortUsageType.IDLE;
    }

    public BULKIO.PortStatistics statistics()
    {
        return null;
    }

    public Servant servant()
    {
        if (_servant == null) {
            _servant = _makeServant();
        }
        return _servant;
    }

    public org.omg.CORBA.Object _this()
    {
        return servant()._this_object();
    }

    protected abstract Servant _makeServant();
    private Servant _servant = null;
}
