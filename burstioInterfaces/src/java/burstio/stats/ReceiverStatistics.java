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
import java.util.List;

import burstio.Utils;

public class ReceiverStatistics extends AbstractStatistics<ReceiverStatPoint> {
    
    public ReceiverStatistics (final String name, int bitsPerElement)
    {
        super(name, bitsPerElement);
    }

    public void record (int bursts, int elements, float queueDepth, double delay)
    {
        this.addSample(new ReceiverStatPoint(bursts, elements, queueDepth, delay, 0, 0));
    }

    public void flushOccurred (int bursts)
    {
        ReceiverStatPoint last = this.statistics_.getLast();
        last.flushes++;
        last.dropped += bursts;

        this.flushCount_++;
        this.burstsDropped_ += bursts;
    }

    protected Analysis<ReceiverStatPoint> analyze (Collection<ReceiverStatPoint> samples)
    {
        Analysis<ReceiverStatPoint> analysis = new ReceiverAnalysis();
        analysis.add(samples);
        return analysis;
    }

    protected void addKeywords (List<CF.DataType> keywords)
    {
        if (this.flushCount_ > 0) {
            Utils.addKeyword(keywords, "FLUSH_COUNT", this.flushCount_);
            Utils.addKeyword(keywords, "BURSTS_DROPPED", this.burstsDropped_);
        }
    }

    private int flushCount_ = 0;
    private int burstsDropped_ = 0;
}
