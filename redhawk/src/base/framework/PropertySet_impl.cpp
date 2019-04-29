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


#include <iostream>

#include "ossie/ThreadedComponent.h"
#include "ossie/PropertySet_impl.h"
#include "ossie/CorbaUtils.h"
#include <ossie/prop_helpers.h>
#include "ossie/concurrent.h"
#include "ossie/Events.h"
#include "ossie/ossieSupport.h"
#include "ossie/PropertyMap.h"


//
// PropertyChangeThread 
// Performs monitoring activity for any property change registrations 
// assigned to a PropertySet
//

class PropertyChangeThread : public ThreadedComponent {
  friend class PropertySet_impl;
public:
  PropertyChangeThread( PropertySet_impl &p): 
    parent(p)
 {};
  virtual ~PropertyChangeThread() {};
  int serviceFunction() {
    return parent._propertyChangeServiceFunction();
  }
private:
  PropertySet_impl &parent;
};


// 
// EC_PropertyChangeListener
// Wrapper class that implements a notification for property change events via an EventChannel
//
class PropertySet_impl::EC_PropertyChangeListener : public PropertySet_impl::PropertyChangeListener {

public:
  EC_PropertyChangeListener( CORBA::Object_ptr obj );
   ~EC_PropertyChangeListener();
  int  notify( PropertyChangeRec *rec, CF::Properties &changes );

private:
  ossie::events::EventChannel_ptr     ec;               // event channel provided during PropertyChangeListener
  redhawk::events::PublisherPtr       pub;              // publisher interface for an EventChannel
};


// 
// INFO_PropertyChangeListener
// Wrapper class that implements a notification for property change events to a CORBA object that implements PropertyChange Interface
//
class PropertySet_impl::INF_PropertyChangeListener : public PropertySet_impl::PropertyChangeListener {

public:
  INF_PropertyChangeListener( CORBA::Object_ptr obj );
  ~INF_PropertyChangeListener()  {};
  int  notify( PropertyChangeRec *rec, CF::Properties &changes );

private:
  
  CORBA::Object_ptr               obj;                    // remote object that will receive notifications            
  CF::PropertyChangeListener_ptr  listener;               // actual object that is narrowed to PropertyChange Interface

};

std::string PropertySet_impl::PropertyChangeRec::RSC_ID("UNK_RSC_ID");

PREPARE_CF_LOGGING(PropertySet_impl);

PropertySet_impl::PropertySet_impl ():
  propertyChangePort(0),
  _propertyQueryTimestamp("QUERY_TIMESTAMP"),
  _propChangeThread( new PropertyChangeThread(*this), 0.1 ),
  _propertiesInitialized(false)
{
  
}

PropertySet_impl::~PropertySet_impl ()
{

    // clean up property change listener context
    _propChangeThread.stop();
    _propChangeThread.release();

   // Clean up all property wrappers created by descendents.
    for (std::vector<PropertyInterface*>::iterator ii = ownedWrappers.begin(); ii != ownedWrappers.end(); ++ii) {
        delete *ii;
    }

    // Clean up all property wrappers created by descendents.
    for ( PropertyMonitorTable::iterator ii = _propMonitors.begin(); ii != _propMonitors.end(); ++ii) {
      delete ii->second;
    }
    

    // Clean up all property callback functors.
    propCallbacks.clear();
}

void PropertySet_impl::setExecparamProperties(std::map<std::string, char*>& execparams)
{
    RH_TRACE(_propertysetLog, "Setting " << execparams.size() << " exec parameters");

    std::map<std::string, char*>::iterator iter;
    for (iter = execparams.begin(); iter != execparams.end(); iter++) {
        RH_TRACE(_propertysetLog, "Property: " << iter->first << " = "
                                              << iter->second);
        const std::string id = iter->first;
        PropertyInterface* property = getPropertyFromId(id);
        // the property can belong to a resource, device, or Device/Domain
        // Manager.  If the property is not found, then it might be a resource
        // property passed through the nodeBooter to the DeviceManager
        if (property) {
            CORBA::Any val = ossie::string_to_any(iter->second, property->type);
            property->setValue(val);
        } else {
            RH_WARN(_propertysetLog, "Property: " << id << " is not defined, ignoring it!!");
        }
    }
    RH_TRACE(_propertysetLog, "Done setting exec parameters");
}

