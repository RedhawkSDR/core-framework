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
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <iostream>
#include <cstdio>
#include <cstddef>
#include <sstream>
#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <omniORB4/omniURI.h>
#include "corba.h"
#include "logdebug.h"

namespace corba {

  //
  // used for boost shared pointer instantion when user
  // supplied callback is provided
  //
  struct null_deleter
  {
    void operator()(void const *) const
    {
    }
  };



  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  //
  //
  //  Orb - Singleton class methods and declarations
  //

  OrbPtr        OrbContext::_singleton;

  bool          OrbContext::_share = true;

  //
  // Terminate
  //
  //   Get the current execution context for interfacing with the ORB.
  //      resolve NamingService, rootPOA, POAManager, POACurrent, and omniINSPOA
  //
  void OrbContext::Terminate( bool forceShutdown ) {
    if ( forceShutdown || _share == false ) {
      if ( CORBA::is_nil(_singleton->orb) == false ) {
	//_singleton->orb->shutdown(true);
	_singleton->orb->destroy();
      }
    }
    _singleton.reset();
  }
   
 
  //
  // Init
  //
  OrbPtr   OrbContext::Init( ) {
    return Init(0,NULL);
  }

  //
  // Init
  //
  //   Get the current execution context for interfacing with the ORB.
  //      resolve NamingService, rootPOA, POAManager, POACurrent, and omniINSPOA
  //
  OrbPtr   OrbContext::Init( int argc, char **argv, const char* options[][2], bool share ) {
    
    int retval=1;
    const char *action="";
    _share = share;
    try {
      _singleton = boost::shared_ptr< OrbContext >( new OrbContext() );
      OrbContext &corba_ctx = *_singleton;
      corba_ctx.orb = CORBA::ORB_init(argc,argv, "omniORB4", options);

      corba_ctx.namingServiceCtx=CosNaming::NamingContextExt::_nil();
      corba_ctx.rootPOA=PortableServer::POA::_nil();

      CORBA::Object_var obj;
      action="resolve initial reference 'RootPOA'";
      LNDEBUG( "ORB", action );
      obj=corba_ctx.orb->resolve_initial_references("RootPOA");
      corba_ctx.rootPOA =PortableServer::POA::_narrow(obj);
      if(CORBA::is_nil(corba_ctx.rootPOA))
	throw CORBA::OBJECT_NOT_EXIST();

      action="activate the RootPOA's POAManager";
      LNDEBUG( "ORB", action );
      corba_ctx.poaManager =corba_ctx.rootPOA->the_POAManager();
      corba_ctx.poaManager->activate();

      action="resolve initial reference 'NameService'";
      LNDEBUG( "ORB", action );
      obj=corba_ctx.orb->resolve_initial_references("NameService");
      corba_ctx.namingService = CosNaming::NamingContext::_narrow(obj);
      if(CORBA::is_nil(corba_ctx.namingService))
	throw CORBA::OBJECT_NOT_EXIST();

      action="resolve initial reference 'NameServiceExt'";
      LNDEBUG( "ORB", action );
      obj=corba_ctx.orb->resolve_initial_references("NameService");
      corba_ctx.namingServiceCtx = CosNaming::NamingContextExt::_narrow(obj);
      if(CORBA::is_nil(corba_ctx.namingServiceCtx))
	throw CORBA::OBJECT_NOT_EXIST();

      retval=0;
    }
    catch(CORBA::ORB::InvalidName& ex) { // resolve_initial_references
      LNERROR( "ORB", "Failed to "<<action<<". ORB::InvalidName");
    }
    catch(CosNaming::NamingContext::InvalidName& ex) { // resolve
      LNERROR(  "ORB", "Failed to "<<action<<". NamingContext::InvalidName");
    }
    catch(CosNaming::NamingContext::NotFound& ex) { // resolve
      LNERROR( "ORB", "Failed to "<<action<<". NamingContext::NotFound");
    }
    catch(CosNaming::NamingContext::CannotProceed& ex) { // resolve
      LNERROR( "ORB", "Failed to "<<action<<". NamingContext::CannotProceed");
    }
    catch(CORBA::TRANSIENT& ex) { // _narrow()
      LNERROR( "ORB", "Failed to "<<action<<". TRANSIENT");
    }
    catch(CORBA::OBJECT_NOT_EXIST& ex) { // _narrow()
      LNERROR( "ORB", "Failed to "<<action<<". OBJECT_NOT_EXIST");
    }
    catch(CORBA::SystemException& ex) {
      LNERROR( "ORB", "Exception: " << ex._name() << " ("<<ex.NP_minorString()<<")"  );
    }
    catch(CORBA::Exception& ex) {
      LNERROR( "ORB", "Failed to "<<action << ". " <<ex._name() );
    }
  
    return _singleton;
  }


  //
  // convenience routine to convert stringified name to CosNaming path
  //
  CosNaming::Name *stringToName(std::string &name ) {
    return omni::omniURI::stringToName(name.c_str());
  }

  //
  // convenience routine to convert stringified name to CosNaming path
  //
  CosNaming::Name str2name(const char* namestr)
  {
    CosNaming::Name name;
    CORBA::ULong nameLen=0;
    name.length(nameLen);

    std::string n =namestr;
    std::string::size_type pos=0;
    char last='/';
    while(true)
      {
	pos=n.find_first_not_of("/.",pos);
	if(std::string::npos==pos) break;
	std::string::size_type sep =n.find_first_of("/.",pos);
	std::string piece =n.substr(pos, (std::string::npos==sep? sep: sep-pos) );
	if(last=='/')
	  {
	    name.length(++nameLen);
	    name[nameLen-1].id=CORBA::string_dup(piece.c_str());
	  }
	else
	  {
	    name[nameLen-1].kind=CORBA::string_dup(piece.c_str());
	  }
	if(std::string::npos==sep) break;
	pos=sep;
	last=n[sep];
      }
    return name;
  }


