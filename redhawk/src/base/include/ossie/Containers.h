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
/* 
 * File:   Containers.h
 * Author: pmrobe3
 *
 * Created on December 29, 2014, 1:21 PM
 */

#ifndef CONTAINERS_H
#define	CONTAINERS_H

#include "CF/cf.h"

namespace redhawk {
    class DomainManagerContainer {
    public:
        DomainManagerContainer() {
                ref = CF::DomainManager::_nil();
        };
        DomainManagerContainer(CF::DomainManager_ptr domMgr) {
            ref = CF::DomainManager::_duplicate(domMgr);
        }
        CF::DomainManager_ptr getRef() {
            return ref;
        };
    private:
        CF::DomainManager_var ref;
    };
    class DeviceManagerContainer {
    public:
        DeviceManagerContainer() {
            ref = CF::DeviceManager::_nil();
        };
        DeviceManagerContainer(CF::DeviceManager_ptr devMgr) {
            ref = CF::DeviceManager::_duplicate(devMgr);
        }
        CF::DeviceManager_ptr getRef() {
            return ref;
        };
    private:
        CF::DeviceManager_var ref;
    };
    class ApplicationContainer {
    public:
        ApplicationContainer() {
            ref = CF::Application::_nil();
        };
        ApplicationContainer(CF::Application_ptr app) {
            ref = CF::Application::_duplicate(app);
        };
        CF::Application_ptr getRef() {
            return ref;
        };
    private:
        CF::Application_var ref;
    };

    class NetworkContainer {
    public:
        NetworkContainer() {
            nic = "";
        };
        NetworkContainer(std::string _nic) {
            nic = _nic;
        };
        std::string getNic() {
            return nic;
        };
    private:
        std::string nic;
    };

}

#endif	/* CONTAINERS_H */

