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

import java.util.Collection;

public class SenderStatistics extends AbstractStatistics<StatPoint> {

    public SenderStatistics (final String name, int bitsPerElement)
    {
        super(name, bitsPerElement);
    }

    public void record (int bursts, int elements, float queueDepth, double delay)
    {
        this.addSample(new StatPoint(bursts, elements, queueDepth, delay));
    }

    protected Analysis<StatPoint> analyze (Collection<StatPoint> samples)
    {
        Analysis<StatPoint> analysis = new Analysis<StatPoint>();
        analysis.add(samples);
        return analysis;
    }
}
