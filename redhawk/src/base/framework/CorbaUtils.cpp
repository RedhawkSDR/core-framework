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

#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/omniORB.h>
#include <omniORB4/internal/orbParameters.h>

#include "ossie/CorbaUtils.h"

static CORBA::ORB_var orb = CORBA::ORB::_nil();
static PortableServer::POA_var root_poa = PortableServer::POA::_nil();
static CosNaming::NamingContext_var inc = CosNaming::NamingContext::_nil();

static bool persistenceEnabled = false;

namespace ossie {
namespace corba {

CREATE_LOGGER(CorbaUtils);

CORBA::ORB_ptr CorbaInit (int argc, char* argv[])
{
    // Initialize the ORB.
    CORBA::ORB_ptr orb = OrbInit(argc, argv, false);

    // Automatically activate the root POA manager.
    PortableServer::POAManager_var manager = RootPOA()->the_POAManager();
    manager->activate();

    return orb;
}

CORBA::ORB_ptr OrbInit (int argc, char* argv[], bool persistentIORs)
{
    if (CORBA::is_nil(orb)) {
        if (persistentIORs) {
            persistenceEnabled = true;

            // Check for a user-specified ORB endpoint; if one is given,
            // do not override it. This allows multiple persistent endpoints
            // on the same machine.
            for (int ii = 1; ii < argc; ++ii) {
                if (strcmp(argv[ii], "-ORBendPoint") == 0) {
                    orb = CORBA::ORB_init(argc, argv);
                    return orb;
                }
            }
            
            const char* corba_args[][2] = {
                { "endPoint", "giop:tcp::5678" },
                { 0, 0 }
            };
 
            orb = CORBA::ORB_init(argc, argv, "omniORB4", corba_args);
       } else {
            orb = CORBA::ORB_init(argc, argv);
       }
    }

    return orb;
}

void OrbShutdown (bool wait)
{
    if (CORBA::is_nil(orb)) {
        return;
    }

    // Release our references to the initial naming context and root POA.
    inc = CosNaming::NamingContext::_nil();
    root_poa = PortableServer::POA::_nil();

    if (wait) {
        orb->shutdown(1);
        orb->destroy();
    } else {
        orb->shutdown(0);
    }

    // Release our reference to the ORB, which should cause it to be destroyed.
    orb = CORBA::ORB::_nil();
}

CORBA::ORB_ptr Orb ()
{
    return orb;
}

PortableServer::POA_ptr RootPOA ()
{
    if (CORBA::is_nil(root_poa) && !CORBA::is_nil(orb)) {
        CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
        root_poa = PortableServer::POA::_narrow(obj);
    }

    return root_poa;
}

unsigned long giopMaxMsgSize ()
{
    return omni::orbParameters::giopMaxMsgSize;
}

CosNaming::NamingContext_ptr InitialNamingContext ()
{
    if (CORBA::is_nil(inc) && !CORBA::is_nil(orb)) {
        CORBA::Object_var obj = orb->resolve_initial_references("NameService");
        inc = CosNaming::NamingContext::_narrow(obj);
    }

    return inc;
}


bool isPersistenceEnabled ()
{
    return persistenceEnabled;
}

bool isValidType (const CORBA::Any& lhs, const CORBA::Any& rhs)
{
    CORBA::TypeCode_var tc1 = lhs.type();
    CORBA::TypeCode_var tc2 = rhs.type();

    return (tc1->equal(tc2));
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



CosNaming::Name* stringToName (const std::string& name)
{
    return omni::omniURI::stringToName(name.c_str());
}

CORBA::Object_ptr objectFromName (const std::string& name)
{
    CosNaming::Name_var cosName = stringToName(name);
    return InitialNamingContext()->resolve(cosName);
}

std::string objectToString (const CORBA::Object_ptr obj)
{
    return returnString(orb->object_to_string(obj));
}

CORBA::Object_ptr stringToObject (const std::string& ior)
{
    return orb->string_to_object(ior.c_str());
}

 std::vector<std::string> listRootContext( ) {
    std::vector<std::string> t;
    if ( CORBA::is_nil(inc) == false )
      return listContext( InitialNamingContext(),"" );
    else
      return t;
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
      // skip to end
    }

    return t;
  }




void bindObjectToName (const CORBA::Object_ptr obj, const std::string& name)
{
    bindObjectToContext(obj, InitialNamingContext(), name);
}

void bindObjectToContext(const CORBA::Object_ptr obj, const CosNaming::NamingContext_ptr context, const std::string& name)
{
    CosNaming::Name_var cosName = stringToName(name);
    context->rebind(cosName, obj);
}

unsigned int numberBoundObjectsToContext(CosNaming::NamingContext_ptr context)
{
    CosNaming::BindingIterator_var it;
    CosNaming::BindingList_var bl;
    const CORBA::ULong CHUNK = 100;

    context->list(CHUNK, bl, it);

    return bl->length();
}



void unbindAllFromContext(CosNaming::NamingContext_ptr context)
{
    ///\todo Add support for deleting more than 100 names
    CosNaming::BindingIterator_var it;
    CosNaming::BindingList_var bl;
    const CORBA::ULong CHUNK = 100;

    context->list(CHUNK, bl, it);

    for (unsigned int ii = 0; ii < bl->length(); ++ii) {
        context->unbind(bl[ii].binding_name);
    }
}


PortableServer::ObjectId* activatePersistentObject (PortableServer::POA_ptr poa, PortableServer::Servant servant, const std::string& identifier)
{
    if (!persistenceEnabled) {
        return poa->activate_object(servant);
    }

    PortableServer::ObjectId_var oid = PortableServer::string_to_ObjectId(identifier.c_str());
    poa->activate_object_with_id(oid, servant);
    return oid._retn();
}


static CORBA::Boolean handleCommFailure (void* cookie, CORBA::ULong retry, const CORBA::COMM_FAILURE& ex)
{
    int maxRetries = reinterpret_cast<long>(cookie);
    if (maxRetries < 0) {
        if (retry == 0) {
            LOG_WARN(CorbaUtils, "CORBA::COMM_FAILURE retrying indefinitely");
        }
        return true;
    } else if ((int)retry < maxRetries) {
        LOG_WARN(CorbaUtils, "CORBA::COMM_FAILURE retrying " << retry);
        return true;
    } else {
        LOG_WARN(CorbaUtils, "CORBA::COMM_FAILURE not retrying " << retry);
        return false;
    }
}


void setCommFailureRetries (int numRetries)
{
    omniORB::installCommFailureExceptionHandler(reinterpret_cast<void*>(numRetries), handleCommFailure);
}


void setObjectCommFailureRetries (CORBA::Object_ptr obj, int numRetries)
{
    omniORB::installCommFailureExceptionHandler(obj, reinterpret_cast<void*>(numRetries), handleCommFailure);
}

#define LNTRACE( lname, expression ) RH_TRACE( rh_logger::Logger::getLogger(lname), expression )
#define LNDEBUG( lname, expression ) RH_DEBUG( rh_logger::Logger::getLogger(lname), expression )
#define LNINFO( lname, expression )  RH_INFO( rh_logger::Logger::getLogger(lname), expression )
#define LNWARN( lname, expression )  RH_WARN( rh_logger::Logger::getLogger(lname), expression )
#define LNERROR( lname, expression ) RH_ERROR( rh_logger::Logger::getLogger(lname), expression )
#define LNFATAL( lname, expression ) RH_FATAL( rh_logger::Logger::getLogger(lname), expression )

