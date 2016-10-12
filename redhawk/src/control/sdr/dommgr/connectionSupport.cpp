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
#include <vector>

#if HAVE_BOOST_SERIALIZATION
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/extended_type_info.hpp>
#include "PersistanceStore.h"
#endif

#include <ossie/CF/cf.h>
#include <ossie/CorbaUtils.h>
#include <ossie/ossieSupport.h>

#include "connectionSupport.h"

using namespace ossie;

PREPARE_LOGGING(ConnectionManager);

ConnectionManager::ConnectionManager(DomainLookup* domainLookup,
                                     ComponentLookup* componentLookup,
                                     const std::string& namingContext) :
    _domainLookup(domainLookup),
    _componentLookup(componentLookup),
    _namingContext(namingContext)
{
    assert(_domainLookup != 0);
    assert(_componentLookup != 0);
}

ConnectionManager::~ConnectionManager()
{
}

void ConnectionManager::disconnectAll(std::vector<ConnectionNode>& connections, ossie::DomainLookup* domainLookup)
{
    // Disconnect all connections made for the application in the reverse order of their creation.
    LOG_TRACE(ConnectionManager, "Disconnecting " << connections.size() << " ports");
    for (std::vector<ConnectionNode>::reverse_iterator connection = connections.rbegin(); connection != connections.rend(); ++connection) {
        LOG_TRACE(ConnectionManager, "Disconnecting connection " << connection->identifier);
        connection->disconnect(domainLookup);
    }
    connections.clear();

    LOG_TRACE(ConnectionManager, "All connected ports disconnected");
}

CORBA::Object_ptr ConnectionManager::resolveComponent(const std::string& identifier)
{
    CORBA::Object_var target = _componentLookup->lookupComponentByInstantiationId(identifier);
    if (CORBA::is_nil(target)) {
        target = _componentLookup->lookupDeviceManagerByInstantiationId(identifier);
    }
    if (CORBA::is_nil(target)) {
        LOG_DEBUG(ConnectionManager, "Could not locate component with instantiation id " << identifier);
    }
    return target._retn();
}

CORBA::Object_ptr ConnectionManager::resolveDomainObject(const std::string& type, const std::string& name)
{
    return _domainLookup->lookupDomainObject(type, name);
}

CORBA::Object_ptr ConnectionManager::resolveFindByNamingService(const std::string& name)
{
    std::string findbyName;
    
    if (name.find("/") == 0) {
        findbyName = name;
        findbyName.erase(findbyName.begin());
    } else {
        findbyName = _namingContext + "/" + name;
    }
    
    LOG_TRACE(ConnectionManager, "resolveFindByNamingService: The findname that I'm using is: " << findbyName);
    try {
        return ossie::corba::objectFromName(findbyName);
    } catch (CosNaming::NamingContext::NotFound) {
        // The name was not found, continue on and return nil.
    } CATCH_LOG_ERROR(ConnectionManager, "Exception trying to resolve findbynamingservice \"" << findbyName << "\"");

    return CORBA::Object::_nil();
}

PREPARE_LOGGING(AppConnectionManager);

AppConnectionManager::AppConnectionManager(DomainLookup* domainLookup,
                                           ComponentLookup* componentLookup,
                                           DeviceLookup* deviceLookup,
                                           const std::string& namingContext) :
    ConnectionManager(domainLookup, componentLookup, namingContext),
    _deviceLookup(deviceLookup),
    _connections()
{
    assert(_deviceLookup != 0);
}

AppConnectionManager::~AppConnectionManager()
{
}

bool AppConnectionManager::resolveConnection(const Connection& connection)
{
    std::auto_ptr<ConnectionNode> connectionNode(ConnectionNode::ParseConnection(connection));
    if (!connectionNode.get()) {
        LOG_ERROR(AppConnectionManager, "Unable to parse connection");
        return false;
    }

    LOG_TRACE(AppConnectionManager, "Attempting to resolve connection " << connectionNode->identifier);
    if (connectionNode->connect(*this)) {
        addConnection_(*connectionNode);
        return true;
    }
    return false;
}

