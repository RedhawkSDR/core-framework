/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

package org.ossie.buffer;

import java.util.BitSet;

public class bitbuffer extends BitSet {

    //
    // bitbuffer implementation
    //

    // Declare npos here so that storage is allocated for it, even though it most
    // cases it is used directly as a constant; otherwise, some uses may fail to
    // link.
    public int length;

    public bitbuffer()
    {
        super();
        length = 0;
    }

    public bitbuffer(int nbits)
    {
        super(nbits);
        length = nbits;
    }

    public set(int bitlocation) {
        super.set(bitlocation);
        if (bitlocation > this.length) {
            length = bitlocation;
        }
    }
};