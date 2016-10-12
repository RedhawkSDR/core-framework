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

package bulkio.sri;

import org.ossie.properties.AnyUtils;
import BULKIO.StreamSRI;

public class DefaultComparator implements bulkio.sri.Comparator {

    public boolean compare(StreamSRI SRI_1, StreamSRI SRI_2){
	
	if ( SRI_1 == null || SRI_2 == null ) 
	    return false;
        if (SRI_1.hversion != SRI_2.hversion)
            return false;
        if (SRI_1.xstart != SRI_2.xstart)
            return false;
        if (SRI_1.xdelta != SRI_2.xdelta)
            return false;
        if (SRI_1.xunits != SRI_2.xunits)
            return false;
        if (SRI_1.subsize != SRI_2.subsize)
            return false;
        if (SRI_1.ystart != SRI_2.ystart)
            return false;
        if (SRI_1.ydelta != SRI_2.ydelta)
            return false;
        if (SRI_1.yunits != SRI_2.yunits)
            return false;
        if (SRI_1.mode != SRI_2.mode)
            return false;
        if (SRI_1.blocking != SRI_2.blocking)
            return false;
        if (!SRI_1.streamID.equals(SRI_2.streamID))
            return false;
        if (SRI_1.keywords == null || SRI_2.keywords == null )
	    return false;
        if (SRI_1.keywords.length != SRI_2.keywords.length)
            return false;
        String action = "eq";
        for (int i=0; i < SRI_1.keywords.length; i++) {
            if (!SRI_1.keywords[i].id.equals(SRI_2.keywords[i].id)) {
                return false;
            }
            if (!SRI_1.keywords[i].value.type().equivalent(SRI_2.keywords[i].value.type())) {
                return false;
            }
            if (!AnyUtils.compareAnys(SRI_1.keywords[i].value, SRI_2.keywords[i].value, action)) {
                return false;
            }
        }
        return true;
    } 

}


