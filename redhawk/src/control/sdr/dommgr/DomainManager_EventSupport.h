/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef __RH_CF_DOMMGR_EVENTS_H__
#define __RH_CF_DOMMGR_EVENTS_H__
#include <ossie/RedhawkDefs.h>
#include <ossie/Events.h>


  class DOM_Publisher;
  class DOM_Subscriber;
  typedef boost::shared_ptr<DOM_Publisher>       DOM_Publisher_ptr;
  typedef boost::shared_ptr<DOM_Subscriber>      DOM_Subscriber_ptr;

class DOM_Publisher : public redhawk::events::Publisher  {

  public:
    //
    //  DOM_Publisher interface for an Event Channel
    //
    // @param channel    event channel returned from the EventChannelManager
    DOM_Publisher( ossie::events::EventChannel_ptr           channel );

    virtual ~DOM_Publisher();

  private:

  };

class DOM_Subscriber : public redhawk::events::Subscriber {

  public:

    //
    // Create a Subscriber interface to an Event Channel. Subscribers will listen for event messages and provide 1 of two methods
    // for data retrieval.  Via a data queue that can be extracted or from a registred callback. In either case the user of the Subscriber
    // interface is required to know  apriori the message format (i.e. message structure) to unmarshall the encoded stream.
    //
    // @param EventChannel   in event channel
    DOM_Subscriber(  ossie::events::EventChannel_ptr  inChannel );

    ~DOM_Subscriber();

  };


#endif   // __RH_CF_ODM_EVENTS_H__
