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

#ifndef __OSSIE_EXCEPTIONS_H__
#define __OSSIE_EXCEPTIONS_H__
#include <stdexcept>

namespace ossie {

    class parser_error : public std::runtime_error {
        public:
            parser_error(const std::string& what_arg) : std::runtime_error(what_arg)
        {}
    };

    class PropertyMatchingError : public std::runtime_error {
        public:
            PropertyMatchingError(const std::string& what_arg) : std::runtime_error(what_arg)
        {}
    };

    class PersistenceException : public std::runtime_error {
        public:
            PersistenceException(const std::string& what_arg) : std::runtime_error(what_arg)
        {}
    };
}
#endif
