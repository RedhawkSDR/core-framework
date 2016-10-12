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

#ifndef __PERSISTENCE_STORE_H__
#define __PERSISTENCE_STORE_H__
#include <exception>
#include <list>
#include <vector>
#include "ossie/exceptions.h"
#include "applicationSupport.h"
#include "connectionSupport.h"

namespace ossie {

    // Structures are used for domain manager persistence
    // All Persistence implementations must know how to store these 
    // objects.

    class DeviceManagerNode {
        public:
            std::string identifier;
            std::string label;
            CF::DeviceManager_var deviceManager;
    };

    typedef std::list<DeviceManagerNode> DeviceManagerList;

    class DeviceNode {
        public:
            CF::Device_var  device;
            DeviceManagerNode devMgr;

            std::string   softwareProfile;
            std::string   label;
            std::string identifier;
            CF::Properties  properties;
            std::map<std::string, CORBA::Object_var> connections;
    };

    typedef std::list<DeviceNode> DeviceList;

    class ApplicationFactoryNode {
        public:
            PortableServer::ObjectId_var servant_id;
            std::string profile;
            std::string identifier;
    };

    class ApplicationNode {
        public:
            std::string name;
            std::string profile;
            std::string identifier;
            std::string contextName;
            std::string contextIOR;
            std::vector<ossie::DeviceAssignmentInfo> componentDevices;
            CF::Application::ComponentElementSequence componentNamingContexts;
            CF::Application::ComponentElementSequence componentImplementations;
            CF::Application::ComponentProcessIdSequence componentProcessIds;
            CF::Components _registeredComponents;
            CF::Resource_var assemblyController;
            std::vector<CF::Resource_ptr> _startOrder;
            std::vector<ConnectionNode> connections;
            std::map<std::string, std::string> fileTable;
            std::map<std::string, std::vector<ossie::AllocPropsInfo> > allocPropsTable;
    };
    
    class ServiceNode {
        public:
            CORBA::Object_var service;
            std::string deviceManagerId;
            std::string name;
            std::string serviceId;
    };

    typedef std::list<ServiceNode> ServiceList;
    
#if ENABLE_EVENTS
    class EventChannelNode {
        public:
            CosEventChannelAdmin::EventChannel_var channel;
            unsigned int connectionCount;
            std::string boundName;
            std::string name;
    };

    struct consumerContainer {
        CosEventChannelAdmin::ProxyPushSupplier_var proxy_supplier;
        std::string channelName;
    };
#endif
    // Enable compile-time selection of persistence
    // backends using templates
    template<typename PersistenceImpl>
    class _PersistenceStore {
        public:
            _PersistenceStore() {
            }

            _PersistenceStore(const std::string& locationUrl) throw (PersistenceException) {
                impl.open(locationUrl);        
            }

            ~_PersistenceStore() {
                impl.close();
            }
        
            void open(const std::string& locationUrl) throw (PersistenceException) {
                impl.open(locationUrl);
            }

            void close() {
                impl.close();
            }

            template<typename T>
            void store(const std::string& key, const T& value) throw (PersistenceException) {
                impl.store(key, value);
            }

            template<typename T>
            void fetch(const std::string& key, T& value, bool consume = false) throw (PersistenceException) {
                impl.fetch(key, value, consume);
            }

            void del(const std::string& key) throw (PersistenceException) {
                impl.del(key);
            }


        private:
            PersistenceImpl impl;
    };
}

// Provide Boost serialization for all persistence implementations that want it
// if boost serialization is enabled
#if HAVE_BOOST_SERIALIZATION
#include "CF/cf.h"
#include "CorbaUtils.h"
#include "prop_helpers.h"
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/scoped_ptr.hpp>