void PropertySet_impl::setLogger(rh_logger::LoggerPtr logptr)
{
    _propertysetLog = logptr;
}

void PropertySet_impl::setCommandLineProperty(const std::string& id, const redhawk::Value& value)
{
    RH_TRACE(_propertysetLog, "Property: " << id << " = " << value.toString());
    PropertyInterface* property = getPropertyFromId(id);
    // the property can belong to a resource, device, or Device/Domain
    // Manager.  If the property is not found, then it might be a resource
    // property passed through the nodeBooter to the DeviceManager
    if (property) {
        property->setValue(value, false);
    } else {
        RH_WARN(_propertysetLog, "Property: " << id << " is not defined, ignoring it!!");
    }
}

void
PropertySet_impl::initializeProperties(const CF::Properties& ctorProps)
throw (CF::PropertyEmitter::AlreadyInitialized, CF::PropertySet::PartialConfiguration,
       CF::PropertySet::InvalidConfiguration, CORBA::SystemException)
{
    TRACE_ENTER(PropertySet_impl);
    boost::mutex::scoped_lock lock(propertySetAccess);

    // Disallow multiple calls
    if (_propertiesInitialized) {
        throw CF::PropertyEmitter::AlreadyInitialized();
    }
    _propertiesInitialized = true;

    CF::Properties invalidProperties;
    for (CORBA::ULong ii = 0; ii < ctorProps.length(); ++ii) {
        PropertyInterface* property = getPropertyFromId((const char*)ctorProps[ii].id);
        if (property && property->isProperty()) {
            RH_TRACE(_propertysetLog, "Constructor property: " << property->id);
            try {
                property->setValue(ctorProps[ii].value, false);
            } catch (std::exception& e) {
                RH_ERROR(_propertysetLog, "Setting property " << property->id << ", " << property->name << " failed.  Cause: " << e.what());
                ossie::corba::push_back(invalidProperties, ctorProps[ii]);
            } catch (CORBA::Exception& e) {
                RH_ERROR(_propertysetLog, "Setting property " << property->id << " failed.  Cause: " << e._name());
                ossie::corba::push_back(invalidProperties, ctorProps[ii]);
            }
        } else {
            ossie::corba::push_back(invalidProperties, ctorProps[ii]);
        }
    }


    if (invalidProperties.length () > 0) {
        if (invalidProperties.length() < ctorProps.length()) {
            throw CF::PropertySet::PartialConfiguration(invalidProperties);
        } else {
            throw CF::PropertySet::InvalidConfiguration("No matching properties found", invalidProperties);
        }
    }

    TRACE_EXIT(PropertySet_impl);
}



void
PropertySet_impl::configure (const CF::Properties& configProperties)
throw (CORBA::SystemException, CF::PropertySet::InvalidConfiguration,
       CF::PropertySet::PartialConfiguration)
{
    TRACE_ENTER(PropertySet_impl);
    boost::mutex::scoped_lock lock(propertySetAccess);

    int validProperties = 0;
    CF::Properties invalidProperties;

    for (CORBA::ULong ii = 0; ii < configProperties.length(); ++ii) {
        PropertyInterface* property = getPropertyFromId((const char*)configProperties[ii].id);
        if (property && property->isConfigurable()) {
            RH_TRACE(_propertysetLog, "Configure property: " << property->id);
            try {
                std::vector<std::string>::iterator kind = property->kinds.begin();
                bool sendEvent = false;
                bool eventType = false;
                if (propertyChangePort != NULL) {
                    // searching for event type
                    while (kind != property->kinds.end()) {
                        if (!kind->compare("event")) {
                            // it is of event type
                            eventType = true;
                            break;
                        }
                        kind++;
                    }
                    if (eventType) {
                        // comparing values
                        if (property->compare(configProperties[ii].value)) {
                            // the incoming value is different from the current value
                            sendEvent = true;
                        }
                    }
                }
                CORBA::Any before_value, after_value;
                property->getValue(before_value);
                property->setValue(configProperties[ii].value);
                property->getValue(after_value);
                std::string comparator("eq");
                if (ossie::compare_anys(before_value, after_value, comparator)) {
                    RH_TRACE(_propertysetLog, "Value has not changed on configure for property " << property->id << ". Not triggering callback");
                } else {
                    executePropertyCallback(property->id);
                    if (sendEvent) {
                        // sending the event
                        propertyChangePort->sendPropertyEvent(property->id);
                    }
                }
                ++validProperties;
            } catch (std::exception& e) {
                RH_ERROR(_propertysetLog, "Setting property " << property->id << ", " << property->name << " failed.  Cause: " << e.what());
                CORBA::ULong count = invalidProperties.length();
                invalidProperties.length(count + 1);
                invalidProperties[count].id = CORBA::string_dup(configProperties[ii].id);
                invalidProperties[count].value = configProperties[ii].value;
            } catch (CF::PropertySet::InvalidConfiguration& e) {
                throw;
            } catch (CORBA::Exception& e) {
                RH_ERROR(_propertysetLog, "Setting property " << property->id << " failed.  Cause: " << e._name());
                CORBA::ULong count = invalidProperties.length();
                invalidProperties.length(count + 1);
                invalidProperties[count].id = CORBA::string_dup(configProperties[ii].id);
                invalidProperties[count].value = configProperties[ii].value;
            }
        } else {
            CORBA::ULong count = invalidProperties.length();
            invalidProperties.length(count + 1);
            invalidProperties[count].id = CORBA::string_dup(configProperties[ii].id);
            invalidProperties[count].value = configProperties[ii].value;
        }
    }

    if (invalidProperties.length () > 0) {
        if (validProperties > 0) {
            throw CF::PropertySet::PartialConfiguration(invalidProperties);
        } else {
            throw CF::PropertySet::InvalidConfiguration("No matching properties found", invalidProperties);
        }
    }

    TRACE_EXIT(PropertySet_impl);
}


