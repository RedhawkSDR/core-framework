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

package org.ossie.events;


import org.omg.CORBA.Any;
import org.omg.CosEventChannelAdmin.*;
import org.omg.PortableServer.POA;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.List;
import java.util.ArrayList;
import java.util.LinkedList;
import org.apache.log4j.Logger;
import org.ossie.corba.utils.*;
import org.ossie.component.*;
import org.ossie.redhawk.*;
import CF.EventChannelManager;
import CF.EventChannelManagerPackage.*;

/**
 * @generated
 */
public class Manager {

    final public static String IDM_CHANNEL_SPEC = "IDM_CHANNEL";
    final public static String ODM_CHANNEL_SPEC = "ODM_CHANNEL";

    public class RegistrationExists  extends Exception {
	
	public RegistrationExists( String cname ) {
	    channel_name = cname;
	    
	}
	
	public String channel_name;
    }

    public class RegistrationFailed  extends  Exception {
	
	public RegistrationFailed( String cname, String inReason ) {
	    channel_name = cname;
	    reason = inReason;
	    
	}
	
	public String channel_name;
	public String reason;
    }


    public class OperationFailed  extends  Exception {
	
	public OperationFailed( String msg ) {
	    msg=msg;
	}
	
	public String msg;
    }


    static public Manager GetManager( Resource obj ) throws OperationFailed {

	if ( _Manager == null ) {
            _Manager = new Manager( obj ) ;
	   _Manager._logger.debug( "Created EventManager for Resource: " + obj.identifier() );
	}

	return _Manager;
    }

    static public Manager GetManager( Service obj ) throws OperationFailed {

	if ( _Manager == null ) {
            _Manager = new Manager( obj ) ;
	   _Manager._logger.debug( "Created EventManager for Service: " + obj.getName() );
	}

	return _Manager;
    }

    static public void Terminate() {

	if ( _Manager != null ) {
	    _Manager._logger.debug( "Terminal all EventChannels ");
	    _Manager._terminate();
	}
    }

    class EM_Publisher extends Publisher {

	public EM_Publisher( Manager ecm, EventChannelReg creg ) 
	throws OperationNotAllowed
	{
	    super(creg.channel);
	    _ecm = ecm;
	    _creg = creg;
	}

	public void unregister( ) {
	    if ( _ecm != null ) _ecm.unregister(_creg);
	    _ecm=null;
	}

	private Manager          _ecm;
	private EventChannelReg  _creg;

    };


    class EM_Subscriber  extends Subscriber {

	public EM_Subscriber( Manager ecm, EventChannelReg creg ) 
	    throws OperationNotAllowed
	{
	    super(creg.channel);
	    _ecm = ecm;
	    _creg = creg;
	}

	public void unregister( ) {
	    if ( _ecm != null ) _ecm.unregister(_creg);
	    _ecm=null;
	    
	}

	private Manager          _ecm;
	private EventChannelReg  _creg;

    };


    //
    // Public Member Section
    //
    public Publisher Publisher( String cname )  
	throws RegistrationExists, RegistrationFailed
    {
	return this.Publisher( cname, "" );
    }

    public Publisher Publisher( String cname, String registrationId ) 
	throws RegistrationExists, RegistrationFailed
    {
	_logger.debug( "Requesting Publisher Interface for Channel:" + cname );

	EM_Publisher pub=null;

	synchronized(this) {

	    try {
		if ( _ecm != null  ) {
		    EventRegistration  ereg = new EventRegistration();
		    EventChannelReg    registration;
		
		    ereg.channel_name = cname;
		    ereg.reg_id = registrationId;
		
		    _logger.debug( "Requesting Channel:" + cname  + " from Domain's EventChannelManager " );
		    registration = _ecm.registerResource( ereg );
		    pub = new EM_Publisher( this, registration );
		
		    _logger.info( "Channel:" + cname  + " Reg-Id" + registration.reg.reg_id );
		    _registrations.add( registration );
		}
	    }
	    catch( CF.EventChannelManagerPackage.RegistrationAlreadyExists e) {  
                _logger.debug( "Exception RegistrationAlreadyExists" );
		throw new RegistrationExists(cname);
	    }
	    catch( CF.EventChannelManagerPackage.InvalidChannelName e) {  
                _logger.debug( "Exception Invalid channel name" );
		throw new RegistrationFailed(cname, "Invalid channel name." );
	    }
	    catch( CF.EventChannelManagerPackage.OperationFailed e) {  
                _logger.debug( "Exception Operation failed" );
		throw new RegistrationFailed(cname, "Operation failed." );
	    }
	    catch( CF.EventChannelManagerPackage.OperationNotAllowed e) {  
                _logger.debug( "Exception Operation Not Allowed" );
		throw new RegistrationFailed(cname, "Operation failed." );
	    }
	    catch( CF.EventChannelManagerPackage.ServiceUnavailable e) {  
                _logger.debug( "Exception Service Unavailable." );
		throw new RegistrationFailed(cname, "Service unavailable." );
	    }
	}
	return pub;

    }


