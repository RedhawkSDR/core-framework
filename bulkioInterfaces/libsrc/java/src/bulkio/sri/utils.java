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

import java.lang.System;
import BULKIO.StreamSRI;
import CF.DataType;

public class  utils {

    public static StreamSRI  create() {
	return create("defaultSRI", 1.0, (short)1, false );
    }

    public static StreamSRI  create( String sid, 
				     double srate, 
				     short xunits,
				     boolean blocking ) {

	StreamSRI tsri = new StreamSRI();
	tsri.streamID = sid;
	tsri.hversion =1;
	tsri.xstart = 0.0;
	if ( srate <= 0.0 ) {
	    tsri.xdelta = 1.0;
	}
	else {
	    tsri.xdelta = 1.0/srate;
	}
	tsri.xunits = xunits;
	tsri.subsize = 0;
	tsri.ystart = 0.0;
	tsri.ydelta = 0.0;
	tsri.yunits = 0;
	tsri.mode = 0;
	tsri.blocking=blocking;
	tsri.keywords = new DataType[0];

	return tsri;
    }
}
