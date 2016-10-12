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

#ifndef OSSIE_CORBAGC_H
#define OSSIE_CORBAGC_H

#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>

#include <omniORB4/CORBA.h>

#include <ossie/debug.h>

#include "GCContext.h"

namespace ossie {

    namespace corba {

        class GCServantLocator : public virtual POA_PortableServer::ServantLocator, private GCContext {

            ENABLE_LOGGING;

        public:
            GCServantLocator();
            ~GCServantLocator();

            virtual PortableServer::Servant preinvoke(const PortableServer::ObjectId& oid,
                                                      PortableServer::POA_ptr adapter,
                                                      const char* operation,
                                                      PortableServer::ServantLocator::Cookie& the_cookie);

            virtual void postinvoke(const PortableServer::ObjectId& oid,
                                    PortableServer::POA_ptr adapter,
                                    const char* operation,
                                    PortableServer::ServantLocator::Cookie the_cookie,
                                    PortableServer::Servant the_servant);

            void register_servant(const PortableServer::ObjectId& oid,
                                  PortableServer::Servant servant,
                                  boost::posix_time::time_duration ttl,
                                  const std::string& destroy);

        private:
            void gc_sweep();

            struct ServantEntry {
                PortableServer::Servant servant;
                boost::posix_time::time_duration ttl;
                std::string destroy;
                boost::system_time last_access;
            };

            typedef boost::unordered_map<PortableServer::ObjectId,ServantEntry> ServantMap;
            ServantMap activeMap_;
            boost::mutex activeMapMutex_;
        };

        PortableServer::POA_ptr createGCPOA(PortableServer::POA_ptr parent, const std::string& name);

        CORBA::Object* activateGCObject(PortableServer::POA_ptr poa,
                                        PortableServer::Servant servant,
                                        boost::posix_time::time_duration ttl=boost::posix_time::seconds(60),
                                        const std::string& destroy=std::string("destroy"));
    }
}

#endif // OSSIE_CORBAGC_H