    public Subscriber Subscriber( String cname ) 
	throws RegistrationExists, RegistrationFailed
    {
	return this.Subscriber(cname, "");
    }


    public Subscriber Subscriber( String cname, String registrationId ) 
	throws RegistrationExists, RegistrationFailed
    {

	_logger.debug( "Requesting Subscriber Interface for Channel:" + cname );

	EM_Subscriber sub=null;

	synchronized(this) {
	    try {
		if ( _ecm != null  ) {
		    EventRegistration  ereg = new EventRegistration();
		    EventChannelReg     registration;

		    ereg.channel_name = cname;
		    ereg.reg_id = registrationId;
		
		    _logger.debug( "Requesting Channel:" + cname  + " from Domain's EventChannelManager " );
		    registration = _ecm.registerResource( ereg );
		    sub = new EM_Subscriber( this, registration );
		
		    _logger.info( "Channel:" + cname  + " Reg-Id" + registration.reg.reg_id );
		    _registrations.add( registration );
		}
	    }
	    catch( CF.EventChannelManagerPackage.RegistrationAlreadyExists e) {  
		throw new RegistrationExists(cname);
	    }
	    catch( CF.EventChannelManagerPackage.InvalidChannelName e) {  
		throw new RegistrationFailed(cname, "Invalid channel name." );
	    }
	    catch( CF.EventChannelManagerPackage.OperationFailed e) {  
		throw new RegistrationFailed(cname, "Operation failed." );
	    }
	    catch( CF.EventChannelManagerPackage.OperationNotAllowed e) {  
		throw new RegistrationFailed(cname, "Operation failed." );
	    }
	    catch( CF.EventChannelManagerPackage.ServiceUnavailable e) {  
		throw new RegistrationFailed(cname, "Service unavailable." );
	    }
	}
	return sub;
    }


    //
    // Private Section
    //

    // singleton
    private static Manager              _Manager;

    private List<  EventChannelReg >    _registrations;

    private Logger                      _logger;

    // handle to Domain's EventChannelManager
    private EventChannelManager         _ecm;

    // allow event channel usage
    private boolean                     _allow;

    private POA                         _poa;

    private Manager ( Resource obj ) throws OperationFailed {

        _allow=true;
        _logger = Logger.getLogger("ossie::events::Manager");
        if ( obj != null  ){

            _poa = obj.getPoa();
            
            _logger.debug( "Resolve Device and Domain Managers...");
            // setup create publisher as internal methods
            org.ossie.redhawk.DomainManagerContainer dm = obj.getDomainManager();
            if ( dm == null ) {
                _logger.debug( "Domain Manager return null.......");
                throw new Manager.OperationFailed("Domain Manager access failed");
            }

            _logger.debug( "Resolve Domain Managers Reference...");
            CF.DomainManager  domMgr =  dm.getRef();
            if ( !org.ossie.corba.utils.objectExists( domMgr ) ) {
                _logger.debug( "Domain Manager reference is invalid.....");
                throw new OperationFailed("Domain Manager access failed");
            }

            _logger.debug( "Getting Event Channel Manager...");
            CF.EventChannelManager  ecm = domMgr.eventChannelMgr();
            if ( !org.ossie.corba.utils.objectExists( ecm ) ) {
                _logger.debug( "Event Channel Manager interface not available.....");
                throw new OperationFailed("Event Channel Manager access failed");
            }
            _ecm = ecm;
        }
        
        _registrations = new LinkedList< EventChannelReg >();
    }


