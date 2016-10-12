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

/**
 * This file provides the data-structure classes that are shared between
 * the SoftwareAssembly and the DeviceManagerConfiguration.
 */
#ifndef __COMPONENT_PROFILE_H__
#define __COMPONENT_PROFILE_H__

#include "ossie/ossieparser.h"
#include <vector>
#include <map>
#include <string>

namespace ossie {
    /**
     *
     */
    class ComponentFile {
    public:
        std::string filename;
        std::string id;
        std::string type;

    public:
        const char* getFileName() const;

        const char* getID() const;
    };

    /**
     *
     */
    class ComponentProperty {
    public:
        std::string _id;

        virtual ~ComponentProperty();

        const char* getID() const;

        virtual ComponentProperty* clone() const = 0;
    
        virtual const std::string asString() const = 0;
    }
    ;
    template< typename charT, typename Traits>
    std::basic_ostream<charT, Traits>& operator<<(std::basic_ostream<charT, Traits> &out, const ComponentProperty& prop)
    {
        out << prop.asString();
        return out;
    }

    /**
     *
     */
    class SimplePropertyRef : public ComponentProperty {
    public:
        std::string _value;

        const char* getValue() const;

        virtual ComponentProperty* clone() const;
        
        virtual const std::string asString() const;
    };

    /**
     *
     */
    class SimpleSequencePropertyRef : public ComponentProperty {
    public:
        std::vector<std::string> _values;

        virtual ComponentProperty* clone() const;

        const std::vector<std::string>& getValues() const;

        virtual const std::string asString() const;
    };

    /**
     *
     */
    class StructPropertyRef : public ComponentProperty {
    public:
        std::map<std::string, std::string> _values;
        virtual ComponentProperty* clone() const;
        const std::map<std::string, std::string>& getValue() const;
        virtual const std::string asString() const;
    };

    /**
     *
     */
    class StructSequencePropertyRef : public ComponentProperty {
        public:
            std::vector<std::map<std::string, std::string> > _values;
            virtual ComponentProperty* clone() const;
            const std::vector<std::map<std::string, std::string> >& getValues() const;
            virtual const std::string asString() const;
    };

    /**
     *
     */
    class ComponentInstantiation {
    public:
        std::string instantiationId;
        ossie::optional_value<std::string> namingservicename;
        ossie::optional_value<std::string> usageName;
        std::vector<ComponentProperty*> properties;
        std::string _startOrder;

    public:
        ComponentInstantiation();

        ComponentInstantiation(const ComponentInstantiation& other);

        ComponentInstantiation& operator=(ComponentInstantiation other);

        virtual ~ComponentInstantiation();

    public:
        const char* getID() const;
        
        const char* getStartOrder() const;

        const char* getUsageName() const;

        const std::vector<ComponentProperty*>& getProperties() const;

        bool isNamingService() const;

        const char* getFindByNamingServiceName() const;
    };

    /**
     *
     */
    class ComponentPlacement {
    public:
        bool ifDomainManager;
        std::string _componentFileRef;

        ossie::optional_value<std::string> deployOnDeviceID;
        ossie::optional_value<std::string> compositePartOfDeviceID;
        ossie::optional_value<std::string> DPDFile;
        std::vector<ComponentInstantiation> instantiations;

    public:
        const char* getDeployOnDeviceID() const;

        const char* getCompositePartOfDeviceID() const;

        const std::string getDPDFile() const;
        
        const std::vector<ComponentInstantiation>& getInstantiations() const;
        
        const char* getFileRefId() const;

        bool isDeployOn() const;

        bool isCompositePartOf() const;

        bool isDomainManager() const;
    };

    /**
     *
     */
    class FindBy
    {
    public:
        ossie::optional_value<std::string> findByNamingService;
        ossie::optional_value<std::string>  findByStringifiedObjectRef;
        ossie::optional_value<std::string>  findByDomainFinderType;
        ossie::optional_value<std::string>  findByDomainFinderName;

        const char* getFindByDomainFinderName() const {
            if (findByDomainFinderName.isSet()) {
                return findByDomainFinderName->c_str();
            } else {
                return 0;
            }
        };

        const char* getFindByDomainFinderType() const {
            if (findByDomainFinderType.isSet()) {
                return findByDomainFinderType->c_str();
            } else {
                return 0;
            }
        };

        const char* getFindByNamingServiceName() const {
            if (findByNamingService.isSet()) {
                return findByNamingService->c_str();
            } else {
                return 0;
            }
        };

