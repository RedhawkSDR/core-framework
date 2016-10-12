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

#ifndef OSSIE_CORBAUTILS_H
#define OSSIE_CORBAUTILS_H

#include <string>
#include <vector>
#include <omniORB4/CORBA.h>
#include "CorbaSequence.h"
#include "ossie/debug.h"

namespace ossie {

    namespace corba {

        // Initialize the CORBA system, including the ORB and the root POA. The
        // manager for the root POA will also be started.
        CORBA::ORB_ptr CorbaInit (int argc, char* argv[]);

        // Initialize the CORBA ORB.
        CORBA::ORB_ptr OrbInit (int argc, char* argv[], bool persistentIORs);

        // Get the ORB instance.
        CORBA::ORB_ptr Orb ();

        // Shutdown the ORB and clean up any associated objects.
        void OrbShutdown (bool wait);

        // Get the root POA instance. If it has not yet been resolved, it will
        // be looked up from the ORB's initial references.
        PortableServer::POA_ptr RootPOA ();

        // Get the root POA instance. If it has not yet been resolved, it will
        // be looked up from the ORB's initial references.
        unsigned long giopMaxMsgSize ();

        // Get the root naming context. If it has not yet been resolved, it
        // will be looked up from the ORB's initial references.
        CosNaming::NamingContext_ptr InitialNamingContext ();

        // Dynamic persistence support
        bool isPersistenceEnabled ();
        PortableServer::ObjectId* activatePersistentObject (PortableServer::POA_ptr poa, PortableServer::Servant servant, const std::string& identifier);

        // CosNaming utilities
	CosNaming::Name str2name(const char* namestr);
        CosNaming::Name* stringToName (const std::string& name);
        CORBA::Object_ptr objectFromName (const std::string& name);

        // String (IOR) to/from object utilities
        std::string objectToString (const CORBA::Object_ptr obj);
        CORBA::Object_ptr stringToObject (const std::string& ior);

	// list NamingContext contents as vector of strings...
	std::vector<std::string> listRootContext( );
	std::vector<std::string> listContext(const CosNaming::NamingContext_ptr ctx, const std::string &dname );


        // narrow utility
        template <class T> typename T::_ptr_type _narrowSafe(const CORBA::Object_ptr obj) {
            try {
                // check to make sure that it's a valid reference before attempting to narrow
                if (CORBA::Object::_PR_is_valid(obj)) {
                    return T::_narrow(obj);
                } else {
                    return T::_nil();
                }
            } catch (...) {
                return T::_nil();
            }
        };

	// naming service actions
	enum  NS_ACTION { NS_NOBIND=0, NS_BIND=1, NS_REBIND=2, NS_UNBIND=3 };

	//
	// Create a naming context from the root directory
	//
	int  CreateNamingContext( const std::string &namingContext );
	
	//
	// Create the naming context and all the members in its path
	//
	CosNaming::NamingContext_ptr CreateNamingContextPath(const std::string &namingContext);

	//
	// Return a naming context for a specified naming context path
	//
	CosNaming::NamingContext_ptr ResolveNamingContextPath( const std::string &namingContext );

	//
	// Delete a naming context
	//
	int  DeleteNamingContext( const std::string &namingContext );

	//
	// Delete a naming context path components
	//
	int  DeleteNamingContextPath( const std::string &namingContext );

	//
	// Unbind a name from the specified naming context object
	//
	int  Unbind( const std::string &name , CosNaming::NamingContext_ptr namingContext );

	//
	// Unbind a name from the stringified naming context paths
	//
	int  Unbind( const std::string &name, const std::string &namingContext="" );

	//
	// Bind to naming context    id = name, kind=""
	//
	int  Bind( const std::string &name,  CORBA::Object_ptr obj,  CosNaming::NamingContext_ptr namingContext  );

	//
	// Bind an object to a a name in the specified NamingContext.
	//
	int  Bind( const std::string &name,  CORBA::Object_ptr obj, const std::string &dir="", const bool create_nc=false );

        void bindObjectToContext (const CORBA::Object_ptr obj, const CosNaming::NamingContext_ptr context, const std::string& name);

        // Bind an object to a name in the root naming context.
        void bindObjectToName (const CORBA::Object_ptr obj, const std::string& name);

