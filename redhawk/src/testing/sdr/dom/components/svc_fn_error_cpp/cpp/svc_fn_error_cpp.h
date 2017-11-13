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
#ifndef SVC_FN_ERROR_CPP_I_IMPL_H
#define SVC_FN_ERROR_CPP_I_IMPL_H

#include "svc_fn_error_cpp_base.h"

class svc_fn_error_cpp_i : public svc_fn_error_cpp_base
{
    ENABLE_LOGGING
    public:
        svc_fn_error_cpp_i(const char *uuid, const char *label);
        ~svc_fn_error_cpp_i();

        void constructor();

        int serviceFunction();
};

#endif // SVC_FN_ERROR_CPP_I_IMPL_H
