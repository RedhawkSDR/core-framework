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
#ifndef PROP_TRIGGER_TIMING_I_IMPL_H
#define PROP_TRIGGER_TIMING_I_IMPL_H

#include "prop_trigger_timing_base.h"

class prop_trigger_timing_i : public prop_trigger_timing_base
{
    ENABLE_LOGGING
    public:
        prop_trigger_timing_i(const char *uuid, const char *label);
        ~prop_trigger_timing_i();

        void constructor();

        int serviceFunction();
        void prop_1_changed(const std::string* oldValue, const std::string* newValue);
        void prop_2_changed(const std::vector<std::string>* oldValue, const std::vector<std::string>* newValue);
        void prop_3_changed(const prop_3_struct* oldValue, const prop_3_struct* newValue);
        void prop_4_changed(const std::vector<prop_4_a_struct>* oldValue, const std::vector<prop_4_a_struct>* newValue);
};

#endif // PROP_TRIGGER_TIMING_I_IMPL_H