        unsigned int numberBoundObjectsToContext(CosNaming::NamingContext_ptr context);

        void unbindAllFromContext (CosNaming::NamingContext_ptr context);

        inline std::string returnString (CORBA::String_var corbaString) {
            return std::string(static_cast<const char*>(corbaString));
        }

        bool isValidType (const CORBA::Any& lhs, const CORBA::Any& rhs);

        inline bool objectExists(CORBA::Object_ptr obj) {
            try {
                return (!CORBA::is_nil(obj) && !obj->_non_existent());
            } catch ( CORBA::Exception& e ) {
                return false;
            }
        }

        // Set up a default handler for retrying calls on a COMM_FAILURE exception.
        void setCommFailureRetries (int numRetries);

        // Set up a handler for retrying calls to the provided object on a COMM_FAILURE exception.
        void setObjectCommFailureRetries (CORBA::Object_ptr obj, int numRetries);

        // Mapping of C++ types to type codes.
        template <typename T>
        static CORBA::TypeCode_ptr TypeCode (void)
        {
            return CORBA::_tc_null;
        }

        template<>
        inline CORBA::TypeCode_ptr TypeCode<char> (void)
        {
            return CORBA::_tc_char;
        }
        
        template<>
        inline CORBA::TypeCode_ptr TypeCode<bool> (void)
        {
            return CORBA::_tc_boolean;
        }
        
        template<>
        inline CORBA::TypeCode_ptr TypeCode<CORBA::Octet> (void)
        {
            return CORBA::_tc_octet;
        }
        
        template<>
        inline CORBA::TypeCode_ptr TypeCode<CORBA::Short> (void)
        {
            return CORBA::_tc_short;
        }
        
        template<>
        inline CORBA::TypeCode_ptr TypeCode<CORBA::Long> (void)
        {
            return CORBA::_tc_long;
        }

        template<>
        inline CORBA::TypeCode_ptr TypeCode<CORBA::UShort> (void)
        {
            return CORBA::_tc_ushort;
        }

        template<>
        inline CORBA::TypeCode_ptr TypeCode<CORBA::ULong> (void)
        {
            return CORBA::_tc_ulong;
        }
        
        template<>
        inline CORBA::TypeCode_ptr TypeCode<CORBA::Float> (void)
        {
            return CORBA::_tc_float;
        }
        
        template<>
        inline CORBA::TypeCode_ptr TypeCode<CORBA::Double> (void)
        {
            return CORBA::_tc_double;
        }
        
        template<>
        inline CORBA::TypeCode_ptr TypeCode<CORBA::LongLong> (void)
        {
            return CORBA::_tc_longlong;
        }
        
        template<>
        inline CORBA::TypeCode_ptr TypeCode<CORBA::ULongLong> (void)
        {
            return CORBA::_tc_ulonglong;
        }
        
        template<>
        inline CORBA::TypeCode_ptr TypeCode<std::string> (void)
        {
            return CORBA::_tc_string;
        }

        // Instantiates POAs on demand
        class POACreator : public virtual POA_PortableServer::AdapterActivator
        {
            ENABLE_LOGGING;

        public:
            POACreator() {}
            virtual ~POACreator() { }

            virtual CORBA::Boolean unknown_adapter(PortableServer::POA_ptr parent, const char* name) throw(CORBA::SystemException);
        private:
            // copy not supported
            POACreator(const POACreator&);
            void operator=(const POACreator&);
        }; // class POACreator

        // Vector insertion/extraction operator helpers
        template <class T, class SeqT>
        inline bool vector_extract (const CORBA::Any& _a, std::vector<T>& _s)
        {
            SeqT* seq;
            if (_a >>= seq) {
                size_t length = seq->length();
                if (length == 0) {
                    _s.clear();
                } else {
                    T* begin = (T*)&(*seq)[0];
                    _s.assign(begin, begin+length);
                }
                return true;
            }
            return false;
        }

        template <class T, class SeqT, class ElemT>
        inline void vector_insert (CORBA::Any& _a, const std::vector<T>& _s)
        {
            SeqT seq(_s.size(), _s.size(), (ElemT*)&_s[0], 0);
            _a <<= seq;
        }

        inline bool element_convert(bool in) {
            return in;
        }

