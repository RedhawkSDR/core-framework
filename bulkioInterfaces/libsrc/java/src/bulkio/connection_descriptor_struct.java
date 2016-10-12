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

import org.ossie.component.*;
import org.ossie.properties.*;

public class connection_descriptor_struct extends StructDef {
    public final StringProperty connection_id =
        new StringProperty(
            "connectionTable::connection_id", //id
            "connection_id", //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    public final StringProperty stream_id =
        new StringProperty(
            "connectionTable::stream_id", //id
            "stream_id", //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    public final StringProperty port_name =
        new StringProperty(
            "connectionTable::port_name", //id
            "port_name", //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
        public connection_descriptor_struct(String connection_id, String stream_id, String port_name) {
            this();
            this.connection_id.setValue(connection_id);
            this.stream_id.setValue(stream_id);
            this.port_name.setValue(port_name);
        }
    
        /**
         * @generated
         */
        public connection_descriptor_struct() {
            addElement(this.connection_id);
            addElement(this.stream_id);
            addElement(this.port_name);
        }
    
        public String getId() {
            return "connection_descriptor";
        }
};
