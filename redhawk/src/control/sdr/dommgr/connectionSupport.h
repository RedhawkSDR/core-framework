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
    // Exception type for connections that cannot be parsed into our internal
    // structures.
    class InvalidConnection : public std::runtime_error {
    public:
        InvalidConnection(const std::string& msg) :
            std::runtime_error(msg)
        {
        }
    };

    // Interface to look up components by their identifier.
    class ComponentLookup
    {
    public:
        virtual ~ComponentLookup() {};
        virtual CF::Resource_ptr lookupComponentByInstantiationId(const std::string& identifier) = 0;
        virtual CF::DeviceManager_ptr lookupDeviceManagerByInstantiationId(const std::string& identifier) = 0;
    };

    // Interface to look up objects within the domain.
    class DomainLookup
    {
    public:
        virtual ~DomainLookup() {};
        virtual CORBA::Object_ptr lookupDomainObject(const std::string& type, const std::string& name) = 0;
        virtual unsigned int incrementEventChannelConnections(const std::string &EventChannelName) = 0;
        virtual unsigned int decrementEventChannelConnections(const std::string &EventChannelName) = 0;
    };

    // Interface to look up devices by their relationship to a given component.
    class DeviceLookup
    {
    public:
        virtual ~DeviceLookup() {};
        virtual CF::Device_ptr lookupDeviceThatLoadedComponentInstantiationId(const std::string& componentId) = 0;
        virtual CF::Device_ptr lookupDeviceUsedByComponentInstantiationId(const std::string& componentId, const std::string& usesId) = 0;
        virtual CF::Device_ptr lookupDeviceUsedByApplication(const std::string& usesRefId) = 0;
    };


    class ConnectionManager;

    class Endpoint {

        ENABLE_LOGGING;

    public:
        typedef enum {
            COMPONENT,
            SERVICENAME
        } DependencyType;

        virtual ~Endpoint() { };
        CORBA::Object_ptr resolve(ConnectionManager& manager);
        CORBA::Object_ptr object();
        bool isResolved();

        virtual bool allowDeferral() = 0;
        virtual bool checkDependency(DependencyType type, const std::string& identifier) const = 0;

        void release();

        // Virtual copy contstructor
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
        }
#endif

        CORBA::Object_var object_;
    };

    class ConnectionNode {

        ENABLE_LOGGING;

    public:
        static ConnectionNode* ParseConnection(const ossie::Connection& connection);

        ConnectionNode(Endpoint* uses, Endpoint* provides, const std::string& identifier);
        ConnectionNode(const ConnectionNode&);

        bool connect(ConnectionManager& manager);
        void disconnect(DomainLookup* domainLookup);

        bool allowDeferral();
        bool checkDependency(Endpoint::DependencyType type, const std::string& identifier) const;

        // Default ctor and assignment exist only for deserialization support.
        ConnectionNode() { }
        const ConnectionNode& operator=(const ConnectionNode& other)
        {
            uses.reset(other.uses->clone());
            provides.reset(other.provides->clone());
            identifier = other.identifier;
            connected = other.connected;
            return *this;
        }

        // Use boost::scoped_ptr instead of std::auto_ptr for serialization
        // purposes.
        boost::scoped_ptr<Endpoint> uses;
        boost::scoped_ptr<Endpoint> provides;
        std::string identifier;
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
        ConnectionManager(DomainLookup* domainLookup,
                          ComponentLookup* componentLookup,
                          const std::string& namingContext);
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

    protected:
        ossie::DomainLookup* _domainLookup;
        ossie::ComponentLookup* _componentLookup;
        std::string _namingContext;

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

        void addConnection(const std::string& deviceManagerId, const Connection& connection);
        void restoreConnection(const std::string& deviceManagerId, ConnectionNode connection);

        void deviceManagerUnregistered(const std::string& deviceManagerId);

        void deviceRegistered(const std::string& deviceId);
        void deviceUnregistered(const std::string& deviceId);

        void serviceRegistered(const std::string& serviceName);
        void serviceUnregistered(const std::string& serviceName);

        const ConnectionTable& getConnections() const;

    protected:
        virtual CF::Device_ptr resolveDeviceThatLoadedThisComponentRef(const std::string& refid);
        virtual CF::Device_ptr resolveDeviceUsedByThisComponentRef(const std::string& refid, const std::string& usesrefid);
        virtual CF::Device_ptr resolveDeviceUsedByApplication(const std::string& usesrefid);

        void addConnection_(const std::string& deviceManagerId, const ConnectionNode& connection);
        void tryPendingConnections_(Endpoint::DependencyType type, const std::string& identifier);
        void breakConnections_(Endpoint::DependencyType type, const std::string& identifier);

        boost::mutex _connectionLock;
        ConnectionTable _connections;
    };

    // Miscellaneous helper functions
    std::string eventChannelName(const FindBy* findby);
    CORBA::Object_ptr getPort(CORBA::Object_ptr obj, const std::string& portId);
}

#endif // CONNECTIONSUPPORT_H
