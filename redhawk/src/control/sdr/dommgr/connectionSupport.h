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

#ifndef CONNECTIONSUPPORT_H
#define CONNECTIONSUPPORT_H

#include <string>
#include <vector>
#include <stdexcept>

#include <boost/thread/mutex.hpp>
#include <boost/scoped_ptr.hpp>
#if HAVE_BOOST_SERIALIZATION
#include <boost/serialization/access.hpp>
#endif

#include "ossie/CF/cf.h"
#include "ossie/ossieSupport.h"
#include "ossie/debug.h"
#include "ossie/componentProfile.h"
#include "ossie/CorbaUtils.h"
#include "ossie/EventChannelSupport.h"

namespace ossie
{
    extern rh_logger::LoggerPtr connectionSupportLog;

    // Exception type for connections that cannot be parsed into our internal
    // structures.
    class InvalidConnection : public std::runtime_error {
    public:
        InvalidConnection(const std::string& msg) :
            std::runtime_error(msg)
        {
        }
    };

    // Exception type that may be thrown when an implementation of one of the
    // lookup interfaces cannot find the requested object.
    class LookupError : public std::runtime_error {
    public:
        LookupError(const std::string& message) :
            std::runtime_error(message)
        {
        }
    };

    // Interface to look up components by their identifier.
    class ComponentLookup
    {
    public:
        virtual ~ComponentLookup() {};

        /* Given a component instantiation id, returns the associated CORBA Resource pointer
         */
        virtual CF::Resource_ptr lookupComponentByInstantiationId(const std::string& identifier) = 0;
    };

    // Interface to look up objects within the domain.
    class DomainLookup
    {
    public:
        virtual ~DomainLookup() {};
        virtual CORBA::Object_ptr lookupDomainObject(const std::string& type, const std::string& name) = 0;
        virtual CF::DeviceManager_ptr lookupDeviceManagerByInstantiationId(const std::string& identifier) = 0;
        virtual unsigned int incrementEventChannelConnections(const std::string &EventChannelName) = 0;
        virtual unsigned int decrementEventChannelConnections(const std::string &EventChannelName) = 0;
    };

    // Interface to look up devices by their relationship to a given component.
    class DeviceLookup
    {
    public:
        virtual ~DeviceLookup() {};

        /* Given a component instantiation id, returns the associated CORBA Device pointer
         */
        virtual CF::Device_ptr lookupDeviceThatLoadedComponentInstantiationId(const std::string& componentId) = 0;

        /* Given a component instantiation id and uses id, returns the associated CORBA Device pointer
         */
        virtual CF::Device_ptr lookupDeviceUsedByComponentInstantiationId(const std::string& componentId, const std::string& usesId) = 0;

        /* Given a uses id, returns the associated CORBA Device pointer
         */
        virtual CF::Device_ptr lookupDeviceUsedByApplication(const std::string& usesRefId) = 0;
    };


    class ConnectionManager;

    class Endpoint {

        ENABLE_LOGGING;

    public:
        typedef enum {
            COMPONENT,
            SERVICENAME,
            APPLICATION
        } DependencyType;

        Endpoint() :
            terminated_(false)
        {
        }

        virtual ~Endpoint() { }
        CORBA::Object_ptr resolve(ConnectionManager& manager);
        CORBA::Object_ptr object();
        std::string getIdentifier();
        void setIdentifier(std::string identifier);

        bool isResolved() const;
        bool isTerminated() const;

        virtual CF::ConnectionManager::EndpointStatusType toEndpointStatusType() const;

        virtual bool allowDeferral() = 0;
        virtual bool checkDependency(DependencyType type, const std::string& identifier) const = 0;

        void dependencyTerminated();

        void release();

        virtual std::string description() const = 0;

        // Virtual copy constructor
        virtual Endpoint* clone() const = 0;

        static Endpoint* ParsePortSupplier(const Port* port);
        static Endpoint* ParsePort(const Port* port);
        static Endpoint* ParseProvidesEndpoint(const ossie::Connection& connection);
        static Endpoint* ParseFindBy(const ossie::FindBy* findby);
    private:
        // Subclasses must implement their own resolution method.
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager) = 0;

        // Subclasses that have additional side effects on release may override
        // this method.
        virtual void release_() { }

#if HAVE_BOOST_SERIALIZATION
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & object_;
            ar & terminated_;
        }
