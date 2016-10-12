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
package burstio.traits;

import java.util.Collection;

public interface BurstTraits<B,A>
{
    public int byteSize();
    public int burstLength(B burst);
    public B[] toArray(Collection<B> bursts);
    public B createBurst(A data, BURSTIO.BurstSRI sri, BULKIO.PrecisionUTCTime timestamp, boolean eos);
    public BURSTIO.BurstSRI sri(B burst);
    public boolean eos(B burst);
}
