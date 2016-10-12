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


#ifndef ORBSUPPORT_H
#define ORBSUPPORT_H

#include <string>
#include <vector>

#include <sched.h>

#ifdef HAVE_OMNIORB4_CORBA_H
#include "omniORB4/CORBA.h"
#endif

#include "ossie/CF/cf.h"
#include "ossie/CF/StandardEvent.h"

#include "debug.h"

namespace ossie
{
  std::string getCurrentDirName();
  std::string generateUUID();

    namespace helpers {

      /*
         is_jarfile 
     
         Helper method to test if a file is valid jar file in lue of "file" command
         due to results differences from various OS distros

         @return 0   contents of jarPath is a valid jar format
         @return 1   contents of file does not match magic number format
         @return -1  file acces/open error

      */

      int is_jarfile( const std::string &jarPath );

    };
}  // Close ossieSupport Namespace
#endif