void
PropertySet_impl::query (CF::Properties& configProperties)
throw (CORBA::SystemException, CF::UnknownProperties)
{
    TRACE_ENTER(PropertySet_impl);
    boost::mutex::scoped_lock lock(propertySetAccess);

    // For queries of zero length, return all id/value pairs in propertySet.
    if (configProperties.length () == 0) {
        RH_TRACE(_propertysetLog, "Query all properties");
        PropertyMap::iterator jj = propTable.begin();
        for (CORBA::ULong ii = 0; ii < propTable.size(); ++ii) {
            if (jj->second->isQueryable()) {
                configProperties.length(configProperties.length()+1);
                configProperties[configProperties.length()-1].id = CORBA::string_dup(jj->second->id.c_str());
                if (jj->second->isNilEnabled()) {
                    if (jj->second->isNil()) {
                        configProperties[configProperties.length()-1].value = CORBA::Any();
                    } else {
                        jj->second->getValue(configProperties[configProperties.length()-1].value);
                    }
                } else {
                    jj->second->getValue(configProperties[configProperties.length()-1].value);
                }
            }
            ++jj;
        }
        /*configProperties.length(configProperties.length() + 1);
        configProperties[configProperties.length()-1].id = CORBA::string_dup(_propertyQueryTimestamp.c_str());
        configProperties[configProperties.length()-1].value <<= _makeTime(-1,0,0);*/
    } else {
        // For queries of length > 0, return all requested pairs in propertySet
        CF::Properties invalidProperties;

        // Returns values for valid queries in the same order as requested
        for (CORBA::ULong ii = 0; ii < configProperties.length (); ++ii) {
            const std::string id = (const char*)configProperties[ii].id;
            RH_TRACE(_propertysetLog, "Query property " << id);
            if (id == _propertyQueryTimestamp) {
                configProperties[ii].value <<= _makeTime(-1,0,0);
                continue;
            }
            PropertyInterface* property = getPropertyFromId(id);
            if (property && property->isQueryable()) {
                if (property->isNilEnabled()) {
                    if (property->isNil()) {
                        configProperties[ii].value = CORBA::Any();
                    } else {
                        property->getValue(configProperties[ii].value);
                    }
                } else {
                    property->getValue(configProperties[ii].value);
                }
            } else {
                CORBA::ULong count = invalidProperties.length();
                invalidProperties.length(count + 1);
                invalidProperties[count].id = CORBA::string_dup(configProperties[ii].id);
                invalidProperties[count].value = configProperties[ii].value;
            }
        }

        if (invalidProperties.length () != 0) {
            throw CF::UnknownProperties(invalidProperties);
        }
    }

    RH_TRACE(_propertysetLog, "Query returning " << configProperties.length() << " properties");

    TRACE_EXIT(PropertySet_impl);
}