CORBA::Object_ptr AppConnectionManager::resolveFindByNamingService(const std::string& name)
{
    // Try the default lookup first.
    CORBA::Object_ptr retval = ConnectionManager::resolveFindByNamingService(name);
    if (!CORBA::is_nil(retval)) {
        return retval;
    }

    // Remove the first naming context after the domain name (probably the waveform name) and try again.
    std::string::size_type islash = _namingContext.find('/');
    if (islash != std::string::npos) {
        std::string findbyName = _namingContext.substr(0, islash + 1) + name;
        LOG_TRACE(AppConnectionManager, "resolveFindBy: The findname that I'm using is: " << findbyName);
        
        try {
            return ossie::corba::objectFromName(findbyName);
        } catch (CosNaming::NamingContext::NotFound) {
            // The name was not found, continue on and return nil.
        } CATCH_LOG_ERROR(AppConnectionManager, "Exception trying to resolve findbynamingservice \"" << findbyName << "\"");
    }
    return CORBA::Object::_nil();
}

CF::Device_ptr AppConnectionManager::resolveDeviceThatLoadedThisComponentRef(const std::string& refid)
{
    CF::Device_ptr device = _deviceLookup->lookupDeviceThatLoadedComponentInstantiationId(refid);
    if (CORBA::is_nil(device)) {
        LOG_ERROR(AppConnectionManager, "devicethatloadedthiscomponentref not found");
    }
    return device;
}


CF::Device_ptr AppConnectionManager::resolveDeviceUsedByThisComponentRef(const std::string& refid, const std::string& usesrefid)
{
    CF::Device_ptr device = _deviceLookup->lookupDeviceUsedByComponentInstantiationId(refid, usesrefid);
    if (CORBA::is_nil(device)) {
        LOG_ERROR(AppConnectionManager, "deviceusedbythiscomponentref not found");
    }
    return device;
}

const std::vector<ConnectionNode>& AppConnectionManager::getConnections() {
    return _connections;
}

void AppConnectionManager::addConnection_(const ConnectionNode& connection)
{
    LOG_TRACE(AppConnectionManager, "Adding connection " << connection.identifier << " to connection list");
    _connections.push_back(connection);
}


PREPARE_LOGGING(DomainConnectionManager);

DomainConnectionManager::DomainConnectionManager(DomainLookup* domainLookup,
                                                 ComponentLookup* componentLookup,
                                                 const std::string& domainName) :
    ConnectionManager(domainLookup, componentLookup, domainName),
    _connections()
{
}

DomainConnectionManager::~DomainConnectionManager()
{
}

CF::Device_ptr DomainConnectionManager::resolveDeviceThatLoadedThisComponentRef(const std::string&)
{
    LOG_ERROR(DomainConnectionManager, "Not supported in this context: Port is devicethatloadedthiscomponentref");
    return CF::Device::_nil();
}

CF::Device_ptr DomainConnectionManager::resolveDeviceUsedByThisComponentRef(const std::string&, const std::string&)
{
    LOG_ERROR(DomainConnectionManager, "Not supported in this context: Port is deviceusedbythiscomponentref ");
    return CF::Device::_nil();
}

void DomainConnectionManager::addConnection(const std::string& deviceManagerId, const Connection& connection)
{
    boost::scoped_ptr<ConnectionNode> connectionNode(ConnectionNode::ParseConnection(connection));
    if (!connectionNode.get()) {
        LOG_ERROR(DomainConnectionManager, "Skipping invalid connection for DeviceManager " << deviceManagerId);
        return;
    }

    if (!connectionNode->connect(*this)) {
        if (!connectionNode->allowDeferral()) {
            LOG_ERROR(DomainConnectionManager, "Connection " << connectionNode->identifier << " is not resolvable");
            return;
        }
    }
    
    LOG_DEBUG(DomainConnectionManager, "Connection " << connectionNode->identifier << " could not be resolved, marked as pending");
    addConnection_(deviceManagerId, *connectionNode);
}

