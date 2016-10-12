#include <iostream>
#include <iomanip>
#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/omniORB.h>
#include "RH_NamingContext.h"
#include "ossie/CorbaUtils.h"

typedef boost::shared_mutex Lock;
typedef boost::unique_lock< Lock > WriterLock;
typedef boost::shared_lock< Lock > ReaderLock;

#ifdef DEBUG_NC
#define DB(x) x
#else
#define DB(x)
#endif

#ifdef DEBUG_MEM
#define DB_MEM(x) x
#else
#define DB_MEM(x)
#endif

static PortableServer::POA_var names_poa = PortableServer::POA::_nil();

#ifdef DEBUG_MEM
static uint64_t RH_memAlloc=0;
static uint64_t RH_memFree=0;
static uint64_t RH_objAlloc=0;
static uint64_t RH_objFree=0;
#endif


class ObjectBinding {

public:

  CosNaming::Binding binding;
  CORBA::Object_var object;

  RH_NamingContext* nc;
  ObjectBinding* prev;
  ObjectBinding* next;

  ObjectBinding(const CosNaming::Name& n, CosNaming::BindingType t,
		CORBA::Object_ptr o, RH_NamingContext *nct, 
                ObjectBinding* nx = 0)
  {
    binding.binding_name = n;
    binding.binding_type = t;

    object = CORBA::Object::_duplicate(o);
    nc = nct;
    next = nx;

    if (next) {
      prev = next->prev;
      next->prev = this;
    } else {
      prev = nc->tailBinding;
      nc->tailBinding = this;
    }
    if (prev) {
      prev->next = this;
    } else {
      nc->headBinding = this;
    }
    nc->size++;

    DB_MEM( 
           RH_memAlloc += sizeof(ObjectBinding);
           RH_objAlloc++;
           std::ios::fmtflags tf=std::cerr.flags();
           std::cerr << "  OBJECT CTOR " << " in context/size " << nc << "/" << nc->size 
	   << ": OBJ name (" << n[0].id << "," << n[0].kind
           << ") obj ptr/size :  " << (void*)((CORBA::Object_ptr)o) << "/" ;
           std::cerr.flags(tf);
           std::cerr <<  sizeof(ObjectBinding) 
	   << "  alloc/free " << RH_memAlloc << "/" << RH_memFree 
           << "  obj/free " << RH_objAlloc << "/" << RH_objFree << std::endl; );

  }

  ~ObjectBinding()
  {

    DB_MEM(
           RH_memFree += sizeof(ObjectBinding);
           RH_objFree++;
           std::ios::fmtflags tf=std::cerr.flags();
           std::cerr << "  OBJECT DTOR " << " in context/size " << nc << "/" << nc->size 
           << ": OBJ name (" << binding.binding_name[0].id << "," << binding.binding_name[0].kind
           << ") obj ptr/size :  " << (void*)((CORBA::Object_ptr)this) << "/";
           std::cerr.flags(tf);
           std::cerr <<  sizeof(ObjectBinding) 
           << "  alloc/free " << RH_memAlloc << "/" << RH_memFree 
           << "  obj/free " << RH_objAlloc << "/" << RH_objFree << std::endl;);

    if (prev) {
      prev->next = next;
    } else {
      nc->headBinding = next;
    }
    if (next) {
      next->prev = prev;
    } else {
      nc->tailBinding = prev;
    }
    if (nc) nc->size--;
  }

};




CosNaming::NamingContext_ptr RH_NamingContext::GetNamingContext( const std::string &domain, const bool useNS )  {

  CosNaming::NamingContext_ptr ctx = CosNaming::NamingContext::_nil();
  if ( useNS ) {
    CORBA::Object_var obj_DN;
    // Get a reference to the domain
    try {
        obj_DN = ossie::corba::objectFromName( domain.c_str());
    } catch( CORBA::SystemException& ex ) {
      DB(std::cerr << "  GetNamingContext get_object_from_name threw CORBA::SystemException" << std::endl);
      throw;
    } catch ( std::exception& ex ) {
      DB(std::cerr << "The following standard exception occurred: "<<ex.what()<<" while retrieving the domain name" <<std::endl);
      throw;
    } catch ( const CORBA::Exception& ex ) {
        DB(std::cerr << "The following CORBA exception occurred: "<<ex._name()<<" while retrieving the domain name" << std::endl);
        throw;
    } catch( ... ) {
        DB(std::cerr << "get_object_from_name threw Unknown Exception" << std::endl);
        throw;
    }

    // Get the naming context from the domain
    CosNaming::NamingContext_var _domainContext = ossie::corba::_narrowSafe<CosNaming::NamingContext> (obj_DN);
    if (CORBA::is_nil(_domainContext)) {
      DB(std::cerr << "CosNaming::NamingContext::_narrow threw Unknown Exception" << std::endl);
      throw;
    }

    return CosNaming::NamingContext::_duplicate(_domainContext);
  }
  else {
    if ( CORBA::is_nil(names_poa) )     {
      names_poa = PortableServer::POA::_duplicate( ossie::corba::RootPOA());
    }

    DB(std::cout << " Using DomainManager's NamingContext facility ##############################" << std::endl;);
    if ( !CORBA::is_nil(names_poa) )  {
      RH_NamingContext *rh_ctx = new RH_NamingContext( names_poa );
      ctx = rh_ctx->_this();
      rh_ctx->_remove_ref();
      return ctx;
    }
  }

  return ctx;

}


