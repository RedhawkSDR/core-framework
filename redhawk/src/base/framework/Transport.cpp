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

#include <ossie/Transport.h>
#include <ossie/debug.h>

namespace redhawk {

    class TransportRegistry::Impl {
    public:
        Impl()
        {
        }

        void registerTransport(TransportFactory* transport)
        {
            const std::string repo_id = transport->repoId();
            int priority = getPriority(transport);

            TransportList& list = _registry[repo_id];
            TransportList::iterator pos = list.begin();
            while ((pos != list.end()) && (pos->priority <= priority)) {
                ++pos;
            }
            list.insert(pos, Entry(priority, transport));
        }

        TransportStack getTransports(const std::string& repoId)
        {
            TransportStack stack;
            TransportMap::iterator transport = _registry.find(repoId);
            if (transport != _registry.end()) {
                TransportList& list = transport->second;
                for (TransportList::iterator iter = list.begin(); iter != list.end(); ++iter) {
                    stack.push_back(iter->transport);
                }
            }
            return stack;
        }

        int getPriority(TransportFactory* transport)
        {
            return transport->defaultPriority();
        }

    private:
        struct Entry {
        public:
            Entry(int priority, TransportFactory* transport) :
                priority(priority),
                transport(transport)
            {
            }

            int priority;
            TransportFactory* transport;
        };

        typedef std::vector<Entry> TransportList;
        typedef std::map<std::string,TransportList> TransportMap;
        TransportMap _registry;
    };

    void TransportRegistry::RegisterTransport(TransportFactory* transport)
    {
        Instance().registerTransport(transport);
    }

    TransportStack TransportRegistry::GetTransports(const std::string& repoId)
    {
        return Instance().getTransports(repoId);
    }

    TransportRegistry::Impl& TransportRegistry::Instance()
    {
        static TransportRegistry::Impl instance;
        return instance;
    }
}
