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

package org.ossie.redhawk.time;

import java.lang.System;
import CF.UTCTime;

/**
 * @generated
 */
public class DefaultComparator implements org.ossie.redhawk.time.Comparator {

    public boolean compare(final UTCTime T1, final UTCTime T2) {
        if (T1.tcstatus != T2.tcstatus)
            return false;
        if (T1.tfsec != T2.tfsec)
            return false;
        if (T1.twsec != T2.twsec)
            return false;
        return true;
    }

}
