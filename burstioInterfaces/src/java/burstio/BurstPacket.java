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
package burstio;

abstract class BurstPacket<A> {
    public String getStreamID() {
        return this.sri_.streamID;
    }

    public abstract int getSize ();
    public abstract A getData ();

    public boolean isComplex () {
        return (this.sri_.mode == 1);
    }

    public boolean getEOS () {
        return this.eos_;
    }

    public BULKIO.PrecisionUTCTime getTime () {
        return this.time_;
    }

    public BURSTIO.BurstSRI getSRI () {
        return this.sri_;
    }

    public boolean blockOccurred () {
        return this.blockOccurred_;
    }

    protected BurstPacket(boolean eos, BURSTIO.BurstSRI sri, BULKIO.PrecisionUTCTime time, boolean blockOccurred) {
        this.eos_ = eos;
        this.sri_ = sri;
        this.time_ = time;
        this.blockOccurred_ = blockOccurred;
    }

    private boolean eos_;
    private BURSTIO.BurstSRI sri_;
    private BULKIO.PrecisionUTCTime time_;
    private boolean blockOccurred_;
}
