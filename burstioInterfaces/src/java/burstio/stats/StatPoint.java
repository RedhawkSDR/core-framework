/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package burstio.stats;

import burstio.Utils;

class StatPoint {
    public StatPoint(int bursts, int elements, float queueDepth, double delay)
    {
        this.timestamp = Utils.now();
        this.bursts = bursts;
        this.elements = elements;
        this.queueDepth = queueDepth;
        this.delay = delay;
    }

    public BULKIO.PrecisionUTCTime timestamp;
    public int bursts;
    public int elements;
    public float queueDepth;
    public double delay;
}
