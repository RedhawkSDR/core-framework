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

import java.util.HashMap;
import java.util.Map;
import bulkio.vita49.VITA49Stream;
import bulkio.vita49.VITA49StreamAttachment;
import bulkio.SriMapStruct;
import bulkio.OutVITA49Port;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Arrays;
import BULKIO.dataVITA49Package.AttachError;
import BULKIO.dataVITA49Package.DetachError;
import BULKIO.dataVITA49Package.StreamInputError;
import BULKIO.VITA49StreamDefinition;
import BULKIO.dataVITA49Operations;
import BULKIO.StreamSRI;
import BULKIO.PrecisionUTCTime;
import org.apache.log4j.Logger;

//
// StreamContainer is a unique list of Stream objects
//
public class VITA49StreamContainer {
        public VITA49StreamContainer() {
            this.streamMap = new HashMap<String,VITA49Stream>();
            this.logger = null;
            this.bio_port = null;
        }

        public VITA49StreamContainer(VITA49Stream[] streams) {
            this.streamMap = new HashMap<String,VITA49Stream>();
            for(VITA49Stream s : streams ){
                this.streamMap.put(s.streamId, s);
            }
            this.bio_port = null;
        }

        public VITA49StreamContainer(OutVITA49Port bport) {
            this.streamMap = new HashMap<String,VITA49Stream>();
            this.logger = null;
            this.bio_port = bport;
        }