// Explicitly instantiate the serialization templates for a given class.
#define EXPORT_SERIALIZATION_TEMPLATE(T,A) template void T::serialize<A>(A&, unsigned int);
#define EXPORT_CLASS_SERIALIZATION(T) BOOST_CLASS_EXPORT(T) \
        EXPORT_SERIALIZATION_TEMPLATE(T, boost::archive::text_iarchive) \
        EXPORT_SERIALIZATION_TEMPLATE(T, boost::archive::text_oarchive)

namespace boost {
    namespace serialization {
        template<class Archive>
        void serialize(Archive& ar, ossie::DeviceManagerNode& node, const unsigned int version) {
            ar & (node.identifier);
            ar & (node.label);
            ar & (node.deviceManager);
        }

        template<class Archive>
        void serialize(Archive& ar, ossie::DeviceNode& node, const unsigned int version) {
            ar & (node.identifier);
            ar & (node.label);
            ar & (node.device);
            ar & (node.devMgr);
        }
    
        template<class Archive>
        void serialize(Archive& ar, ossie::EventChannelNode& node, const unsigned int version) {
            ar & (node.channel);
            ar & (node.connectionCount);
            ar & (node.boundName);
            ar & (node.name);
        }

        template<class Archive>
        void serialize(Archive& ar, ossie::ApplicationFactoryNode& node, const unsigned int version) {
            ar & (node.profile);
        }

        template<class Archive>
        void serialize(Archive& ar, ossie::ApplicationNode& node, const unsigned int version) {
            ar & (node.identifier);
            ar & (node.name);
            ar & (node.profile);
            ar & (node.contextName);
            ar & (node.contextIOR);
            ar & (node.componentDevices);
            ar & (node.componentNamingContexts);
            ar & (node.componentImplementations);
            ar & (node.componentProcessIds);
            ar & (node.assemblyController);
            ar & (node.fileTable);
            ar & (node.allocPropsTable);
            ar & (node.connections);
        }

        template<class Archive>
        void serialize(Archive& ar, ossie::ServiceNode& node, const unsigned int version) {
            ar & (node.name);
            ar & (node.service);
            ar & (node.deviceManagerId);
        }

        template<class Archive>
        void serialize(Archive& ar, ossie::ConnectionNode& node, const unsigned int version) {
            ar & (node.uses);
            ar & (node.provides);
            ar & (node.identifier);
            ar & (node.connected);
        }

        template<class Archive>
        void serialize(Archive& ar, ossie::AllocPropsInfo& pi, const unsigned int version) {
            ar & (pi.device);
            ar & (pi.properties);
        }
        
        template<class Archive>
        void serialize(Archive& ar, ossie::DeviceAssignmentInfo& dai, const unsigned int version) {
            ar & (dai.deviceAssignment);
            ar & (dai.device);
        }

        template<class CorbaClass, class Archive>
        typename CorbaClass::_ptr_type loadAndNarrow(Archive& ar, const unsigned int version) {
            std::string ior;
            ar >> ior;
            CORBA::Object_var obj = ::ossie::corba::stringToObject(ior);
            return ::ossie::corba::_narrowSafe<CorbaClass>(obj);
        }

        template<class Archive>
        void save(Archive& ar, const CF::DeviceAssignmentType& dat, const unsigned int version) {
            std::string componentId(dat.componentId);
            std::string assignedDeviceId(dat.assignedDeviceId);
            ar << componentId;
            ar << assignedDeviceId;
        }

        template<class Archive>
        void load(Archive& ar, CF::DeviceAssignmentType& dat, const unsigned int version) {
            std::string componentId;
            std::string assignedDeviceId;
            ar >> componentId;
            ar >> assignedDeviceId;
            dat.componentId = componentId.c_str();
            dat.assignedDeviceId = assignedDeviceId.c_str();
        }

        template<class Archive>
        void save(Archive& ar, const CF::DeviceAssignmentSequence& das, const unsigned int version) {
            std::size_t const count(das.length());
            ar << count;
            for (std::size_t i = 0; i < das.length(); ++i) {
                ar << das[i];
            }
        }