RH_NamingContext::RH_NamingContext( PortableServer::POA_ptr poa ) :
  headBinding(0),
  tailBinding(0),
  nc_poa( poa ),
  size(0)
{
  //PortableServer::ObjectId_var oid = poa->servant_to_id(this);
  //poa->activate_object_with_id( oid, this);
  PortableServer::ObjectId_var oid = poa->activate_object( this);

  DB_MEM(
         RH_memAlloc += sizeof(RH_NamingContext);
         std::cerr << "NameContext CTOR  " << " in context/size " << this << "/" << sizeof(RH_NamingContext)
         << "  alloc/free " << RH_memAlloc << "/" << RH_memFree 
         << "  obj/free " << RH_objAlloc << "/" << RH_objFree << std::endl; );
}


RH_NamingContext::~RH_NamingContext()
{
  DB(cerr << "~RH_NamingContext_i" << endl);

  WriterLock w(_access);

  // we do not support compound contexts... just single directory entries for now..

  while (headBinding) {
    DB(std::cerr << "   DTOR OBJ - removing (" 
	      << headBinding->binding.binding_name[0].id 
	      << "," 
	      << headBinding->binding.binding_name[0].kind 
	      << ")"
       << ":" << size << ") in context/size " << this << "/" << std::endl;);
    delete headBinding;
    size--;
  }

  DB_MEM(
         RH_memFree += sizeof(RH_NamingContext);
         std::cerr << "DTOR NameContext " << " in context/size " << this << "/" << sizeof(RH_NamingContext)
	        << "  alloc/free " << RH_memAlloc << "/" << RH_memFree 
         << "  obj/free " << RH_objAlloc << "/" << RH_objFree << std::endl; );
}


CosNaming::NamingContext_ptr RH_NamingContext::new_context() 
{
  CosNaming::NamingContext_ptr ctx = CosNaming::NamingContext::_nil();
  if ( !CORBA::is_nil(names_poa) )  {
    RH_NamingContext *rh_ctx = new RH_NamingContext(names_poa);
    ctx = rh_ctx->_this();
    rh_ctx->_remove_ref();
  }

  return ctx;

};


ObjectBinding* RH_NamingContext::resolve_simple(const CosNaming::Name& n)
{
  assert(n.length() == 1);

  DB(cerr << "  resolve_simple name (" << n[0].id << "," << n[0].kind << ")"
     << " in context " << this << endl);

  for (ObjectBinding* ob = headBinding; ob; ob = ob->next) {

    assert(ob->binding.binding_name.length() == 1);

    if ((strcmp(n[0].id,ob->binding.binding_name[0].id) == 0) &&
	(strcmp(n[0].kind,ob->binding.binding_name[0].kind) == 0))
      {
	DB(cerr << "  resolve_simple: found (" << n[0].id << "," << n[0].kind
	   << ")" << " in context " << this << ", bound to "
	   << (void*)((CORBA::Object_ptr)ob->object) << endl);

	return ob;
      }
  }

  DB(cerr << "  resolve_simple: didn't find (" << n[0].id << "," << n[0].kind
     << ")" << " in context " << this << "; raising exception"
     << endl);

  throw CosNaming::NamingContext::NotFound(CosNaming::NamingContext::
					   missing_node, n);
}


CORBA::Object_ptr RH_NamingContext::resolve(const CosNaming::Name &n) 
{
  if (n.length() == 1) {
    DB(cerr << "resolve simple name (" << n[0].id << "," << n[0].kind
       << ") in context " << this << endl);

    ReaderLock r(_access);

    ObjectBinding* ob = resolve_simple(n);

    DB(cerr << "returning " << (void*)((CORBA::Object_ptr)ob->object) << endl);

    return CORBA::Object::_duplicate(ob->object);

  } 

  // compound not supported

  return CORBA::Object::_nil(); 
}