        public void printState(String title){
            ArrayList<VITA49Stream> streamList = new ArrayList<VITA49Stream>();
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream stream = entry.getValue();
                printBlock("Stream", stream.streamId,0);
                if (stream.streamAttachments != null){
                    Iterator<VITA49StreamAttachment> iterator = stream.streamAttachments.iterator();
                    while (iterator.hasNext()){
                        VITA49StreamAttachment nextAttachment = iterator.next();
                        if (nextAttachment.attachId != null){
                            printBlock("Attachment",nextAttachment.attachId, 1);
                        }
                    }
                }
            } 
            if (logger != null){
                logger.debug("");
            }
        }

        public void printBlock(String title, String id, int indents){
            String indent = "";
            String line = "----------------------";
            if (indents > 0){
                indent = "    ";
            }
            for (int ii=0; ii< indents; ii++){
                indent += indent;
            }

            if (logger != null){
                logger.debug(indent + " |" + line);
                logger.debug(indent + " |" + title);
                logger.debug(indent + " |   '" + id + "'");
                logger.debug(indent + " |" + line);
            }
        } 

        public void addStream(VITA49Stream s){
            this.streamMap.put(s.streamId,s); 
        }

        public void removeStreamByStreamId(String streamId) throws DetachError, StreamInputError {
            VITA49Stream s = this.findByStreamId(streamId); 
            if (s != null){
                s.detachAll();
            }
            this.streamMap.remove(streamId);
        }

        public VITA49Stream[] findByAttachId(String attachId){
            ArrayList<VITA49Stream> streamList = new ArrayList<VITA49Stream>();
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream stream = entry.getValue();
                if (stream.streamAttachments != null){
                    Iterator<VITA49StreamAttachment> iterator = stream.streamAttachments.iterator();
                    while (iterator.hasNext()){
                        VITA49StreamAttachment nextAttachment = iterator.next();
                        if (nextAttachment.attachId != null && nextAttachment.attachId.equals(attachId)){
                            streamList.add(stream);
                        }
                    }
                }
            }
            return streamList.toArray(new VITA49Stream[0]); 
        }

        public VITA49Stream findByStreamId(String streamId){
            return this.streamMap.get(streamId);
        }

        public VITA49Stream[] findByConnectionId(String connectionId){
            ArrayList<VITA49Stream> streamList = new ArrayList<VITA49Stream>();
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream stream = entry.getValue();
                if (stream.streamAttachments != null){
                    Iterator<VITA49StreamAttachment> iterator = stream.streamAttachments.iterator();
                    while (iterator.hasNext()){
                        VITA49StreamAttachment nextAttachment = iterator.next();
                        if (nextAttachment.connectionId != null && nextAttachment.connectionId.equals(connectionId)){
                            streamList.add(stream);
                        }
                    }
                }
            }
            return streamList.toArray(new VITA49Stream[0]);
        }

        public void createNewAttachmentForAllStreams(String connectionId, dataVITA49Operations inputPort ) throws AttachError,StreamInputError {
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream stream = entry.getValue();
                if (stream != null){
                    stream.createNewAttachment(connectionId, inputPort, this.bio_port );
                }
            }
        }

        // detach all attachments for all streams
        public void detach() throws DetachError, StreamInputError{
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream stream = entry.getValue();
                if (logger != null){
                    logger.debug("VITA49StreamContainer:detach() calling detachAll for streamId  " + stream.getStreamId());
                }
                stream.detachAll();
            }
        }

        public void detachByConnectionId(String connectionId) throws DetachError, StreamInputError{
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream stream = entry.getValue();
                try{
                    if (stream.hasConnectionId(connectionId)){
                        stream.detachByConnectionId(connectionId);
                    } 
                } catch (DetachError e){
                    if (logger != null){
                        logger.warn("VITA49StreamContainer:detachByConnectionId() DetachError UNABLE TO DETACH CONNECTIONID: " + connectionId);
                    }
                    throw e;
               } catch (StreamInputError e){
                    if (logger != null){
                        logger.warn("VITA49StreamContainer:detachByConnectionId() StreamInputError UNABLE TO DETACH CONNECTIONID: " + connectionId);
                    }
                    throw e;
                }
            }
        }

        public void detachByAttachId(String attachId) throws DetachError, StreamInputError{
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream stream = entry.getValue();
                try{
                    stream.detachByAttachId(attachId);
                } catch (DetachError e){
                    if (logger != null){
                        logger.warn("VITA49StreamContainer:detachByAttachId() DetachError UNABLE TO DETACH ATTACHID: " + attachId);
                    }
                    throw e;
                } catch (StreamInputError e){
                    if (logger != null){
                        logger.warn("VITA49StreamContainer:detachByAttachId() StreamInputError UNABLE TO DETACH ATTACHID: " + attachId);
                    }
                    throw e;
                }
            }
        }

        public void detachByAttachIdConnectionId(String attachId, String connectionId) throws DetachError, StreamInputError{
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream stream = entry.getValue();
                try{
                    stream.detachByAttachIdConnectionId(attachId, connectionId);
                    if (logger != null){
                        logger.debug("VITA49StreamContainer:detachByAttachIdConnectionId() DETACH COMPLETED ID:" + attachId + "/" + connectionId);
                    }
                } catch (DetachError e){
                    if (logger != null){
                        logger.warn("VITA49StreamContainer:detachByAttachIdConnectionId() Detach Error UNABLE TO DETACH ATTACHID/CONNECTIONID: " + attachId + "/" + connectionId);
                    }
                    throw e;
                } catch (StreamInputError e){
                    if (logger != null){
                        logger.warn("VITA49StreamContainer:detachByAttachIdConnectionId() StreamInputError UNABLE TO DETACH ATTACHID/CONNECTIONID: " + attachId + "/" + connectionId);
                    }
                    throw e;
                }
            }
        }

        public void detachAllStreams() throws DetachError, StreamInputError{
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream stream = entry.getValue();
                try{
                    stream.detachAll();
                    if (logger != null){
                        logger.debug("VITA49StreamContainer:detachAllStreams() DETACH ALL COMPLETED ");
                    }
                } catch (DetachError e){
                    if (logger != null){
                        logger.debug("VITA49StreamContainer:detachAllStreams() DetachError UNABLE TO DETACH ALL");
                    }
                    throw e;
                } catch (StreamInputError e){
                    if (logger != null){
                        logger.debug("VITA49StreamContainer:detachAllStreams() StreamInputError UNABLE TO DETACH ALL");
                    }
                    throw e;
                }
            }
        }

        public VITA49StreamAttachment[] findStreamAttachmentsByConnectionId(String connectionId){
            ArrayList<VITA49StreamAttachment> streamAttList = new ArrayList<VITA49StreamAttachment>();
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream stream = entry.getValue();
                if (stream.streamAttachments != null){
                    Iterator<VITA49StreamAttachment> iterator = stream.streamAttachments.iterator();
                    while (iterator.hasNext()){
                        VITA49StreamAttachment nextAttachment = iterator.next();
                        if (nextAttachment.connectionId != null && nextAttachment.connectionId.equals(connectionId)){
                            streamAttList.add(nextAttachment);
                        }
                    }
                }
            }
            return streamAttList.toArray(new VITA49StreamAttachment[0]);
        }

        public VITA49StreamAttachment[] findStreamAttachmentsByAttachId(String attachId){
            ArrayList<VITA49StreamAttachment> streamAttList = new ArrayList<VITA49StreamAttachment>();
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream stream = entry.getValue();
                if (stream.streamAttachments != null){
                    Iterator<VITA49StreamAttachment> iterator = stream.streamAttachments.iterator();
                    while (iterator.hasNext()){
                        VITA49StreamAttachment nextAttachment = iterator.next();
                        if (nextAttachment.attachId != null && nextAttachment.attachId.equals(attachId)){
                            streamAttList.add(nextAttachment);
                        }
                    }
                }
            }
            return streamAttList.toArray(new VITA49StreamAttachment[0]);
        }

        public BULKIO.VITA49StreamDefinition[] getCurrentStreamDefinitions(){
            ArrayList<BULKIO.VITA49StreamDefinition> streamDefList = new ArrayList<BULKIO.VITA49StreamDefinition>();
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                streamDefList.add(entry.getValue().streamDef);
            }
            return streamDefList.toArray(new BULKIO.VITA49StreamDefinition[0]);
        }

        public VITA49Stream[] getStreams(){
            ArrayList<VITA49Stream> streamList = new ArrayList<VITA49Stream>();
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                streamList.add(entry.getValue());
            }
            return streamList.toArray(new VITA49Stream[0]);
        }
     
        public boolean hasStreams(){
            if (this.streamMap.size() > 0){
                return true;
            }else{
                return false;
            }
        }

        public boolean hasStreamId(String streamId){
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                if (entry.getValue().getStreamId().equals(streamId)){
                    return true;
                }
            }
           return false; 
        }

        public String[] getStreamIds(){
            ArrayList<String> streamIdList = new ArrayList<String>();
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                streamIdList.add(entry.getValue().getStreamId());
            }
            return streamIdList.toArray(new String[0]);
        }

        public String[] getAttachmentIds(){
            ArrayList<String> attachIdList = new ArrayList<String>();
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                attachIdList.addAll(Arrays.asList(entry.getValue().getAttachIds()));
            }
            return attachIdList.toArray(new String[0]);
        }

        public void addConnectionToAllStreams(String connectionId, dataVITA49Operations inputPort) throws DetachError, AttachError, StreamInputError {
            if (logger != null){
                logger.trace("VITA49StreamContainer:addConnectionToAllStreams() for connectionId " + connectionId);
            }
            if (this.streamMap.size() < 1){
                if (this.logger != null){
                    this.logger.trace("VITA49StreamContainer:addConnectionToAllStreams() NO STREAMS DEFINED. NO ATTACHMENTS WERE MADE");
                }
            }
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream s = entry.getValue();
                if (!s.hasConnectionId(connectionId)){
                    if (logger != null){
                        logger.trace("VITA49StreamContainer:addConnectionToAllStreams() calling createNewAttachment for streamId  " + s.streamId);
                    }
                    s.createNewAttachment(connectionId, inputPort, this.bio_port );
                }
            }
        }

        public void addConnectionToStream(String connectionId, dataVITA49Operations inputPort, String streamId) throws DetachError, AttachError, StreamInputError {
            if (logger != null){
                logger.trace("VITA49StreamContainer:addConnectionToStream() for connectionId " + connectionId + " and streamId " + streamId);
            }
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream s = entry.getValue();
                if (s.streamId.equals(streamId)){
                    if (!s.hasConnectionId(connectionId)){
                        s.createNewAttachment(connectionId, inputPort, this.bio_port);
                    }
                }
            }
        }

        public void updateSRIForAllStreams(Map<String, SriMapStruct> sriMap) {
            if (logger != null){
                logger.trace("VITA49StreamContainer:updateSRIForAllStreams()");
            }
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream s = entry.getValue();
                SriMapStruct sriStruct = sriMap.get(s.streamId);
                if (sriStruct != null){
                    s.setSRI(sriStruct.sri);
                    s.setTime(sriStruct.time);
                }
            }
        }

        public void updateStreamSRI(String streamId, StreamSRI sri) {
            if (logger != null){
                logger.trace("VITA49StreamContainer:updateStreamSRI() for streamId " + streamId);
            }
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream s = entry.getValue();
                if (s.streamId.equals(streamId)){
                    s.setSRI(sri);
                }
            }
        }

        public void updateStreamTime(String streamId, PrecisionUTCTime time) {
            if (logger != null){
                logger.trace("VITA49StreamContainer:updateStreamTime() for streamId " + streamId);
            }
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream s = entry.getValue();
                if (s.streamId.equals(streamId)){
                    s.setTime(time);
                }
            }
        }

        public void updateStreamSRIAndTime(String streamId, StreamSRI sri, PrecisionUTCTime time) {
            if (logger != null){
                logger.trace("VITA49StreamContainer:updateStreamSRIAndTime() for streamId " + streamId);
            }
            for(Map.Entry<String, VITA49Stream > entry: this.streamMap.entrySet()) {
                VITA49Stream s = entry.getValue();
                if (s.streamId.equals(streamId)){
                    s.setSRI(sri);
                    s.setTime(time);
                }
            }
        }

        public void setLogger( Logger newlogger ){
            logger = newlogger;
        }

        protected Map<String, VITA49Stream> streamMap;
        protected Logger logger;
        protected OutVITA49Port bio_port;

};
