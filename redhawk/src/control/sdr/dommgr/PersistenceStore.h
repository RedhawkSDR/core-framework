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

#include <ossie/exceptions.h>
#include "applicationSupport.h"
#include "connectionSupport.h"


namespace ossie {

    // Structures are used for domain manager persistence
    // All Persistence implementations must know how to store these 
    // objects.

    typedef std::string     ID;

    class DeviceManagerNode {
        public:
            std::string identifier;
            std::string label;
            CF::DeviceManager_var deviceManager;
    };

    class DomainManagerNode {
        public:
            std::string identifier;
            std::string name;
            CF::DomainManager_var domainManager;
    };

    typedef std::list<DeviceManagerNode> DeviceManagerList;

    typedef std::list<DomainManagerNode> DomainManagerList;

    typedef  ID    DeviceID;

    struct DeviceNode {
        std::string identifier;
        std::string label;
        CF::Device_var device;
        DeviceManagerNode devMgr;

        // Cached profile, not saved to persistence store
        std::string softwareProfile;
        ossie::SoftPkg spd;
        ossie::Properties prf;
        std::string implementationId;
        bool isLoadable;
        bool isExecutable;
    };

    typedef std::list<boost::shared_ptr<DeviceNode> > DeviceList;
    typedef std::list< DeviceID >   DeviceIDList;

    struct AllocationType {
        std::string allocationID;
        std::string requestingDomain;
        std::string sourceID;
        CF::Properties allocationProperties;
        CF::Device_var allocatedDevice;
        CF::DeviceManager_var allocationDeviceManager;
    };

    struct RemoteAllocationType : public AllocationType {
        CF::AllocationManager_var allocationManager;
    };

    typedef std::pair<std::string, boost::shared_ptr<DeviceNode> > AllocationResult;

    typedef std::map<std::string, AllocationType> AllocationTable;
    typedef std::map<std::string, RemoteAllocationType> RemoteAllocationTable;

    struct ApplicationNode {
        std::string name;
        std::string profile;
        std::string identifier;
        std::string contextName;
        CosNaming::NamingContext_var context;
        std::vector<ossie::DeviceAssignmentInfo> componentDevices;
        ossie::ComponentList components;
        CF::Resource_var assemblyController;
        std::vector<ConnectionNode> connections;
        std::vector<std::string> allocationIDs;
        std::vector<CF::Resource_var> componentRefs;
        std::map<std::string, CORBA::Object_var> ports;
        // Ext Props map :  extid -> (propid, compid)
        std::map<std::string, std::pair<std::string, std::string> > properties;
        bool aware_application;
    };
    
    class ServiceNode {
        public:
            CORBA::Object_var service;
            std::string deviceManagerId;
            std::string name;
            std::string serviceId;
    };

    typedef std::list<ServiceNode> ServiceList;
    

    class EventChannelNode {
        public:
            CosEventChannelAdmin::EventChannel_var   channel;
            unsigned int connectionCount;
            std::string boundName;
            std::string name;
    };

    struct consumerContainer {
        CosEventChannelAdmin::ProxyPushSupplier_var proxy_supplier;
        std::string channelName;
    };

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
#include <ossie/CF/cf.h>
#include <ossie/CorbaUtils.h>
#include <ossie/prop_helpers.h>
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
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
        void serialize(Archive& ar, ossie::DomainManagerNode& node, const unsigned int version) {
            ar & (node.identifier);
            ar & (node.name);
            ar & (node.domainManager);
        }

        template<class Archive>
        void serialize(Archive& ar, ossie::DeviceNode& node, const unsigned int version) {
            ar & (node.identifier);
            ar & (node.label);
            ar & (node.device);
            ar & (node.devMgr);
        }

        template<class Archive>
        void serialize(Archive& ar, boost::shared_ptr<ossie::DeviceNode>& ptr, const unsigned int version) {
            if (!ptr) {
                ptr.reset(new ossie::DeviceNode());
            }
            ar & (*ptr);
        }
    
        template<class Archive>
        void serialize(Archive& ar, ossie::EventChannelNode& node, const unsigned int version) {
            ar & (node.channel);
            ar & (node.connectionCount);
            ar & (node.boundName);
            ar & (node.name);
        }

