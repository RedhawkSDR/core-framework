package org.ossie.events;

import java.util.Queue;
import java.util.LinkedList;
import org.apache.log4j.Logger;
import org.omg.CORBA.ORB;
import org.omg.CORBA.Any;
import org.omg.CORBA.*;
import org.omg.CosEventComm.*;
import org.omg.CosEventChannelAdmin.*;
import CF.EventChannelManagerPackage.*;
import org.ossie.properties.AnyUtils;
import java.lang.InterruptedException;
import java.util.concurrent.locks.*;
import java.util.concurrent.TimeUnit;



public class  Subscriber  {

    public abstract class EventSubscriberConsumer extends PushConsumerPOA {
    };

    public interface  DataArrivedListener  {
	public void processData( final Any data );
    };


    public abstract class Receiver extends PushConsumerPOA {
        
        public Receiver() {};

        public boolean   get_disconnect() { return _recv_disconnect; };

        public void   reset() { _recv_disconnect = false; };
      
        public void disconnect_push_consumer () {
            _logger.debug("::disconnect_push_consumer handle disconnect_push_supplier." );
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


    private class DefaultConsumer extends Receiver  {

	public DefaultConsumer ( Subscriber inParent ) {
	    parent = inParent;
	}

    
	public  void push( final org.omg.CORBA.Any data ) {
	    if ( parent != null ) {
		// if parent defines a callback
		if ( parent.dataArrivedCB != null ) {
                    // RESOLVE need to allow for Generic callback method to convert data and
                    // push data...
		    try{
                        parent.dataArrivedCB.processData( data );
		    }
		    catch( Throwable e){
		    }
		}
		else {
		    parent.events.add(data);
		}
	    };
	};


	private Subscriber         parent;
  };



    //
    //  Subscriber for an Event Channel
    //
    // @param channel    event channel returned from the EventChannelManager
    // @param pub   interface that is notified when a disconnect occurs
    // @param retries    number of retries to perform when trying to establish  publisher interface
    // @param retry_wait number of millisecs to wait between retries
    public Subscriber( EventChannel       inChannel ) 
	throws OperationNotAllowed
    {
	this(inChannel,null );
    }

    public Subscriber( EventChannel            inChannel,
		       DataArrivedListener     newListener )
	throws OperationNotAllowed
    {
        _init( inChannel, newListener );
    }


    public int getData( Any ret ) {

      int retval=-1;
      try{

        // check if callback method is enable.. it so then return
        if ( dataArrivedCB != null ) return retval;

        // check if data is available
        if ( events.size() < 1 ) return retval;
          
        Any  rawdata =  events.remove();
          
        ret = rawdata;
        retval=0;
      }
      catch( Throwable e) {
      }

      return retval;
    }


    public void terminate () {

        logger.trace("Subscriber: TERMINATE START" );

        if ( consumer != null ) {
            org.ossie.corba.utils.deactivateObject(consumer);
        }

        disconnect();

        proxy = null;
        consumer = null;

        logger.trace("Subscriber: TERMINATE END." );
    }


    public int disconnect() {
        return this.disconnect(10,10);
    }

    public int disconnect( int retries, int retry_wait ) {

        logger.debug("Subscriber: disconnect ....." );
        int retval = -1;
        int tries = retries;
        if ( proxy != null ) {
            do {
                try {
                    logger.debug("Subscriber: disconnect_push_supplier... ");
                    proxy.disconnect_push_supplier();
                    retval=0;
                    break;
                }
                catch (COMM_FAILURE ex) {
                    logger.warn("Caught COMM_FAILURE Exception,  diconnecting Push Consumer for Subscriber,  Retrying..." );
                }
                if ( retry_wait > 0 ) {
                    try {
                        java.lang.Thread.sleep(retry_wait*1000);
                    } catch (final InterruptedException ex) {
                    }
                }
                tries--;
            } while(tries>0);

            if ( consumer  != null ){
                logger.debug("Subscriber::disconnect Waiting for disconnect ........" );
                consumer.wait_for_disconnect(1,3);
                logger.debug("Subscriber::disconnect received disconnect." );     
            }
            logger.debug("Subscriber: ProxyPushSupplier disconnected." );
        }
        
        return retval;
    }

    public int connect() {
        return this.connect(10,10);
    }

    public int connect( int retries, int retry_wait ) {

        int retval=-1;
	int tries=retries;

	if ( channel == null ) {
	    logger.warn( "Subscriber,  Channel resource not available ");
	    return retval;
	}

        if ( proxy == null ) {
            ConsumerAdmin         admin=null;
            do {
                try {
                    logger.debug( "Subscriber,  Grab admin object...");
                    admin = channel.for_consumers ();
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


            if ( admin == null  ) return retval;

            logger.debug( "Subscriber,  Obtained ConsumerAdmin...");
            tries=retries;
            do {
                try {
                    logger.debug( "Subscriber,  Grab push supplier proxy.");
                    proxy = admin.obtain_push_supplier();
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

            logger.debug( "Subscriber,  Obtained ProxyPushConsumer." );
        }

	if ( proxy == null  ) return retval;

        PushConsumer sptr=null;
	if  ( consumer != null ) {
	    sptr = consumer._this();
	}

	// now attach supplier to the proxy
        tries=retries;
	do {
	    try {
		proxy.connect_push_consumer( sptr );
                consumer.reset();
                logger.debug( "Subscriber, Connected Consumer to EventChannel....." );
                retval=0;
		break;
	    }
	    catch(TypeError ex) {
                logger.error( "Subscriber, Caught TypeError exception connecting Push Consumer!");
		break;
	    }
	    catch(BAD_PARAM ex) {
                logger.error( "Subscriber, Caught BAD_PARAM exception connecting Push Consumer!");
		break;
	    }
	    catch(AlreadyConnected ex) {
                logger.warn( "Subscriber, Push Consumer already connected!");
                consumer.reset();
                retval=0;
		break;
	    }
	    catch(COMM_FAILURE ex) {
                logger.error( "Subscriber, Caught COMM_FAILURE exception "
				   +"connecting Push Supplier! Retrying...");
	    }
	    if ( retry_wait > 0 ) {
               try {
		   java.lang.Thread.sleep(retry_wait*1000);
                } catch (final InterruptedException ex) {
	       }

	    }
	    tries--;
	} while ( tries > 0);


        return retval;

    };


    // handle to the Event Channel ... duplicate the channel so we own this object 
    protected EventChannel                  channel;

    // handle to object that publishes the event to the channel's consumers
    //protected ProxyPushSupplierOperations   proxy;
    protected ProxyPushSupplier             proxy;

    // handle to object that responds to disconnect messages
    protected Receiver                      consumer;

    //
    // Logger object 
    //
    protected Logger                        logger=null;

    //
    // Callback interface when event messages arrive
    //
    protected DataArrivedListener           dataArrivedCB=null;

    
    protected Queue< Any >                  events=null;


    private  void _init( EventChannel inChannel, DataArrivedListener newListener ) 
        throws OperationNotAllowed 
    {
	logger = Logger.getLogger("ossie.events.Subscriber");

        dataArrivedCB = newListener;

        // create queue to hold events
        events = new LinkedList< Any >();

        // if user passes a bad param then throw...
        channel=inChannel;
	if ( inChannel == null ) throw new OperationNotAllowed();

        // create a local consumer object for the event channel
        consumer = new DefaultConsumer(this);
        if ( consumer != null ) {
            org.ossie.corba.utils.activateObject(consumer, null);
        }

	// connect to the event channel for a subscriber pattern
        connect();
    }



};