void DomainConnectionManager::restoreConnection(const std::string& deviceManagerId, ConnectionNode connection)
{
    if (connection.connected) {
        // Try to determine whether the connection is still active.
        if (ossie::corba::objectExists(connection.uses->object())) {
            // The uses port still exists; check the provides side.
            if (!ossie::corba::objectExists(connection.provides->object())) {
                // The provides side no longer exists; break the connection and treat is as pending.
                connection.disconnect(_domainLookup);
            }
        }
    } else {
        // Our current domain state is a subset of the state when the DomainManager terminated. As such,
        // if the connection was not previously made, nothing has changed yet to make it possible. If a new
        // DeviceManager has come online, it will register after the restoration completes.
    }

    addConnection_(deviceManagerId, connection);
}

void DomainConnectionManager::deviceManagerUnregistered(const std::string& deviceManagerId)
{
    TRACE_ENTER(DomainConnectionManager);
    boost::mutex::scoped_lock(_connectionLock);
    ConnectionTable::iterator devMgr = _connections.find(deviceManagerId);
    if (devMgr == _connections.end()) {
        // DeviceManager has no connections.
        return;
    }
    ConnectionList& connectionList = devMgr->second;
    LOG_TRACE(DomainConnectionManager, "Deleting " << connectionList.size() << " connection(s) from DeviceManager " << deviceManagerId);
    for (ConnectionList::iterator connection = connectionList.begin(); connection != connectionList.end(); ++connection) {
        connection->disconnect(_domainLookup);
    }
    _connections.erase(devMgr);
    TRACE_EXIT(DomainConnectionManager);
}

void DomainConnectionManager::deviceRegistered(const std::string& deviceId)
{
    TRACE_ENTER(DomainConnectionManager);
    tryPendingConnections_(Endpoint::COMPONENT, deviceId);
    TRACE_EXIT(DomainConnectionManager);
}

void DomainConnectionManager::deviceUnregistered(const std::string& deviceId)
{
    TRACE_ENTER(DomainConnectionManager);
    breakConnections_(Endpoint::COMPONENT, deviceId);
    TRACE_EXIT(DomainConnectionManager);
}

void DomainConnectionManager::serviceRegistered(const std::string& serviceName)
{
    TRACE_ENTER(DomainConnectionManager);
    tryPendingConnections_(Endpoint::SERVICENAME, serviceName);
    TRACE_EXIT(DomainConnectionManager);
}

void DomainConnectionManager::serviceUnregistered(const std::string& serviceName)
{
    TRACE_ENTER(DomainConnectionManager);
    breakConnections_(Endpoint::SERVICENAME, serviceName);
    TRACE_EXIT(DomainConnectionManager);
}

const ConnectionTable& DomainConnectionManager::getConnections() const
{
    return _connections;
}

void DomainConnectionManager::addConnection_(const std::string& deviceManagerId, const ConnectionNode& connection)
{
    boost::mutex::scoped_lock(_connectionMutex);
    if (_connections.find(deviceManagerId) == _connections.end()) {
        _connections[deviceManagerId] = ConnectionList();
    }
    _connections[deviceManagerId].push_back(connection);
}

void DomainConnectionManager::tryPendingConnections_(Endpoint::DependencyType type, const std::string& identifier)
{
    TRACE_ENTER(DomainConnectionManager);

    boost::mutex::scoped_lock lock(_connectionLock);
    for (ConnectionTable::iterator devMgr = _connections.begin(); devMgr != _connections.end(); ++devMgr) {
        // Go through the list of connections for each DeviceManager to check
        // for pending connections that have the given dependency, and attempt
        // to complete the connection.
        ConnectionList& connections = devMgr->second;
        for (ConnectionList::iterator connection = connections.begin(); connection != connections.end(); ++connection) {
            if (connection->connected || !connection->checkDependency(type, identifier)) {
                continue;
            }
            LOG_TRACE(DomainConnectionManager, "Resolving pending connection " << connection->identifier);
            if (!connection->connect(*this)) {
                if (!connection->allowDeferral()) {
                    LOG_ERROR(DomainConnectionManager, "Connection " << connection->identifier << " cannot be resolved");
                    // TODO: Remove this connection.
                } else {
                    LOG_TRACE(DomainConnectionManager, "Connection " << connection->identifier << " still has pending dependencies");
                }
            } else {
                LOG_DEBUG(DomainConnectionManager, "Connection " << connection->identifier << " resolved");
            }
        }
    }
    TRACE_EXIT(DomainConnectionManager);
}