char *PropertySet_impl::registerPropertyListener( CORBA::Object_ptr listener, const CF::StringSequence &prop_ids, const CORBA::Float interval) 
  throw(CF::UnknownProperties, CF::InvalidObjectReference)
{

  RH_TRACE(_propertysetLog, "Start RegisterListener");

  CF::Properties invalidProperties;
  int ii;
  CORBA::ULong count = invalidProperties.length();
  int len =  (int)prop_ids.length();
  PropertyReportTable props;
 
  // synchronize acces to the resource's properties
  SCOPED_LOCK(propertySetAccess);
  if ( len == 0 ) {
    // add all queryable properties...
    PropertyMap::iterator jj = propTable.begin();
    for (CORBA::ULong ii = 0; ii < propTable.size(); ++ii) {
      if (jj->second->isQueryable()) {
        RH_DEBUG(_propertysetLog, "RegisterListener: registering property id: " << jj->second->id);
        // Add callback to monitor changes to specified properties, use smart pointers for clean up
        props.insert( std::pair< std::string, PCL_CallbackPtr >( jj->second->id, 
                                                                 PCL_CallbackPtr( new PCL_Callback() ) ) );
      }
      jj++;
    }
  }
  else {
    for ( ii=0; ii < len; ii++ ) {
      // check for matching propids..
      PropertyInterface* property = getPropertyFromId((const char*)prop_ids[ii]);
      if (property && property->isQueryable()) {
        RH_DEBUG(_propertysetLog, "RegisterListener: registering property id: " << property->id);
        // Add callback to monitor changes to specified properties, use smart pointers for clean up
        props.insert( std::pair< std::string, PCL_CallbackPtr >(property->id, 
                                                                PCL_CallbackPtr( new PCL_Callback() ) ) );
      }
      else {
        count = invalidProperties.length();
        invalidProperties.length(count + 1);
        invalidProperties[count].id = CORBA::string_dup(prop_ids[ii]);
      }
    }
  }
  
  if ( count > 0 ){ 
    throw CF::UnknownProperties(invalidProperties);
  }

  RH_DEBUG(_propertysetLog, "RegisterListener: Determine listener type... ");
  // listener can be either an EventChannel or PropertyChangeListener 
  PropertyChangeListener *pcl=NULL;
  bool is_ec = false;
  try {
    RH_DEBUG(_propertysetLog, "RegisterListener: Checking for event channel....." );
    ossie::events::EventChannel_ptr ec = ossie::events::EventChannel::_narrow(listener);
    if ( !CORBA::is_nil(ec) ) {
      pcl = new EC_PropertyChangeListener(listener);
      is_ec = true;
    }
  }
  catch(...) {
    if ( pcl ) delete pcl;
    RH_DEBUG(_propertysetLog, "RegisterListener: Registrant not an event channel....." );
    // this ok... need to check additional types
  }
  
  if ( !is_ec ) {
    try {
      RH_DEBUG(_propertysetLog, "RegisterListener: Trying for PropertyChangeListener......." );
      pcl = new INF_PropertyChangeListener(listener);
    }
    catch(...) {
      if ( pcl ) delete pcl;
      // ok this is bad.. need to throw invalid object reference
      throw CF::InvalidObjectReference();
    }
  }

  // create a registration record
  PropertyChangeRec rec;
  std::string reg_id = ossie::generateUUID();
  rec.regId = reg_id;
  rec.rscId = PropertyChangeRec::RSC_ID;
  rec.listener = listener;
  long  sec=0;
  long  fsec=500;
  sec = (long)interval;
  fsec = (interval - sec)*1e6;
  rec.reportInterval = boost::posix_time::time_duration( 0, 0,sec,fsec);
  rec.expiration =  boost::posix_time::microsec_clock::local_time() + rec.reportInterval;
  rec.props = props;
  rec.pcl.reset(pcl);
  PropertyReportTable::iterator p = rec.props.begin();
  for ( ; p != rec.props.end(); p++ ) {
    RH_DEBUG(_propertysetLog, "RegisterListener: Setting Callback.... REG-ID:" << p->first << " FUNC:" << p->second );
    PropertyInterface *prop = getPropertyFromId(p->first);
    if ( prop ) {
      // check for matching propids..
     prop->addChangeListener( p->second, &PCL_Callback::recordChanged );
    }
  }

  RH_DEBUG(_propertysetLog, "RegisterListener: adding record.. ");
  RH_DEBUG(_propertysetLog, "RegisterListener .....  reg:" << rec.regId );
  RH_DEBUG(_propertysetLog, "RegisterListener .....  sec:" << sec );
  RH_DEBUG(_propertysetLog, "RegisterListener .....  fsec:" << fsec );
  RH_DEBUG(_propertysetLog, "RegisterListener .....  dur:" << rec.reportInterval.total_milliseconds() );

  // add  the registration record to our registry
  _propChangeRegistry.insert( std::pair< std::string, PropertyChangeRec >( reg_id, rec ) );

  //  enable monitoring thread...
  if ( !_propChangeThread.threadRunning()  ) _propChangeThread.start();

  RH_TRACE(_propertysetLog, "RegisterListener: End Registration");
  return CORBA::string_dup(reg_id.c_str() );

}

