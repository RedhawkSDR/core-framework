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

#ifndef __SAD_PARSER_H__
#define __SAD_PARSER_H__

#include<sstream>
#include<istream>
#include"ossie/exceptions.h"
#include "sad-pimpl.h"

#include <iostream>

namespace ossie {
    namespace internalparser {
        inline std::auto_ptr<ossie::SoftwareAssembly::SAD> parseSAD(std::istream& input) throw (ossie::parser_error) {
            try {
                // Instantiate individual parsers.
                //
                ::sad::softwareassembly_pimpl softwareassembly_p;
                ::xml_schema::string_pimpl string_p;
                ::sad::componentfiles_pimpl componentfiles_p;
                ::sad::componentfile_pimpl componentfile_p;
                ::sad::localfile_pimpl localfile_p;
                ::sad::partitioning_pimpl partitioning_p;
                ::sad::componentplacement_pimpl componentplacement_p;
                ::sad::componentfileref_pimpl componentfileref_p;
                ::sad::componentinstantiation_pimpl componentinstantiation_p;
                ::sad::componentproperties_pimpl componentproperties_p;
                ::sad::simpleref_pimpl simpleref_p;
                ::sad::simplesequenceref_pimpl simplesequenceref_p;
                ::sad::values_pimpl values_p;
                ::sad::structref_pimpl structref_p;
                ::sad::structsequenceref_pimpl structsequenceref_p;
                ::sad::structvalue_pimpl structvalue_p;
                ::sad::findcomponent_pimpl findcomponent_p;
                ::sad::componentresourcefactoryref_pimpl componentresourcefactoryref_p;
                ::sad::resourcefactoryproperties_pimpl resourcefactoryproperties_p;
                ::sad::namingservice_pimpl namingservice_p;
                ::sad::hostcollocation_pimpl hostcollocation_p;
                ::sad::assemblycontroller_pimpl assemblycontroller_p;
                ::sad::componentinstantiationref_pimpl componentinstantiationref_p;
                ::sad::connections_pimpl connections_p;
                ::sad::connectinterface_pimpl connectinterface_p;
                ::sad::usesport_pimpl usesport_p;
                ::sad::devicethatloadedthiscomponentref_pimpl devicethatloadedthiscomponentref_p;
                ::sad::deviceusedbythiscomponentref_pimpl deviceusedbythiscomponentref_p;
                ::sad::deviceusedbyapplication_pimpl deviceusedbyapplication_p;
                ::sad::findby_pimpl findby_p;
                ::sad::domainfinder_pimpl domainfinder_p;
                ::sad::providesport_pimpl providesport_p;
                ::sad::componentsupportedinterface_pimpl componentsupportedinterface_p;
                ::sad::externalports_pimpl externalports_p;
                ::sad::port_pimpl port_p;
                ::sad::externalproperties_pimpl externalproperties_p;
                ::sad::property_pimpl property_p;
                ::sad::usesdevicedependencies_pimpl usesdevicedependencies_p;
                ::sad::usesdevice_pimpl usesdevice_p;
                ::sad::propertyref_pimpl propertyref_p;

                // Connect the parsers together.
                //
                softwareassembly_p.parsers (string_p,
                    componentfiles_p,
                    partitioning_p,
                    assemblycontroller_p,
                    connections_p,
                    externalports_p,
                    externalproperties_p,
                    usesdevicedependencies_p,
                    string_p,
                    string_p,
                    string_p);

                componentfiles_p.parsers (componentfile_p);

                componentfile_p.parsers (localfile_p,
                                        string_p,
                                        string_p);

                localfile_p.parsers (string_p);

                partitioning_p.parsers (componentplacement_p,
                                        hostcollocation_p);

                componentplacement_p.parsers (componentfileref_p,
                                            componentinstantiation_p);

                componentfileref_p.parsers (string_p);

                componentinstantiation_p.parsers (string_p,
                                                componentproperties_p,
                                                findcomponent_p,
                                                string_p,
                                                string_p);

                componentproperties_p.parsers (simpleref_p,
                                            simplesequenceref_p,
                                            structref_p,
                                            structsequenceref_p);

                simpleref_p.parsers (string_p,
                                    string_p);

                simplesequenceref_p.parsers (values_p,
                                            string_p);

                values_p.parsers (string_p);

                structref_p.parsers (simpleref_p,
                                    string_p);

                structsequenceref_p.parsers (structvalue_p,
                                            string_p);

                structvalue_p.parsers (simpleref_p);

                findcomponent_p.parsers (componentresourcefactoryref_p,
                                        namingservice_p);

                componentresourcefactoryref_p.parsers (resourcefactoryproperties_p,
                                                    string_p);

                resourcefactoryproperties_p.parsers (simpleref_p,
                                                    simplesequenceref_p,
                                                    structref_p,
                                                    structsequenceref_p);

                namingservice_p.parsers (string_p);

                hostcollocation_p.parsers (componentplacement_p,
                                        string_p,
                                        string_p);

                assemblycontroller_p.parsers (componentinstantiationref_p);

                componentinstantiationref_p.parsers (string_p);

                connections_p.parsers (connectinterface_p);

                connectinterface_p.parsers (usesport_p,
                                            providesport_p,
                                            componentsupportedinterface_p,
                                            findby_p,
                                            string_p);

                usesport_p.parsers (string_p,
                                    componentinstantiationref_p,
                                    devicethatloadedthiscomponentref_p,
                                    deviceusedbythiscomponentref_p,
                                    deviceusedbyapplication_p,
                                    findby_p);

                devicethatloadedthiscomponentref_p.parsers (string_p);

                deviceusedbythiscomponentref_p.parsers (string_p,
                                                    string_p);

                deviceusedbyapplication_p.parsers(string_p);

                findby_p.parsers (namingservice_p,
                                string_p,
                                domainfinder_p);

                domainfinder_p.parsers (string_p,
                                        string_p);

                providesport_p.parsers (string_p,
                                        componentinstantiationref_p,
                                        devicethatloadedthiscomponentref_p,
                                        deviceusedbythiscomponentref_p,
                                        deviceusedbyapplication_p,
                                        findby_p);

                componentsupportedinterface_p.parsers (string_p,
                                                    componentinstantiationref_p,
                                                    devicethatloadedthiscomponentref_p,
                                                    deviceusedbythiscomponentref_p,
                                                    deviceusedbyapplication_p,
                                                    findby_p);

                externalports_p.parsers (port_p);

                port_p.parsers (string_p,
                                string_p,
                                string_p,
                                string_p,
                                componentinstantiationref_p,
                                string_p);

                externalproperties_p.parsers (property_p);

                property_p.parsers (string_p,
                                    string_p,
                                    string_p);

                usesdevicedependencies_p.parsers (usesdevice_p);

                usesdevice_p.parsers (propertyref_p,
                                      simpleref_p,
                                      simplesequenceref_p,
                                      structref_p,
                                      structsequenceref_p,
                                      string_p,
                                      string_p);

                propertyref_p.parsers (string_p,
                                       string_p);

                // Parse the XML document.
                //
                ::xml_schema::document doc_p (
                        softwareassembly_p,
                        "",
                        "softwareassembly");

                softwareassembly_p.pre ();
                doc_p.parse (input);
                return (softwareassembly_p.post_softwareassembly ());
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
