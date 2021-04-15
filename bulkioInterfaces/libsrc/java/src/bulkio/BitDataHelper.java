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

import java.util.Arrays;
import org.ossie.buffer.bitbuffer;

class BitDataHelper implements DataHelper<bitbuffer> {
    public int bitSize() {
        return 1;
    }

    public int arraySize(bitbuffer data) {
        return data.length;
    }

    public boolean isEmpty(bitbuffer data)
    {
        return (data.length == 0);
    }

    public bitbuffer emptyArray() {
        bitbuffer array = new bitbuffer();
        return array;
    }

    public bitbuffer slice(bitbuffer data, int start, int end) {
        // Without a bit array API, limit slicing to byte boundaries
        if (start % 8 != 0) {
            throw new IllegalArgumentException("start index <" + start + "> is not byte-aligned");
        } else if (end % 8 != 0) {
            throw new IllegalArgumentException("end index <" + end + "> is not byte-aligned");
        }
        bitbuffer result = new bitbuffer();
        for (int i=0; i<end-start; i++) {
            if (data.get(i+start)) {
                result.set(i);
            }
        }
        result.length = end-start;
        return result;
    }
}