void PropertySet_impl::unregisterPropertyListener( const char *reg_id )   
      throw(CF::InvalidIdentifier)
{
  {
    SCOPED_LOCK(propertySetAccess);
    PropertyChangeRegistry::iterator reg = _propChangeRegistry.find(reg_id);
    if ( reg != _propChangeRegistry.end()  )  {
        PropertyChangeRec *rec = &(reg->second);
        // need to unregister callback with property
        PropertyReportTable::iterator p = rec->props.begin();
        for ( ; p != rec->props.end(); p++ ) {
            RH_DEBUG(_propertysetLog, "RegisterListener: Unregister callback...:" << p->first << " FUNC:" << p->second );
            PropertyInterface *prop = getPropertyFromId(p->first);
            if ( prop ) {
                // check for matching propids..
                prop->removeChangeListener( p->second, &PCL_Callback::recordChanged );
            }
        }
    } else {
        throw CF::InvalidIdentifier();
    }
    // remove registration record
    _propChangeRegistry.erase(reg);
  }

  if( _propChangeRegistry.size() == 0   ){
    _propChangeThread.stop();
    _propChangeThread.release();
  }
}



PropertyInterface* PropertySet_impl::getPropertyFromId (const std::string& id)
{
  PropertyMap::iterator property = propTable.find(id);
  if (property != propTable.end()) {
    return property->second;
  }
  return 0;
}

PropertyInterface* PropertySet_impl::getPropertyFromName (const std::string& name)
{
    for (PropertyMap::iterator property = propTable.begin(); property != propTable.end(); ++property) {
        if (name == property->second->name) {
            return property->second;
        }
    }

    return 0;
}

PropertyInterface* PropertySet_impl::getPropertyFromAddress(const void* address)
{
    for (PropertyMap::iterator property = propTable.begin(); property != propTable.end(); ++property) {
        if (property->second->matchesAddress(address)) {
            return property->second;
        }
    }
    return 0;
}

void
PropertySet_impl::validate (CF::Properties property,
                            CF::Properties& validProps,
                            CF::Properties& invalidProps)
{
    for (CORBA::ULong ii = 0; ii < property.length (); ++ii) {
        std::string id((const char*)property[ii].id);
        if (getPropertyFromId(id)) {
            CORBA::ULong count = validProps.length();
            validProps.length(count + 1);
            validProps[count].id = property[ii].id;
            validProps[count].value = property[ii].value;
        } else {
            CORBA::ULong count = invalidProps.length();
            invalidProps.length(count + 1);
            invalidProps[count].id = property[ii].id;
            invalidProps[count].value = property[ii].value;
        }
    }
}


CF::DataType PropertySet_impl::getProperty (CORBA::String_var _id)
{
    TRACE_ENTER(PropertySet_impl);
    CF::DataType value;
    std::string id = (const char*)(_id);
    PropertyInterface* property = getPropertyFromId(id);
    if (property) {
        value.id = CORBA::string_dup(_id);
        property->getValue(value.value);
    }
    TRACE_EXIT(PropertySet_impl);
    return value;
}


void PropertySet_impl::setPropertyChangeListener (const std::string& id, PropertyCallbackFn func)
{
    PropertyCallback cb;
    cb = func;
    setPropertyCallback(id, cb);
}

void PropertySet_impl::executePropertyCallback (const std::string& id)
{
    PropertyCallbackMap::iterator func = propCallbacks.find(id);
    if (propCallbacks.end() == func) {
        return;
    }
    (func->second)(id);
}