void DomainConnectionManager::breakConnections_(Endpoint::DependencyType type, const std::string& identifier)
{
    TRACE_ENTER(DomainConnectionManager);

    boost::mutex::scoped_lock lock(_connectionLock);
    for (ConnectionTable::iterator devMgr = _connections.begin(); devMgr != _connections.end(); ++devMgr) {
        // Go through the list of connections for each DeviceManager to check
        // for connections that have the given dependency, and disconnect the
        // uses side.
        ConnectionList& connections = devMgr->second;
        for (ConnectionList::iterator connection = connections.begin(); connection != connections.end(); ++connection) {
            if (!connection->connected || !connection->checkDependency(type, identifier)) {
                continue;
            }
            LOG_TRACE(DomainConnectionManager, "Breaking connection " << connection->identifier);
            connection->disconnect(_domainLookup);
        }
    }

    TRACE_EXIT(DomainConnectionManager);
}


namespace ossie {

    class ComponentEndpoint : public Endpoint {
    public:
        ComponentEndpoint() { }

        ComponentEndpoint(const std::string& identifier) :
            identifier_(identifier)
        {
        }

        ComponentEndpoint(const ComponentEndpoint& other):
            Endpoint(other),
            identifier_(other.identifier_)
        {
        }

        virtual bool allowDeferral(void) { return true; }

        virtual bool checkDependency(DependencyType type, const std::string& identifier) const
        {
            return ((type == COMPONENT) && (identifier == identifier_));
        }

        virtual ComponentEndpoint* clone() const
        {
            return new ComponentEndpoint(*this);
        }

    private:
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager)
        {
            return manager.resolveComponent(identifier_);
        }

#if HAVE_BOOST_SERIALIZATION
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, unsigned int version)
        {
            ar & boost::serialization::base_object<Endpoint>(*this);
            ar & identifier_;
        }
#endif

        std::string identifier_;
    };


    class DeviceLoadedEndpoint : public Endpoint {
    public:
        DeviceLoadedEndpoint() { }

        DeviceLoadedEndpoint(const std::string& identifier) :
            identifier_(identifier)
        {
        }

        DeviceLoadedEndpoint(const DeviceLoadedEndpoint& other):
            Endpoint(other),
            identifier_(other.identifier_)
        {
        }

        virtual bool allowDeferral(void) { return false; }

        virtual bool checkDependency(DependencyType type, const std::string& identifier) const
        {
            // NOTE: Presently, this endpoint type will only appear in an
            // application context, where we do not need to determine whether
            // it depends on a given object.
            return false;
        }

        virtual DeviceLoadedEndpoint* clone() const
        {
            return new DeviceLoadedEndpoint(*this);
        }

    private:
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager)
        {
            return manager.resolveDeviceThatLoadedThisComponentRef(identifier_);
        }

#if HAVE_BOOST_SERIALIZATION
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, unsigned int version)
        {
            ar & boost::serialization::base_object<Endpoint>(*this);
            ar & identifier_;
        }
#endif

        std::string identifier_;
    };


    class DeviceUsedEndpoint : public Endpoint {
    public:
        DeviceUsedEndpoint() { }

        DeviceUsedEndpoint(const std::string& componentIdentifier, const std::string& usesIdentifier) :
            componentIdentifier_(componentIdentifier),
            usesIdentifier_(usesIdentifier)
        {
        }

        DeviceUsedEndpoint(const DeviceUsedEndpoint& other):
            Endpoint(other),
            componentIdentifier_(other.componentIdentifier_),
            usesIdentifier_(other.usesIdentifier_)
        {
        }

        virtual bool allowDeferral(void) { return false; }

        virtual bool checkDependency(DependencyType type, const std::string& identifier) const
        {
            // NOTE: Presently, this endpoint type will only appear in an
            // application context, where we do not need to determine whether
            // it depends on a given object.
            return false;
        }

        virtual DeviceUsedEndpoint* clone() const
        {
            return new DeviceUsedEndpoint(*this);
        }

    private:
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager)
        {
            return manager.resolveDeviceUsedByThisComponentRef(componentIdentifier_, usesIdentifier_);
        }

