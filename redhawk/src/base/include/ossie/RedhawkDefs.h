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
#ifndef __RH_CF_NS_DEFS__
#define __RH_CF_NS_DEFS__

#ifdef  NO_NAMESPACE

#define REDHAWK_CPP_NAMESPACE    
#define REDHAWK_CPP_NAMESPACE_BEGIN  
#define REDHAWK_CPP_NAMESPACE_END  
#define REDHAWK_CPP_NAMESPACE_USE  
#define REDHAWK_CPP_NAMESPACE_QUALIFIER 

#else

#define REDHAWK_CPP_NAMESPACE   redhawk
#define REDHAWK_CPP_NAMESPACE_BEGIN  namespace REDHAWK_CPP_NAMESPACE {   
#define REDHAWK_CPP_NAMESPACE_END  };
#define REDHAWK_CPP_NAMESPACE_USE  using namespace REDHAWK_CPP_NAMESPACE;
#define REDHAWK_CPP_NAMESPACE_QUALIFIER REDHAWK_CPP_NAMESPACE::

namespace REDHAWK_CPP_NAMESPACE  {};

#endif

#endif // ifndef __REDHAWK_NS_DEFS__
