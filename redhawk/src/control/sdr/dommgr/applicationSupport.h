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


#ifndef APPLICATIONSUPPORT_H
#define APPLICATIONSUPPORT_H

#include <string>
#include <vector>
#include <list>

#include <ossie/CF/cf.h>
#include <ossie/SoftPkg.h>
#include <ossie/SoftwareAssembly.h>
#include <ossie/ComponentDescriptor.h>
#include <ossie/Properties.h>
#include <ossie/ossieparser.h>
#include <ossie/componentProfile.h>

// Application support routines

// Base class to contain data for the components required to
// create an application
// Application support routines

namespace ossie
{
    struct ApplicationComponent {
        std::string identifier;
        std::string softwareProfile;
        std::string namingContext;
        std::string implementationId;
        std::vector<std::string> loadedFiles;
        unsigned long processId;
        CORBA::Object_var componentObject;
        CF::Device_var assignedDevice;
    };
    typedef std::list<ApplicationComponent> ComponentList;

    class SoftpkgInfo;

    /* Base class to contain data for implementations
     *  - Used to store information about about implementations
     */
    class ImplementationInfo
    {
        ENABLE_LOGGING;

    public:
        typedef std::vector< ImplementationInfo* >  List;

        ImplementationInfo(const SPD::Implementation& spdImpl);
        ~ImplementationInfo();

        const ossie::SPD::Implementation* getImplementation() const;

        const std::string& getId() const;
        const std::vector<std::string>& getProcessorDeps() const;
        const std::vector<ossie::SPD::NameVersionPair>& getOsDeps() const;
        const std::vector<SoftpkgInfo*>& getSoftPkgDependency() const;

        bool checkProcessorAndOs(const ossie::Properties& prf) const;

        static ImplementationInfo* buildImplementationInfo(CF::FileSystem_ptr fileSys, const SPD::Implementation& spdImpl);


    private:
        ImplementationInfo (const ImplementationInfo&);
        void setStackSize(const unsigned long long *_stackSize);
        void setPriority(const unsigned long long *_priority);
        void addSoftPkgDependency(SoftpkgInfo* softpkg);

        const ossie::SPD::Implementation* implementation;

        std::vector<SoftpkgInfo*> softPkgDependencies;

    };

    class SoftpkgInfo
    {
        ENABLE_LOGGING

    public:
        
        SoftpkgInfo (const std::string& spdFileName);
        ~SoftpkgInfo ();

        const std::string& getSpdFileName() const;
        const std::string& getName() const;

        void addImplementation(ImplementationInfo* impl);
        const ImplementationInfo::List& getImplementations() const;

        static SoftpkgInfo* buildSoftpkgInfo (CF::FileSystem_ptr fileSys, const char* spdFileName);

        SoftPkg spd;

    protected:
        bool parseProfile (CF::FileSystem_ptr fileSys);

        const std::string _spdFileName;

        ImplementationInfo::List _implementations;
    };

    /* Base class to contain data for components
     *  - Used to store information about about components
     */
    class ComponentInfo : public SoftpkgInfo
    {
        ENABLE_LOGGING

    public:
      typedef ossie::ComponentInstantiation::AffinityProperties AffinityProperties;

        ComponentInfo (const std::string& spdFileName, const ComponentInstantiation* instantiation);
        ~ComponentInfo ();

        const ComponentInstantiation* getInstantiation() const;

        void setUsageName(const char* usageName);
        void setAffinity( const AffinityProperties &affinity );

        void addFactoryParameter(CF::DataType dt);
        void addExecParameter(CF::DataType dt);
        void addDependencyProperty(std::string implId, CF::DataType dt);
        void addConfigureProperty(CF::DataType dt);
        void addConstructProperty(CF::DataType dt);

        void overrideProperty(const ossie::ComponentProperty* propref);
        void overrideProperty(const char* id, const CORBA::Any& value);

        const std::string& getInstantiationIdentifier() const;
        const char* getUsageName() const;
        bool isResource() const;
        bool isConfigurable() const;
        bool isAssemblyController() const;
        bool isScaCompliant() const;

        CF::Properties containsPartialStructConfig() const;
        CF::Properties containsPartialStructConstruct() const;
        CF::Properties iteratePartialStruct(const CF::Properties &props) const;
        bool checkStruct(const CF::Properties &props) const;

        CF::Properties getNonNilConfigureProperties() const;
        CF::Properties getInitializeProperties() const;
        CF::Properties getConfigureProperties() const;
        CF::Properties getConstructProperties() const;
        CF::Properties getOptions();
        CF::Properties getAffinityOptions() const;
        CF::Properties getExecParameters();
        CF::Properties getCommandLineParameters() const;

        static ComponentInfo* buildComponentInfoFromSPDFile(CF::FileSystem_ptr fileSys,
                                                            const std::string& spdFileName,
                                                            const ComponentInstantiation* instantiation);
        ComponentDescriptor scd;
        ossie::Properties prf;

    private:
        ComponentInfo (const ComponentInfo&);

        void process_overrides(CF::Properties* props, const char* id, CORBA::Any value);
        bool _isConfigurable;

        std::string usageName;

        ossie::Properties _affinity_prf;

        CF::Properties configureProperties;
        CF::Properties ctorProperties;
        CF::Properties options;
        CF::Properties factoryParameters;
        CF::Properties execParameters;
        CF::Properties affinityOptions;

        const ComponentInstantiation* instantiation;
    };

}
#endif
