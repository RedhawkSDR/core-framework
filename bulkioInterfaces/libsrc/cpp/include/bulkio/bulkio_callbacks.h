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

#ifndef __bulkio_callbacks_h
#define __bulkio_callbacks_h

#include <boost/shared_ptr.hpp>

namespace bulkio {

  //
  // Callback interface used by BULKIO Ports when connect/disconnect event happens
  //
  typedef void   (*ConnectionEventCallbackFn)( const char *connectionId );

  //
  // Interface definition that will be notified when a connect/disconnect event happens
  //
  class ConnectionEventListener {

  public:
    virtual void operator() ( const char *connectionId ) = 0;
    virtual ~ConnectionEventListener() {};

  };

  /*
   * Allow for member functions to receive connect/disconnect notifications
   */
  template <class T>
  class MemberConnectionEventListener : public ConnectionEventListener
  {
  public:
    typedef boost::shared_ptr< MemberConnectionEventListener< T > > SPtr;
      
    typedef void (T::*MemberFn)( const char *connectionId );

    static SPtr Create( T &target, MemberFn func ){
      return SPtr( new MemberConnectionEventListener(target, func ) );
    };

    virtual void operator() ( const char *connectionId )
    {
      (target_.*func_)(connectionId);
    }

    // Only allow PropertySet_impl to instantiate this class.
    MemberConnectionEventListener ( T& target,  MemberFn func) :
      target_(target),
      func_(func)
    {
    }
  private:
    T& target_;
    MemberFn func_;
  };

  /*
   * Wrap Callback functions as ConnectionEventListener objects
   */
  class StaticConnectionListener : public ConnectionEventListener
  {
  public:
    virtual void operator() ( const char *connectionId )
    {
      (*func_)(connectionId);
    }

    StaticConnectionListener ( ConnectionEventCallbackFn func) :
      func_(func)
    {
    }

  private:

    ConnectionEventCallbackFn func_;
  };

  //
  // Listener signature to register when a new SRI.streamID is received via pushSRI method
  //
  typedef void (*SriListenerCallbackFn)( BULKIO::StreamSRI &sri );

  //
  // Interface definition that will be notified when a new SRI object
  // is received via pushSRI method
  //
  class SriListener {

    public:
      virtual void operator() ( BULKIO::StreamSRI &sri ) = 0;
      virtual ~SriListener() {};

  };

  /*
   * Allow for member functions to be used as SRI notifications
   */
  template <class T>
  class MemberSriListener : public SriListener
  {
  public:
    typedef boost::shared_ptr< MemberSriListener< T > > SPtr;
      
    typedef void (T::*MemberFn)( BULKIO::StreamSRI &sri );

    static SPtr Create( T &target, MemberFn func ){
      return SPtr( new MemberSriListener(target, func ) );
    };

    virtual void operator() (BULKIO::StreamSRI &sri )
    {
      (target_.*func_)(sri);
    }

    // Only allow PropertySet_impl to instantiate this class.
    MemberSriListener ( T& target,  MemberFn func) :
      target_(target),
      func_(func)
    {
    }
  private:
    T& target_;
    MemberFn func_;
  };

} // end of namespace bulkio

#endif
