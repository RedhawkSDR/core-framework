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

#ifndef REDHAWK_SHM_ENVIRONMENT_H
#define REDHAWK_SHM_ENVIRONMENT_H

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <boost/lexical_cast.hpp>

namespace redhawk {

    namespace env {

        namespace detail {
            template <typename T>
            inline T convert(const char* value)
            {
                return boost::lexical_cast<T>(value);
            }

            template<>
            inline const char* convert(const char* value)
            {
                return value;
            }
        }

        template <typename T>
        inline T getVariable(const char* var, T defval)
        {
            const char* env_value = getenv(var);
            if (env_value && (*env_value != '\0')) {
                try {
                    return detail::convert<T>(env_value);
                } catch (...) {
                    std::cerr << "Invalid value for " << var << ": '" << env_value << "'" << std::endl;
                }
            }

            return defval;
        }

        inline bool getEnable(const char* var, bool defval)
        {
            const char* env_value = getenv(var);
            if (env_value) {
                return (env_value != std::string("disable"));
            }
            return defval;
        }
    }
}

#endif // REDHAWK_SHM_ENVIRONMENT_H