void PropertySet_impl::setPropertyCallback (const std::string& id, PropertyCallback callback)
{
    std::string propId;

    // Check whether the supplied id is actually a property name; if so, map the name
    // to an id, otherwise assume that 'id' is really a property id.
    PropertyInterface* property = getPropertyFromName(id);
    if (property) {
        propId = property->id;
    } else {
        // Check if property exists
        if (!getPropertyFromId(id)){
            RH_WARN(_propertysetLog, "Setting listener for property " << id << " that does not exist");
        }
        propId = id;
    }

    propCallbacks[propId] = callback;
}



void PropertySet_impl::startPropertyChangeMonitor(const std::string &rsc_id ) 
{
  // start thread when first registration happens
  PropertyChangeRec::RSC_ID = rsc_id;
  if ( rsc_id == "" )   PropertyChangeRec::RSC_ID = "UNK_RSC_ID";
  return;
}


void PropertySet_impl::stopPropertyChangeMonitor()
{
  _propChangeThread.stop();
}


int PropertySet_impl::_propertyChangeServiceFunction() 
{
  RH_TRACE(_propertysetLog, "Starting property change service function.");
  time_t delay = 0;
  {
    SCOPED_LOCK(propertySetAccess);

    // get current time stamp....
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();

    // for each registration record
    PropertyChangeRegistry::iterator iter = _propChangeRegistry.begin();
    for( ; iter != _propChangeRegistry.end() && _propChangeThread.threadRunning(); iter++) {

      PropertyChangeRec *rec = &(iter->second);
      RH_DEBUG(_propertysetLog, "Change Listener ... reg_id/interval :" << rec->regId << "/" << rec->reportInterval.total_milliseconds());

      PropertyReportTable::iterator rpt_iter = rec->props.begin();
      // check all registered properties for changes
      for( ; rpt_iter != rec->props.end() && _propChangeThread.threadRunning(); rpt_iter++) {
	// check if property changed
	RH_DEBUG(_propertysetLog, "   Property/set :" << rpt_iter->first << "/" << rpt_iter->second->isSet());
	try{
	  if ( _propMonitors[rpt_iter->first]->isChanged() ) {
	    rpt_iter->second->recordChanged();
	    RH_DEBUG(_propertysetLog, "   Recording Change Property/set :" << rpt_iter->first << "/" << rpt_iter->second->isChanged());
	  }
	}
	catch(...) {}
      }

      // determine if time has expired
      boost::posix_time::time_duration dur = rec->expiration - now;
      RH_DEBUG(_propertysetLog, "   Check for expiration, dur=" << dur.total_milliseconds() );
      if ( dur.total_milliseconds()  <= 0 )  {
        CF::Properties  rpt_props;
        CORBA::ULong idx = 0;
	PropertyReportTable::iterator rpt_iter = rec->props.begin();
        // check all registered properties for changes
	for( ; rpt_iter != rec->props.end() && _propChangeThread.threadRunning(); rpt_iter++) {
	  RH_DEBUG(_propertysetLog, "   Sending Change Property/set :" << rpt_iter->first << "/" << rpt_iter->second->isChanged());
	  if (rpt_iter->second->isChanged() ) {
                
	    // add to reporting change list
	    idx = rpt_props.length();
	    rpt_props.length( idx+1 );
	    rpt_props[idx].id     = CORBA::string_dup(rpt_iter->first.c_str());
	    RH_DEBUG(_propertysetLog, "   Getting getValue from property....prop: " << rpt_iter->first << " reg_id:" << rec->regId );
	    PropertyInterface *property = getPropertyFromId(rpt_iter->first);
	    if ( property ) {
	      RH_DEBUG(_propertysetLog, "   Getting getValue from property....prop: " << rpt_iter->first << " reg_id:" << rec->regId );
	      property->getValue( rpt_props[idx].value );
	    }
            
            // reset change indicator for next reporting cycle
	    rpt_iter->second->reset();
	  }

	}


	// publish changes to listener
	if ( rec->pcl && rpt_props.length() > 0 ) {
	  RH_DEBUG(_propertysetLog, "   Calling notifier....size :" << rpt_props.length());
	  if ( rec->pcl->notify( rec, rpt_props ) != 0 ) {
	    RH_ERROR(_propertysetLog, "Publishing changes to PropertyChangeListener FAILED, reg_id:" << rec->regId );
	  }
	}
	
	// reset reporting interval..
	rec->expiration = boost::posix_time::microsec_clock::local_time() + rec->reportInterval;
	dur = rec->reportInterval;
      }

      if ( _propChangeRegistry.size() > 0 ) {
	for( PropertyMonitorTable::iterator ii=_propMonitors.begin(); ii != _propMonitors.end();  ii++) ii->second->reset();
      }
      
      // determine delay interval based on shortest remaining duration interval
      if ( delay == 0 ) delay=dur.total_milliseconds();
      RH_DEBUG(_propertysetLog, "   Test for delay/duration (millisecs) ... :" << delay << "/" << dur.total_milliseconds());
      if ( dur.total_milliseconds() > 0 ) delay = std::min( delay, (time_t)dur.total_milliseconds() );
      RH_DEBUG(_propertysetLog, "   Minimum  delay (millisecs) ... :" << delay );
    }
  }     

  RH_DEBUG(_propertysetLog, "Request sleep delay........(millisecs) :" << delay);
  // figure out how long to wait till next iteration
  if ( delay > 0 )  _propChangeThread.updateDelay( delay/1000.0 );
  return NOOP;
}