  std::vector<std::string> listRootContext( ) {
    return listContext( OrbContext::Inst()->namingServiceCtx,"" );
  }

    
  std::vector<std::string> listContext(const CosNaming::NamingContext_ptr ctx, const std::string &dname ) {
    CosNaming::BindingIterator_var bi;
    CosNaming::BindingList_var bl;
    CosNaming::Binding_var b;
    const CORBA::ULong CHUNK = 0;
    
    //std::cout << " DIR:" << dname << std::endl;
    std::vector<std::string> t;
    try{
      ctx->list(CHUNK, bl, bi);
      while ( CORBA::is_nil(bi) == false &&  bi->next_one(b) ) {
	//std::cout << " Convert Name " << std::endl;
	CORBA::String_var s = CORBA::string_dup(b->binding_name[0].id);
	std::string bname = s.in();
	if ( b->binding_type == CosNaming::nobject ) {
	  std::string n = dname;
	  n = n + "/" + bname;
	  //std::cout << " OBJ:" << n << std::endl;
	  t.push_back( n );
	}
	else if ( b->binding_type == CosNaming::ncontext ) {
	  std::vector< std::string > slist;
	  CORBA::Object_ptr obj=ctx->resolve( b->binding_name );
	  if ( CORBA::is_nil(obj) == false ) {
	    CosNaming::NamingContext_ptr nc= CosNaming::NamingContext::_narrow(obj);
	    std::string sdir=dname;
	    sdir = sdir+"/"+ bname;
	    slist = listContext( nc, sdir );
	    t.insert(t.end(), slist.begin(), slist.end() );
	  }
	}
      }
	
    }
    catch(...) {
      //std::cout << " Ut ohhhhh.... something bad happened" << std::endl;
    }

    return t;
  }


  //
  // Create a naming context from root directory
  //
  int  CreateNamingContext( OrbPtr orb, const std::string &namingContext ) {
    int retval=1;
    CosNaming::NamingContext_ptr ctx  = CreateNamingContextPath( orb, namingContext );
    if(!CORBA::is_nil(ctx)) retval=0;
    return retval;
  }


  CosNaming::NamingContext_ptr CreateNamingContextPath( OrbPtr orb, const std::string &namingContext) {

    LNDEBUG( "CreateNamingContextPath", " NamingContext: " << namingContext );
    CosNaming::Name  cname = str2name(namingContext.c_str());      
    CosNaming::NamingContext_ptr ctx  = CosNaming::NamingContext::_nil();
    CosNaming::NamingContext_ptr naming_ctx  = orb->namingService;

    if(!CORBA::is_nil(orb->namingService) ) {
      
      try {
	CosNaming::Name n;
	n.length(1);

	// Drill down through contexts.
	for(CORBA::ULong i=0; i<(cname.length()); ++i)       {
	  n[0]=cname[i];
	  try  {
	    naming_ctx=naming_ctx->bind_new_context(n);
	  }
	  catch(CosNaming::NamingContext::AlreadyBound&) {
	    CORBA::Object_var obj2 =naming_ctx->resolve(n);
	    naming_ctx=CosNaming::NamingContext::_narrow(obj2);
	  }
	  // One of the context names is already bound to an object. Bail out!
	  if(CORBA::is_nil(naming_ctx))
	    return ctx;
	}
	
	ctx = naming_ctx;
      } catch( const CORBA::Exception& ex) {
	LNERROR( "CreateNamingContextPath", " CORBA " << ex._name() << " exception during, bind context:" << namingContext );
      }
    }
    return ctx;
  }



  CosNaming::NamingContext_ptr ResolveNamingContextPath( OrbPtr orb, const std::string &namingContext ) {

    LNDEBUG( "ResolveNamingContextPath", " NamingContext: " << namingContext );
    CosNaming::Name  cname = str2name(namingContext.c_str());      
    CosNaming::NamingContext_ptr ctx  = CosNaming::NamingContext::_nil();
    CosNaming::NamingContext_ptr naming_ctx  = orb->namingService;

    if(!CORBA::is_nil(orb->namingService) ) {
      
      try {
	CosNaming::Name n;
	n.length(1);

	// Drill down through contexts.
	for(CORBA::ULong i=0; i<(cname.length()); ++i)       {
	  n[0]=cname[i];
	  try  {
	    naming_ctx->bind_context(n, naming_ctx );
	  }
	  catch(CosNaming::NamingContext::AlreadyBound&) {
	    CORBA::Object_var obj2 =naming_ctx->resolve(n);
	    naming_ctx=CosNaming::NamingContext::_narrow(obj2);
	  }

	  // One of the context names is already bound to an object. Bail out!
	  if(CORBA::is_nil(naming_ctx))
	    return ctx;
	}
	
	ctx = naming_ctx;
      } catch( const CORBA::Exception& ex) {
	LNERROR( "ResolveNamingContextPath", " CORBA " << ex._name() << " exception during, bind context:" << namingContext );
      }
    }
    return ctx;
  }



  //
  // Create a naming context from root directory
  //
  int  DeleteNamingContext( OrbPtr orb, const std::string &namingContext )  {
    LNDEBUG( "DeleteNamingContext", " NamingContext: " << namingContext );
    int retval=1;
    CosNaming::Name  cname = str2name(namingContext.c_str());      
    if(!CORBA::is_nil(orb->namingService) ) {
      try {

	CORBA::Object_var obj = orb->namingService->resolve(cname);
        CosNaming::NamingContext_var context
          = CosNaming::NamingContext::_narrow(obj);

        if (CORBA::is_nil(context))  return 1;

        context->destroy();

        orb->namingService->unbind(cname);

	retval = 0;
      } catch(CosNaming::NamingContext::NotFound& ex) {
	LNWARN( "DeleteNamingContext", " Not Found :" << namingContext );
      } catch(CosNaming::NamingContext::CannotProceed & ex) {
	LNWARN( "DeleteNamingContext", " CannotProceed :" << namingContext );
      } catch(CosNaming::NamingContext::InvalidName & ex) {
	LNWARN( "DeleteNamingContext", " InvalidName :" << namingContext );
      } catch( const CORBA::Exception& ex) {
	LNERROR( "DeleteNamingContext", " CORBA " << ex._name() << " exception during, bind context:" << namingContext );
      }
     }
    return retval;
  }