#if HAVE_BOOST_SERIALIZATION
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, unsigned int version)
        {
            ar & boost::serialization::base_object<Endpoint>(*this);
            ar & componentIdentifier_;
            ar & usesIdentifier_;
        }
#endif

        std::string componentIdentifier_;
        std::string usesIdentifier_;
    };


    class FindByNamingServiceEndpoint : public Endpoint {
    public:
        FindByNamingServiceEndpoint() { }

        FindByNamingServiceEndpoint(const std::string& name) :
            name_(name)
        {
        }

        FindByNamingServiceEndpoint(const FindByNamingServiceEndpoint& other):
            Endpoint(other),
            name_(other.name_)
        {
        }

        virtual bool allowDeferral(void) { return false; }

        virtual bool checkDependency(DependencyType type, const std::string& identifier) const
        {
            // NOTE: There is no simple way to map a namingservice reference
            // to a given object in the domain, so assume that there is no
            // relationship.
            return false;
        }

        virtual FindByNamingServiceEndpoint* clone() const
        {
            return new FindByNamingServiceEndpoint(*this);
        }

    private:
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager)
        {
            return manager.resolveFindByNamingService(name_);
        }

#if HAVE_BOOST_SERIALIZATION
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, unsigned int version)
        {
            ar & boost::serialization::base_object<Endpoint>(*this);
            ar & name_;
        }
#endif

        std::string name_;
    };


    class FindByDomainFinderEndpoint : public Endpoint {
    public:
        FindByDomainFinderEndpoint() { }

        FindByDomainFinderEndpoint(const std::string& type, const std::string& name) :
            type_(type),
            name_(name)
        {
        }

        FindByDomainFinderEndpoint(const FindByDomainFinderEndpoint& other):
            Endpoint(other),
            type_(other.type_),
            name_(other.name_)
        {
        }

        virtual bool allowDeferral(void) { return true; }

        virtual bool checkDependency(DependencyType type, const std::string& identifier) const
        {
            return ((type == Endpoint::SERVICENAME) && (identifier == name_));
        }

        virtual FindByDomainFinderEndpoint* clone() const
        {
            return new FindByDomainFinderEndpoint(*this);
        }

        std::string type() const
        {
            return type_;
        }

        std::string name() const
        {
            return name_;
        }

    private:
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager)
        {
            return manager.resolveDomainObject(type_, name_);
        }

#if HAVE_BOOST_SERIALIZATION
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, unsigned int version)
        {
            ar & boost::serialization::base_object<Endpoint>(*this);
            ar & type_;
            ar & name_;
        }
#endif

        std::string type_;
        std::string name_;
    };


    class PortEndpoint : public Endpoint {

        ENABLE_LOGGING;

    public:
        PortEndpoint() { }

        PortEndpoint(Endpoint* supplier, const std::string& name) :
            supplier_(supplier),
            name_(name),
            invalidPort_(false)
        {
            assert(supplier != 0);
        }

        PortEndpoint(const PortEndpoint& other) :
            Endpoint(other),
            supplier_(other.supplier_->clone()),
            name_(other.name_),
            invalidPort_(other.invalidPort_)
        {
        }

        virtual bool allowDeferral(void)
        {
            // If the port is known to be invalid, we will never be able to resolve it.
            if (invalidPort_) {
                return false;
            }

            // Otherwise, it's up to the port supplier.
            return supplier_->allowDeferral();
        }

        virtual bool checkDependency(DependencyType type, const std::string& identifier) const
        {
            // Defer to the port supplier.
            return supplier_->checkDependency(type, identifier);
        }

        virtual PortEndpoint* clone() const
        {
            return new PortEndpoint(*this);
        }

    private:
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager)
        {
            CORBA::Object_var supplierObject = supplier_->resolve(manager);
            CF::PortSupplier_var portSupplier = ossie::corba::_narrowSafe<CF::PortSupplier>(supplierObject);
            if (!CORBA::is_nil(portSupplier)) {
                try {
                    return portSupplier->getPort(name_.c_str());
                } catch (const CF::PortSupplier::UnknownPort&) {
                    LOG_ERROR(PortEndpoint, "Port supplier reports no port with name " << name_);
                } CATCH_LOG_ERROR(PortEndpoint, "Failure in getPort");
        
                invalidPort_ = true;
            } else {
                LOG_DEBUG(PortEndpoint, "Unable to resolve port supplier");
            }
            return CORBA::Object::_nil();
        }

        virtual void release_()
        {
            // Pass along the release to the port supplier.
            supplier_->release();

            // Clear "invalid port" status, since a future resolution could
            // yield a new port supplier that has a port with the given name.
            invalidPort_ = false;
        }

