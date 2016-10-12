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

#ifndef COMPONENT_H
#define	COMPONENT_H
#include "Resource_impl.h"
#include "ossie/debug.h"
#include "ossie/Events.h"
#include "ossie/Autocomplete.h"

class Component : public Resource_impl {
public:
    Component(const char* _uuid);
    Component(const char* _uuid, const char *label);
    virtual ~Component();
    void setAdditionalParameters(std::string &softwareProfile, std::string &application_registrar_ior, std::string &nic);
    /*
     * Return a pointer to the Application that the Resource is deployed on 
     */
    redhawk::ApplicationContainer* getApplication() {
        return this->_app;
    }
    /*
     * Return the network information that was allocated to this Component (if applicable)
     */
    redhawk::NetworkContainer* getNetwork() {
        return this->_net;
    }
private:
    redhawk::ApplicationContainer *_app;
    redhawk::NetworkContainer *_net;

};

#endif	/* COMPONENT_H */

