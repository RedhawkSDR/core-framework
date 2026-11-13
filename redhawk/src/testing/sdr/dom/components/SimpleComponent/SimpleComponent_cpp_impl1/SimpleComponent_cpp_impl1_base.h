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

 
#ifndef SIMPLECOMPONENT_CPP_IMPL1_IMPL_BASE_H
#define SIMPLECOMPONENT_CPP_IMPL1_IMPL_BASE_H

#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include "port_impl.h"
#include "struct_props.h"

class SimpleComponent_cpp_impl1_base;

class  SimpleComponent_cpp_impl1_base  : public Component, protected ThreadedComponent
{

    public: 
        SimpleComponent_cpp_impl1_base(const char *uuid, const char *label);
        
        ~SimpleComponent_cpp_impl1_base(void);

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

        void configure(const CF::Properties&);

    protected:
    
    	// Member variables exposed as properties
		std::string ep_only;
		std::string ep_cfg;
		unsigned char myOct;

    private:
        void construct();
};
#endif