        template<class Archive>
        void load(Archive& ar, CF::DeviceAssignmentSequence& das, const unsigned int version) {
            std::size_t count;
            ar >> count;
            das.length(count);
            for (std::size_t i = 0; i < das.length(); ++i) {
                ar >> das[i];
            }
        }

        template<class Archive>
        void save(Archive& ar, const CF::Application::ComponentElementSequence& ces, const unsigned int version) {
            std::size_t const count(ces.length());
            ar << count;
            for (std::size_t i = 0; i < ces.length(); ++i) {
                std::string componentId(ces[i].componentId);
                std::string elementId(ces[i].elementId);
                ar << componentId;
                ar << elementId;
            }
        }

        template<class Archive>
        void load(Archive& ar, CF::Application::ComponentElementSequence& ces, const unsigned int version) {
            std::size_t count;
            ar >> count;
            ces.length(count);
            for (std::size_t i = 0; i < ces.length(); ++i) {
                std::string componentId;
                std::string elementId;
                ar >> componentId;
                ar >> elementId;
                ces[i].componentId = componentId.c_str();
                ces[i].elementId = elementId.c_str();
            }
        }

        template<class Archive>
        void save(Archive& ar, const CF::Application::ComponentProcessIdSequence& cpids, const unsigned int version) {
            std::size_t const count(cpids.length());
            ar << count;
            for (std::size_t i = 0; i < cpids.length(); ++i) {
                std::string componentId(cpids[i].componentId);
                ar << componentId;
                ar << cpids[i].processId;
            }
        }

        template<class Archive>
        void load(Archive& ar, CF::Application::ComponentProcessIdSequence& cpids, const unsigned int version) {
            std::size_t count;
            ar >> count;
            cpids.length(count);
            for (std::size_t i = 0; i < cpids.length(); ++i) {
                std::string componentId;
                ar >> componentId;
                ar >> cpids[i].processId;
                cpids[i].componentId = componentId.c_str();
            }
        }

        template<class Archive>
        void save(Archive& ar, const CF::Properties& props, const unsigned int version) {
            std::size_t const count(props.length());
            ar << count;
            for (std::size_t i = 0; i < props.length(); ++i) {
                std::string id(props[i].id);
                std::string value(ossie::any_to_string(props[i].value));
                CORBA::TCKind kind = props[i].value.type()->kind();
                ar << id;
                ar << kind;
                ar << value;
            }
        }

        template<class Archive>
        void load(Archive& ar, CF::Properties& props, const unsigned int version) {
            std::size_t count;
            ar >> count;
            props.length(count);
            for (std::size_t i = 0; i < props.length(); ++i) {
                std::string id;
                CORBA::TCKind kind;
                std::string value;
                ar >> id;
                ar >> kind;
                ar >> value;
                props[i].id = id.c_str();
                props[i].value = ::ossie::string_to_any(value, kind);
            }
        }

        template<class Archive>
        void save(Archive& ar, const CF::Port_var& obj, const unsigned int version) {
            std::string ior = ::ossie::corba::objectToString(obj);
            ar << ior;
        }

        template<class Archive>
        void load(Archive& ar, CF::Port_var& obj, const unsigned int version) {
            obj = loadAndNarrow<CF::Port>(ar, version);
        }

        template<class Archive>
        void save(Archive& ar, const CF::Resource_var& obj, const unsigned int version) {
            std::string ior = ::ossie::corba::objectToString(obj);
            ar << ior; 
        }

        template<class Archive>
        void load(Archive& ar, CF::Resource_var& obj, const unsigned int version) {
            obj = loadAndNarrow<CF::Resource>(ar, version);
        }

        template<class Archive>
        void save(Archive& ar, const CF::Device_var& obj, const unsigned int version) {
            std::string ior = ::ossie::corba::objectToString(obj);
            ar << ior; 
        }