  int  DeleteNamingContextPath( OrbPtr orb, const std::string &namingContext )  {

    LNDEBUG( "DeleteNamingContextPath", " NamingContext: " << namingContext );
    int retval=0;
    CosNaming::Name  cname = str2name(namingContext.c_str());      
    std::vector<  std::pair< CosNaming::NamingContext_ptr, CosNaming::Name >  > nc_list;
    CosNaming::NamingContext_ptr naming_ctx  = orb->namingService;
    if(!CORBA::is_nil(orb->namingService) ) {

      try {
	CosNaming::Name n;
	n.length(1);

	// Drill down through contexts.
	for(CORBA::ULong i=0; i<(cname.length()); ++i)       {
	  n[0]=cname[i];
	  try  {
	    naming_ctx->bind_context(n, naming_ctx );
	  }
	  catch(CosNaming::NamingContext::AlreadyBound&) {
	    CORBA::Object_var obj2 =naming_ctx->resolve(n);
	    naming_ctx=CosNaming::NamingContext::_narrow(obj2);
	  }

	  // One of the context names is already bound to an object. Bail out!
	  if(CORBA::is_nil(naming_ctx))
	    return -1;

	  CosNaming::Name tname;
	  tname.length(i+1);
	  for(CORBA::ULong a=0; a<i+1; ++a) tname[a] = cname[a];
	  
	  nc_list.push_back( std::pair< CosNaming::NamingContext_ptr, CosNaming::Name >( naming_ctx, tname ) );
	}
	std::vector< std::pair< CosNaming::NamingContext_ptr, CosNaming::Name > >::reverse_iterator r=nc_list.rbegin();
	for ( ; r != nc_list.rend(); ++r ) {
	  try {
	    r->first->destroy();
	    orb->namingService->unbind( r->second );
	  } catch(CosNaming::NamingContext::NotFound& ex) {
	    LNWARN( "DeleteNamingContextPath", " Not Found :" << namingContext );
	    retval=1;
	    break;
	  } catch(CosNaming::NamingContext::CannotProceed & ex) {
	    LNWARN( "DeleteNamingContextPath", " CannotProceed :" << namingContext );
	    retval=1;
	    break;
	  } catch(CosNaming::NamingContext::InvalidName & ex) {
	    LNWARN( "DeleteNamingContextPath", " InvalidName :" << namingContext );
	    retval=1;
	    break;
	  } catch( const CORBA::Exception& ex) {
	    LNERROR( "DeleteNamingContextPath", " CORBA " << ex._name() << " exception during, bind context:" << namingContext );
	    retval=1;
	    break;
	  }
	}
      } catch( const CORBA::Exception& ex) {
	LNERROR( "DeleteNamingContextPath", " CORBA " << ex._name() << " exception during, bind context:" << namingContext );
      }
    }
    return retval;
  };

  //
  // Unbind from a namingcontext
  //
  int  Unbind( const std::string &name , CosNaming::NamingContext_ptr namingContext ) {

    LNDEBUG( "Unbind", " Name: " << name );
    int retval=1;
    try {
      if(!CORBA::is_nil(namingContext) ) {
	CosNaming::Name  cname = str2name(name.c_str());
	namingContext->unbind(cname);
	retval=0;
      }
    } 
    catch(CosNaming::NamingContext::NotFound &ex) {
      LNWARN( "Unbind", " NameContext : Name NotFound ");
      retval=0;
    }
    catch(CosNaming::NamingContext::CannotProceed &ex) {
      LNERROR( "Unbind", " NameContext : CannotProceed ");
    }
    catch(CosNaming::NamingContext::InvalidName &ex) {
      LNERROR( "Unbind", " NameContext : InvalidName ");
    }
    catch (const CORBA::Exception& ex) {
      LNERROR( "Unbind", " CORBA " << ex._name() << " exception during unbind operation, name:" << name );
    }

    return retval;

  }



  //
  // Unbind object from naming service
  //
  int  Unbind( OrbPtr orb, const std::string &name , const std::string &namingContext ) {

    LNDEBUG( "Unbind", " NamingContext: <" << namingContext << "> Name:" << name );
    int retval=1;
    CosNaming::Name  cname = str2name(namingContext.c_str());      
    CosNaming::NamingContext_var ctx  = CosNaming::NamingContext::_nil();
    if(!CORBA::is_nil(orb->namingService) ) {
      try {
	if ( namingContext == "" ) {
	  LNDEBUG( "Unbind", " Use Root NamingContext ");
	  ctx = orb->namingService;
	}
	else {
	  LNDEBUG( "Bind", " LOOK UP NamingContext: " << namingContext  );
	  orb->namingService->bind_context( cname, ctx );
	}
	LNDEBUG( "Unbind", " DIR: <" << namingContext << "> Name:" << name );
	return Unbind( name, ctx );
      } catch(CosNaming::NamingContext::AlreadyBound& ex) {
	LNDEBUG( "Unbind", " Already Bound NamingContext : " << namingContext  );
	CORBA::Object_var tmp = orb->namingService->resolve(cname);
	ctx = CosNaming::NamingContext::_narrow(tmp);
	LNDEBUG( "Unbind", " DIR: <" << namingContext << "> Name:" << name );
	return Unbind( name, ctx );
      } catch( const CORBA::Exception& ex) {
	LNERROR( "Unbind", " CORBA " << ex._name() << " exception during, bind context:" << namingContext );
      }

    }
    return retval;
  };


  //
  // Bind to naming context    id = name, kind=""
  //
  int  Bind(const std::string &name,  CORBA::Object_ptr obj,  CosNaming::NamingContext_ptr namingContext  ) {

    LNDEBUG( "Bind", " created event channel " << name );
    int retval=1;
    try {
      if(!CORBA::is_nil(namingContext) ) {
	CosNaming::Name  cname = str2name(name.c_str());
	try{
	  LNDEBUG( "Bind", "Attempt to Bind, Name:" << name  );
	  namingContext->bind(cname, obj);
	  LNDEBUG( "Bind", "SUCCESS, for Name:" << name  );
	  retval=0;
	} catch(CosNaming::NamingContext::AlreadyBound& ex) {
	  LNDEBUG( "Bind", "Already Bound, Name:" << name  );
	  namingContext->rebind(cname, obj);
	  retval=0;
	}
      }
    } catch (const CORBA::Exception& ex) {
      LNERROR( "Bind", " CORBA " << ex._name() << " exception during bind operation, name:" << name );
    }

    return retval;
  };


