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

#include <ossie/Versions.h>

namespace redhawk {
    int compareVersions(const std::string& a, const std::string& b) {
    
        if (a == b) {
            return 0;
        } else if (a == "sca_compliant") {
            return 1;
        } else if (b == "sca_compliant") {
            return -1;
        }
    
        std::vector<int> first_tokens;
        std::vector<int> second_tokens;
        try {
            std::string token;
            std::istringstream first(a);
            while (std::getline(first, token, '.')) {
                if (!token.empty())
                    first_tokens.push_back(atoi(token.c_str()));
            }
            std::istringstream second(b);
            while (std::getline(second, token, '.')) {
                if (!token.empty())
                    second_tokens.push_back(atoi(token.c_str()));
            }
        } catch ( ... ) {
            throw std::runtime_error("Invalid tokens");
        }
        if (first_tokens.size() != second_tokens.size())
            throw std::runtime_error("Mismatched token set size");
    
        for (unsigned int i=0; i<first_tokens.size(); i++) {
            if (first_tokens[i] > second_tokens[i])
                return -1;
            if (first_tokens[i] < second_tokens[i])
                return 1;
        }
        return 0;
    }
}
