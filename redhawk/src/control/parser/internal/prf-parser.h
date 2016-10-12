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

#ifndef __PRF_PARSER_H__
#define __PRF_PARSER_H__

#include"prf-pimpl.h"
#include"ossie/exceptions.h"
#include"ossie/Properties.h"

namespace ossie {
    namespace internalparser {
        inline std::auto_ptr<ossie::PRF> parsePRF(std::istream& input) throw (ossie::parser_error) {
            using namespace prf;
            try {
                // Instantiate individual parsers.
                //
                properties_pimpl properties_p;
                ::xml_schema::string_pimpl string_p;
                simple_pimpl simple_p;
                Unit_pimpl Unit_p;
                range_pimpl range_p;
                enumerations_pimpl enumerations_p;
                enumeration_pimpl enumeration_p;
                kind_pimpl kind_p;
                PropertyConfigurationType_pimpl PropertyConfigurationType_p;
                action_pimpl action_p;
                ActionType_pimpl ActionType_p;
                AccessType_pimpl AccessType_p;
                PropertyValueType_pimpl PropertyValueType_p;
                simpleSequence_pimpl simpleSequence_p;
                values_pimpl values_p;
                test_pimpl test_p;
                inputValue_pimpl inputValue_p;
                resultValue_pimpl resultValue_p;
                struct_pimpl struct_p;
                configurationKind_pimpl configurationKind_p;
                StructPropertyConfigurationType_pimpl StructPropertyConfigurationType_p;
                structSequence_pimpl structSequence_p;
                structValue_pimpl structValue_p;
                IsComplex_pimpl IsComplex_p;
                simpleRef_pimpl simpleRef_p;

                // Connect the parsers together.
                //
                properties_p.parsers (string_p,
                                    simple_p,
                                    simpleSequence_p,
                                    test_p,
                                    struct_p,
                                    structSequence_p);

                simple_p.parsers (string_p,
                                string_p,
                                Unit_p,
                                range_p,
                                enumerations_p,
                                kind_p,
                                action_p,
                                string_p,
                                AccessType_p,
                                string_p,
                                IsComplex_p,
                                PropertyValueType_p);

                range_p.parsers (string_p,
                                string_p);

                enumerations_p.parsers (enumeration_p);

                enumeration_p.parsers (string_p,
                                    string_p);

                kind_p.parsers (PropertyConfigurationType_p);

                action_p.parsers (ActionType_p);

                simpleSequence_p.parsers (string_p,
                                        values_p,
                                        Unit_p,
                                        range_p,
                                        kind_p,
                                        action_p,
                                        string_p,
                                        AccessType_p,
                                        string_p,
                                        PropertyValueType_p,
                                        IsComplex_p);

                values_p.parsers (string_p);

                test_p.parsers (string_p,
                                inputValue_p,
                                resultValue_p,
                                string_p);

                inputValue_p.parsers (simple_p);

                resultValue_p.parsers (simple_p);

                struct_p.parsers (string_p,
                                simple_p,
                                configurationKind_p,
                                string_p,
                                AccessType_p,
                                string_p);

                configurationKind_p.parsers (StructPropertyConfigurationType_p);

                structSequence_p.parsers (string_p,
                                        struct_p,
                                        structValue_p,
                                        configurationKind_p,
                                        string_p,
                                        AccessType_p,
                                        string_p);

                structValue_p.parsers (simpleRef_p);

                simpleRef_p.parsers (string_p,
                                    string_p);

                // Parse the XML document.
                //
                ::xml_schema::document doc_p (properties_p, "properties");

                properties_p.pre ();
                doc_p.parse (input);
                return properties_p.post_properties ();
            } catch (const ::xml_schema::exception& e) {
                std::ostringstream message;
                message << e;
                throw ossie::parser_error(message.str());
            } catch (const std::ios_base::failure& e) {
                throw ossie::parser_error(e.what());
            }
        }
    }
}
#endif
