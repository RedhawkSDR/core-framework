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
package bulkio;

import org.apache.log4j.Logger;

import org.ossie.component.PortBase;

public interface InDataPort<E,A> extends PortBase {
    public String getName();

    public Logger getLogger();
    public void setLogger(Logger logger);

    public void setSriListener(bulkio.SriListener sriCallback);

    public int getCurrentQueueDepth();
    public int getMaxQueueDepth();
    public void setMaxQueueDepth(int newDepth);

    public DataTransfer<A> getPacket(long wait);

    public void enableStats(boolean enable);
}
