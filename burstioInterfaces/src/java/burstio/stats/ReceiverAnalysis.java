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

import java.util.List;

import burstio.Utils;

class ReceiverAnalysis extends Analysis<ReceiverStatPoint> {
    protected void addKeywords (List<CF.DataType> keywords)
    {
        super.addKeywords(keywords);
        Utils.addKeyword(keywords, "QUEUE_FLUSHES", this.totalFlushes);
        Utils.addKeyword(keywords, "DROPPED_RATIO", this.totalDropped / (double)this.totalBursts);
    }

    public void add (ReceiverStatPoint sample) {
        super.add(sample);
        this.totalFlushes += sample.flushes;
        this.totalDropped += sample.dropped;
    }

    private int totalFlushes = 0;
    private int totalDropped = 0;
}
