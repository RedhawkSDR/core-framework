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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import burstio.Utils;

class Analysis<E extends StatPoint> {
    public void add (Collection<E> samples)
    {
        for (E sample : samples) {
            this.add(sample);
        }
    }

    public void add (E sample) {
        if (this.elapsed == 0.0) {
            this.elapsed = Utils.elapsed(sample.timestamp);
        }
        this.lastTimestamp = sample.timestamp;
        this.totalCalls++;
        this.totalBursts += sample.bursts;
        this.totalElements += sample.elements;
        this.totalQueueDepth += sample.queueDepth;
        this.totalDelay += sample.delay;
    }

    public float getElementsPerSecond ()
    {
        if (this.elapsed == 0.0) {
            return 0.0f;
        }
        return (float)(this.totalElements / this.elapsed);
    }

    public float getCallsPerSecond ()
    {
        if (this.elapsed == 0.0) {
            return 0.0f;
        }
        return (float)(this.totalCalls / this.elapsed);
    }

    public float getAverageQueueDepth ()
    {
        if (this.totalCalls == 0) {
            return 0.0f;
        }
        return (float)(this.totalQueueDepth / this.totalCalls);
    }

    public float getTimeSinceLastCall ()
    {
        if (this.lastTimestamp == null) {
            return 0.0f;
        }
        return (float)Utils.elapsed(this.lastTimestamp);
    }

    public List<CF.DataType> getKeywords ()
    {
        List<CF.DataType> keywords = new ArrayList<CF.DataType>();
        this.addKeywords(keywords);
        return keywords;
    }

    protected void addKeywords(List<CF.DataType> keywords)
    {
        double bursts_per_second = 0.0;
        double bursts_per_push = 0.0;
        double elements_per_burst = 0.0;
        double average_latency = 0.0;
        if (this.totalCalls > 0) {
            bursts_per_second = this.totalBursts / this.elapsed;
            bursts_per_push = this.totalBursts / (double)this.totalCalls;
            elements_per_burst = this.totalElements / (double)this.totalBursts;
            average_latency = this.totalDelay / this.totalCalls;
        }

        Utils.addKeyword(keywords, "BURSTS_PER_SECOND", bursts_per_second);
        Utils.addKeyword(keywords, "BURSTS_PER_PUSH", bursts_per_push);
        Utils.addKeyword(keywords, "ELEMENTS_PER_BURST", elements_per_burst);
        Utils.addKeyword(keywords, "AVERAGE_LATENCY", average_latency);
    }

    protected BULKIO.PrecisionUTCTime lastTimestamp = null;
    protected double elapsed = 0.0;

    protected int totalCalls = 0;
    protected long totalBursts = 0;
    protected long totalElements = 0;
    protected double totalQueueDepth = 0.0;
    protected double totalDelay = 0.0;
}
