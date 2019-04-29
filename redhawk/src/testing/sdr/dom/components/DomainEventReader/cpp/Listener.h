/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any 
 * later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR 
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef LISTENER_H_
#define LISTENER_H_

#include <ossie/Component.h>


class DomainEventReader_i;


class Listener : public redhawk::events::DomainEventReader::AddRemoveListener
{
private:
    DomainEventReader_i * pReader;

public:
    Listener(DomainEventReader_i * comp);

    void operator() (const redhawk::events::DomainStateEvent &);
};


#endif /* LISTENER_H_ */
