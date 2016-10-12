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


#ifndef ORBSUPPORT_H
#define ORBSUPPORT_H

#include <string>
#include <vector>

#include <sched.h>

#ifdef HAVE_OMNIORB4_CORBA_H
#include "omniORB4/CORBA.h"
#endif

#include "ossie/CF/cf.h"
#include "ossie/CF/StandardEvent.h"

#include "debug.h"

/**
The ossieSupport namespace contains useful functions used throughout the
OSSIE framework. The classes in this namespace are also useful for SCA
component developers.
*/

namespace ossie
{

    /**
    The ORB class provides access to a CORBA orb variable across multiple classes.

    */
    class ORB
    {
    public:
        /**
           The first time the constructor runs, a CORBA orb variable is
           initialized. Additionally, variables for the POA, POA manager, and
           initial naming context are created. Creating additional ORB objects
           provides access to these variables.
         */
        ORB();

        /// This constructor provides argc and argv to the CORBA orb when it is created.
        ORB(int argc, char* argv[]);

        /**
           The destructor decrements a reference count, when the last reference
           to the orb is destroyed, the destructor releases the orb variable.
        */
        ~ORB();


        ///\todo Define exceptions for these methods to through

        /// Return an object reference for the object name contained in name.
        CORBA::Object_ptr get_object_from_name(const char* name);

        CORBA::Object_ptr get_object_from_name(const CosNaming::NamingContext_ptr nc, const char* name);

        /// Return a pointer to a CosName from a string. Free the returned pointer when you are finished with it.
        CosNaming::Name_var string_to_CosName(const char* name);

        /// Create an entry, name, in the Naming Service for the object reference obj.
        void bind_object_to_name(CORBA::Object_ptr obj, const char* name);
        void bind_object_to_name(CORBA::Object_ptr obj, const CosNaming::NamingContext_ptr nc, const char* name);

        /// Remove a name from the Naming Service
        void unbind_name(const char* name);
        void unbind_name(const CosNaming::NamingContext_ptr nc, const char* name);

        /// Remove all names from a Context
        void unbind_all_from_context(const CosNaming::NamingContext_ptr nc);

        // ORB variables
        static CORBA::ORB_var orb;
        static PortableServer::POA_var poa;
        static PortableServer::POAManager_var pman;
        static CosNaming::NamingContext_var inc;

    private:
        void init(void);
    };

// Miscellaneous helper functions

    void createProfileFromFileName(std::string fileName, std::string& profile);
    std::string getFileNameFromProfile(std::string profile);
    bool isValidFileName(const char* fileName);
    const char* spd_rel_file(const char* spdfile, const char* name, std::string& fileName);

    void configureLogging(const char* logcfgUri, int defaultLevel);

    std::string generateUUID();

    class ossieComponent
    {
        ENABLE_LOGGING

    public:
        ossieComponent(int argc, char* argv[]);
        ~ossieComponent();

        void bind(CF::Resource_ptr res);
        void unbind();

        const char* getUuid();
        void getCLArgs(CF::Properties& props);

    private:
        ossieComponent();

        std::string namingContextIOR;
        std::string uuid;
        std::string nameBinding;
        bool namingContextIORFound;
        bool uuidFound;
        bool nameBindingFound;

        int scheduler;
        sched_param p;

        CosNaming::NamingContext_var nc;

        unsigned int numArgs;
        CF::Properties CLArgs;
    };

    
    namespace helpers {

      /**
         is_jarfile 
     
         Helper method to test if a file is valid jar file in lue of "file" command
         due to results differences from various OS distros

         @return 0   contents of jarPath is a valid jar format
         @return 1   contents of file does not match magic number format
         @return -1  file acces/open error

      */

      int is_jarfile( const std::string &jarPath );

    };


}  // Close ossieSupport Namespace
#endif
