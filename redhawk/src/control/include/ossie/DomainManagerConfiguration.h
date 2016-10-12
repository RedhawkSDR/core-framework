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


#ifndef DMDPARSER_H
#define DMDPARSER_H

#include <string>
#include <istream>
#include "ossie/debug.h"
#include "ossie/exceptions.h"
#include "ossie/ossieparser.h"

namespace ossie {

    class DomainManagerConfiguration
    {
        ENABLE_LOGGING

        /////////////////////////////
        // Internal class definitions
        public:
            class DMD {
                public:
                    std::string id;
                    std::string name;
                    std::string softpkg;
            };

        ///////////////////////////
        // Constructors/Destructors
        public:
            /**
             * The default constructor can be used to create a DomainManagerConfiguration
             * but you cannot call any 'gettors' before first calling 'load'.  Since this
             * is a programming error the class uses assert statements (rather than exceptions)
             * to check that this requirement is held.
             */
            DomainManagerConfiguration() : _dmd(0) {};

            DomainManagerConfiguration(std::istream& input) throw (ossie::parser_error);

            ~DomainManagerConfiguration();

            DomainManagerConfiguration& operator=(DomainManagerConfiguration other);

        //////////
        // Methods
        public:
            void load(std::istream& input) throw (ossie::parser_error);

            const char* getID() const;

            const char* getName() const;

            const char* getDomainManagerSoftPkg() const;

            const char* toString() const;

        protected:
            DomainManagerConfiguration(DomainManagerConfiguration& c); // For now don't support copy constructor

        //////////
        // Members
        protected:
            std::auto_ptr<DMD> _dmd;
    };
}
#endif