  //
  // Bind to naming context    id = name, kind=""
  //
  int  Bind( OrbPtr orb, const std::string &name,  CORBA::Object_ptr obj, const std::string &namingContext, bool create_nc ) {

    LNDEBUG( "Bind", " NamingContext: " << namingContext << " Name:" << name );
    int retval=1;
    CosNaming::Name  cname = str2name(namingContext.c_str());      
    CosNaming::NamingContext_ptr ctx  = CosNaming::NamingContext::_nil();
    if(!CORBA::is_nil(orb->namingService) ) {
      try {
	if ( namingContext == "" ) {
	  LNDEBUG( "Bind", " Use Root NamingContext ");
	  ctx = orb->namingService;
	}
	else {
	  if ( create_nc ) {
	    LNDEBUG( "Bind", " Create NamingContext Path" << namingContext  );
	    ctx = CreateNamingContextPath(orb, namingContext);
	  }
	  else {
	    LNDEBUG( "Bind", " LOOK UP NamingContext " << namingContext  );
	    orb->namingService->bind( cname, ctx );
	  }
	}

      } catch(CosNaming::NamingContext::AlreadyBound& ex) {
	LNDEBUG( "Bind", " Already Bound NamingContext : " << namingContext  );
	CORBA::Object_var tmp = orb->namingService->resolve(cname);
	ctx = CosNaming::NamingContext::_narrow(tmp);

      } catch( const CORBA::Exception& ex) {
	LNERROR( "Bind", " CORBA " << ex._name() << " exception during, bind context:" << namingContext );
      }

      if ( !CORBA::is_nil(ctx) ) {
	LNDEBUG( "Bind", " DIR:" << namingContext << " Name:" << name );
	return Bind( name, obj, ctx );
      }
    }
    return retval;
  };




  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////

  //
  // EventChannel Convenience Methods
  //

  //
  // GetEventChannel
  //
  // Will first lookup an event channel given the value of the name parameter... it will try to resolve the
  // name using different event channel resolution methods:
  // -) resolve if channel defined with InitRef method and resolve_initial_reference method
  // -) resolve as corbaname   corbaname::#channelname
  // -) resolve with  corbaloc
  //
  // If channel was not found and create==true then create the channel from the EventChannelFactory
  //
  CosEventChannelAdmin::EventChannel_ptr GetEventChannel ( corba::OrbPtr &orb, 
							   const std::string& name, 
							   const bool create,
							   const std::string &host ) {
    LNDEBUG("GetEventChannel",  " : NamingService look up, Channel " << name );

    // return value if no event channel was found or error occured
    CosEventChannelAdmin::EventChannel_var event_channel = CosEventChannelAdmin::EventChannel::_nil();

    //
    // Look up event channel
    //   if no channel is found then try to lookup using InitRef
    //   if no channel is found then try to lookup using corbaname method
    //   if no channel is found then try to lookup using corbaloc method.
    // 
    // if all options fail then return nil if create== false
    //

    bool found=false;
    std::string tname;
    std::string nc_name("");

    //
    // try to resolve using channel name as InitRef  and resolve_initial_references
    //
    try {
      if ( found == false ) {
	LNDEBUG( "GetEventChannel", " : Trying InitRef Lookup " << name );
	CORBA::Object_var obj = orb->orb->resolve_initial_references(name.c_str());
	event_channel = CosEventChannelAdmin::EventChannel::_narrow(obj);
	found =true;
	LNDEBUG( "GetEventChannel", " : FOUND EXISTING, Channel " << name );
      } 
    }catch (const CORBA::Exception& e) {
      LNWARN( "GetEventChannel", "  Unable to lookup with InitRef:" << name << ",  CORBA RETURNED(" << e._name() << ")" );
    } 


    //
    // try to resolve with corbaname and string_to_object method
    //
    try {
      std::ostringstream os;
      if ( found == false ) {
	if ( name.find("corbaname") == std::string::npos ) {
	  if ( nc_name != "" )
	    os << "corbaname:rir:#"<< nc_name << "/" << name;
	  else
	    os << "corbaname:rir:#"<< name;
	}
	else
	  os << name;
	tname=os.str();
	LNDEBUG( "GetEventChannel", " : Trying corbaname resolution " << tname );
	CORBA::Object_var obj = obj=orb->orb->string_to_object(tname.c_str());
	event_channel = CosEventChannelAdmin::EventChannel::_narrow(obj);
	found =true;
	LNDEBUG( "GetEventChannel", " : FOUND EXISTING, Channel " << tname );
      } 

    }catch (const CORBA::Exception& e) {
      LNWARN( "GetEventChannel",  "  Unable to lookup with corbaname:  URI:" << tname << ",  CORBA RETURNED(" << e._name() << ")");
    }


    //
    // try to resolve with corbaloc method and string_to_object method
    //
    try {
      if ( found == false ) {	
	std::ostringstream os;
	//
	// last gasp... try the corbaloc method...corbaloc::host:11169/<channel name>
	// 
	os << "corbaloc::"<<host<<":11169/"<< name;
	tname=os.str();
	LNDEBUG( "GetEventChannel"," : Trying corbaloc resolution " << tname );
	CORBA::Object_var obj = orb->orb->string_to_object(tname.c_str());
	if ( !CORBA::is_nil(obj) ) {
	    event_channel = CosEventChannelAdmin::EventChannel::_narrow(obj);
	    found = true;
	    LNDEBUG( "GetEventChannel", " : FOUND EXISTING, Channel " << tname );
	}
	else {
	  LNDEBUG( "GetEventChannel", " : SEARCH FOR Channel " << tname << " FAILED");
	}
      } 
    }catch (const CORBA::Exception& e) {
      LNWARN( "GetEventChannel", "  Unable to lookup with corbaloc URI:" << tname << ", CORBA RETURNED(" << e._name() << ")" );
    }

    try{
      if ( !found && create ) {

	LNDEBUG( "GetEventChannel", " CREATE NEW CHANNEL " << name );
	event_channel = CreateEventChannel( orb, name );
	if ( !CORBA::is_nil(event_channel) )
	  LNINFO( "GetEventChannel", " --- CREATED NEW CHANNEL ---" << name );
      }
    } catch (const CORBA::Exception& e) {
      LNERROR( "GetEventChannel", "  CORBA (" << e._name() << ") during event creation, channel " << name );
    }

    return event_channel._retn();
  }


