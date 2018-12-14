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

import BULKIO.StreamSRI;
import BULKIO.VITA49StreamDefinition;
import BULKIO.dataVITA49Operations;
import bulkio.vita49.VITA49StreamAttachment;
import bulkio.OutVITA49Port;
import java.util.Iterator;
import java.util.Set;
import java.util.HashSet;
import java.util.Arrays;
import java.util.ArrayList;
import BULKIO.dataVITA49Package.AttachError;
import BULKIO.dataVITA49Package.DetachError;
import BULKIO.dataVITA49Package.StreamInputError;
import BULKIO.PrecisionUTCTime;
import org.apache.log4j.Logger;

//
// Streams represent the flow of a single stream
//
public class VITA49Stream {
        public VITA49Stream()  {
            this(null, null, null, null, null, null, null);
        }

        public VITA49Stream(BULKIO.VITA49StreamDefinition streamDef)  {
            this(streamDef, null , streamDef.id, null, null, null, null);
        }

        public VITA49Stream(BULKIO.VITA49StreamDefinition streamDef, String name)  {
            this(streamDef, name, streamDef.id, null, null, null, null);
        }

        public VITA49Stream(BULKIO.VITA49StreamDefinition streamDef, String name, String streamId)  {
            this(streamDef, name, streamId, null, null, null, null);
            logger = null;
        }

    public VITA49Stream(BULKIO.VITA49StreamDefinition streamDef, String name, String streamId, VITA49StreamAttachment[] streamAttachments, StreamSRI sri, PrecisionUTCTime time, OutVITA49Port bport )  {
            this.streamDef = streamDef;
            this.name = name;
            this.streamId = streamId;
            if (streamAttachments != null){
                this.streamAttachments = new HashSet<VITA49StreamAttachment>(Arrays.asList(streamAttachments));
            }else{
                this.streamAttachments = new HashSet<VITA49StreamAttachment>();
            }
            this.sri = sri;
            this.time = time;
            this.bio_port = bport;
        }

        // detach all attachments with given attachId and connectionId for this stream
        public void detachByAttachIdConnectionId(String attachId, String connectionId) throws DetachError, StreamInputError {
            Iterator<VITA49StreamAttachment> iterator = this.streamAttachments.iterator();
            while(iterator.hasNext()){
                VITA49StreamAttachment nextAttachment = iterator.next();
                if (nextAttachment.connectionId.equals(connectionId) && 
                   nextAttachment.attachId != null &&
                   nextAttachment.attachId.equals(attachId)){
                    nextAttachment.inputPort.detach(nextAttachment.attachId);
                    iterator.remove(); 
                }
            }
        }

        // detach all attachments with given connectionId for this stream
        public void detachByConnectionId(String connectionId) throws DetachError, StreamInputError {
            Iterator<VITA49StreamAttachment> iterator = this.streamAttachments.iterator();
            while(iterator.hasNext()){
                VITA49StreamAttachment nextAttachment = iterator.next();
                if (nextAttachment.connectionId.equals(connectionId) && nextAttachment.attachId != null){
                    nextAttachment.detach();
                    iterator.remove(); 
                }
            }
        }

        public void detachByAttachId(String attachId) throws DetachError, StreamInputError {
            Iterator<VITA49StreamAttachment> iterator = this.streamAttachments.iterator();
            while(iterator.hasNext()){
                VITA49StreamAttachment nextAttachment = iterator.next();
                if (nextAttachment.attachId != null && nextAttachment.attachId.equals(attachId)){
                    nextAttachment.detach();
                    iterator.remove(); 
                }
            }
        }

        // detach all attachments for this stream
        public void detachAll() throws DetachError, StreamInputError {
            Iterator<VITA49StreamAttachment> iterator = this.streamAttachments.iterator();
            while(iterator.hasNext()){
                VITA49StreamAttachment nextAttachment = iterator.next();
                if (nextAttachment.attachId != null){
                    nextAttachment.detach();
                    iterator.remove(); 
                }
            }
        }

        public void createNewAttachment(String connectionId, dataVITA49Operations inputPort) throws AttachError, StreamInputError {
            VITA49StreamAttachment newAttachment = new VITA49StreamAttachment(connectionId, inputPort, this.bio_port);
            try {
                newAttachment.attachId = newAttachment.inputPort.attach(this.streamDef, this.name);
                if ( this.bio_port != null ) {
                    this.bio_port.updateStats( connectionId );
                }

            } catch( AttachError e ) {
                throw e;
            } catch( StreamInputError e ) {
                throw e;
            } catch( Exception e ) {
                String msg = " Unable to create attachment for CONNECTION: " + connectionId;
                if ( this.bio_port != null ) {
                    this.bio_port.reportConnectionErrors( connectionId, msg );
                }
           }
            this.streamAttachments.add(newAttachment);
        }

    public void createNewAttachment(String connectionId, dataVITA49Operations inputPort, OutVITA49Port bio_port ) throws AttachError, StreamInputError {
            VITA49StreamAttachment newAttachment = new VITA49StreamAttachment(connectionId, inputPort, bio_port);
            try {
                newAttachment.attachId = newAttachment.inputPort.attach(this.streamDef, this.name);
                if ( bio_port != null ) {
                     bio_port.updateStats( connectionId );
                }

            } catch( AttachError e ) {
                throw e;
            } catch( StreamInputError e ) {
                throw e;
            } catch( Exception e ) {
                String msg = " Unable to create attachment for CONNECTION: " + connectionId;
                if ( bio_port != null ) {
                   bio_port.reportConnectionErrors( connectionId, msg );
                }
           }
            this.streamAttachments.add(newAttachment);
        }