        template<class Archive>
        void load(Archive& ar, CF::Device_var& obj, const unsigned int version) {
            obj = loadAndNarrow<CF::Device>(ar, version);
        }

        template<class Archive>
        void save(Archive& ar, const CF::DeviceManager_var& obj, const unsigned int version) {
            std::string ior = ::ossie::corba::objectToString(obj);
            ar << ior; 
        }

        template<class Archive>
        void save(Archive& ar, const CosEventChannelAdmin::EventChannel_var& obj, const unsigned int version) {
            std::string ior = ::ossie::corba::objectToString(obj);
            ar << ior; 
        }

        template<class Archive>
        void load(Archive& ar, CF::DeviceManager_var& obj, const unsigned int version) {
            obj = loadAndNarrow<CF::DeviceManager>(ar, version);
        }
    
        template<class Archive>
        void load(Archive& ar, CosEventChannelAdmin::EventChannel_var& obj, const unsigned int version) {
            obj = loadAndNarrow<CosEventChannelAdmin::EventChannel>(ar, version);
        }

        template<class Archive>
        void save(Archive& ar, const std::pair<CF::Device_ptr, CF::Properties>& p, const unsigned int version) {
            std::string ior = ::ossie::corba::objectToString(p.first);
            ar << p.second;
        }

        template<class Archive>
        void load(Archive& ar, std::pair<CF::Device_ptr, CF::Properties>& p, const unsigned int version) {
            p.first = loadAndNarrow<CF::Device>(ar, version);
            ar >> p.second;
        }

        template<class Archive>
        void save(Archive& ar, const CORBA::Object_var& obj, const unsigned int version) {
            std::string ior = ::ossie::corba::objectToString(obj);
            ar << ior; 
        }

        template<class Archive>
        void load(Archive& ar, CORBA::Object_var& obj, const unsigned int version) {
            std::string ior;
            ar >> ior;
            obj = ::ossie::corba::stringToObject(ior);
        }
    }
}

// For some reason we can only serialize _var objects and not _ptr objects
BOOST_SERIALIZATION_SPLIT_FREE(CORBA::Any)
BOOST_SERIALIZATION_SPLIT_FREE(CF::Application::ComponentElementSequence);
BOOST_SERIALIZATION_SPLIT_FREE(CF::DeviceAssignmentType);
BOOST_SERIALIZATION_SPLIT_FREE(CF::DeviceAssignmentSequence);
BOOST_SERIALIZATION_SPLIT_FREE(CF::Application::ComponentProcessIdSequence);
BOOST_SERIALIZATION_SPLIT_FREE(CF::Properties);
BOOST_SERIALIZATION_SPLIT_FREE(CF::Port_var);
BOOST_SERIALIZATION_SPLIT_FREE(CF::Resource_var);
BOOST_SERIALIZATION_SPLIT_FREE(CF::Device_var);
BOOST_SERIALIZATION_SPLIT_FREE(CF::DeviceManager_var);
BOOST_SERIALIZATION_SPLIT_FREE(CosEventChannelAdmin::EventChannel_var);
BOOST_SERIALIZATION_SPLIT_FREE(CORBA::Object_var);
#endif