  CosEventChannelAdmin::EventChannel_ptr GetEventChannel ( corba::OrbPtr &orb, 
							   const std::string& name, 
							   const std::string &nc_name, 
							   const bool create,
							   const std::string &host )
  {

    // return value if no event channel was found or error occured
    CosEventChannelAdmin::EventChannel_var event_channel = CosEventChannelAdmin::EventChannel::_nil();

    //
    // Look up event channel in NamingService from root context...
    // if lookup fails then return nil if create== false,
    // else try and create a new EventChannel with name and nc_name 
    //

    bool found=false;
    std::string tname;

    try {
      //
      // Lookup in NamingService...
      //
      LNDEBUG("GetEventChannel",  " : NamingService look up, NC<"<<nc_name<<"> Channel " << name );
      std::string cname=name;
      if ( nc_name != "" )
	cname=nc_name+"/"+name;

      LNDEBUG("GetEventChannel",  " : NamingService look up : " << cname );
      CORBA::Object_var obj = orb->namingServiceCtx->resolve_str(cname.c_str());
      event_channel = CosEventChannelAdmin::EventChannel::_narrow(obj);

      LNDEBUG("GetEventChannel", " : FOUND EXISTING, Channel NC<"<<nc_name<<"> Channel " << name );
      found = true;
    } catch (const CosNaming::NamingContext::NotFound&) {
      LNWARN("GetEventChannel",  "  Unable to resolve event channel (" << name << ") in NamingService..." );
    } catch (const CORBA::Exception& e) {
      LNERROR("GetEventChannel", "  CORBA (" << e._name() << ") exception during event channel look up, CH:" << name );
    }


    try{
      if ( !found && create ) {

	LNDEBUG( "GetEventChannel", " CREATE NEW CHANNEL " << name );
	event_channel = CreateEventChannel( orb, name, nc_name );
	if ( !CORBA::is_nil(event_channel) )
	  LNINFO( "GetEventChannel", " --- CREATED NEW CHANNEL ---" << name );
      }
    } catch (const CORBA::Exception& e) {
      LNERROR( "GetEventChannel", "  CORBA (" << e._name() << ") during event creation, channel " << name );
    }

    return event_channel._retn();
  }

  //
  // CreateEventChannel
  //
  // @param orb  context of the orb we are associated with 
  // @param name human readable path to the event channel being requested
  // @parm  bind bind the channel name to the object in the NamingService if channel was created
  //
  CosEventChannelAdmin::EventChannel_ptr CreateEventChannel ( corba::OrbPtr &orb, 
							      const std::string& name, 
							      corba::NS_ACTION action  ) {
    return CreateEventChannel( orb, name, "", action );
    }

  CosEventChannelAdmin::EventChannel_ptr CreateEventChannel ( corba::OrbPtr &orb, 
							      const std::string& name, 
							      const std::string &nc_name, 
							      corba::NS_ACTION action  )
  {
    
    CosEventChannelAdmin::EventChannel_var event_channel = CosEventChannelAdmin::EventChannel::_nil();
    omniEvents::EventChannelFactory_var event_channel_factory = GetEventChannelFactory( orb );

    LNDEBUG( "CreateEventChannel", " Request to create event channel:" << name << " bind action:" << action );

    if (CORBA::is_nil(event_channel_factory)) {
      LNERROR( "CreateEventChannel", "CHANNEL(CREATE): Could not find EventChannelFactory" );
      return event_channel._retn();
    }

    CosLifeCycle::Key key;
    key.length (1);
    key[0].id = CORBA::string_dup("EventChannel");
    key[0].kind = CORBA::string_dup("object interface");

    LNDEBUG( "CreateEventChannel", " action - event channel factory api" );
    if(!event_channel_factory->supports(key))
      {
	LNWARN( "CreateEventChannel", " EventChannelFactory does not support Event Channel Interface!" );
	return event_channel._retn();	
      }

    // 
    // Our EventChannels will always be created with InsName
    //
    LNINFO( "CreateEventChannel", " Request to create event channel:" << name.c_str() << " bind action:" << action );
    CosLifeCycle::Criteria criteria;
    criteria.length(2);
    criteria[0].name=CORBA::string_dup("InsName");
    criteria[0].value<<=name.c_str();
    criteria[1].name=CORBA::string_dup("CyclePeriod_ns");
    criteria[1].value<<=(CORBA::ULong)10;

    //
    // Create Event Channel Object.
    //
    LNDEBUG( "CreateEventChannel", " action - create EventChannel object" );

    CORBA::Object_var obj;
    try {
      obj =event_channel_factory->create_object(key, criteria);
    }
    catch (CosLifeCycle::CannotMeetCriteria& ex) /* create_object() */ {
      LNERROR( "CreateEventChannel", "create_object failed, channel: " << name << " reason: CannotMeetCriteria " );
    }
    catch (CosLifeCycle::InvalidCriteria& ex) /* create_object() */ {
      LNERROR( "CreateEventChannel", "create_object failed, channel: " << name << " reason: InvalidCriteria " );
      if(ex.invalid_criteria.length()>0) {
	int j;
	for (  j=0; (unsigned int)j < ex.invalid_criteria.length(); j++ ) { 
	  LNERROR( "CreateEventChannel", "--- Criteria Name: " << ex.invalid_criteria[j].name );
	  CORBA::ULong xx;
	  ex.invalid_criteria[j].value >>= xx;
	  LNERROR( "CreateEventChannel", "--- Criteria Value : " << xx );
	}
      }
    }
    catch( CORBA::Exception &ex ) {
      LNERROR( "CreateEventChannel", " create_object failed, channel:" << name << " reason: corba exception" );
    }

    if (CORBA::is_nil(obj)) {
      LNERROR( "CreateEventChannel", " Factory failed to create channel: " << name );
      return event_channel._retn();
    }

    try {
      LNDEBUG( "CreateEventChannel", " action - Narrow EventChannel" );
      event_channel = CosEventChannelAdmin::EventChannel::_narrow(obj);
    }
    catch( CORBA::Exception &ex ) {
      LNERROR( "CreateEventChannel", " Failed Narrow to EventChannel for:" << name  );
    }
    LNDEBUG( "CreateEventChannel", " created event channel " << name );
    try {

      if(!CORBA::is_nil(orb->namingService) && ( action == NS_BIND) ) {
	Bind(orb, name, event_channel.in(), nc_name, true );
      }

    } 
    catch (const CosLifeCycle::InvalidCriteria& ex) {
      LNERROR( "CreateEventChannel", " CHANNEL: Invalid Criteria: " << ex._name() << " - for creating event channel " << name );
    } catch (const CORBA::Exception& ex) {
      LNERROR( "CreateEventChannel", " CHANNEL: CORBA " << ex._name() << " exception during create operation, CHANNEL:" << name );
    }


    LNDEBUG( "CreateEventChannel", " completed create event channel : " << name );
    return event_channel._retn();
  };