#if HAVE_BOOST_SERIALIZATION
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, unsigned int version)
        {
            ar & boost::serialization::base_object<Endpoint>(*this);
            ar & supplier_;
            ar & name_;
        }
#endif

        boost::scoped_ptr<Endpoint> supplier_;
        std::string name_;
        bool invalidPort_;
    };

    PREPARE_LOGGING(PortEndpoint);
    
};

#if HAVE_BOOST_SERIALIZATION
EXPORT_CLASS_SERIALIZATION(ComponentEndpoint);
EXPORT_CLASS_SERIALIZATION(DeviceLoadedEndpoint);
EXPORT_CLASS_SERIALIZATION(DeviceUsedEndpoint);
EXPORT_CLASS_SERIALIZATION(FindByDomainFinderEndpoint);
EXPORT_CLASS_SERIALIZATION(FindByNamingServiceEndpoint);
EXPORT_CLASS_SERIALIZATION(PortEndpoint);
#endif


PREPARE_LOGGING(Endpoint);

Endpoint* Endpoint::ParsePortSupplier(const Port* port)
{
    if (port->isFindBy()) {
        return Endpoint::ParseFindBy(port->getFindBy());
    } else if (port->isComponentInstantiationRef()) {
        assert(port->getComponentInstantiationRefID() != 0);
        std::string identifier = port->getComponentInstantiationRefID();
        LOG_TRACE(Endpoint, "ComponentEndpoint refid=" << identifier);
        return new ComponentEndpoint(identifier);
    } else if (port->isDeviceThatLoadedThisComponentRef()) {
        return new DeviceLoadedEndpoint(port->getDeviceThatLoadedThisComponentRef());
    } else if (port->isDeviceUsedByThisComponentRef()) {
        std::string refid = port->getDeviceUsedByThisComponentRefID();
        std::string usesrefid = port->getDeviceUsedByThisComponentRefUsesRefID();
        LOG_TRACE(Endpoint, "DeviceUsedEndpoint refid=" << refid << " usesrefid=" << usesrefid);
        return new DeviceUsedEndpoint(refid, usesrefid);
    } else {
        LOG_ERROR(Endpoint, "Unknown port location type");
    }

    return 0;
}


Endpoint* Endpoint::ParseProvidesEndpoint(const Connection& connection)
{
    if (connection.isProvidesPort()) {
        return Endpoint::ParsePort(connection.getProvidesPort());
    } else if (connection.isFindBy()) {
        assert(connection.getFindBy() != 0);
        return Endpoint::ParseFindBy(connection.getFindBy());
    } else if (connection.isComponentSupportedInterface()) {
        const ComponentSupportedInterface* interface = connection.getComponentSupportedInterface();
        if (interface->isComponentInstantiationRef()) {
            return new ComponentEndpoint(interface->getComponentInstantiationRefId());
        } else if (interface->isFindBy()) {
            return Endpoint::ParseFindBy(interface->getFindBy());
        } else {
            LOG_ERROR(Endpoint, "Component supported interface is neither findby or componentinstantiationref")
        }
    } else {
        LOG_ERROR(Endpoint, "Cannot find port information for provides port");
    }

    return 0;
}

Endpoint* Endpoint::ParsePort(const Port* port)
{
    Endpoint* supplier = Endpoint::ParsePortSupplier(port);
    if (supplier) {
        assert(port->getID() != 0);
        std::string name = port->getID();
        LOG_TRACE(Endpoint, "PortEndpoint name=" << name);
        return new PortEndpoint(supplier, name);
    }
    return 0;
}

