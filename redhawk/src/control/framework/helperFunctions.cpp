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


#include <string>
#include <uuid/uuid.h>
#include <string.h>

#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>
#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/omniORB.h>
#include <omniORB4/omniORB.h>
#include <omniORB4/omniTransport.h>
#include <omniORB4/internal/omniIdentity.h>
#include <omniORB4/internal/localIdentity.h>
#include <omniORB4/internal/inProcessIdentity.h>
#include <omniORB4/internal/remoteIdentity.h>

#include <ossie/ossieSupport.h>
#include <ossie/debug.h>
#include <ossie/boost_compat.h>

namespace fs = boost::filesystem;

bool ossie::isValidFileName(const std::string& fileName)
{
    if (fileName.empty()) {
        return false;
    }

    const fs::path testPath(fileName);
    return true;
}

std::string ossie::generateUUID()
{
    uuid_t id;
    uuid_generate(id);

    // Per the man page, UUID strings are 36 characters plus the '\0' terminator.
    char strbuf[37];
    uuid_unparse(id, strbuf);

    return std::string("DCE:") + strbuf;
}


static std::string _trim_addr( const std::string &addr, const std::string &exp="(.*):([^:]*)$" )
{
    std::string ret;
    boost::regex expr(exp.c_str());
    boost::smatch what;
    if (boost::regex_search(addr, what, expr))
        {
            ret =  what[1];
        }
    return ret;
}


static bool _match_remotes( CORBA::Object_ptr aobj, CORBA::Object_ptr bobj)
{
    bool retval=false;
    omniIOR *a_ior = aobj->_PR_getobj()->_getIOR();
    omniIOR *b_ior = bobj->_PR_getobj()->_getIOR();
    omniIdentity* a_identity = aobj->_PR_getobj()->_identity();
    omniIdentity* b_identity = bobj->_PR_getobj()->_identity();
    omniRemoteIdentity *a_remote = omniRemoteIdentity::downcast(a_identity);
    omniRemoteIdentity *b_remote = omniRemoteIdentity::downcast(b_identity);
    if ( a_remote != NULL and b_remote != NULL ) {
        omni::Rope *a_rope = a_remote->rope();
        omni::Rope *b_rope = b_remote->rope();
        RH_NL_TRACE("redhawk.corba.internal", "Rope A: " << a_rope << " Rope B: " << b_rope );
        if ( a_rope != NULL and b_rope != NULL and
             a_rope == b_rope ) {
            RH_NL_TRACE("redhawk.corba.internal", "Identities Same Rope == Same Remote Host.");
            retval = true;
        }
    }

    // last ditch.. try IORInfo address list resolution
    if ( !retval and a_ior and b_ior ) {
        omniIOR::IORInfo *a_iorinfo = a_ior->getIORInfo();
        omniIOR::IORInfo *b_iorinfo = b_ior->getIORInfo();
        const omni::giopAddressList &a_addrs = a_iorinfo->addresses();
        const omni::giopAddressList &b_addrs = b_iorinfo->addresses();

        omni::giopAddressList::const_iterator i, i_last, j, j_last, j_first;
        i      = a_addrs.begin();
        i_last = a_addrs.end();
        j_first= b_addrs.begin();
        j_last = b_addrs.end();
        if (  a_addrs.size() >  b_addrs.size() ) {
            i        = b_addrs.begin();
            i_last   = b_addrs.end();
            j_first  = a_addrs.begin();
            j_last   = a_addrs.end();
        }

        for (; i != i_last; i++) {
            // try to match address space.. remove port string has
            std::string a_addr = _trim_addr((*i)->address());
            j = j_first;
            for (; j != j_last; j++) {
                std::string b_addr = _trim_addr((*j)->address());
                RH_NL_TRACE("redhawk.corba.internal", "Identities A addr: " << a_addr << " B addr: " << b_addr );
                if ( a_addr == b_addr) { retval=true; return retval; }
            }
        }
    }

    return retval;
}


bool ossie::sameHost(CORBA::Object_ptr aobj, CORBA::Object_ptr bobj)
{
    bool retval=false;
    // if both identifies are the same
    omniIdentity* a_identity = aobj->_PR_getobj()->_identity();
    omniIdentity* b_identity = bobj->_PR_getobj()->_identity();
    if ( a_identity->is_equivalent(b_identity) == true ) {
        RH_NL_TRACE("redhawk.corba.internal", "Same identifies, so same host");
        retval = true;
    }
    else {
        // if both identities are in the same process space then
        // they are on the same host
        if ( a_identity->inThisAddressSpace() and
             b_identity->inThisAddressSpace() ) {
            RH_NL_TRACE("redhawk.corba.internal", "Identifies Same address space...");
            retval = true;
        }
        else {
            // if both identities processes then are on the same host
            omniInProcessIdentity *a_proc = omniInProcessIdentity::downcast(a_identity);
            omniInProcessIdentity *b_proc = omniInProcessIdentity::downcast(b_identity);
            if ( a_proc != NULL and b_proc != NULL ) {
                RH_NL_TRACE("redhawk.corba.internal", "Objects have ProcessIdentities,  they are on same LOCAL HOST.");
                retval= true;
            }
            else {
                retval=_match_remotes( aobj, bobj );
                if ( retval ) {
                   RH_NL_TRACE("redhawk.corba.internal", "Remote Identities are on the SAME HOST.");
                }
                else {
                    RH_NL_TRACE("redhawk.corba.internal", "Remote Identities are different.");
                }
            }
        }
    }
    return retval;
}
std::string ossie::getCurrentDirName()
{
  std::string retval;
  char *tdir = get_current_dir_name();
  if ( tdir ) {
    retval = tdir;
    free(tdir);
  }
  return retval;
}

#include <fstream>

namespace ossie {

  namespace helpers {
  
  /*
     is_jarfile 
     
     Helper method to test if parameter is a valid jar file in lue of "file" command
     result differences from various OS distros

     @return 0   contents of jarPath is a valid jar format
     @return 1   contents of file does not match magic number format
     @return -1  file acces/open error

   */
    int is_jarfile( const std::string &jarPath ) {
      int retval=-1;
      std::ifstream r_fs(jarPath.c_str() );
      // test if file was opened...
      if ( r_fs.fail() == false )  {
        int mlen=6;
        // magic numbers for jar/zip files
        uint8_t java_m1[6] = { 0x50, 0x4b, 0x03, 0x04, 0x14, 0x00 };
        uint8_t java_m2[6] = { 0x50, 0x4b, 0x03, 0x04, 0x0a, 0x00 };
        uint8_t tbuf[mlen];
        r_fs.read( (char *)tbuf, mlen );
        if (r_fs) {
          retval=memcmp( tbuf, java_m1, mlen);
          // check file contents against magic number sequence
          if ( retval != 0 ) {
            retval = memcmp( tbuf, java_m2, mlen);
            if ( retval != 0 ) retval=1;
          }
        }
      }
      return retval;
    };

  };

};