void RH_NamingContext::bind_helper(const CosNaming::Name &n, CORBA::Object_ptr obj,
                                   CosNaming::BindingType t, CORBA::Boolean rebind)
{
  DB(cerr << "bind in context " << this << " (type " << t << " rebind "
     << (int)rebind << ")" << endl);

  if (n.length() == 1) {

    //
    // Bind a simple name - i.e. bind object in this context.
    //

    DB(cerr << "  bind simple name (" << n[0].id << "," << n[0].kind << ") to "
       << obj << " in context " << this << endl);

    WriterLock w(_access);

    ObjectBinding* ob = 0;

    try {
      ob = resolve_simple(n);
      if (!rebind)
	throw CosNaming::NamingContext::AlreadyBound();
    }
    catch (CosNaming::NamingContext::NotFound& ex) {
      ob = 0;
      DB(cerr << "  bind in context " << this
	 << ": caught not found exception from resolving simple name\n"
	 << "    reason " << ex.why << " rest of name ");

      for (unsigned int i = 0; i < ex.rest_of_name.length(); i++) {
	DB(cerr << "(" << ex.rest_of_name[i].id << ","
	   << ex.rest_of_name[i].kind << ")");
      }
      DB(cerr << endl);
    }

    if (ob) {
      DB(cerr << "  rebind in context " << this
	 << ": unbinding simple name (" << n[0].id << "," << n[0].kind
	 << ") from " << (void*)((CORBA::Object_ptr)ob->object) << endl);
      delete ob;
    }

    DB_MEM(std::cerr << "  Bindhelper " << " in context/size " << this << "/" << size 
	      << ": ADDING simple name (" << n[0].id << "," << n[0].kind
	   << ") obj ptr/size :  " << (void*)((CORBA::Object_ptr)obj) << std::endl; );
    new ObjectBinding(n, t, obj, this);

    DB(cerr << "  bind in context " << this << ": bound simple name ("
       << n[0].id << "," << n[0].kind << ") to " << obj << endl);

  } else {

    DB(cerr << "  bind in context " << this << ": bound compound not support name ("
       << n[0].id << "," << n[0].kind << ") to " << obj << endl);
  }


}


void RH_NamingContext::unbind(const CosNaming::Name &n) 
{

  DB(cerr << "unbind in context " << this << endl);

  if (n.length() == 1) {

    //
    // Unbind a simple name - i.e. remove it from this context.
    //

    DB(cerr << "  unbind simple name (" << n[0].id << "," << n[0].kind << ")"
       << " in context " << this << endl);

    WriterLock w(_access);

    ObjectBinding* ob = resolve_simple(n);

    DB_MEM(std::cerr << "  Unbind: removing (" << n[0].id << "," << n[0].kind << ")"
	   << " from context " << this << " (was bound to "
	   << (void*)((CORBA::Object_ptr)ob->object) << ")" << std::endl;);
    delete ob;

  } 
  /// no suport for compound names...

}




CosNaming::NamingContext_ptr RH_NamingContext::bind_new_context(const CosNaming::Name &n) 
{
  if (n.length() == 1) {

    //
    // Bind a new context with a simple name - i.e. create a new context and
    // bind it in this context.
    //

    DB(cerr << "bind_new_context simple (" << n[0].id << "," << n[0].kind
       << ") in context " << this << endl);
    CosNaming::NamingContext_ptr nc = new_context();
    try {
      bind_context(n, nc);
      DB_MEM(std::cerr << "bind_new_context simple (" << n[0].id << "," << n[0].kind
	     << ") to context/listsize " << this << "/" << size << std::endl;);
    } catch (...) {
      nc->destroy();
      CORBA::release(nc);
      throw;
    }
      
    return nc;
  }
  else {
    // multi level context names not supported not supported...
    return CosNaming::NamingContext::_nil();
  }

};


void RH_NamingContext::destroy() 
{
  DB(cerr << "destroy" << endl);

  WriterLock w(_access);

  if (headBinding)
    throw CosNaming::NamingContext::NotEmpty();

  DB_MEM(std::cerr << "destroy ( objects ) in context/listsize " << this << "/" << size 
	 << "  alloc/free " << RH_memAlloc << "/" << RH_memFree 
	 << "  obj/free " << RH_objAlloc << "/" << RH_objFree << std::endl;);
  PortableServer::ObjectId_var id = nc_poa->servant_to_id(this);
  nc_poa->deactivate_object(id);
};


void RH_NamingContext::list(CORBA::ULong length, 
                            CosNaming::BindingList_out out, 
                            CosNaming::BindingIterator_out iterator) 
{};
    

//
// CosNaming::NamingContextExt operations
//

char*
RH_NamingContext::to_string(const CosNaming::Name& name)
{
  return omni::omniURI::nameToString(name);
}

CosNaming::Name*
RH_NamingContext::to_name(const char* sn)
{
  return omni::omniURI::stringToName(sn);
}

char*
RH_NamingContext::to_url(const char* addr, const char* sn)
{
  return omni::omniURI::addrAndNameToURI(addr, sn);
}

CORBA::Object_ptr
RH_NamingContext::resolve_str(const char* sn)
{
  CosNaming::Name_var name = omni::omniURI::stringToName(sn);
  return resolve(name);
}



