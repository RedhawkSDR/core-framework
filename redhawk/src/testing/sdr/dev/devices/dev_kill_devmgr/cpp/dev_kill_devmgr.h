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
#ifndef DEV_KILL_DEVMGR_I_IMPL_H
#define DEV_KILL_DEVMGR_I_IMPL_H

#include "dev_kill_devmgr_base.h"

class dev_kill_devmgr_i : public dev_kill_devmgr_base
{
    ENABLE_LOGGING
    public:
        dev_kill_devmgr_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        dev_kill_devmgr_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        dev_kill_devmgr_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        dev_kill_devmgr_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~dev_kill_devmgr_i();

        void constructor();

        int serviceFunction();

    protected:
        void updateUsageState();
};

#endif // DEV_KILL_DEVMGR_I_IMPL_H