   struct OrbContext;
   typedef OrbContext*  OrbPtr;
 
    //
    // Orb
    //
    // Context for access to ORB and common CORBA services
    //
    struct OrbContext {

      // orb instantiation
      CORBA::ORB_ptr                          orb;

      // root POA for to handle object requests
      PortableServer::POA_ptr                 rootPOA;

      // handle to naming service
      CosNaming::NamingContext_ptr            namingService;

      // handle to naming service
      CosNaming::NamingContextExt_ptr         namingServiceCtx;

      virtual ~OrbContext() {};

      OrbContext() {
	orb = ossie::corba::Orb();
	rootPOA = ossie::corba::RootPOA();
	namingService = ossie::corba::InitialNamingContext();
	namingServiceCtx = CosNaming::NamingContextExt::_nil();
	try {
	  CORBA::Object_ptr obj;
	  obj=orb->resolve_initial_references("NameService");
	  namingServiceCtx = CosNaming::NamingContextExt::_narrow(obj);
	}
	catch(...){
	};
      };

    };


  //
  // Create a naming context from root directory
  //
  int  CreateNamingContext(const std::string &namingContext ) {
    int retval=1;
    CosNaming::NamingContext_ptr ctx  = CreateNamingContextPath(namingContext );
    if(!CORBA::is_nil(ctx)) retval=0;
    return retval;
  }


  CosNaming::NamingContext_ptr CreateNamingContextPath(const std::string &namingContext) {

    LNDEBUG( "CreateNamingContextPath", " NamingContext: " << namingContext );
    OrbContext _orb;
    OrbPtr     orb = &_orb;
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



  CosNaming::NamingContext_ptr ResolveNamingContextPath( const std::string &namingContext ) {

    LNDEBUG( "ResolveNamingContextPath", " NamingContext: " << namingContext );
    OrbContext _orb;
    OrbPtr     orb = &_orb;
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
  int  DeleteNamingContext( const std::string &namingContext )  {
    LNDEBUG( "DeleteNamingContext", " NamingContext: " << namingContext );
    OrbContext _orb;
    OrbPtr     orb = &_orb;
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



  int  DeleteNamingContextPath( const std::string &namingContext )  {

    LNDEBUG( "DeleteNamingContextPath", " NamingContext: " << namingContext );
    OrbContext _orb;
    OrbPtr     orb = &_orb;
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
  int  Unbind( const std::string &name , const std::string &namingContext ) {

    LNDEBUG( "Unbind", " NamingContext: <" << namingContext << "> Name:" << name );
    OrbContext _orb;
    OrbPtr     orb = &_orb;
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
  int  Bind( const std::string &name,  CORBA::Object_ptr obj, const std::string &namingContext, bool create_nc ) {

    LNDEBUG( "Bind", " NamingContext: " << namingContext << " Name:" << name );
    OrbContext _orb;
    OrbPtr     orb = &_orb;
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
	    ctx = CreateNamingContextPath( namingContext);
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



}; // namespace corba
}; // namespace ossie