  //
  // DeleteEventChannel
  //
  // @param orb  context of the orb we are associated with 
  // @param name name of the event channel to delete
  // @parm  unbind perform an unbind operation with the NamingService if channel was deleted
  //
  // @returns 0 operation passed no issues or errors
  // @returns > 0 operation passed but issues were found but not a failure
  // @returns < 0 operation failed due to execeptions from the orb.
  //
  //
  //
  int DeleteEventChannel ( corba::OrbPtr &orb, const std::string& name, corba::NS_ACTION action ) {
    int retval = 0;

    // return value if no event channel was found or error occured
    CosEventChannelAdmin::EventChannel_var event_channel = CosEventChannelAdmin::EventChannel::_nil();

    event_channel = corba::GetEventChannel( orb, name, false  );
    if ( CORBA::is_nil(event_channel) == true ) {
      LNDEBUG( "DeleteEventChannel", " Cannot find event channel name " << name << " to object, try to remove from naming context." );
      if ( ( action == NS_UNBIND) &&  Unbind( orb, name ) == 0 ) {
	  LNINFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
	}
      retval=-1;
      return retval;
    }

    try {
      event_channel->destroy();
      LNINFO( "DeleteEventChannel", " Deleted event channel, CHANNEL: " << name );
    } 
    catch(CORBA::SystemException& ex) {
      // this will happen if channel is destroyed but 
      LNWARN( "DeleteEventChannel", " System exception occured, ex " << ex._name() );
      retval=-1;
    }
    catch(CORBA::Exception& ex) {
      LNWARN( "DeleteEventChannel", " CORBA exception occured, ex " << ex._name() );
      retval=-1;
    }

    try {
      if( (action == NS_UNBIND) &&  Unbind( orb, name ) == 0 ) {
	LNINFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
      }

    } catch(CosNaming::NamingContext::InvalidName& ex) {
      LNWARN( "DeleteEventChannel", " Invalid name to unbind, name: " << name  );
    }
    catch(CosNaming::NamingContext::NotFound& ex) { // resolve
      LNWARN( "DeleteEventChannel", " Name not found, name: " << name  );
    }
    catch(CosNaming::NamingContext::CannotProceed& ex) { // resolve
      LNERROR( "DeleteEventChannel", " Cannot Process error, name: " << name );
      retval=-1;
    }

    return retval;
  }

  int DeleteEventChannel ( corba::OrbPtr &orb, 
			   const std::string& name, 
			   const std::string& nc_name, 
			   corba::NS_ACTION action )
  {
    
    int retval = 0;

    // return value if no event channel was found or error occured
    CosEventChannelAdmin::EventChannel_var event_channel = CosEventChannelAdmin::EventChannel::_nil();

    event_channel = corba::GetEventChannel( orb, name, nc_name, false  );
    if ( CORBA::is_nil(event_channel) == true ) {
      LNDEBUG( "DeleteEventChannel", " Cannot find event channel name " << name << " to object, try to remove from naming context." );
      if ( ( action == NS_UNBIND) &&  Unbind( orb, name, nc_name ) == 0 ) {
	  LNINFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
	}
      retval=-1;
      return retval;
    }

    try {
      event_channel->destroy();
      LNINFO( "DeleteEventChannel", " Deleted event channel, CHANNEL: " << name );
    } 
    catch(CORBA::SystemException& ex) {
      // this will happen if channel is destroyed but 
      LNWARN( "DeleteEventChannel", " System exception occured, ex " << ex._name() );
      retval=-1;
    }
    catch(CORBA::Exception& ex) {
      LNWARN( "DeleteEventChannel", " CORBA exception occured, ex " << ex._name() );
      retval=-1;
    }

    try {
      if( (action == NS_UNBIND) &&  Unbind( orb, name, nc_name ) == 0 ) {
	LNINFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
      }

    } catch(CosNaming::NamingContext::InvalidName& ex) {
      LNWARN( "DeleteEventChannel", " Invalid name to unbind, name: " << name  );
    }
    catch(CosNaming::NamingContext::NotFound& ex) { // resolve
      LNWARN( "DeleteEventChannel", " Name not found, name: " << name  );
    }
    catch(CosNaming::NamingContext::CannotProceed& ex) { // resolve
      LNERROR( "DeleteEventChannel", " Cannot Process error, name: " << name );
      retval=-1;
    }

    return retval;
  }


