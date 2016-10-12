package org.ossie.component;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Map;
import java.util.Properties;
import java.util.UUID;

import org.apache.log4j.Logger;
import org.apache.log4j.Level;
import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;
import org.omg.CORBA.UserException;
import org.omg.CosEventChannelAdmin.*;
import org.ossie.events.PropertyEventSupplier;
import org.ossie.properties.IProperty;
import org.ossie.properties.PropertyListener;
import org.ossie.properties.AnyUtils;
import org.ossie.logging.logging;
import org.ossie.corba.utils.*;
import org.ossie.events.Publisher;

import CF.DataType;
import CF.PropertiesHolder;
import CF.UnknownProperties;
import CF.InvalidIdentifier;
import CF.PropertyChangeListener;
import CF.PropertyChangeListenerHelper;
import CF.PropertyChangeListenerPackage.PropertyChangeEvent;
import CF.PropertyChangeListenerPackage.PropertyChangeEventHelper;

class PCL_Callback implements PropertyListener<Object> {

    public boolean    isChanged;

    public PCL_Callback() { isChanged=false; };

    public void reset() { isChanged = false; };

    public boolean isSet() { return isChanged; };

    public void valueChanged(Object a,Object b ) { 
        Logger.getLogger("PCL_CALLBACK").trace("....Value Changed....");
        isChanged=true;
    };

}


interface PCL_Listener {
    public int  notify( PropertyChangeRec prec, DataType[]  props);
};

class EC_PropertyChangeListener  implements PCL_Listener {

    protected Logger                   logger = Logger.getLogger("EC_PropertyChangeListener");

    protected EventChannel             ec;

    protected Publisher                 pub;

    public EC_PropertyChangeListener( org.omg.CORBA.Object obj )  
        throws Exception
    {
        try {
            ec  = EventChannelHelper.narrow(obj);
            if ( ec != null ) {
                logger.debug( "Creating Publisher interface ..." );
                pub = new Publisher( ec );
            }
            else {
                throw new Exception();
            }
        }
        catch( Throwable t ) {
            throw new Exception();
        }
    }

    public int  notify( PropertyChangeRec prec, DataType[]  props) {
        
        String uuid = UUID.randomUUID().toString();
        PropertyChangeEvent evt = new PropertyChangeEvent( uuid,
                                                           prec.regId,
                                                           prec.rscId,
                                                           props);

        final Any any = ORB.init().create_any();
        PropertyChangeEventHelper.insert( any, evt);
        int retval=pub.push(any);
        if ( retval != 0 ){
        }
        
        return retval;
    }

}

class INF_PropertyChangeListener implements PCL_Listener {

    protected  Logger                logger = Logger.getLogger("INF_PropertyChangeListener");
    PropertyChangeListener           listener;

    public INF_PropertyChangeListener( org.omg.CORBA.Object obj ) 
        throws Exception 
    {
        try {
            logger.debug("Narrowing PublisherChangeListener interface ..." );
            listener  = PropertyChangeListenerHelper.narrow(obj);
            if ( listener == null ) {
                throw new Exception();
            }
        }
        catch( Throwable t ) {
            throw new Exception();
        }

    };

    public int  notify( PropertyChangeRec prec, DataType[]  props) {

        String uuid = UUID.randomUUID().toString();
        PropertyChangeEvent evt = new PropertyChangeEvent( uuid,
                                                           prec.regId,
                                                           prec.rscId,
                                                           props);

        int retval=0;
        try {
            listener.propertyChange( evt );
        }
        catch( Throwable t ) {
            retval=-1;
        }
        
        return retval;
    }

}

/**
   class: PropertyChangeRec
   
   Registration context established when a PropertySet receives registerPropertyListener 
   invocation.  The registration maintains a list of property id values the listener
   wishes to receive change notification.  A map of property id to callback listeners
   is setup by the constructor to monitor changes.
   
   This class is a companion class used by a Resource when its _propertyChangeServiceFunction
   is executed.  That method interrogates the current state of the PropertChangeRec
   and reports changes to the registered listener.
 */
public class PropertyChangeRec {

    public String                                    regId=null;

    public String                                    rscId="UNK_RSC_ID";

    public org.omg.CORBA.Object                      listener=null;
        
    public PCL_Listener                              pcl=null;
        
    public int                                       reportInterval=1000;

    public long                                      expiration=0;
        
    public  Hashtable<String, PCL_Callback >         props=null;

    public PropertyChangeRec( org.omg.CORBA.Object obj,          // remote listener .. EventChannel or PropertyChangeListener
                              final String   inRscId,            // resource that provides the event conditionals
                              float interval,                    // in seconds
                              ArrayList<String> pids,            // set of Property Ids to monitor
                              Hashtable<String, IProperty> propSet ) {

        regId = UUID.randomUUID().toString();
        rscId = inRscId;
        listener = obj;
        reportInterval = (int)(interval*1000.0f);
        Logger.getLogger("PropertyChangeRec").trace(" ... RPT Interval:" + reportInterval  );
        expiration = System.currentTimeMillis() + reportInterval;

        // check if our listener is valid....
        PCL_Listener tmp=null;
        pcl = null;
        try {
            tmp = new EC_PropertyChangeListener(listener);
            Logger.getLogger("PropertyChangeRec").debug("Assign EventChannel as destination reg:" + regId  );
            pcl = tmp;
        }
        catch(Exception e ){
            tmp=null;
            pcl = null;
        };

        if ( pcl == null ) {
            try {
                tmp = new INF_PropertyChangeListener(listener);
                Logger.getLogger("PropertyChangeRec").trace("Assign PropertyChangeListener as destination reg:" + regId  );
                pcl=tmp;
            }
            catch(Exception e ){
                tmp=null;
                pcl = null;
            };
        };


        // we are valid so add callbacks...        
        if ( pcl != null ) {
            props = new Hashtable<String, PCL_Callback >();
            // assign property callbacks to the each property
            for( String pid : pids ) {
                final IProperty prop = propSet.get(pid);
                if ((prop != null) && prop.isQueryable()) {
                    PCL_Callback pcl = new PCL_Callback();
                    props.put(pid, pcl );
                    Logger.getLogger("PropertyChangeRec").trace("Adding callback for Property " + pid  );
                    prop.addObjectListener(pcl);
                }
            }
        }


    };
};