// If we get too many different persistence modes, split the
// implementations out into separate files
#if ENABLE_BDB_PERSISTENCE
#include <db4/db_cxx.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace ossie {
    class BdbPersistenceBackend {
        public:
            BdbPersistenceBackend() : db(0, 0), _isopen(false) {
            }

            void open(const std::string& locationUrl) throw (PersistenceException) {
                if (_isopen) return;

                try {
                    db.open(NULL, locationUrl.c_str(), NULL, DB_HASH, DB_CREATE, 0);
                    _isopen = true;
                } catch ( DbException &e ) {
                    throw PersistenceException(e.what());
                }
            }
        
            template<typename T>
            void store(const std::string& key, const T& value) throw (PersistenceException) {
                if (!_isopen) return;

                std::ostringstream out;
                try {
                    boost::archive::text_oarchive oa(out);
                    oa << value;
                } catch (boost::archive::archive_exception &e) {
                    throw PersistenceException(e.what());
                }
                storeRaw(key, out.str());
            }

            void store(const std::string& key, const char* value) throw (PersistenceException) {
                if (!_isopen) return;

                std::string strvalue(value);
                store(key, strvalue);
            }
        
            template<typename T>
            void fetch(const std::string& key, T& value, bool consume) throw (PersistenceException) {
                if (!_isopen) return;

                std::string v;
                if (fetchRaw(key, v, consume)) {;
                    std::istringstream in(v);
                    try {
                        boost::archive::text_iarchive ia(in);
                        ia >> value;
                    } catch (boost::archive::archive_exception &e) {
                        throw PersistenceException(e.what());
                    }
                }
            }
    
            void close() {
                if (!_isopen) return;
                try {
                    db.close(0);
                } catch ( DbException &e ) {
                    // pass
                }
                _isopen = false;
            }

            void del(const std::string& key) {
                if (!_isopen) return;

                Dbt k((void*)key.c_str(), key.size() + 1);
                try { 
                    db.del(NULL, &k, 0);
                    db.sync(0);
                } catch (DbException& e) {
                    throw PersistenceException(e.what());
                }
            }

        protected:
            void storeRaw(const std::string& key, const std::string& value) {
                Dbt k((void*)key.c_str(), key.size() + 1);
                Dbt v((void*)value.c_str(), value.size() + 1);
                try { 
                    db.put(NULL, &k, &v, 0);
                    db.sync(0);
                } catch (DbException& e) {
                    throw PersistenceException(e.what());
                }
            }

            bool fetchRaw(const std::string& key, std::string& value, bool consume) {
                Dbt k((void*)key.c_str(), key.size() + 1);
                Dbt v;
                if (db.get(NULL, &k, &v, 0) != DB_NOTFOUND) {;
                    value = static_cast<char*>(v.get_data());
                    if (consume) {
                        db.del(NULL, &k, 0);
                        db.sync(0);
                    }
                    return true;
                } else {
                    return false;
                }
            }

        private:
            Db db;
            bool _isopen;
    };

    typedef _PersistenceStore<BdbPersistenceBackend> PersistenceStore;
}

