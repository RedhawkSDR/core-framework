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
package bulkio.vita49;

import BULKIO.dataVITA49Operations;
import BULKIO.dataVITA49Package.DetachError;
import BULKIO.dataVITA49Package.StreamInputError;

//
// Stream Attachment represents a single port attachment
//
public class VITA49StreamAttachment {
        public VITA49StreamAttachment() {
            this(null, null, null);
        }

        public VITA49StreamAttachment(String connectionId, dataVITA49Operations inputPort) {
            this(connectionId, null, inputPort);
        }

        public VITA49StreamAttachment(String connectionId, String attachId, dataVITA49Operations inputPort) {
            this.connectionId = connectionId;
            this.attachId = attachId;
            this.inputPort = inputPort;
        }

        public void detach() throws DetachError,StreamInputError {
            if (this.attachId != null && !this.attachId.isEmpty()){
                this.inputPort.detach(attachId);
                this.attachId = null;
            }
        }

        public String getConnectionId(){
            return this.connectionId;
        }

        public String getAttachId(){
            return this.attachId;
        }

        public dataVITA49Operations getInputPort(){
            return this.inputPort;
        }

        public void setConnectionId(String id){
            this.connectionId = id;
        }

        public void setAttachId(String id){
            this.attachId = id;
        }

        public void setInputPort(dataVITA49Operations port){
            this.inputPort = port;
        }

        protected String connectionId;
        protected String attachId;
        protected dataVITA49Operations inputPort;
};