        public String[] getAttachIds(){
            ArrayList<String> attachIdList = new ArrayList<String>();
            Iterator<VITA49StreamAttachment> iterator = this.streamAttachments.iterator();
            while(iterator.hasNext()){
                VITA49StreamAttachment nextAttachment = iterator.next();
                attachIdList.add(nextAttachment.attachId);
            }
            return attachIdList.toArray(new String[0]);
        }

        public BULKIO.VITA49StreamDefinition getStreamDefinition(){
            return this.streamDef;
        }

        public String getStreamId(){
            return this.streamId;
        }

        public String getName(){
            return this.name;
        }

        public StreamSRI getSRI(){
            return this.sri;
        }

        public PrecisionUTCTime getTime(){
            return this.time;
        }

        public Set<String> getConnectionIds() {
            Set<String> connIds = new HashSet<String>();
            Iterator<VITA49StreamAttachment> iterator = this.streamAttachments.iterator();
            while (iterator.hasNext()){
                connIds.add(iterator.next().getConnectionId());
            }
            return connIds;
        }

        public void setStreamDefinition(BULKIO.VITA49StreamDefinition def){
            this.streamDef = def;
        }

        public void setStreamId(String id){
            this.streamId = id;
        }

        public void setName(String name){
            this.name = name;
        }

        public void setSRI(StreamSRI sri){
            this.sri = sri;
        }

        public void setTime(PrecisionUTCTime time){
            this.time = time;
        }

        public boolean hasAttachId(String attachId){
            Iterator<VITA49StreamAttachment> iterator = this.streamAttachments.iterator();
            while(iterator.hasNext()){
                VITA49StreamAttachment nextAttachment = iterator.next();
                if (nextAttachment.attachId != null && nextAttachment.attachId.equals(attachId)){
                    return true;
                }
            }
            return false;
        }

        public boolean hasConnectionId(String connectionId){
            Iterator<VITA49StreamAttachment> iterator = this.streamAttachments.iterator();
            while(iterator.hasNext()){
                VITA49StreamAttachment nextAttachment = iterator.next();
                if (nextAttachment.connectionId != null && nextAttachment.connectionId.equals(connectionId)){
                    return true;
                }
            }
            return false;
        }

        public boolean isValid(){
            if (this.streamDef != null && this.streamId != null && !this.streamId.isEmpty()){
               return true;
            }else{
               return false;
            }
        }

        public VITA49StreamAttachment[] findAttachmentsByAttachId(String attachId){
            ArrayList<VITA49StreamAttachment> streamAttList = new ArrayList<VITA49StreamAttachment>();
            if (this.streamAttachments != null){
                Iterator<VITA49StreamAttachment> iterator = this.streamAttachments.iterator();
                while (iterator.hasNext()){
                    VITA49StreamAttachment nextAttachment = iterator.next();
                    if (nextAttachment.attachId != null && nextAttachment.attachId.equals(attachId)){
                       streamAttList.add(nextAttachment);
                    }
                }
            }
            return streamAttList.toArray(new VITA49StreamAttachment[0]);
        }

        public VITA49StreamAttachment[] findAttachmentsByConnectionId(String connectionId){
            ArrayList<VITA49StreamAttachment> streamAttList = new ArrayList<VITA49StreamAttachment>();
            if (this.streamAttachments != null){
                Iterator<VITA49StreamAttachment> iterator = this.streamAttachments.iterator();
                while (iterator.hasNext()){
                    VITA49StreamAttachment nextAttachment = iterator.next();
                    if (nextAttachment.connectionId != null && nextAttachment.connectionId.equals(connectionId)){
                       streamAttList.add(nextAttachment);
                    }
                }
            }
            return streamAttList.toArray(new VITA49StreamAttachment[0]);
        }

        public void updateAttachments(VITA49StreamAttachment[] expectedAttachments) throws DetachError, AttachError, StreamInputError {
            Set<String> expectedConnectionIds = new HashSet<String>();
            Set<String> connectionsToRemove = new HashSet<String>();

            // Add new attachments that do not already exist
            for (VITA49StreamAttachment att: expectedAttachments){
                if (!this.hasConnectionId(att.getConnectionId())){
                    this.createNewAttachment(att.getConnectionId(), att.getInputPort(), this.bio_port );
                }
                expectedConnectionIds.add(att.getConnectionId());
            }

            // Remove unnecessary attachments
            // Iterate through attachments and compare to expected connectionIds
            if (this.streamAttachments != null){
                Iterator<VITA49StreamAttachment> streamAttIter = this.streamAttachments.iterator();
                while (streamAttIter.hasNext()){
                    VITA49StreamAttachment nextAttachment = streamAttIter.next();
                    String existingConnectionId = nextAttachment.getConnectionId();

                    boolean detachConnection = true;
                    Iterator<String> expectedConnIdIter = expectedConnectionIds.iterator();
                    while (expectedConnIdIter.hasNext()){
                        if (existingConnectionId.equals(expectedConnIdIter.next())){
                            detachConnection = false;
                            break;
                        }
                    }
                    if (detachConnection){
                        connectionsToRemove.add(existingConnectionId);
                    }
                }
            }
            for (String connId: connectionsToRemove){
                this.detachByConnectionId(connId);
            }
        }

        public void setLogger( Logger newlogger ){
            logger = newlogger;
        }

        protected BULKIO.VITA49StreamDefinition streamDef;
        protected String name;
        protected String streamId;
        protected Set<VITA49StreamAttachment> streamAttachments;
        protected StreamSRI sri;
        protected PrecisionUTCTime time; 
        protected Logger logger;
        protected OutVITA49Port  bio_port = null;
};