    private Manager ( Service obj ) throws OperationFailed {

        _allow=true;
        _logger = Logger.getLogger("ossie::events::Manager");
        if ( obj != null  ){

            _poa = org.ossie.corba.utils.RootPOA();
            if ( _poa == null ) {
                _logger.debug( "Middleware unavailable .....");
                throw new Manager.OperationFailed("Middleware unavailable");
            }
            
            _logger.debug( "Resolve Device and Domain Managers...");
            // setup create publisher as internal methods
            org.ossie.redhawk.DomainManagerContainer dm = obj.getDomainManager();
            if ( dm == null ) {
                _logger.debug( "Domain Manager return null.......");
                throw new Manager.OperationFailed("Domain Manager access failed");
            }

            _logger.debug( "Resolve Domain Managers Reference...");
            CF.DomainManager  domMgr =  dm.getRef();
            if ( !org.ossie.corba.utils.objectExists( domMgr ) ) {
                _logger.debug( "Domain Manager reference is invalid.....");
                throw new OperationFailed("Domain Manager access failed");
            }

            _logger.debug( "Getting Event Channel Manager...");
            CF.EventChannelManager  ecm = domMgr.eventChannelMgr();
            if ( !org.ossie.corba.utils.objectExists( ecm ) ) {
                _logger.debug( "Event Channel Manager interface not available.....");
                throw new OperationFailed("Event Channel Manager access failed");
            }
            _ecm = ecm;
        }
        
        _registrations = new LinkedList< EventChannelReg >();
    }



    private void _terminate( ) {

	synchronized(this) {
	    _allow = false;

	    _logger.info( "Terminate All Registrations.: " + _registrations.size() );
	    for ( EventChannelReg creg: this._registrations ){
        
		if ( _ecm != null ) {
		    try {
			// unregister from the Domain
			_logger.info(  "Unregister REG=ID:" + creg.reg.reg_id );
			_ecm.unregister( creg.reg );
		    }
		    catch( ChannelDoesNotExist e ) {
			_logger.debug( "Event Channel does not exists,  REG=ID:" + creg.reg.reg_id );
		    }
		    catch( RegistrationDoesNotExist e ){
			_logger.debug( "Registration does not exists,  REG=ID:" + creg.reg.reg_id );
		    }
		    catch( ServiceUnavailable e ) {
			_logger.debug( "Event Service Unavailable, unable to UNREGISTER,  REG=ID:" + creg.reg.reg_id );
		    }
		}
	    }

	    // need to cleanup Publisher memory
	    _registrations.clear();

	    _logger.info( "Terminate Completed.");

	}

    }

    public void unregister ( EventChannelReg reg  ) {

	if (!_allow ) return;

	synchronized(this) {

	    _logger.info( "Terminate All Registrations.: " + _registrations.size() );
	    for ( EventChannelReg creg: this._registrations ){
        
		if ( creg.reg.reg_id.equals( reg.reg.reg_id )  ) {
		    try {
			// unregister from the Domain
			_logger.info(  "Unregister REG=ID:" + creg.reg.reg_id );
			_ecm.unregister( creg.reg );
			_registrations.remove( _registrations.indexOf(creg) );
		    }
		    catch( ChannelDoesNotExist e ) {
			_logger.debug( "Event Channel does not exists,  REG=ID:" + creg.reg.reg_id );
		    }
		    catch( RegistrationDoesNotExist e ){
			_logger.debug( "Registration does not exists,  REG=ID:" + creg.reg.reg_id );
		    }
		    catch( ServiceUnavailable e ) {
			_logger.debug( "Event Service Unavailable, unable to UNREGISTER,  REG=ID:" + creg.reg.reg_id );
		    }
		}
	    }

	    _logger.info( "Terminate Completed.");

	}

    }


    
};