Endpoint* Endpoint::ParseFindBy(const FindBy* findby)
{
    if (findby->isFindByNamingService()) {
        assert(findby->getFindByNamingServiceName() != 0);
        std::string name = findby->getFindByNamingServiceName();
        LOG_TRACE(Endpoint, "FindByNamingServiceEndpoint name=" << name);
        return new FindByNamingServiceEndpoint(name);
    } else if (findby->isFindByDomainFinder()) {
        assert(findby->getFindByDomainFinderType() != 0);
        assert(findby->getFindByDomainFinderName() != 0);
        std::string type = findby->getFindByDomainFinderType();
        std::string name = findby->getFindByDomainFinderName();
        LOG_TRACE(Endpoint, "FindByDomainFinderEndpoint type=" << type << " name=" << name);
        return new FindByDomainFinderEndpoint(type, name);
    } else {
        LOG_ERROR(Endpoint, "Unknown findby type");
    }
    return 0;
}

bool Endpoint::isResolved()
{
    return !(CORBA::is_nil(object_));
}

CORBA::Object_ptr Endpoint::resolve(ConnectionManager& manager)
{
    if (!isResolved()) {
        LOG_TRACE(Endpoint, "Resolving endpoint");
        object_ = resolve_(manager);
    }
    return CORBA::Object::_duplicate(object_);
}

CORBA::Object_ptr Endpoint::object()
{
    return object_;
}

void Endpoint::release()
{
    // Call the subclass-specific release method.
    release_();

    // Clear the cached object reference.
    object_ = CORBA::Object::_nil();
}

PREPARE_LOGGING(ConnectionNode);

ConnectionNode* ConnectionNode::ParseConnection(const Connection& connection)
{
    // Parse the uses port.
    std::auto_ptr<Endpoint> usesEndpoint(Endpoint::ParsePort(connection.getUsesPort()));
    if (!usesEndpoint.get()) {
        LOG_ERROR(ConnectionNode, "Unable to parse uses endpoint");
        return 0;
    }

    // Parse the provides port.
    std::auto_ptr<Endpoint> providesEndpoint(Endpoint::ParseProvidesEndpoint(connection));
    if (!providesEndpoint.get()) {
        LOG_ERROR(ConnectionNode, "Unable to parse provides endpoint");
        return 0;
    }

    // Get the connection identifier, or generate one if needed.
    std::string connectionId = connection.getID();
    if (connectionId.empty()) {
        connectionId = ossie::generateUUID();
    }

    return new ConnectionNode(usesEndpoint.release(), providesEndpoint.release(), connectionId);
}

ConnectionNode::ConnectionNode(Endpoint* uses_, Endpoint* provides_, const std::string& identifier_) :
    uses(uses_),
    provides(provides_),
    identifier(identifier_),
    connected(false)
{
    assert(uses);
    assert(provides);
}

ConnectionNode::ConnectionNode(const ConnectionNode& other) :
    uses(other.uses->clone()),
    provides(other.provides->clone()),
    identifier(other.identifier),
    connected(other.connected)
{
}


bool ConnectionNode::connect(ConnectionManager& manager)
{
    if (connected) {
        return true;
    }

    CORBA::Object_var usesObject = CORBA::Object::_nil();
    CORBA::Object_var providesPort = CORBA::Object::_nil();
    try {
        usesObject = uses->resolve(manager);
    } catch ( ... ) {
        LOG_TRACE(ConnectionNode, "Unable to resolve the uses object");
    }
    try {
        providesPort = provides->resolve(manager);
    } catch ( ... ) {
        LOG_TRACE(ConnectionNode, "Unable to resolve the provides object");
    }

    if (CORBA::is_nil(usesObject) || CORBA::is_nil(providesPort)) {
        LOG_TRACE(ConnectionNode, "Unable to establish a connection because one or more objects cannot be resolved (i.e.: cannot create an event channel or device is not available)");
        if (uses->allowDeferral() && provides->allowDeferral()) {
            LOG_DEBUG(ConnectionNode, "Connection is deferred to a later date");
            return false;
        }
        //throw InvalidConnection("Connection cannot be deferred");
        return false;
    }

    CF::Port_var usesPort = ossie::corba::_narrowSafe<CF::Port>(usesObject);
    if (CORBA::is_nil(usesPort)) {
        LOG_ERROR(ConnectionNode, "Uses port is not a CF::Port");
        //throw InvalidConnection("Uses port is not a CF::Port");
        return false;
    }

    try {
        usesPort->connectPort(providesPort, identifier.c_str());
        connected = true;
        return true;
    } catch (const CF::Port::InvalidPort& ip) {
        LOG_ERROR(ConnectionNode, "Invalid port: " << ip.msg);
        //throw InvalidConnection(std::string("Invalid port: ") + ip.msg);
    } catch (const CF::Port::OccupiedPort& op) {
        LOG_ERROR(ConnectionNode, "Port is occupied");
        //throw InvalidConnection("Port is occupied");
    } CATCH_LOG_ERROR(ConnectionNode, "Port connection failed for connection " << identifier);

    //throw InvalidConnection("Unknown error");
    return false;
}

