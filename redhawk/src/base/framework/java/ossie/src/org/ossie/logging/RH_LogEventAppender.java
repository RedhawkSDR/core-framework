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

package org.ossie.logging;

import java.lang.Integer;
import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;

import org.apache.log4j.AppenderSkeleton;
import org.apache.log4j.helpers.LogLog;
import org.apache.log4j.spi.Filter;
import org.apache.log4j.Layout;
import org.apache.log4j.spi.LoggingEvent;
import org.ossie.events.*;
import org.ossie.events.Manager.*;
import CF.LogEvent;
import CF.LogEventHelper;
import CF.LogEventHolder;
import org.ossie.component.Resource;

public class RH_LogEventAppender extends AppenderSkeleton  {

    //
    // Command line arguments used to configure orb
    //
    private String                                              _args;

    //
    // reconnection limits and retries...
    //
    private int                                                 _reconnect_retries=10;
    private int                                                 _reconnect_delay=10;

    //
    // channel name
    //
    private String                                              channelName;

    //
    // naming context 
    //
    private String                                              nameContext;

    //
    // Producer Identifier
    //
    private String                                              prodId = "RESOURCE.ID";

    //
    // Producer Name
    //
    private String                                              prodName="RESOURCE.NAME";

    //
    // Producer FQN - fully qualified domain name for resource
    //
    private String                                              prodFQN="RESOURCE.FQN";

    //
    // channel name, shadow variable
    //
    private String                                              _channelName;

    //
    // naming context, shadow variable
    //
    private String                                              _nameContext;

    //
    // Event Channel Manager that manages those resources for the Domain
    //
    private Manager                                             _ecm=null;
    
    //
    // Interface use to publish LogEvent messages to consumers
    //
    private Publisher                                           _pub=null; 

    // clean up event channel when appender is removed
    private int                                                 _cleanup_event_channel=0;

    // sychronization object
    private Object                                              updatingLock = null;

    public RH_LogEventAppender(){
        super();
        this.updatingLock = new Object();
    }

    public void activateOptions() {
        synchronized(this.updatingLock){
            if ( _channelName != channelName && channelName != "" ) {
                _channelName = channelName;
                _nameContext = nameContext;
                LogLog.debug("activate options :" + channelName );
                connect();
            }
            super.activateOptions();
        }
    }

    public void append(final LoggingEvent logRecord){
        synchronized(this.updatingLock){
            if (this.layout != null){

                if ( _pub != null ) {
                    LogLog.debug("Publish event to channel :" + channelName );
                    try {
                        final Any any = ORB.init().create_any();
                        int eventLogLevel = logging.ConvertLog4ToCFLevel(logRecord.getLevel());
                        long now =  System.currentTimeMillis()/1000;
                        String msg = layout.format(logRecord);
                        CF.LogEvent evt = new CF.LogEvent( prodId,
                                                           prodName,
                                                           prodFQN,
                                                           now,
                                                           eventLogLevel,
                                                           msg );
                        CF.LogEventHelper.insert( any, evt);
                        if ( _pub.push( any ) != 0 ) {
                            errorHandler.error("Unable to push message to channel:" + channelName );
                        }
                    }
                    catch( Throwable e ) {
                        errorHandler.error("Unable to push message to channel:" + channelName );
                    }
                
                }
                else {
                    if( _ecm != null ) {
                        errorHandler.error("No Publisher available for channel:" + channelName );
                    }
                }
            }
            else {
                errorHandler.error("No Layout set for the appender name [ " + name + " ].");
            }
        }
        return;
    }

    //
    // perform connect operation to establish a corba context 
    //
    public int setEventChannelManager( org.ossie.events.Manager ECM ) {
        _ecm = ECM;
        // if we are not connected then try to connect
        if ( _pub == null ) {
            LogLog.debug("Setting event channel manager  channel:" + channelName );
            return connect();
        }
        return 0;
    }

    //
    // perform connect operation to establish a corba context 
    //
    public int connect() {
        return connect(_ecm);
    }

    public int connect( org.ossie.events.Manager ECM ) {
        int retval = 0;

        LogLog.debug("Trying to Established Publisher for Channel:" + _channelName );
        if ( ECM != null )  {
            retval=-1;
            try {
                if ( _channelName != null && !_channelName.equals("") ) {
                    _pub = ECM.Publisher(_channelName);
                    retval=0;
                    LogLog.debug("Established Publisher for Channel:" + _channelName );
                }
            }
            catch( Manager.RegistrationExists e ){
            }
            catch( Manager.RegistrationFailed e ){
            }
        }
        else {
            /** RESOLVE 
                Get Event channel from org.ossie.corba.utils - getPublisher...
            **/
        }

        return retval;
    }

	 
    public void close()
    {
        LogLog.debug( "RH_LogEventAppender::close START");
        if ( closed ) return;
        
        if ( _pub != null ) {
            _pub.terminate();
        }
        _pub = null;

        
        closed=true;
        LogLog.debug( "RH_LogEventAppender::close END");
    }

    public boolean requiresLayout(){
        return true;
    }

    // set/get for event_channel or EVENT_CHANNEL
    public void setevent_channel(String value){
        synchronized(this.updatingLock){
            this.channelName = value;
        }
    }

    public String getevent_channel(){
        return this.channelName;
    }

    // set/get for name_context or NAME_CONTEXT
    public void setname_context(String value){
        synchronized(this.updatingLock){
            this.nameContext = value;
        }
    }

    public String getname_context(){
        return this.nameContext;
    }


    // set/get for producer_id or PRODUCER_ID
    public void setproducer_id(String value){
        synchronized(this.updatingLock){
            this.prodId = value;
        }
    }

    public String getproducer_id(){
        return this.prodId;
    }

    // set/get for producer_name or PRODUCER_NAME
    public void setproducer_name(String value){
        synchronized(this.updatingLock){
            this.prodName = value;
        }
    }

    public String getproducer_name(){
        return this.prodName;
    }


    // set/get for producer_fqn or PRODUCER_FQN
    public void setproducer_fqn(String value){
        synchronized(this.updatingLock){
            this.prodFQN = value;
        }
    }

    public String getproducer_fqn(){
        return this.prodFQN;
    }

    // set/get for argv or ARGV
    public void setargv(String value){
        synchronized(this.updatingLock){
            System.out.println("argv: " + value);
            this._args = value;
        }
    }

    public String getargv(){
        return this._args;
    }

    // set/get for remove_on_destroy or REMOVE_ON_DESTROY
    public void setremove_on_destroy(String value){
        synchronized(this.updatingLock){
            int nds = Integer.parseInt(value);
            if (nds == 0 || nds == 1){
                this._cleanup_event_channel = nds;
            }
        }
    }

    public int getremove_on_destroy(){
        return this._cleanup_event_channel;
    }

    // set/get for retries or RETRIES
    public void setretries(String value){
        synchronized(this.updatingLock){
            this._reconnect_retries = Integer.parseInt(value);
        }
    }

    public int getretries(){
        return this._reconnect_retries;
    }

    // set/get for retry_delay or RETRY_DELAY
    public void setretry_delay(String value){
        synchronized(this.updatingLock){
            this._reconnect_delay = Integer.parseInt(value);
        }
    }

    public int getretry_delay(){
        return this._reconnect_delay;
    }

}
