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

import java.util.HashSet;
import java.util.Set;
import BULKIO.StreamSRI;
import BULKIO.PrecisionUTCTime;

public class SriMapStruct {
        public SriMapStruct(StreamSRI sri, PrecisionUTCTime time) {
            this.sri = sri;
            this.connections = new HashSet<String>();
            this.time = time; 
        }

        public SriMapStruct(StreamSRI sri, Set<String> connections) {
            this.sri = sri;
            this.connections = connections;
            this.time = null;
        }

        public SriMapStruct(StreamSRI sri, Set<String> connections, PrecisionUTCTime time) {
            this.sri = sri;
            this.connections = connections;
            this.time = time;
        }

        public SriMapStruct(StreamSRI sri) {
            this.sri = sri;
            this.connections = new HashSet<String>(); 
            this.time = null;
        }
    
        public SriMapStruct() {
            this.sri = bulkio.sri.utils.create();
            this.connections = new HashSet<String>();
            this.time = null;
        }

        public StreamSRI sri;
        public Set<String> connections;
        public PrecisionUTCTime time;
};
