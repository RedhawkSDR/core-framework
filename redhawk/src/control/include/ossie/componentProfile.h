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

/*
 * This file provides the data-structure classes that are shared between
 * the SoftwareAssembly and the DeviceManagerConfiguration.
 */
#ifndef __COMPONENT_PROFILE_H__
#define __COMPONENT_PROFILE_H__

#include <vector>
#include <map>
#include <string>
#include <memory>
#include "ossie/ossieparser.h"

namespace ossie {
    /*
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

    /*
     *
     */
    class ComponentProperty {
    public:
        std::string _id;

        ComponentProperty() {};

        virtual ~ComponentProperty() {};

        const char* getID() const;
        
        ComponentProperty* clone() const {
          return _clone();
        }       
        
        const std::string asString() const {
          return _asString();
        }
        

    protected:
    ComponentProperty( const ComponentProperty &a ):
        _id(a._id)
          {};

        void operator=( const ComponentProperty &src) {
          _id = src._id;
        };
        
    private:
        virtual const std::string _asString() const = 0;

    
        virtual ComponentProperty* _clone() const = 0;

    };

    ComponentProperty *new_clone(const ComponentProperty &a);

    template< typename charT, typename Traits>
    std::basic_ostream<charT, Traits>& operator<<(std::basic_ostream<charT, Traits> &out, const ComponentProperty& prop)
    {
        out << prop.asString();
        return out;
    }

    /*
     *
     */
    class SimplePropertyRef : public ComponentProperty {
    public:
      
      std::string _value;
      
      const char* getValue() const;

    private:
        ComponentProperty* _clone() const;
        
        const std::string _asString() const;
    };

    /*
     *
     */
    class SimpleSequencePropertyRef : public ComponentProperty {
    public:
      typedef std::vector<std::string> ValuesList;

      const ValuesList & getValues() const;

      ValuesList _values;
   
    private:
        ComponentProperty* _clone() const;
        const std::string  _asString() const;
    };

    /*
     *
     */
    class StructPropertyRef : public ComponentProperty {
    public:
      typedef ComponentPropertyMap  ValuesMap;
      
      virtual ~StructPropertyRef();
      const ValuesMap & getValue() const;

      ValuesMap   _values;

    private:
      ComponentProperty* _clone() const;
      const std::string  _asString() const;
    };

    /*
     *
     */
    class StructSequencePropertyRef : public ComponentProperty {
    public:
      typedef std::vector< ossie::ComponentPropertyMap > ValuesList;

           virtual ~StructSequencePropertyRef();
           const ValuesList & getValues() const;

           ValuesList _values;
    private:
           ComponentProperty* _clone() const;
           const std::string  _asString() const;

    };

    /*
     *
     */
    class ComponentInstantiation {
    public:
      typedef std::pair<std::string,std::string>  LoggingConfig;
      typedef ossie::ComponentPropertyList        AffinityProperties;

        std::string instantiationId;
        ossie::optional_value<std::string> namingservicename;
        ossie::optional_value<std::string> usageName;
        ossie::ComponentPropertyList       properties;
        std::string _startOrder;
        AffinityProperties affinityProperties;
        LoggingConfig loggingConfig;
    public:
        ComponentInstantiation();

        ComponentInstantiation(const ComponentInstantiation& other);

        ComponentInstantiation& operator=(const ComponentInstantiation &other);

        virtual ~ComponentInstantiation();

    public:
        const char* getID() const;
        
        const char* getStartOrder() const;

        const char* getUsageName() const;

        const ossie::ComponentPropertyList & getProperties() const;

        const LoggingConfig &getLoggingConfig() const;

        const AffinityProperties &getAffinity() const;

        bool isNamingService() const;

        const char* getFindByNamingServiceName() const;
    };

    /*
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

    /*
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

    /*
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
            DEVICEUSEDBYAPPLICATION,
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

        /*
         * componentinstantiationref
         */
        bool isComponentInstantiationRef() const {
            return (type == COMPONENTINSTANTIATIONREF);
        }

        const char* getComponentInstantiationRefID() const {
            return componentRefId.c_str();
        }

        void setComponentInstantiationRef(const std::string& refID) {
            componentRefId = refID;
            type = COMPONENTINSTANTIATIONREF;
        }

        /*
         * devicethatloadedthiscomponentref
         */
        bool isDeviceThatLoadedThisComponentRef() const {
            return (type == DEVICETHATLOADEDTHISCOMPONENTREF);
        }

        const char* getDeviceThatLoadedThisComponentRef() const {
            return componentRefId.c_str();
        }

        void setDeviceThatLoadedThisComponentRef(const std::string& refID) {
            componentRefId = refID;
            type = DEVICETHATLOADEDTHISCOMPONENTREF;
        }

        /*
         * deviceusedbythiscomponentref
         */
        bool isDeviceUsedByThisComponentRef() const {
            return (type == DEVICEUSEDBYTHISCOMPONENTREF);
        }

        const char* getDeviceUsedByThisComponentRefID() const {
            return componentRefId.c_str();
        }

        const char* getDeviceUsedByThisComponentRefUsesRefID() const {
            return usesRefId.c_str();
        }

        void setDeviceUsedByThisComponentRef(const std::string& componentRef, const std::string& usesRef) {
            componentRefId = componentRef;
            usesRefId = usesRef;
            type = DEVICEUSEDBYTHISCOMPONENTREF;
        }

        /*
         * deviceusedbyapplication
         */
        bool isDeviceUsedByApplication() const {
            return (type == DEVICEUSEDBYAPPLICATION);
        }

        const char* getDeviceUsedByApplicationUsesRefID() const {
            return usesRefId.c_str();
        }

        void setDeviceUsedByApplication(const std::string& refID) {
            usesRefId = refID;
            type = DEVICEUSEDBYAPPLICATION;
        }

        /*
         * findby
         */
        bool isFindBy() const {
            return (type == FINDBY);
        }

        const FindBy* getFindBy() const {
            return findBy.get();
        }

        void setFindBy(const FindBy& findby) {
            findBy = findby;
            type = FINDBY;
        }

        virtual const char* getID() const = 0;

    };

    /*
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

    /*
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

    /*
     *
     */
    class ComponentSupportedInterface : public Port
    {
    public:
        std::string identifier;

        virtual const char* getID() const {
            return identifier.c_str();
        }
    };

    /*
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
