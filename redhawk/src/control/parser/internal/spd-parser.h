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

#ifndef __SPD_PARSER_H__
#define __SPD_PARSER_H__

#include<sstream>
#include<istream>
#include"ossie/exceptions.h"
#include "spd-pimpl.h"

#include <iostream>

namespace ossie {
    namespace internalparser {
        inline std::auto_ptr<ossie::SPD> parseSPD(std::istream& input) throw (ossie::parser_error) {
            using namespace spd;

            try {
                // Instantiate individual parsers.
                //
                ::spd::softPkg_pimpl softPkg_p;
                ::xml_schema::string_pimpl string_p;
                ::spd::author_pimpl author_p;
                ::xml_schema::uri_pimpl uri_p;
                ::spd::propertyFile_pimpl propertyFile_p;
                ::spd::localFile_pimpl localFile_p;
                ::spd::descriptor_pimpl descriptor_p;
                ::spd::implementation_pimpl implementation_p;
                ::spd::code_pimpl code_p;
                ::xml_schema::unsigned_long_pimpl unsigned_long_p;
                ::spd::codeFileType_pimpl codeFileType_p;
                ::spd::compiler_pimpl compiler_p;
                ::spd::programmingLanguage_pimpl programmingLanguage_p;
                ::spd::humanLanguage_pimpl humanLanguage_p;
                ::spd::runtime_pimpl runtime_p;
                ::spd::os_pimpl os_p;
                ::spd::processor_pimpl processor_p;
                ::spd::dependency_pimpl dependency_p;
                ::spd::softPkgRef_pimpl softPkgRef_p;
                ::spd::implRef_pimpl implRef_p;
                ::spd::propertyRef_pimpl propertyRef_p;
                ::spd::usesDevice_pimpl usesDevice_p;
                ::spd::aepcompliance_pimpl aepcompliance_p;
                ::spd::scaComplianceType_pimpl scaComplianceType_p;
                ::spd::simpleref_pimpl simpleref_p;
                ::spd::simplesequenceref_pimpl simplesequenceref_p;
                ::spd::structref_pimpl structref_p;
                ::spd::structsequenceref_pimpl structsequenceref_p;
                ::spd::values_pimpl values_p;
                ::spd::structvalue_pimpl structvalue_p;

                // Connect the parsers together.
                //
                softPkg_p.parsers (string_p,
                                author_p,
                                string_p,
                                propertyFile_p,
                                descriptor_p,
                                implementation_p,
                                usesDevice_p,
                                string_p,
                                string_p,
                                scaComplianceType_p,
                                string_p);

                author_p.parsers (string_p,
                                string_p,
                                uri_p);

                propertyFile_p.parsers (localFile_p,
                                        string_p);

                localFile_p.parsers (string_p);

                descriptor_p.parsers (localFile_p,
                                    string_p);

                implementation_p.parsers (string_p,
                                        propertyFile_p,
                                        code_p,
                                        compiler_p,
                                        programmingLanguage_p,
                                        humanLanguage_p,
                                        runtime_p,
                                        os_p,
                                        processor_p,
                                        dependency_p,
                                        usesDevice_p,
                                        string_p,
                                        aepcompliance_p);

                code_p.parsers (localFile_p,
                                string_p,
                                unsigned_long_p,
                                unsigned_long_p,
                                codeFileType_p);

                compiler_p.parsers (string_p,
                                    string_p);

                programmingLanguage_p.parsers (string_p,
                                                string_p);

                humanLanguage_p.parsers (string_p);

                runtime_p.parsers (string_p,
                                    string_p);

                os_p.parsers (string_p,
                            string_p);

                processor_p.parsers (string_p);

                dependency_p.parsers (softPkgRef_p,
                                    propertyRef_p,
                                    simpleref_p,
                                    simplesequenceref_p,
                                    structref_p,
                                    structsequenceref_p,
                                    string_p);

                softPkgRef_p.parsers (localFile_p,
                                    implRef_p);

                implRef_p.parsers (string_p);

                propertyRef_p.parsers (string_p,
                                    string_p);

                usesDevice_p.parsers (propertyRef_p,
                                    simpleref_p,
                                    simplesequenceref_p,
                                    structref_p,
                                    structsequenceref_p,
                                    string_p,
                                    string_p);

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

                // Parse the XML document.
                //
                ::xml_schema::document doc_p (
                    softPkg_p,
                    "",
                    "softpkg");

                softPkg_p.pre ();
                doc_p.parse (input);
                return (softPkg_p.post_softPkg ());
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