        template<class Archive>
        void serialize(Archive& ar, ossie::ApplicationComponent& node, const unsigned int version) {
            ar & node.identifier;
            ar & node.softwareProfile;
            ar & node.namingContext;
            ar & node.implementationId;
            ar & node.loadedFiles;
            ar & node.processId;
            ar & node.componentObject;
            ar & node.assignedDevice;
        }

        template<class Archive>
        void serialize(Archive& ar, ossie::ApplicationNode& node, const unsigned int version) {
            ar & (node.identifier);
            ar & (node.name);
            ar & (node.profile);
            ar & (node.contextName);
            ar & (node.context);
            ar & (node.componentDevices);
            ar & (node.components);
            ar & (node.assemblyController);
            ar & (node.allocationIDs);
            ar & (node.connections);
            ar & (node.componentRefs);
            ar & (node.ports);
            ar & (node.properties);
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
        void serialize(Archive& ar, ossie::DeviceAssignmentInfo& dai, const unsigned int version) {
            ar & (dai.deviceAssignment);
            ar & (dai.device);
        }

        template<class Archive>
        void serialize(Archive& ar, ossie::AllocationType& at, const unsigned int version) {
            ar & (at.allocationID);
            ar & (at.requestingDomain);
            ar & (at.allocationProperties);
            ar & (at.allocatedDevice);
            ar & (at.allocationDeviceManager);
        }

        template<class Archive>
        void serialize(Archive& ar, ossie::RemoteAllocationType& at, const unsigned int version) {
            serialize(ar, static_cast<ossie::AllocationType&>(at), version);
            ar & (at.allocationManager);
        }

        template<class Archive>
        void serialize(Archive& ar, CF::DataType& dt, const unsigned int version) {
            ar & dt.id;
            ar & dt.value;
        }

        template<class Archive>
        void serialize(Archive& ar, CF::DeviceAssignmentType& dat, const unsigned int version) {
            ar & dat.componentId;
            ar & dat.assignedDeviceId;
        }

        template<class Archive>
        void serialize(Archive& ar, CF::AllocationManager::AllocationResponseType& resp, const unsigned int version) {
            ar & resp.requestID;
            ar & resp.allocationID;
            ar & resp.allocationProperties;
            ar & resp.allocatedDevice;
            ar & resp.allocationDeviceManager;
        }

        template <class Archive>
        void save(Archive& ar, const CORBA::String_member& in, const unsigned int version)
        {
            const std::string value(in);
            ar << value;
        }

        template <class Archive>
        void load(Archive& ar, CORBA::String_member& out, const unsigned int version)
        {
            std::string value;
            ar >> value;
            out = value.c_str();
        }

        template <class Archive, class ElemT>
        void save(Archive& ar, const _CORBA_Sequence<ElemT>& sequence, const unsigned int version)
        {
            const collection_size_type count(sequence.length());
            ar << BOOST_SERIALIZATION_NVP(count);
            if (count) {
                ar << boost::serialization::make_array(&sequence[0], count);
            }
        }

        template <class Archive, class ElemT>
        void load(Archive& ar, _CORBA_Sequence<ElemT>& sequence, const unsigned int version)
        {
            collection_size_type count;;
            ar >> BOOST_SERIALIZATION_NVP(count);
            sequence.length(count);
            if (count) {
                ar >> boost::serialization::make_array(&sequence[0], count);
            }
        }

        template<class Archive>
        void save(Archive& ar, const CORBA::Any& any, const unsigned int version) {
            std::string value(ossie::any_to_string(any));
            CORBA::TypeCode_var typecode = any.type();
            CORBA::TCKind kind = typecode->kind();
            ar << kind;
            if (typecode->kind() == CORBA::tk_struct) {
                CORBA::TypeCode_var propsValueType = any.type();
                std::string structName = ossie::any_to_string(*(propsValueType->parameter(0)));
                ar << structName;
            } else {
                std::string structName("simple");
                ar << structName;
            }
            ar << value;
        }

        template<class Archive>
        void load(Archive& ar, CORBA::Any& any, const unsigned int version) {
            std::string id, structName, value;
            CORBA::TCKind kind;
            ar >> kind;
            ar >> structName;
            ar >> value;
            CORBA::TypeCode_ptr typecode = ossie::getTypeCode(kind, structName);
            any = ::ossie::string_to_any(value, typecode);
        }

        template<class Archive>
        void save(Archive& ar, const CORBA::Object_ptr obj, const unsigned int version) {
            std::string ior = ::ossie::corba::objectToString(obj);
            ar << ior; 
        }

        template<class Archive>
        void load(Archive& ar, CORBA::Object_var& obj, const unsigned int version) {
            std::string ior;
            ar >> ior;
            obj = ::ossie::corba::stringToObject(ior);
        }

        template<class CorbaClass, class Archive>
        typename CorbaClass::_ptr_type loadAndNarrow(Archive& ar, const unsigned int version) {
            std::string ior;
            ar >> ior;
            CORBA::Object_var obj = ::ossie::corba::stringToObject(ior);
            return ::ossie::corba::_narrowSafe<CorbaClass>(obj);
        }
    }
}