void ConnectionNode::disconnect(DomainLookup* domainLookup)
{
    if (!connected) {
        return;
    }

    // Clear connection state; even if the remaining code fails, we have to
    // assume the connection has been broken.
    connected = false;

    // Get the uses port before releasing both endpoints, which will release
    // the object references.
    CF::Port_var usesPort = ossie::corba::_narrowSafe<CF::Port>(uses->object());
    uses->release();
    provides->release();
    if (CORBA::is_nil(usesPort)) {
        LOG_ERROR(ConnectionNode, "Uses port is not a CF::Port");
        return;
    }

    try {
        unsigned long timeout = 500; // milliseconds
        omniORB::setClientCallTimeout(usesPort, timeout);
        usesPort->disconnectPort(identifier.c_str());
    } CATCH_LOG_WARN(ConnectionNode, "Unable to disconnect port for connection " << identifier);

#if ENABLE_EVENTS
    FindByDomainFinderEndpoint* endpoint = dynamic_cast<FindByDomainFinderEndpoint*>(provides.get());
    if (endpoint && endpoint->type() == "eventchannel") {
        std::string channelName = endpoint->name();
        if (channelName.empty()) {
            channelName = "IDM_Channel";
        }
        domainLookup->decrementEventChannelConnections(channelName);
    }
#endif
}

bool ConnectionNode::allowDeferral()
{
    if (!(uses->isResolved() || uses->allowDeferral())) {
        return false;
    }

    if (!(provides->isResolved() || provides->allowDeferral())) {
        return false;
    }

    return true;
}

bool ConnectionNode::checkDependency(Endpoint::DependencyType type, const std::string& identifier) const
{
    // Test the uses side first, then the provides side. If either side has the
    // given dependency, then so does this connection.
    return (uses->checkDependency(type, identifier) || provides->checkDependency(type, identifier));
}

CREATE_LOGGER(connectionSupport);

std::string ossie::eventChannelName(const FindBy* findby)
{
    if (findby->isFindByDomainFinder ()) {
        std::string dfType = findby->getFindByDomainFinderType();
        std::string dfName = findby->getFindByDomainFinderName();
        if (!dfType.compare("eventchannel")) {
            if (!dfName.compare("")) {
                dfName = "IDM_Channel";
            }
            return dfName;
        }
    }
    return "";
}

CORBA::Object_ptr ossie::getPort(CORBA::Object_ptr obj, const std::string& portId)
{
    LOG_TRACE(connectionSupport, "Finding port");
    CF::PortSupplier_var portSupplier;
    LOG_TRACE(connectionSupport, "Narrowing resource");
    try {
        portSupplier = CF::PortSupplier::_narrow (obj);
    } catch( ... ) {
        LOG_ERROR(connectionSupport, "Failed to narrow CF::Resource before obtaining Port with Unknown Exception");
        return CORBA::Object::_nil();
    }
    LOG_TRACE(connectionSupport, "Getting port with id - " << portId);
    try {
        return portSupplier->getPort(portId.c_str());
    } catch( ... ) {
        LOG_ERROR(connectionSupport, "getPort failed with Unknown Exception");
        return CORBA::Object::_nil();
    }
}
