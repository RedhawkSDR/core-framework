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

package org.ossie.corba;
import java.util.ArrayList;
import java.util.Properties;
import java.io.File;
import java.io.IOException;
import java.util.List;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.charset.Charset;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;
import org.omg.CORBA.OBJECT_NOT_EXIST;
import org.omg.CORBA.COMM_FAILURE;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.PortableServer.*;
import org.omg.PortableServer.POAManagerPackage.*;
import org.omg.PortableServer.POAPackage.*;
import org.omg.CosNaming.*;
import org.omg.CosNaming.NamingContextPackage.*;
import org.omg.CosNaming.NamingContextExt;
import org.omg.CosNaming.NamingContextExtHelper;

public class utils {


    public static String[] readConfigFile() {
        ArrayList<String> orbarg = new ArrayList<String>();
        try {
            String fname = new String("/etc/omniORB.cfg");
            final String eval = System.getenv("OMNIORB_CONFIG");
            if ( eval != null && eval.equals("") == false ) {
                fname = eval;
            }
            List<String> lines = Files.readAllLines(Paths.get(fname),Charset.defaultCharset());
            for ( String l : lines ) {
                if ( l.startsWith("#") == false  && l.equals("") == false ) {
                    int idx = l.indexOf("NameService");
                    if ( idx != -1 ) {
                        orbarg.add("-ORBInitRef"); 
                        orbarg.add(l.substring(idx) );
                    }
                    
                    idx = l.indexOf("EventService");
                    if ( idx != -1 ) {
                        orbarg.add("-ORBInitRef"); 
                        orbarg.add(l.substring(idx) );
                    }
                }
            }

        } catch (IOException ex) {
            ex.printStackTrace();
        }
        return orbarg.toArray( new String[orbarg.size()] );
    }

    /**
     */
    public static boolean objectExists( org.omg.CORBA.Object obj) {
        try {
            return (obj != null && !obj._non_existent());
        } catch ( OBJECT_NOT_EXIST e ) {
                return false;
        }
    }

    /**
       Initialize ORB from command line and additional property set.
       
     */

    public static ORB Init ( final String[] args, final Properties props)  {
        OrbContext.Init( args,props );
        return Orb();
    }
    
    /**
       GetOrb 
       Returns the configured Orb for this library.
       
     */
    public static ORB  Orb() {
        return OrbContext.Orb();
    }

    public static POA RootPOA() {
        return OrbContext.RootPOA();
    }

    public static NamingContextExt NameService() {
        return OrbContext.NameService();
    }
    
    public static class OrbContext {

        org.omg.CORBA.ORB orb = null;

        POA               rootPOA = null;

        NamingContextExt  namingService = null;

        private static  OrbContext   _singleton = null;

        public ORB getOrb() { 
            return orb;
        }
        public POA getRootPOA() { 
            return rootPOA;
        }
        public NamingContextExt getNameService() { 
            return namingService;
        }

        private OrbContext(ORB inOrb, POA poa, NamingContextExt ns ) {
            orb=inOrb;
            rootPOA=poa;
            namingService = ns;
        }

        public static OrbContext Init ( final String[] args, final Properties props)  {

            if ( _singleton == null ) {
                // this  will honor InitRef in omniORB.cfg file
                org.omg.CORBA.ORB orb = ORB.init(org.ossie.corba.utils.readConfigFile(), props);

                POA poa=null;
                try {
                    final org.omg.CORBA.Object tmp_obj = orb.resolve_initial_references("RootPOA");
                    poa = POAHelper.narrow(tmp_obj);
                    poa.the_POAManager().activate();
                    //System.out.println("Found Root POA.....");
                } catch (final AdapterInactive e) {
                    // PASS
                } catch (final InvalidName e) {
                    // PASS
                }
                
                NamingContextExt ns = null;
                java.util.logging.Level logger_level = java.util.logging.Logger.getLogger("javax.enterprise.resource.corba").getLevel();
                try {
                    java.util.logging.Logger.getLogger("javax.enterprise.resource.corba").setLevel(java.util.logging.Level.OFF);
                    org.omg.CORBA.Object tmp_ns = orb.resolve_initial_references("NameService");
                    ns = NamingContextExtHelper.narrow(tmp_ns);
                    if ( ns == null ) {
                        System.out.println("OrbContext::Init.... NameService unavailable");
                    }
                    //System.out.println("Found NameService.....");
                } catch (COMM_FAILURE e) {
                    // name service is not running. The component is probably running on the sandbox
                } catch (final InvalidName e) {
                    System.out.println(e);
                } catch (Exception e) {
                    System.out.println(e);
                }
                java.util.logging.Logger.getLogger("javax.enterprise.resource.corba").setLevel(logger_level);
            
                _singleton = new OrbContext(orb, poa, ns);
            }
            
            return _singleton;
        }

        public static OrbContext Singleton() {
            return _singleton;
        }

        public static ORB Orb() {
            if ( _singleton != null ) {
                return _singleton.orb;
            }
            else {
                return null;
            }
        }

        public static POA RootPOA() {
            if ( _singleton != null ) {
                return _singleton.rootPOA;
            }
            else {
                return null;
            }
        }

        public static NamingContextExt NameService() {
            if ( _singleton != null ) {
                return _singleton.namingService;
            }
            else {
                return null;
            }
        }
            
    }



    public static byte[]  activateObject (  Servant servant, 
                                            byte[] identifier ) {
        return activateObject( OrbContext.RootPOA(), servant, identifier );
    }


   public static byte[]  activateObject ( POA poa, 
                                          Servant servant, 
                                          byte[] identifier )
    {
        try {
            if (identifier == null || identifier.length == 0 ) {
                try {
                    return poa.activate_object(servant);
                }
                catch( ServantAlreadyActive e ) {
                }


                try{ 
                    return poa.servant_to_id(servant);
                }
                catch( ServantNotActive e ) {
                }
            }

            try {
                poa.activate_object_with_id(identifier, servant);
            }
            catch( ServantAlreadyActive e ) {
            }
            catch( ObjectAlreadyActive e ) {
            }

            try{ 
                return poa.servant_to_id(servant);
            }
            catch( ServantNotActive e ) {
            }
        }
        catch( WrongPolicy e ) {
        }

        return null;
    }


    public static void deactivateObject ( Servant obj ) {
        try {
            POA root_poa = RootPOA();
        
            if  (  root_poa != null ) {
                  byte [] oid = root_poa.servant_to_id(obj);
                  root_poa.deactivate_object(oid);
            }
        }
        catch( ServantNotActive e ) {
        }
        catch( ObjectNotActive e ) {
        }
        catch( WrongPolicy e ) {
        }
    }


}