// CORBA types have asymmetric save and load serialization functions and must
// be explicitly marked to the boost library
BOOST_SERIALIZATION_SPLIT_FREE(CORBA::Any)
BOOST_SERIALIZATION_SPLIT_FREE(CORBA::String_member)
BOOST_SERIALIZATION_SPLIT_FREE(CORBA::Object_var);

// There are default implementations for CORBA sequence save and load; however,
// it is not possible to use a templatized serialize function for all CORBA
// sequences because the sequence type is a subclass of the templatized type.
// The compiler will not find the base class serialize, but it works with save
// and load.
BOOST_SERIALIZATION_SPLIT_FREE(CF::DeviceAssignmentSequence);
BOOST_SERIALIZATION_SPLIT_FREE(CF::AllocationManager::AllocationResponseSequence);
BOOST_SERIALIZATION_SPLIT_FREE(CF::Properties);

// Define special handling for CORBA var types, which can use the generic
// CORBA::Object save, but must explicitly define their load because there is
// no way to determine the narrow function via a template.
#define SERIALIZE_CORBA_REFERENCE(T)                                    \
    namespace boost {                                                   \
    namespace serialization {                                           \
        template <class Archive>                                        \
        void load(Archive& ar, T##_var& obj, const unsigned int version) { \
            obj = loadAndNarrow<T>(ar, version);                        \
        }                                                               \
    }                                                                   \
    }                                                                   \
    BOOST_SERIALIZATION_SPLIT_FREE(T##_var);

SERIALIZE_CORBA_REFERENCE(CF::Port);
SERIALIZE_CORBA_REFERENCE(CF::Resource);
SERIALIZE_CORBA_REFERENCE(CF::Device);
SERIALIZE_CORBA_REFERENCE(CF::DeviceManager);
SERIALIZE_CORBA_REFERENCE(CF::DomainManager);
SERIALIZE_CORBA_REFERENCE(CF::AllocationManager);
SERIALIZE_CORBA_REFERENCE(CosEventChannelAdmin::EventChannel);
SERIALIZE_CORBA_REFERENCE(CosNaming::NamingContext);

#undef SERIALIZE_CORBA_REFERENCE

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

                boost::mutex::scoped_lock lock(_bdbLock);
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
                boost::mutex::scoped_lock lock(_bdbLock);
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
                boost::mutex::scoped_lock lock(_bdbLock);
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
                boost::mutex::scoped_lock lock(_bdbLock);
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
                boost::mutex::scoped_lock lock(_bdbLock);
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
            boost::mutex _bdbLock;
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

                boost::mutex::scoped_lock lock(_sqliteLock);
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
                boost::mutex::scoped_lock lock(_sqliteLock);
                sqlite3_close(db);
                db = NULL;
            }

            void del(const std::string& key) {
                std::ostringstream oss;
                oss << "DELETE FROM domainmanager WHERE key=\"" << key << "\";";
                std::string deleteStatement = oss.str();
                char* errmsg;
                boost::mutex::scoped_lock lock(_sqliteLock);
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
                boost::mutex::scoped_lock lock(_sqliteLock);
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
                boost::mutex::scoped_lock lock(_sqliteLock);
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
                } else if (status != SQLITE_ROW) {
                    return false;
                }
                if (consume) {
                    std::string deleteStatement = "DELETE FROM domainmanager WHERE key=\"" + key + "\";";
                    sqlite3_exec(db, deleteStatement.c_str(), NULL, NULL, NULL);
                }
                return true;
            }

        private:
            boost::mutex _sqliteLock;
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
