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

package org.ossie.redhawk;
import java.util.List;
import java.util.ListIterator;

public class PortCallError extends Exception {
    public PortCallError(String msg, List<String> connectionids) {
        super(createPortCallErrorMessage(msg, connectionids));
    }

    private static String createPortCallErrorMessage(String msg, List<String> connectionids) {
        String _msg = msg;
        if (connectionids.size() > 0) {
            _msg += "Connections available: ";
            for (ListIterator<String> iter = connectionids.listIterator(); iter.hasNext(); ) {
                _msg += iter.next();
                if (iter.hasNext()) {
                    _msg += ", ";
                }
            }
        }
        return _msg;
    }
}
