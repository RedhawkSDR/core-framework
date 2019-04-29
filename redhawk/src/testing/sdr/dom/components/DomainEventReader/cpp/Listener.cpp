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

#include <typeinfo>

#include "Listener.h"
#include "DomainEventReader.h"


Listener::Listener(DomainEventReader_i *r) :
    pReader(r)
{
}

void Listener::operator() ( const redhawk::events::DomainStateEvent &data ) {
    std::cout << "....... state change\n"
              << "            prod id:  " << data.prod_id << "\n"
              << "          source id:  " << data.source_id << "\n"
              << "        source name:  " << data.source_name << "\n"
              << "         obj is nil:  " << bool( data.obj == CORBA::Object::_nil() ) << "\n"
              << std::endl;
    if (data.obj == CORBA::Object::_nil()) {
        pReader->increment_remove_events();
    } else {
        pReader->increment_add_events();
    }
}