PropertySet_impl::EC_PropertyChangeListener::EC_PropertyChangeListener( CORBA::Object_ptr obj ) {
  
  try {
     ec = ossie::events::EventChannel::_narrow(obj);
     if ( !CORBA::is_nil(ec) ) {
       RH_NL_DEBUG("EC_PropertyChangeListener", "Creating Publisher interface ..." );
       pub = redhawk::events::PublisherPtr( new redhawk::events::Publisher(ec) );
      }
  } catch(...) {
    throw -1;  // let the caller now we are not valid event channel or publisher interface failed...
  }

}

PropertySet_impl::EC_PropertyChangeListener::~EC_PropertyChangeListener( ) {
  pub.reset();
}

int  PropertySet_impl::EC_PropertyChangeListener::notify( PropertyChangeRec *rec, CF::Properties &changes ) {

  int retval=0;
  CF::PropertyChangeListener::PropertyChangeEvent evt;
  std::string uuid = ossie::generateUUID();
  evt.evt_id = CORBA::string_dup( uuid.c_str() );
  evt.reg_id = CORBA::string_dup( rec->regId.c_str());
  evt.resource_id = CORBA::string_dup( rec->rscId.c_str() );
  evt.properties = changes;
  evt.timestamp = _makeTime(-1,0,0);
  try {
    RH_NL_DEBUG("EC_PropertyChangeListener", "Send change event reg/id:" << rec->regId << "/" << uuid );
    pub->push( evt );
  }
  catch(...) {
    RH_NL_DEBUG("PropertyChangeListener", "PropertyChangeListener(EventChannel) FAILED, reg/event-id:" << rec->regId << "/" << uuid );
    retval=-1;
  }
  
  return retval;
}


PropertySet_impl::INF_PropertyChangeListener::INF_PropertyChangeListener( CORBA::Object_ptr obj ) {

  try {
    RH_NL_DEBUG("INF_PropertyChangeListener", "Narrowing PublisherChangeListener interface ..." );
    listener = CF::PropertyChangeListener::_narrow(obj);
    if ( CORBA::is_nil(listener) )  throw -1;
  } catch(...) {
    throw -1;  // let the caller now we are not valid
  }  
}


int PropertySet_impl::INF_PropertyChangeListener::notify( PropertyChangeRec *rec, CF::Properties &changes ) {
  int retval=0;
  CF::PropertyChangeListener::PropertyChangeEvent evt;
  std::string uuid = ossie::generateUUID();
  evt.evt_id = CORBA::string_dup( uuid.c_str() );
  evt.reg_id = CORBA::string_dup( rec->regId.c_str());
  evt.resource_id = CORBA::string_dup( rec->rscId.c_str() );
  evt.properties = changes;
  evt.timestamp = _makeTime(-1,0,0);
  try {
    RH_NL_DEBUG("INF_PropertyChangeListener", "Send change event reg/id:" << rec->regId << "/" << uuid );
    listener->propertyChange( evt );
  }
  catch(...) {
    RH_NL_DEBUG("PropertyChangeListener", "PropertyChangeListener(Interface) FAILED, reg/event-id:" << rec->regId << "/" << uuid );
    retval=-1;
  }
  return retval;
}