        inline char element_convert(CORBA::Char in) {
            return in;
        }

        inline CORBA::Char element_convert(char in) {
            return in;
        }

        inline std::string element_convert(_CORBA_String_element in) {
            return static_cast<const char*>(in);
        }
        
        inline const char* element_convert(const std::string& in) {
            return in.c_str();
        }
        
        template <class T, class SeqT>
        inline bool vector_extract_convert (const CORBA::Any& _a, std::vector<T>& _s)
        {
            SeqT* seq;
            if (_a >>= seq) {
                _s.resize(seq->length());
                for (size_t ii = 0; ii < _s.size(); ++ii) {
                    _s[ii] = element_convert((*seq)[ii]);
                }
                return true;
            }
            return false;
        }

        template <class T, class SeqT>
        inline void vector_insert_convert (CORBA::Any& _a, const std::vector<T>& _s)
        {
            SeqT seq;
            seq.length(_s.size());
            for (size_t ii = 0; ii < _s.size(); ++ii) {
                seq[ii] = element_convert(_s[ii]);
            }
            _a <<= seq;
        }

    }; // namespace corba

}; // namespace ossie


inline bool operator >>= (const CORBA::Any& _a, std::string& _s)
{
    const char* cstr;
    if (_a >>= cstr) {
        _s = cstr;
        return true;
    }
    return false;
}

inline void operator <<= (CORBA::Any& _a, const std::string& _s)
{
    _a <<= _s.c_str();
}

inline bool operator >>= (const CORBA::Any& _a, bool& _b)
{
    CORBA::Boolean b;
    if (_a >>= CORBA::Any::to_boolean(b)) {
        _b = b;
        return true;
    }
    return false;
}

inline void operator <<= (CORBA::Any& _a, const bool _b)
{
    _a <<= CORBA::Any::from_boolean(_b);
}

// Vector insertion/exctraction operators
#define ANY_VECTOR_OPERATORS(T,SEQ)                                 \
inline bool operator >>= (const CORBA::Any& _a, std::vector<T>& _s) \
{                                                                   \
    return ossie::corba::vector_extract<T,SEQ>(_a, _s);             \
}                                                                   \
inline void operator <<= (CORBA::Any& _a, const std::vector<T>& _s) \
{                                                                   \
    ossie::corba::vector_insert<T,SEQ,T>(_a, _s);                   \
}                                                                   \

ANY_VECTOR_OPERATORS(CORBA::Octet, CORBA::OctetSeq);
ANY_VECTOR_OPERATORS(CORBA::Short, CORBA::ShortSeq);
ANY_VECTOR_OPERATORS(CORBA::UShort, CORBA::UShortSeq);
ANY_VECTOR_OPERATORS(CORBA::Long, CORBA::LongSeq);
ANY_VECTOR_OPERATORS(CORBA::ULong, CORBA::ULongSeq);
ANY_VECTOR_OPERATORS(CORBA::LongLong, CORBA::LongLongSeq);
ANY_VECTOR_OPERATORS(CORBA::ULongLong, CORBA::ULongLongSeq);
ANY_VECTOR_OPERATORS(CORBA::Float, CORBA::FloatSeq);
ANY_VECTOR_OPERATORS(CORBA::Double, CORBA::DoubleSeq);
#undef ANY_VECTOR_OPERATORS

#define ANY_VECTOR_CONVERT_OPERATORS(T,SEQ)                         \
inline bool operator >>= (const CORBA::Any& _a, std::vector<T>& _s) \
{                                                                   \
    return ossie::corba::vector_extract_convert<T,SEQ>(_a, _s);     \
}                                                                   \
inline void operator <<= (CORBA::Any& _a, const std::vector<T>& _s) \
{                                                                   \
    ossie::corba::vector_insert_convert<T,SEQ>(_a, _s);             \
}

ANY_VECTOR_CONVERT_OPERATORS(bool, CORBA::BooleanSeq);
ANY_VECTOR_CONVERT_OPERATORS(char, CORBA::CharSeq);
ANY_VECTOR_CONVERT_OPERATORS(std::string, CORBA::StringSeq);

#undef ANY_VECTOR_CONVERT_OPERATORS

#endif // OSSIE_CORBAUTILS_H