#elif ENABLE_GDBM_PERSISTENCE
#include <gdbm.h>
#include <fcntl.h>
#include <errno.h>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace ossie {
    class GdbmPersistenceBackend {
        public:
            GdbmPersistenceBackend() : _dbf(NULL) {
            }

            void open(const std::string& locationUrl) throw (PersistenceException) {
                if (_dbf != NULL) return;
                
                {
                    boost::mutex::scoped_lock lock(_gdbmLock);
                    _dbf = gdbm_open(const_cast<char*>(locationUrl.c_str()), 
                                     0, 
                                     GDBM_WRCREAT | GDBM_SYNC, 
                                     O_CREAT | O_RDWR | S_IRUSR | S_IWUSR, 
                                     0);
                    if (_dbf == NULL) {
                        std::ostringstream eout;
                        eout << "Failed to open GDBM file " << locationUrl << " - " << gdbm_strerror(gdbm_errno) << " " << strerror(errno);
                        throw PersistenceException(eout.str());
                    }
                }
            }
        
            template<typename T>
            void store(const std::string& key, const T& value) throw (PersistenceException) {
                if (_dbf == NULL) return;

                std::ostringstream out;
                try {
                    boost::archive::text_oarchive oa(out);
                    oa << value;
                } catch (boost::archive::archive_exception &e) {
                    throw PersistenceException(e.what());
                }
                storeRaw(key, out.str());
            }

            void store(const std::string& key, const char* value) throw (PersistenceException) {
                if (_dbf == NULL) return;

                std::string strvalue(value);
                store(key, strvalue);
            }
        
            template<typename T>
            void fetch(const std::string& key, T& value, bool consume) throw (PersistenceException) {
                if (_dbf == NULL) return;

                std::string v;
                if (fetchRaw(key, v, consume)) {;
                    std::istringstream in(v);
                    try {
                        boost::archive::text_iarchive ia(in);
                        ia >> value;
                    } catch (boost::archive::archive_exception &e) {
                        throw PersistenceException(e.what());
                    }
                }
            }
    
            void close() {
                if (_dbf == NULL) return;
                {
                    boost::mutex::scoped_lock lock(_gdbmLock);
                    gdbm_close(_dbf);
                }
                _dbf = NULL;
            }

            void del(const std::string& key) {
                if (_dbf == NULL) return;
                datum d_k;
                d_k.dptr = const_cast<char*>(key.c_str());
                d_k.dsize = strlen(key.c_str());
                {
                    boost::mutex::scoped_lock lock(_gdbmLock);
                    gdbm_delete(_dbf, d_k);
                    gdbm_reorganize(_dbf);
                }
            }

        protected:
            void storeRaw(const std::string& key, const std::string& value) {
                assert(_dbf != NULL);
                datum d_k;
                d_k.dptr = const_cast<char*>(key.c_str());
                d_k.dsize = strlen(key.c_str());

                datum d_v;
                d_v.dptr = const_cast<char*>(value.c_str());
                d_v.dsize = strlen(value.c_str());
                {
                    boost::mutex::scoped_lock lock(_gdbmLock);
                    gdbm_store(_dbf, d_k, d_v, GDBM_REPLACE);
                    gdbm_reorganize(_dbf);
                }
            }

            bool fetchRaw(const std::string& key, std::string& value, bool consume) {
                assert(_dbf != NULL);
                datum d_k;
                d_k.dptr = const_cast<char*>(key.c_str());
                d_k.dsize = strlen(key.c_str());
                {
                    boost::mutex::scoped_lock lock(_gdbmLock);
                    datum d_v = gdbm_fetch(_dbf, d_k);
                    if (d_v.dptr != NULL) {
                        value.assign(d_v.dptr, 0, d_v.dsize);
                        return true;
                    } else {
                        return false;
                    }
                }
            }

        private:
            GDBM_FILE _dbf;
            boost::mutex _gdbmLock;
    };

    typedef _PersistenceStore<GdbmPersistenceBackend> PersistenceStore;
}