  //
  // (Taken from eventc.cc)
  //
  //
  omniEvents::EventChannelFactory_ptr GetEventChannelFactory ( corba::OrbPtr &orb ) {
    
    CORBA::Object_var ecf_obj;
    omniEvents::EventChannelFactory_var ecf = omniEvents::EventChannelFactory::_nil();
    LNDEBUG( "GetEventChannelFactory", " Look up EventChannelFactory" );
    try {
      //ecf_obj = orb.namingServiceCtx->resolve_str(str2name("EventChannelFactory"));
      ecf_obj = orb->namingServiceCtx->resolve_str("EventChannelFactory");
      if (!CORBA::is_nil(ecf_obj)) {
	LNDEBUG( "GetEventChannelFactory",  " Narrow object to EventChannelFactory" );
	ecf = omniEvents::EventChannelFactory::_narrow(ecf_obj);
	LNDEBUG( "GetEventChannelFactory",  " Narrowed to ... EventChannelFactory" );
      }
    } catch (const CosNaming::NamingContext::NotFound&) {
      LNWARN( "GetEventChannelFactory", " No naming service entry for 'EventChannelFactory'" );
    } catch (const CORBA::Exception& e) {
      LNWARN( "GetEventChannelFactory",  " CORBA " << e._name() << ", exception looking up EventChannelFactory." );
    } 
    return ecf._retn();

  }

  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////

  //
  //  PushEventSupplier class implementation
  //
  PushEventSupplier::PushEventSupplier( corba::OrbPtr     &orb, 
					const std::string &channelName, 
					Supplier          *inSupplier,
					int                retries, 
					int                retry_wait ) :
    name(channelName),
    nc_name(""),
    supplier(inSupplier),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init(orb);
  }


  PushEventSupplier::PushEventSupplier( corba::OrbPtr     &orb, 
					const std::string &channelName, 
					const std::string &ncName, 
					Supplier          *inSupplier,
					int                retries, 
					int                retry_wait ) :
    name(channelName),
    nc_name(ncName),
    supplier(inSupplier),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init(orb);
  }

  PushEventSupplier::PushEventSupplier( corba::OrbPtr     &orb, 
					const std::string &channelName, 
					int               retries, 
					int               retry_wait ) :
    name(channelName),
    nc_name(""),
    supplier(NULL),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init(orb);
  }

  PushEventSupplier::PushEventSupplier( corba::OrbPtr     &orb, 
					const std::string &channelName, 
					const std::string &ncName, 
					int                retries, 
					int                retry_wait ):
    name(channelName),
    nc_name(ncName),
    supplier(NULL),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init(orb);
  }

  void PushEventSupplier::_init( corba::OrbPtr &orb ) 
  {
    LNDEBUG("PushEventSupplier", " GetEventChannel " << name );
    channel = corba::GetEventChannel( orb, name, nc_name, true );
   
    if ( CORBA::is_nil(channel) == true ) {
      LNERROR("PushEventSupplier", " Channel resource not available, channel " << name );
      return;
    }

    int tries=retries;
    do
      {
	try {
	  supplier_admin = channel->for_suppliers ();
	  break;
	}
	catch (CORBA::COMM_FAILURE& ex) {
	}
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while ( tries );

    if ( CORBA::is_nil(supplier_admin) ) return;
    
    LNDEBUG("PushEventSupplier", "Obtained SupplierAdmin." );

    tries=retries;
    do {
      try {
	proxy_for_consumer = supplier_admin->obtain_push_consumer ();
	break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
      }
      if ( retry_wait > 0 ) {
	boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
	boost::this_thread::yield();
      }
      tries--;
    } while ( tries );

    LNDEBUG("PushEventSupplier", "Obtained ProxyPushConsumer." );
    if ( CORBA::is_nil(proxy_for_consumer) ) return;

    if ( supplier == NULL ) {      
      LNDEBUG("PushEventSupplier", "Create Local Supplier Object." );
      supplier = new corba::PushEventSupplier::Supplier();
    }

    CosEventComm::PushSupplier_var sptr =CosEventComm::PushSupplier::_nil();
    sptr = supplier->_this();

    // now attach supplier to the proxy
    do {
      try {
	proxy_for_consumer->connect_push_supplier(sptr.in());
      }
      catch (CORBA::BAD_PARAM& ex) {
	LNERROR("PushEventSupplier", "Caught BAD_PARAM " );
	break;
      }
      catch (CosEventChannelAdmin::AlreadyConnected& ex) {
	break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
	LNERROR("PushEventSupplier",  "Caught COMM_FAILURE Exception "  << 
		"connecting Push Supplier! Retrying..." );
      }
      if ( retry_wait > 0 ) {
	boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
	boost::this_thread::yield();
      }
      tries--;
    } while ( tries );


    LNDEBUG("PushEventSupplier",  "Connected Push Supplier." );

  };
    
  PushEventSupplier::~PushEventSupplier( ) {

    LNDEBUG("PushEventSupplier", "DTOR - START." );

    int tries = retries;
    if ( CORBA::is_nil(proxy_for_consumer) == false ) {
     // Disconnect - retrying on Comms Failure.
     do {
        try {
           proxy_for_consumer->disconnect_push_consumer();
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
	  LNERROR("PushEventSupplier",  "Caught COMM_FAILURE Exception "
		  << "disconnecting Push Supplier! Retrying..." );
        }
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while(tries);
      LNDEBUG("PushEventSupplier", "ProxyPushConsumer disconnected." );
      
    }
    
    if ( supplier ) {
      supplier->_remove_ref();
    }

    LNDEBUG("PushEventSupplier", "DTOR - END." );

  }

  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////

  //
  //  PushEventConsumer class implementation
  //
  PushEventConsumer::PushEventConsumer( corba::OrbPtr       &orb, 
					const std::string   &channelName, 
					Consumer            *inConsumer,
					const int            retries, 
					const int            retry_wait ):
    name(channelName),
    nc_name(""),
    consumer(inConsumer),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init(orb);
  }

  PushEventConsumer::PushEventConsumer( corba::OrbPtr       &orb, 
					const std::string   &channelName, 
					const std::string   &ncName, 
					Consumer            *inConsumer,
					const int            retries, 
					const int            retry_wait ):
    name(channelName),
    nc_name(ncName),
    consumer(inConsumer),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init(orb);
  }


  PushEventConsumer::PushEventConsumer( corba::OrbPtr &orb, 
					const std::string   &channelName, 
					const int retries, 
					const int retry_wait ):
    name(channelName),
    nc_name(""),
    consumer(NULL),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init(orb);
  }


  PushEventConsumer::PushEventConsumer( corba::OrbPtr &orb, 
					const std::string   &channelName, 
					const std::string   &ncName, 
					const int retries, 
					const int retry_wait ):
    name(channelName),
    nc_name(ncName),
    consumer(NULL),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init(orb);
  }


  void PushEventConsumer::_init( corba::OrbPtr &orb ) 
  {
    LNDEBUG("PushEventConsumer", " GetEventChannel " << name );
    try {
      channel = corba::GetEventChannel( orb, name, nc_name, true );
    }
    catch(...){
        LNERROR("PushEventConsumer", " Channel " << name );
	return;
    }
    
    if ( CORBA::is_nil(channel) == true ) {
      LNERROR("PushEventConsumer", " Channel resource not available, channel " << name );
      return;
    }

    int tries=retries;
    do
      {
	try {
	  consumer_admin = channel->for_consumers ();
	  break;
	}
	catch (CORBA::COMM_FAILURE& ex) {
	}
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while ( tries );

    if ( CORBA::is_nil(consumer_admin) ) return;
    
    LNDEBUG("PushEventConsumer", "Obtained ConsumerAdmin." );

    tries=retries;
    do {
      try {
	proxy_for_supplier = consumer_admin->obtain_push_supplier ();
	break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
      }
      if ( retry_wait > 0 ) {
	boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
	boost::this_thread::yield();
      }
      tries--;
    } while ( tries );

    LNDEBUG("PushEventConsumer", "Obtained ProxyPushConsumer." );
    if ( CORBA::is_nil(proxy_for_supplier) ) return;

    if  ( consumer == NULL ) {
      consumer = new corba::PushEventConsumer::CallbackConsumer(*this);
    }
    if ( consumer == NULL ) return;
    CosEventComm::PushConsumer_var sptr =CosEventComm::PushConsumer::_nil();
    sptr = consumer->_this();

    // now attach supplier to the proxy
    do {
      try {
	// connect the the consumer object to the supplier's proxy
	proxy_for_supplier->connect_push_consumer(sptr.in());
      }
      catch (CORBA::BAD_PARAM& ex) {
	LNERROR("PushEventConsumer", "Caught BAD_PARAM " );
	break;
      }
      catch (CosEventChannelAdmin::AlreadyConnected& ex) {
	break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
	LNERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "  << 
		"connecting Push Consumer! Retrying..." );
      }
      if ( retry_wait > 0 ) {
	boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
	boost::this_thread::yield();
      }
      tries--;
    } while ( tries );


    LNDEBUG("PushEventConsumer",  "Connected Push Consumer." );

  };
    
  PushEventConsumer::~PushEventConsumer( ) {

    LNDEBUG("PushEventConsumer", "DTOR - START." );
    int tries = retries;
    if ( CORBA::is_nil(proxy_for_supplier) == false ) {
     // Disconnect - retrying on Comms Failure.
     do {
        try {
           proxy_for_supplier->disconnect_push_supplier();
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
	  LNERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "
		  << "disconnecting Push Consumer! Retrying..." );
        }
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while(tries);
      LNDEBUG("PushEventConsumer", "ProxyPushSupplier disconnected." );
      
    }

    if ( consumer ) {
      consumer->_remove_ref();
    }

    LNDEBUG("PushEventConsumer", "DTOR - END." );
  }

