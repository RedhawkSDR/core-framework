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
#ifndef PROP_TRIGGER_TIMING_BASE_IMPL_BASE_H
#define PROP_TRIGGER_TIMING_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include "struct_props.h"

class prop_trigger_timing_base : public Component, protected ThreadedComponent
{
    public:
        prop_trigger_timing_base(const char *uuid, const char *label);
        ~prop_trigger_timing_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: prop_1
        std::string prop_1;
        /// Property: prop_1_trigger
        bool prop_1_trigger;
        /// Property: prop_2_trigger
        bool prop_2_trigger;
        /// Property: prop_3_trigger
        bool prop_3_trigger;
        /// Property: prop_4_trigger
        bool prop_4_trigger;
        /// Property: prop_2
        std::vector<std::string> prop_2;
        /// Property: prop_3
        prop_3_struct prop_3;
        /// Property: prop_4
        std::vector<prop_4_a_struct> prop_4;

    private:
};
#endif // PROP_TRIGGER_TIMING_BASE_IMPL_BASE_H