#endif

        CORBA::Object_var object_;
        bool terminated_;

    protected:
        std::string identifier__;
    };

    class ConnectionNode {

        ENABLE_LOGGING;

    public:
        static ConnectionNode* ParseConnection(const ossie::Connection& connection);

        ConnectionNode(Endpoint* uses, Endpoint* provides, const std::string& identifier, const std::string &requesterId, const std::string &connectionRecordId);
        ConnectionNode(const ConnectionNode&);

        bool connect(ConnectionManager& manager);
        void disconnect(DomainLookup* domainLookup);

        bool allowDeferral();
        bool allowDeferral(Endpoint::DependencyType type, const std::string& identifier);
        bool checkDependency(Endpoint::DependencyType type, const std::string& identifier) const;

        bool dependencyTerminated(Endpoint::DependencyType type, const std::string& identifier);

        // Default ctor and assignment exist only for deserialization support.
        ConnectionNode() { }
        const ConnectionNode& operator=(const ConnectionNode& other)
        {
            uses.reset(other.uses->clone());
            provides.reset(other.provides->clone());
            identifier = other.identifier;
            connected = other.connected;
            requesterId = other.requesterId;
            connectionRecordId = other.connectionRecordId;
            return *this;
        }

        void setrequesterId(std::string _requesterId) { requesterId = _requesterId;}
        void setconnectionRecordId(std::string _connectionRecordId) { connectionRecordId = _connectionRecordId;}

        // Use boost::scoped_ptr instead of std::unique_ptr for serialization
        // purposes.
        boost::scoped_ptr<Endpoint> uses;
        boost::scoped_ptr<Endpoint> provides;
        std::string identifier;
        std::string requesterId;
        std::string connectionRecordId;
        bool connected;

    };

    // Types used for storing connections. The former is for applications, while the
    // latter is for the DomainManager, which must track the owning DeviceManager for
    // each connection.
    typedef std::vector<ConnectionNode> ConnectionList;
    typedef std::map<std::string, ConnectionList> ConnectionTable;

    class ConnectionManager
    {

        ENABLE_LOGGING;

    public:
        virtual ~ConnectionManager();

        static void disconnectAll(ConnectionList& connections, ossie::DomainLookup* domainLookup);

        CORBA::Object_ptr resolveDomainObject(const std::string& type, const std::string& name);

        // Subclasses may override or extend the component lookup behavior.
        virtual CORBA::Object_ptr resolveComponent(const std::string& identifier);

        // Subclasses may override or extend the NamingService lookup behavior.
        virtual CORBA::Object_ptr resolveFindByNamingService(const std::string& name);

        // Subclasses must implement these device lookups as appropriate.
        virtual CF::Device_ptr resolveDeviceThatLoadedThisComponentRef(const std::string& refid) = 0;
        virtual CF::Device_ptr resolveDeviceUsedByThisComponentRef(const std::string& refid, const std::string& usesid) = 0;
        virtual CF::Device_ptr resolveDeviceUsedByApplication(const std::string& usesrefid) = 0;

        bool exceptionsEnabled();

        void setLogger(rh_logger::LoggerPtr logptr)
        {
            _connectionLog = logptr;
        };

    protected:
        ConnectionManager(DomainLookup* domainLookup,
                          ComponentLookup* componentLookup,
                          const std::string& namingContext,
                          bool enableExceptions);

        ossie::DomainLookup* _domainLookup;
        ossie::ComponentLookup* _componentLookup;
        std::string _namingContext;
        bool _enableExceptions;
        rh_logger::LoggerPtr _connectionLog;
    };

    class AppConnectionManager : public ConnectionManager
    {

        ENABLE_LOGGING;

    public:
        AppConnectionManager(DomainLookup* domainLookup,
                             ComponentLookup* componentLookup,
                             DeviceLookup* deviceLookup,
                             const std::string& namingContext);
        virtual ~AppConnectionManager();

        bool resolveConnection(const ossie::Connection& connection);
        const ConnectionList& getConnections();

    protected:
        virtual CORBA::Object_ptr resolveFindByNamingService(const std::string& name);
        virtual CF::Device_ptr resolveDeviceThatLoadedThisComponentRef(const std::string& refid);
        virtual CF::Device_ptr resolveDeviceUsedByThisComponentRef(const std::string& refid, const std::string& usesrefid);
        virtual CF::Device_ptr resolveDeviceUsedByApplication(const std::string& usesrefid);

        void addConnection_(const ConnectionNode& connection);

        ossie::DeviceLookup* _deviceLookup;

        ConnectionList _connections;
    };

    class DomainConnectionManager : public ConnectionManager
    {

        ENABLE_LOGGING;

    public:
        DomainConnectionManager(DomainLookup* domainLookup,
                                ComponentLookup* componentLookup,
                                const std::string& domainName);
        virtual ~DomainConnectionManager();

        std::string addConnection(const std::string& deviceManagerId, const Connection& connection);
        std::string restoreConnection(const std::string& deviceManagerId, ConnectionNode connection);
        void breakConnection(const std::string& connectionRecordId);

        void deviceManagerUnregistered(const std::string& deviceManagerName);

        void deviceRegistered(const std::string& deviceId);
        void deviceUnregistered(const std::string& deviceId);

        void serviceRegistered(const std::string& serviceName);
        void serviceUnregistered(const std::string& serviceName);

        void applicationRegistered(const std::string& applicationId);
        void applicationUnregistered(const std::string& applicationId);

        const ConnectionTable& getConnections() const;

    protected:
        virtual CF::Device_ptr resolveDeviceThatLoadedThisComponentRef(const std::string& refid);
        virtual CF::Device_ptr resolveDeviceUsedByThisComponentRef(const std::string& refid, const std::string& usesrefid);
        virtual CF::Device_ptr resolveDeviceUsedByApplication(const std::string& usesrefid);

        std::string addConnection_(const std::string& deviceManagerId, const ConnectionNode& connection);
        void tryPendingConnections_(Endpoint::DependencyType type, const std::string& identifier);
        void breakConnections_(Endpoint::DependencyType type, const std::string& identifier);

        boost::mutex _connectionLock;
        ConnectionTable _connectionsByRequester;
        std::map< std::string, std::pair<std::string, std::string> > _globalConnections;
    };

    // Miscellaneous helper functions
    std::string eventChannelName(const FindBy* findby);
    CORBA::Object_ptr getPort(CORBA::Object_ptr obj, const std::string& portId);
}

#endif // CONNECTIONSUPPORT_H
