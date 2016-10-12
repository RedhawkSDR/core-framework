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

#ifndef __SCD_PARSER_H__
#define __SCD_PARSER_H__

#include "scd-pimpl.h"
#include "ossie/exceptions.h"

namespace ossie {
    namespace internalparser {
        inline std::auto_ptr<ossie::ComponentDescriptor::SCD> parseSCD(std::istream& input) throw (ossie::parser_error)
        {
            using namespace scd;

            try {
                // Instantiate individual parsers.
                //
                softwarecomponent_pimpl softwarecomponent_p;
                ::xml_schema::string_pimpl string_p;
                componentRepId_pimpl componentRepId_p;
                componentFeatures_pimpl componentFeatures_p;
                supportsInterface_pimpl supportsInterface_p;
                ports_pimpl ports_p;
                provides_pimpl provides_p;
                portType_pimpl portType_p;
                type_pimpl type_p;
                uses_pimpl uses_p;
                interfaces_pimpl interfaces_p;
                interface_pimpl interface_p;
                inheritsInterface_pimpl inheritsInterface_p;
                propertyFile_pimpl propertyFile_p;
                localFile_pimpl localFile_p;

                // Connect the parsers together.
                //
                softwarecomponent_p.parsers (string_p,
                                            componentRepId_p,
                                            string_p,
                                            componentFeatures_p,
                                            interfaces_p,
                                            propertyFile_p);

                componentRepId_p.parsers (string_p);

                componentFeatures_p.parsers (supportsInterface_p,
                                            ports_p);

                supportsInterface_p.parsers (string_p,
                                            string_p);

                ports_p.parsers (provides_p,
                                uses_p);

                provides_p.parsers (portType_p,
                                    string_p,
                                    string_p);

                portType_p.parsers (type_p);

                uses_p.parsers (portType_p,
                                string_p,
                                string_p);

                interfaces_p.parsers (interface_p);

                interface_p.parsers (inheritsInterface_p,
                                    string_p,
                                    string_p);

                inheritsInterface_p.parsers (string_p);

                propertyFile_p.parsers (localFile_p,
                                        string_p);

                localFile_p.parsers (string_p);

                // Parse the XML document.
                //
                ::xml_schema::document doc_p (softwarecomponent_p, "softwarecomponent");

                softwarecomponent_p.pre ();
                doc_p.parse (input);
                return (softwarecomponent_p.post_softwarecomponent ());
            } catch (const ::xml_schema::exception& e) {
                throw ossie::parser_error(e.what());
            } catch (const std::ios_base::failure& e) {
                throw ossie::parser_error(e.what());
            }
        }
    }
}
#endif
