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

#ifndef ENDPOINTS_H
#define ENDPOINTS_H

#include "connectionSupport.h"

namespace ossie {

    class ApplicationEndpoint : public Endpoint {
    public:
        ApplicationEndpoint() { }

        ApplicationEndpoint(const std::string& identifier) :
            identifier_(identifier)
        {
        }
        
        ApplicationEndpoint(const ApplicationEndpoint& other):
            Endpoint(other),
            identifier_(other.identifier_)
        {
        }

        // Application identifiers are generated at creation time, so it's not
        // necessarily safe to try to predict one for a future application; as
        // such, it does not make sense to allow deferred resolution.
        virtual bool allowDeferral(void) { return false; }

        virtual bool checkDependency(DependencyType type, const std::string& identifier) const
        {
            return ((type == APPLICATION) && (identifier == identifier_));
        }

        virtual ApplicationEndpoint* clone() const
        {
            return new ApplicationEndpoint(*this);
        }

    private:
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager)
        {
            return manager.resolveDomainObject("application", identifier_);
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

    class ApplicationUsesDeviceEndpoint : public Endpoint {
    public:
        ApplicationUsesDeviceEndpoint() { }

        ApplicationUsesDeviceEndpoint(const std::string usesIdentifier) :
            usesIdentifier_(usesIdentifier)
        {
        }

        ApplicationUsesDeviceEndpoint(const ApplicationUsesDeviceEndpoint& other) :
            Endpoint(other),
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

        virtual ApplicationUsesDeviceEndpoint* clone() const
        {
            return new ApplicationUsesDeviceEndpoint(*this);
        }

    private:
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager)
        {
            return manager.resolveDeviceUsedByApplication(usesIdentifier_);
        }

#if HAVE_BOOST_SERIALIZATION
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, unsigned int version)
        {
            ar & boost::serialization::base_object<Endpoint>(*this);
            ar & usesIdentifier_;
        }
#endif

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

    class ServiceEndpoint : public Endpoint {
    public:
        ServiceEndpoint() { }

        ServiceEndpoint(const std::string& name) :
            name_(name)
        {
        }

        ServiceEndpoint(const ServiceEndpoint& other):
            Endpoint(other),
            name_(other.name_)
        {
        }

        virtual bool allowDeferral(void) { return true; }

        virtual bool checkDependency(DependencyType type, const std::string& identifier) const
        {
            return ((type == Endpoint::SERVICENAME) && (identifier == name_));
        }

        virtual ServiceEndpoint* clone() const
        {
            return new ServiceEndpoint(*this);
        }

        std::string name() const
        {
            return name_;
        }

    private:
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager)
        {
            return manager.resolveDomainObject("servicename", name_);
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


    class EventChannelEndpoint : public Endpoint {
    public:
        EventChannelEndpoint() { }

        EventChannelEndpoint(const std::string& name) :
            name_(name)
        {
        }

        EventChannelEndpoint(const EventChannelEndpoint& other):
            Endpoint(other),
            name_(other.name_)
        {
        }

        virtual bool allowDeferral(void) { return true; }

        virtual bool checkDependency(DependencyType type, const std::string& identifier) const
        {
            return false;
        }

        virtual EventChannelEndpoint* clone() const
        {
            return new EventChannelEndpoint(*this);
        }

        std::string name() const
        {
            return name_;
        }

    private:
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager)
        {
            return manager.resolveDomainObject("eventchannel", name_);
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


    class ObjectrefEndpoint : public Endpoint {
    public:
        ObjectrefEndpoint() { }

        ObjectrefEndpoint(CORBA::Object_ptr name) :
            objectref_(name)
        {
        }

        ObjectrefEndpoint(const ObjectrefEndpoint& other):
            Endpoint(other),
            objectref_(CORBA::Object::_duplicate(other.objectref_))
        {
        }

        virtual bool allowDeferral(void) { return false; }

        virtual bool checkDependency(DependencyType type, const std::string& identifier) const
        {
            return false;
        }

        virtual ObjectrefEndpoint* clone() const
        {
            return new ObjectrefEndpoint(*this);
        }

    private:
        virtual CORBA::Object_ptr resolve_(ConnectionManager& manager)
        {
            return CORBA::Object::_duplicate(objectref_);
        }

#if HAVE_BOOST_SERIALIZATION
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, unsigned int version)
        {
            ar & boost::serialization::base_object<Endpoint>(*this);
            ar & objectref_;
        }
#endif

        CORBA::Object_var objectref_;
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
            identifier__ = supplier_->getIdentifier();
        }

        PortEndpoint(const PortEndpoint& other) :
            Endpoint(other),
            supplier_(other.supplier_->clone()),
            name_(other.name_),
            invalidPort_(other.invalidPort_)
        {
            identifier__ = supplier_->getIdentifier();
        }

        virtual CF::ConnectionManager::EndpointStatusType toEndpointStatusType() const
        {
            CF::ConnectionManager::EndpointStatusType status = Endpoint::toEndpointStatusType();
            status.portName = name_.c_str();
            return status;
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

};

#endif // ENDPOINTS_H