        const char* getFindByStringifiedObjectRef() const {
            if (findByStringifiedObjectRef.isSet()) {
                return findByStringifiedObjectRef->c_str();
            } else {
                return 0;
            }
        };

        bool isFindByDomainFinder() const {
            return findByDomainFinderName.isSet();
        };

        bool isFindByNamingService() const {
            return findByNamingService.isSet();
        };

        bool isFindByStringifiedObjectRef() const {
            return findByStringifiedObjectRef.isSet();
        };

    };

    /**
     *
     */
    class Port
    {
    public:
        typedef enum {
            NONE = 0,
            COMPONENTINSTANTIATIONREF,
            DEVICETHATLOADEDTHISCOMPONENTREF,
            DEVICEUSEDBYTHISCOMPONENTREF,
            FINDBY
        } port_type;

    public:
        Port() :
        type(NONE),
        componentRefId(""),
        usesRefId("")
        {}

        Port(port_type _type, std::string _crefid, std::string _usesrefid) : 
        type(_type),
        componentRefId(_crefid),
        usesRefId(_usesrefid)
        {}

        virtual ~Port() {}

    public:
        ossie::optional_value<FindBy> findBy;
        port_type type;
        std::string componentRefId;
        std::string usesRefId;

        const char* getComponentInstantiationRefID() const {
            return componentRefId.c_str();
        }

        const char* getDeviceThatLoadedThisComponentRef() const {
            return componentRefId.c_str();
        }

        const char* getDeviceUsedByThisComponentRefID() const {
            return componentRefId.c_str();
        }

        const char* getDeviceUsedByThisComponentRefUsesRefID() const {
            return usesRefId.c_str();
        }

        const FindBy* getFindBy() const {
            return findBy.get();
        }

        bool isComponentInstantiationRef() const {
            return (type == COMPONENTINSTANTIATIONREF);
        }

        bool isDeviceThatLoadedThisComponentRef() const {
            return (type == DEVICETHATLOADEDTHISCOMPONENTREF);
        }

        bool isDeviceUsedByThisComponentRef() const {
            return (type == DEVICEUSEDBYTHISCOMPONENTREF);
        }

        bool isFindBy() const {
            return (type == FINDBY);
        }

        virtual const char* getID() const = 0;

    };

    /**
     *
     */
    class UsesPort : public Port
    {
    public:
        std::string identifier;

        virtual const char* getID() const {
            return identifier.c_str();
        };

    };

    /**
     *
     */
    class ProvidesPort : public Port
    {
    public:
        std::string identifier;

        virtual const char* getID() const {
            return identifier.c_str();
        };
    };

    /**
     *
     */
    class ComponentSupportedInterface
    {
    public:
        std::string identifier;
        ossie::optional_value<std::string>componentInstantiationRefId;
        ossie::optional_value<FindBy> theFindBy;

        const std::string& getComponentInstantiationRefId() const {
            return *componentInstantiationRefId;
        }

        const std::string& getID() const {
            return identifier;
        }

        const FindBy* getFindBy() const {
            return theFindBy.get();
        }

        bool isComponentInstantiationRef() const {
            return componentInstantiationRefId.isSet();
        };

        bool isFindBy() const {
            return theFindBy.isSet();
        };

    };

    /**
     *
     */
    class Connection {
    public:
        typedef enum {
            NONE = 0,
            PROVIDESPORT,
            FINDBY,
            COMPONENTSUPPORTEDINTERFACE
        } port_type;

    public:
        const char* getID() const {
            return connectionId.c_str();
        }

        const port_type getType() const {
            return type;
        }

        const FindBy* getFindBy() const {
            return findBy.get();
        }

        const UsesPort* getUsesPort() const {
            return &usesPort;
        }

        const ProvidesPort* getProvidesPort() const {
            return providesPort.get();
        }

        const ComponentSupportedInterface* getComponentSupportedInterface() const {
            return componentSupportedInterface.get();
        }

        bool isComponentSupportedInterface() const {
            return type == COMPONENTSUPPORTEDINTERFACE;
        }

        bool isFindBy() const {
            return type == FINDBY;
        }

        bool isProvidesPort() const {
            return type == PROVIDESPORT;
        }

    public:
        std::string connectionId;

        UsesPort usesPort;
        ossie::optional_value<ProvidesPort> providesPort;
        ossie::optional_value<FindBy> findBy;
        ossie::optional_value<ComponentSupportedInterface>componentSupportedInterface;

        port_type type;
    };
}
#endif
