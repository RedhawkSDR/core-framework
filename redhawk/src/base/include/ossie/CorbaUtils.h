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

#include <omniORB4/CORBA.h>

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
        CosNaming::Name* stringToName (const std::string& name);
        CORBA::Object_ptr objectFromName (const std::string& name);

        // String (IOR) to/from object utilities
        std::string objectToString (const CORBA::Object_ptr obj);
        CORBA::Object_ptr stringToObject (const std::string& ior);

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

        // Bind an object to a a name in the specified NamingContext.
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
        static CORBA::TCKind TypeCode (void)
        {
            return CORBA::tk_null;
        }

        template<>
        inline CORBA::TCKind TypeCode<char> (void)
        {
            return CORBA::tk_char;
        }
        
        template<>
        inline CORBA::TCKind TypeCode<bool> (void)
        {
            return CORBA::tk_boolean;
        }
        
        template<>
        inline CORBA::TCKind TypeCode<CORBA::Octet> (void)
        {
            return CORBA::tk_octet;
        }
        
        template<>
        inline CORBA::TCKind TypeCode<CORBA::Short> (void)
        {
            return CORBA::tk_short;
        }
        
        template<>
        inline CORBA::TCKind TypeCode<CORBA::Long> (void)
        {
            return CORBA::tk_long;
        }

        
        template<>
        inline CORBA::TCKind TypeCode<CORBA::UShort> (void)
        {
            return CORBA::tk_ushort;
        }
        
        template<>
        inline CORBA::TCKind TypeCode<CORBA::ULong> (void)
        {
            return CORBA::tk_ulong;
        }
        
        template<>
        inline CORBA::TCKind TypeCode<CORBA::Float> (void)
        {
            return CORBA::tk_float;
        }
        
        template<>
        inline CORBA::TCKind TypeCode<CORBA::Double> (void)
        {
            return CORBA::tk_double;
        }
        
        template<>
        inline CORBA::TCKind TypeCode<std::string> (void)
        {
            return CORBA::tk_string;
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

#endif // OSSIE_CORBAUTILS_H
