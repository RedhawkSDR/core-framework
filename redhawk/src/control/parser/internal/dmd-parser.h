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

#ifndef __DMD_PARSER_H__
#define __DMD_PARSER_H__

#include<sstream>
#include<istream>
#include "dmd-pimpl.h"
#include"ossie/exceptions.h"

namespace ossie {
    namespace internalparser {
        inline std::auto_ptr<ossie::DomainManagerConfiguration::DMD> parseDMD(std::istream& input) throw (ossie::parser_error) {
            using namespace dmd;

            try {
                // Instantiate individual parsers.
                //
                domainmanagerconfiguration_pimpl domainmanagerconfiguration_p;
                ::xml_schema::string_pimpl string_p;
                domainmanagersoftpkg_pimpl domainmanagersoftpkg_p;
                localfile_pimpl localfile_p;
                services_pimpl services_p;
                service_pimpl service_p;
                findby_pimpl findby_p;
                namingservice_pimpl namingservice_p;
                ::xml_schema::any_simple_type_pimpl any_simple_type_p;
                domainfinder_pimpl domainfinder_p;

                // Connect the parsers together.
                //
                domainmanagerconfiguration_p.parsers (string_p,
                                                    domainmanagersoftpkg_p,
                                                    services_p,
                                                    string_p,
                                                    string_p);

                domainmanagersoftpkg_p.parsers (localfile_p);

                localfile_p.parsers (string_p);

                services_p.parsers (service_p);

                service_p.parsers (string_p,
                                findby_p);

                findby_p.parsers (namingservice_p,
                                string_p,
                                domainfinder_p);

                namingservice_p.parsers (any_simple_type_p);

                domainfinder_p.parsers (string_p,
                                    string_p);

                // Parse the XML document.
                //
                ::xml_schema::document doc_p (domainmanagerconfiguration_p, "", "domainmanagerconfiguration");

                domainmanagerconfiguration_p.pre ();
                doc_p.parse (input);
                return (domainmanagerconfiguration_p.post_domainmanagerconfiguration ());
            } catch (const ::xml_schema::exception& e) {
                std::ostringstream err;
                err << e;
                throw ossie::parser_error(err.str());
            } catch (const std::ios_base::failure& e) {
                throw ossie::parser_error(e.what());
            }
        }
    }
}

#endif