#elif ENABLE_SQLITE_PERSISTENCE
#include <sqlite3.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace ossie {
    class SQLitePersistenceBackend {
        public:
            SQLitePersistenceBackend() : db(NULL) {
            }

            void open(const std::string& locationUrl) throw (PersistenceException) {
                if (db != NULL) return;

                if (sqlite3_open(locationUrl.c_str(), &db) == SQLITE_OK) {
                    createTable();
                } else {
                    throw PersistenceException(sqlite3_errmsg(db));
                }
            }
        
            template<typename T>
            void store(const std::string& key, const T& value) throw (PersistenceException) {
                if (db == NULL) return;

                std::ostringstream out;
                try {
                    boost::archive::text_oarchive oa(out);
                    oa << value;
                } catch (boost::archive::archive_exception &e) {
                    throw PersistenceException(e.what());
                }
                storeRaw(key, out.str());
            }

            void store(const std::string& key, const char* value) throw (PersistenceException) {
                if (db == NULL) return;

                std::string strvalue(value);
                store(key, strvalue);
            }
        
            template<typename T>
            void fetch(const std::string& key, T& value, bool consume) throw (PersistenceException) {
                if (db == NULL) return;

                std::string v;
                if (fetchRaw(key, v, consume)) {;
                    std::istringstream in(v);
                    try {
                        boost::archive::text_iarchive ia(in);
                        ia >> value;
                    } catch (boost::archive::archive_exception &e) {
                        throw PersistenceException(e.what());
                    }
                }
            }
    
            void close() {
                if (db == NULL) return;
                sqlite3_close(db);
                    db = NULL;
            }

            void del(const std::string& key) {
                std::ostringstream oss;
                oss << "DELETE FROM domainmanager WHERE key=\"" << key << "\";";
                std::string deleteStatement = oss.str();
                char* errmsg;
                if (sqlite3_exec(db, deleteStatement.c_str(), NULL, NULL, &errmsg) != SQLITE_OK) {
                    throw PersistenceException(std::string("delete: ") + errmsg);
                }
            }

        protected:
            void createTable() {
                assert(db != NULL);

                // Try to do a SELECT to test whether the table already exists, because older versions
                // of SQLite 3 do not support the "IF NOT EXISTS" clause to CREATE TABLE.
                if (sqlite3_exec(db, "SELECT * FROM domainmanager;", NULL, NULL, NULL) == SQLITE_OK) {
                    return;
                }
                std::string createStatement = "CREATE TABLE domainmanager (key STRING UNIQUE NOT NULL, value BLOB);";
                char* errmsg;
                if (sqlite3_exec(db, createStatement.c_str(), NULL, NULL, &errmsg)) {
                    throw PersistenceException(std::string("createTable: ") + errmsg);
                }
            }

            void storeRaw(const std::string& key, const std::string& value) {
                assert(db != NULL);

                std::string insertStatement = "INSERT OR REPLACE INTO domainmanager (key, value) VALUES(?,?);";
                sqlite3_stmt* statement;
                const char* tail;
                if (sqlite3_prepare(db, insertStatement.c_str(), -1, &statement, &tail)) {
                    throw PersistenceException(std::string("prepare: ") + sqlite3_errmsg(db));
                }
                sqlite3_bind_text(statement, 1, key.c_str(), key.size(), SQLITE_TRANSIENT);
                sqlite3_bind_text(statement, 2, value.c_str(), value.size(), SQLITE_TRANSIENT);

                int status = sqlite3_step(statement);
                sqlite3_finalize(statement);
                if (status != SQLITE_DONE) {
                    throw PersistenceException(std::string("step: ") + sqlite3_errmsg(db));
                }
            }

            bool fetchRaw(const std::string& key, std::string& value, bool consume) {
                assert(db != NULL);

                std::string selectStatement = "SELECT value FROM domainmanager WHERE key=?;";
                sqlite3_stmt* statement;
                const char* tail;
                if (sqlite3_prepare(db, selectStatement.c_str(), -1, &statement, &tail)) {
                    throw PersistenceException(std::string("prepare: ") + sqlite3_errmsg(db));
                }
                sqlite3_bind_text(statement, 1, key.c_str(), key.size(), SQLITE_TRANSIENT);

                int status = sqlite3_step(statement);
                if (status == SQLITE_ROW) {
                    size_t bytes = sqlite3_column_bytes(statement, 0);
                    const char* blob = reinterpret_cast<const char*>(sqlite3_column_blob(statement, 0));
                    value = std::string(blob, bytes);
                }
                sqlite3_finalize(statement);
                if (status == SQLITE_ERROR) {
                    throw PersistenceException(std::string("step: ") + sqlite3_errmsg(db));
                }
                return (status == SQLITE_ROW);
            }

        private:
            sqlite3* db;
    };

    typedef _PersistenceStore<SQLitePersistenceBackend> PersistenceStore;
}


#else

namespace ossie {
    class NullPersistenceBackend {
        public:
            void open(const std::string& locationUrl) {}

            template<typename T>
            void store(const std::string& key, const T& value) {}

            template<typename T>
            void fetch(const std::string& key, T& value, bool consume) {}

            void del(const std::string& key) {}

            void close() {}
    };
    typedef _PersistenceStore<NullPersistenceBackend> PersistenceStore;

}
#endif

#endif
