
package org.ossie.events;

import org.omg.CORBA.ORB;
import org.omg.CORBA.Any;
import org.omg.CORBA.*;
import org.omg.PortableServer.*;
import org.omg.CosEventChannelAdmin.*;
import org.omg.CosEventComm.*;
import org.omg.PortableServer.POA;
import org.apache.log4j.Logger;
import org.ossie.corba.utils.*;
import CF.EventChannelManagerPackage.*;
import java.lang.InterruptedException;
import java.util.concurrent.locks.*;
import java.util.concurrent.TimeUnit;

public class  Publisher  {

    public abstract class  Supplier extends PushSupplierPOA {

    };


    public class Receiver extends PushSupplierPOA {

        
        public Receiver() {};

        public boolean   get_disconnect() { return _recv_disconnect; };

        public void   reset() { _recv_disconnect = false; };
      
        public void disconnect_push_supplier () {
            _logger.debug("::disconnect_push_supplier handle disconnect_push_supplier." );
            _lock.lock();
            try{
                _recv_disconnect =  true;
                _cond.signalAll();
            }finally {
                _lock.unlock();
            }
        };


        public void wait_for_disconnect ( int wait_time, int retries )
        {
            int tries=retries;
            _lock.lock();
            try {
                while( _recv_disconnect == false ) {
                    if ( wait_time > -1 ) {
                        _logger.debug("::wait_for_disconnect.. Waiting on disconnect." );
                        boolean ret = false;
                        try {
                            ret= _cond.await( wait_time, TimeUnit.MILLISECONDS );
                        } catch( InterruptedException e ) {
                        }
                        if ( !ret && tries == -1  ) {
                            break;
                        }
                        if ( tries-- < 1 )  break;
                    }
                    else {
                        try {
                            _cond.await();

                        } catch( InterruptedException e ) {
                        }
                    }
                }
            } finally {
                _lock.unlock();
            }
        
            return;
        };

      
      protected Lock                        _lock = new ReentrantLock();
      protected Condition                   _cond = _lock.newCondition();
      protected boolean                     _recv_disconnect = true;
      protected Logger                      _logger = Logger.getLogger("Publisher.Receiver");                     

    };



  class DefaultReceiver extends Receiver {

    public DefaultReceiver ( Publisher inParent )  {
	parent=inParent;
    };

    protected Publisher parent;
  };



    //
    //  Publisher for an Event Channel
    //
    // @param channel    event channel returned from the EventChannelManager
    // @param pub   interface that is notified when a disconnect occurs
    // @param retries    number of retries to perform when trying to establish  publisher interface
    // @param retry_wait number of millisecs to wait between retries
    public Publisher( EventChannel    inChannel )
	throws OperationNotAllowed
    {
	logger = Logger.getLogger("ossie.events.Publisher");

        // if user passes a bad param then throw...
        channel = inChannel;
	if ( inChannel == null ) throw new OperationNotAllowed();

        //  local supplier object 
	disconnectReceiver = new DefaultReceiver(this);
	if ( disconnectReceiver != null ) {
            org.ossie.corba.utils.activateObject(disconnectReceiver, null);
	}

        // initialize the event channel for a publisher and the local supplier interface
        connect( );

    }


    public void  terminate() {
        logger.debug("TERMINATE - START." );
        
        // disconnect the channel
        disconnect();

        // stop our disconnectReceiver from receiving data...
        if ( disconnectReceiver != null ) {
            org.ossie.corba.utils.deactivateObject(disconnectReceiver);
        }

        // free up the resource
        proxy = null;
        disconnectReceiver=null;

        logger.debug("TERMINATE - END." );
    }


    public int disconnect() {
        return this.disconnect(10,10);
    }

    public int disconnect( int retries, int retry_wait ) {

        int retval=-1;
        int tries = retries;
        if ( proxy != null ) { 
            do {
                try {
                    proxy.disconnect_push_consumer();
                    retval=0;
                    break;
                }
                catch (COMM_FAILURE ex) {
                    logger.error( "Caught COMM_FAILURE Exception disconnecting Push Supplier! Retrying..." );
                }
                if ( retry_wait > 0 ) {
                    try {
                        java.lang.Thread.sleep(retry_wait*1000);
                    } catch (final InterruptedException ex) {
                    }
                }
                tries--;
            } while(tries>0);

            if ( disconnectReceiver != null ) {
                logger.debug( "Publisher::disconnect waiting for disconnect.." );
                disconnectReceiver.wait_for_disconnect(1,3);
                logger.debug( "Publisher::disconnect received disconnect.." );
            }
                
        }

        logger.debug( "Publisher disconnected ...." );     
        return retval;
    }


    public int connect() {
        return this.connect(10,10);
    }

    public int connect( int retries, int retry_wait ) {

        int retval=-1;

        if ( channel == null ) {
            return retval;
        }

	int tries=retries;
        if ( proxy == null ) {
            SupplierAdmin admin=null;
            do {
                try {
                    admin = channel.for_suppliers ();
                    break;
                }
                catch (COMM_FAILURE ex) {
                }
                if ( retry_wait > 0 ) {
                    try {
                        java.lang.Thread.sleep(retry_wait*1000);
                    } catch (final InterruptedException ex) {
                    }
                } 
                tries--;
            } while ( tries > 0);
	
            if ( admin == null ) return retval;

            tries=retries;
            do {
                try {
                    proxy = admin.obtain_push_consumer ();
                    break;
                }
                catch (COMM_FAILURE ex) {
                }
                if ( retry_wait > 0 ) {
                    try {
                        java.lang.Thread.sleep(retry_wait*1000);
                    } catch (final InterruptedException ex) {
                    }
                }
                tries--;
            } while ( tries > 0 );
        }

        if ( proxy == null ) return retval;

	PushSupplier   sptr=null;
	if ( disconnectReceiver != null ) {
	    sptr = disconnectReceiver._this();
	}

        // now attach supplier to the proxy
        tries=retries;
        do {
            try {
                proxy.connect_push_supplier(sptr);
                disconnectReceiver.reset();
                retval=0;
                break;
            }
            catch(BAD_PARAM ex) {
                logger.error( "Caught BAD_PARAM exception connecting Push Supplier!");
                break;
            }
            catch(AlreadyConnected ex) {
                disconnectReceiver.reset();
                retval=0;
                logger.error("Proxy Push Consumer already connected!");
                break;
            }
            catch(COMM_FAILURE ex) {
                logger.error("Caught COMM_FAILURE exception "
                        +"connecting Push Supplier! Retrying...");
            }
            if ( retry_wait > 0 ) {
                try {
                    java.lang.Thread.sleep(retry_wait*1000);
                } catch (final InterruptedException ex) {
                }
            }
            tries--;
        } while ( tries > 0 );

        return retval;
    };


    public int    push( Any data ) {
      int retval=0;
      try {
        if (  proxy != null ) {
            proxy.push(data);
        }
        else{
          retval=-1;
        }
      }
      catch( Exception ex) {
        retval=-1;
      }

      return retval;
    }



    // handle to the Event Channel ... duplicated channel so we own this object 
    protected EventChannel              channel;

    // handle to object that publishes the event to the channel's consumers
    //protected ProxyPushConsumerOperations   proxy;
    protected ProxyPushConsumer         proxy = null;

    // handle to object that responds to disconnect messages
    protected Receiver                  disconnectReceiver = null;

    //
    // logger
    protected Logger                    logger=null;


};
