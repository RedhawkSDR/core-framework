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

#ifndef __DCD_PARSER_H__
#define __DCD_PARSER_H__

#include<sstream>
#include<istream>
#include"ossie/exceptions.h"
#include "dcd-pimpl.h"

#include <iostream>

namespace ossie {
    namespace internalparser {
        inline std::auto_ptr<ossie::DeviceManagerConfiguration::DCD> parseDCD(std::istream& input) throw (ossie::parser_error) {
            try {
                // Instantiate individual parsers.
                //
                ::dcd::deviceconfiguration_pimpl deviceconfiguration_p;
                ::xml_schema::string_pimpl string_p;
                ::dcd::devicemanagersoftpkg_pimpl devicemanagersoftpkg_p;
                ::dcd::localfile_pimpl localfile_p;
                ::dcd::componentfiles_pimpl componentfiles_p;
                ::dcd::componentfile_pimpl componentfile_p;
                ::dcd::partitioning_pimpl partitioning_p;
                ::dcd::componentplacement_pimpl componentplacement_p;
                ::dcd::componentfileref_pimpl componentfileref_p;
                ::dcd::deployondevice_pimpl deployondevice_p;
                ::dcd::compositepartofdevice_pimpl compositepartofdevice_p;
                ::dcd::devicepkgfile_pimpl devicepkgfile_p;
                ::dcd::componentinstantiation_pimpl componentinstantiation_p;
                ::dcd::componentproperties_pimpl componentproperties_p;
                ::dcd::simpleref_pimpl simpleref_p;
                ::dcd::simplesequenceref_pimpl simplesequenceref_p;
                ::dcd::values_pimpl values_p;
                ::dcd::structref_pimpl structref_p;
                ::dcd::structsequenceref_pimpl structsequenceref_p;
                ::dcd::structvalue_pimpl structvalue_p;
                ::dcd::domainmanager_pimpl domainmanager_p;
                ::dcd::namingservice_pimpl namingservice_p;
                ::dcd::connections_pimpl connections_p;
                ::dcd::connectinterface_pimpl connectinterface_p;
                ::dcd::usesport_pimpl usesport_p;
                ::dcd::componentinstantiationref_pimpl componentinstantiationref_p;
                ::dcd::devicethatloadedthiscomponentref_pimpl devicethatloadedthiscomponentref_p;
                ::dcd::deviceusedbythiscomponentref_pimpl deviceusedbythiscomponentref_p;
                ::dcd::findby_pimpl findby_p;
                ::dcd::domainfinder_pimpl domainfinder_p;
                ::dcd::providesport_pimpl providesport_p;
                ::dcd::componentsupportedinterface_pimpl componentsupportedinterface_p;
                ::dcd::filesystemnames_pimpl filesystemnames_p;
                ::dcd::filesystemname_pimpl filesystemname_p;

                // Connect the parsers together.
                //
                deviceconfiguration_p.parsers (string_p,
                                            devicemanagersoftpkg_p,
                                            componentfiles_p,
                                            partitioning_p,
                                            connections_p,
                                            domainmanager_p,
                                            filesystemnames_p,
                                            string_p,
                                            string_p);

                devicemanagersoftpkg_p.parsers (localfile_p);

                localfile_p.parsers (string_p);

                componentfiles_p.parsers (componentfile_p);

                componentfile_p.parsers (localfile_p,
                                    string_p,
                                    string_p);

                partitioning_p.parsers (componentplacement_p);

                componentplacement_p.parsers (componentfileref_p,
                                            deployondevice_p,
                                            compositepartofdevice_p,
                                            devicepkgfile_p,
                                            componentinstantiation_p);

                componentfileref_p.parsers (string_p);

                deployondevice_p.parsers (string_p);

                compositepartofdevice_p.parsers (string_p);

                devicepkgfile_p.parsers (localfile_p,
                                    string_p);

                componentinstantiation_p.parsers (string_p,
                                            componentproperties_p,
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

                domainmanager_p.parsers (namingservice_p,
                                        string_p);

                namingservice_p.parsers (string_p);

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
                                    findby_p);

                componentinstantiationref_p.parsers (string_p);

                devicethatloadedthiscomponentref_p.parsers (string_p);

                deviceusedbythiscomponentref_p.parsers (string_p,
                                                        string_p);

                findby_p.parsers (namingservice_p,
                                string_p,
                                domainfinder_p);

                domainfinder_p.parsers (string_p,
                                        string_p);

                providesport_p.parsers (string_p,
                                        componentinstantiationref_p,
                                        devicethatloadedthiscomponentref_p,
                                        deviceusedbythiscomponentref_p,
                                        findby_p);

                componentsupportedinterface_p.parsers (string_p,
                                                   componentinstantiationref_p,
                                                   findby_p);

                filesystemnames_p.parsers (filesystemname_p);

                filesystemname_p.parsers (string_p,
                                          string_p);

                // Parse the XML document.
                //
                ::xml_schema::document doc_p (
                        deviceconfiguration_p,
                        "",
                        "deviceconfiguration");

                deviceconfiguration_p.pre ();
                doc_p.parse (input);
                return (deviceconfiguration_p.post_deviceconfiguration ());
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