#if 0
  void   PushEventConsumer::detach() {

    LNDEBUG("PushEventConsumer", "DETTACH - START." );
    int tries = retries;
    if ( CORBA::is_nil(proxy_for_supplier) == false ) {
      // Disconnect - retrying on Comms Failure.
      do {
        try {
	  proxy_for_supplier->disconnect_push_supplier();
	  break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
	  LNERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "
		  << "disconnecting Push Consumer! Retrying..." );
        }
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while(tries);
      LNDEBUG("PushEventConsumer", "ProxyPushSupplier disconnected." );
      
    }
    else {
      LNDEBUG("PushEventConsumer", "DETTACH - ProxyForSupplier is Nil." );    
    }
  }



  void   PushEventConsumer::attach() {

    LNDEBUG("PushEventConsumer", "ATTACH - START." );
    if ( consumer == NULL ) return;

    LNDEBUG("PushEventConsumer", "Register Consumer." );    
    CosEventComm::PushConsumer_var sptr = consumer->_this();
    int tries = retries;

    if ( CORBA::is_nil(proxy_for_supplier) == false ) {
      // now attach supplier to the proxy
      do {
	try {
	  // connect the the consumer object to the supplier's proxy
	  proxy_for_supplier->connect_push_consumer(sptr.in());
	}
	catch (CORBA::BAD_PARAM& ex) {
	  LNERROR("PushEventConsumer", "Caught BAD_PARAM " );
	  break;
	}
	catch (CosEventChannelAdmin::AlreadyConnected& ex) {
	  LNDEBUG("PushEventConsumer", "ATTACH - Already Connected Consumer." );    
	  break;
	}
	catch (CORBA::COMM_FAILURE& ex) {
	  LNERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "  << 
		  "connecting Push Consumer! Retrying..." );
	}
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while ( tries );

    }
    else {
      LNDEBUG("PushEventConsumer", "ATTACH - ProxyForSupplier is Nil." );    
    }
  }

#endif

  void   PushEventConsumer::setDataArrivedListener( DataArrivedListener *newListener ) {
    dataArrivedCB =  boost::shared_ptr< DataArrivedListener >(newListener, null_deleter());
  }

  void   PushEventConsumer::setDataArrivedListener( DataArrivedCallbackFn  newListener ) {
    dataArrivedCB =  boost::make_shared< StaticDataArrivedListener >( newListener );
  }



};


