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


#ifndef _SPD_SUPPORT_H
#define _SPD_SUPPORT_H

#include <string>
#include <vector>
#include <map>
#include <omniORB4/CORBA.h>

#include <ossie/CF/cf.h>
#include <ossie/CF/StandardEvent.h>
#include <ossie/SoftPkg.h>
#include <ossie/SoftwareAssembly.h>
#include <ossie/ComponentDescriptor.h>
#include <ossie/Properties.h>
#include <ossie/exceptions.h>
#include <ossie/ossieparser.h>
#include <ossie/componentProfile.h>
#include <ossie/affinity.h>

//
// Follows model from applicationSupport.h, except with the removal of
// pointer return objects and the removal of UsesDeviceContext.
// 

namespace ossie
{

  namespace SpdSupport {

      extern rh_logger::LoggerPtr spdSupportLog;

      class   ResourceInfo;
      class   SoftpkgInfo;
      class   ImplementationInfo;

      // list of placements
      typedef std::vector < ResourceInfo* >   ResourceList;


    /* Base class for all implementations  */
    class ResourceContext
    {
        ENABLE_LOGGING;

    public:
        ResourceContext(){};
        virtual ~ResourceContext(){};

    protected:

    };

    typedef SoftpkgInfo*                  SoftpkgInfoPtr;
    typedef std::vector< SoftpkgInfoPtr >   SoftpkgInfoList;

    /* Base class to contain data for implementations
     *  - Used to store information about about implementations
     */
    class ImplementationInfo : public ResourceContext
    {
        ENABLE_LOGGING;

    public:
        typedef std::vector< ImplementationInfo *>  List;

        ImplementationInfo(const SPD::Implementation& spdImpl);
        ~ImplementationInfo();

        bool operator==( const ImplementationInfo &other ) const;

        const std::string& getId() const;
        CF::LoadableDevice::LoadType getCodeType() const;
        const std::vector<std::string>& getProcessorDeps() const;
        const std::vector<ossie::SPD::NameVersionPair>& getOsDeps() const;
        const std::string& getPropertyFile() const;
        const std::string& getLocalFileName() const;
        const std::string& getEntryPoint() const;
        const CORBA::ULong getStackSize() const;
        const CORBA::ULong getPriority() const;
        const bool hasStackSize() const;
        const bool hasPriority() const;
        const std::vector<PropertyRef>& getDependencyProperties() const;
        const SoftpkgInfoList & getSoftPkgDependencies() const;

        bool checkProcessorAndOs(const ossie::Properties& prf) const;

        void clearSelectedDependencyImplementations();

        static ImplementationInfo *BuildImplementationInfo( CF::FileSystem_ptr fileSys, 
                                                            const SPD::Implementation& spdImpl,
							    CF::FileSystem_ptr depFileSys );

    protected:
        ImplementationInfo ( const ImplementationInfo&);
        void setLocalFileName(const char* fileName);
        void setEntryPoint(const char* fileName);
        void setCodeType(ossie::SPD::Code::CodeType _type);
        void setPropertyFile(const char* filename);
        void setStackSize(const unsigned long long *_stackSize);
        void setPriority(const unsigned long long *_priority);
        void addDependencyProperty(const ossie::PropertyRef& property);
        void addSoftPkgDependency(SoftpkgInfo *softpkg);

        std::string id;
        CF::LoadableDevice::LoadType codeType;
        std::string _codeType;
        std::string propertyFile;
        std::string localFileName;
        std::string entryPoint;
        CORBA::ULong stackSize;
        CORBA::ULong priority;
        bool _hasStackSize;
        bool _hasPriority;
        std::vector<std::string> processorDeps;
        std::vector<ossie::SPD::NameVersionPair> osDeps;
        std::vector<PropertyRef> dependencyProperties;
        SoftpkgInfoList softPkgDependencies;

    };

    class SoftpkgInfo : public ResourceContext
    {
        ENABLE_LOGGING

    public:
        
        SoftpkgInfo (const std::string& spdFileName);
        SoftpkgInfo ( const SoftpkgInfo &src );
        SoftpkgInfo ();
        ~SoftpkgInfo ();

        //SoftpkgInfo &operator=(const SoftpkgInfo &src );

        const char* getSpdFileName() const;
        const char* getName() const;
        const char* getID() const;

        void addImplementation(ImplementationInfo *impl);
        void getImplementations(ImplementationInfo::List& res) const ;

        const ImplementationInfo *getSelectedImplementation() const;
        ImplementationInfo *selectedImplementation() const;
        void setSelectedImplementation(const ImplementationInfo * implementation);
        void clearSelectedImplementation();

        static SoftpkgInfo*  BuildSoftpkgInfo (CF::FileSystem_ptr fileSys, 
                                               const char* spdFileName,
					       CF::FileSystem_ptr depFileSys );
        SoftPkg spd;

    protected:
        bool parseProfile (CF::FileSystem_ptr fileSys, CF::FileSystem_ptr depFileSys );

        const std::string _spdFileName;
        std::string _identifier;          // identifier from SPD File

        ImplementationInfo::List           _implementations;
        ImplementationInfo::List::iterator _selectedImplementation;
    };


    /* 
     * 
     */
    class ProgramProfile : public SoftpkgInfo
    {
        ENABLE_LOGGING

    public:
      typedef ossie::ComponentInstantiation::AffinityProperties AffinityProperties;
      typedef ossie::ComponentInstantiation::LoggingConfig  LoggingConfig;

        ProgramProfile (const std::string& spdFileName);
        ~ProgramProfile ();

        void setIdentifier(const std::string &identifier, const std::string &instance_id);
        void setNamingService(const bool isNamingService);
        void setNamingServiceName(const std::string &NamingServiceName);
        void setUsageName(const std::string &usageName);
        void setIsAssemblyController(bool isAssemblyController);
        void setIsScaCompliant(bool isScaCompliant);
        void setNicAssignment(const std::string &nic);
        void setAffinity( const AffinityProperties &affinity );
        void mergeAffinityOptions( const CF::Properties &new_affinity );
        void setLoggingConfig( const LoggingConfig &logcfg );
        void addResolvedSoftPkgDependency(const std::string &dep);
        std::vector<std::string> getResolvedSoftPkgDependencies();

        void addFactoryParameter(CF::DataType dt);
        void addExecParameter(CF::DataType dt);
        void addDependencyProperty(std::string implId, CF::DataType dt);
        void addConfigureProperty(CF::DataType dt);
        void addConstructProperty(CF::DataType dt);

        void fillSeqForStructProperty(CF::Properties &props);
        void fillAllSeqForStructProperty();

        void overrideProperty(const ossie::ComponentProperty* propref);
        void overrideProperty(const ossie::ComponentProperty& propref);
        void overrideProperty(const char* id, const CORBA::Any& value);
        void overrideSimpleProperty(const char* id, const std::string &value);

        const char* getInstantiationIdentifier();
        const char* getIdentifier();
        const bool  getNamingService();
        const char* getUsageName();
        const char* getNamingServiceName();
        const bool  isResource();
        const bool  isConfigurable();
        const bool  isAssemblyController();
        const bool  isScaCompliant();
        const std::string getNicAssignment();

        CF::Properties containsPartialStructConfig();
        CF::Properties containsPartialStructConstruct();
        CF::Properties iteratePartialStruct(CF::Properties &props);
        bool checkStruct(CF::Properties &props);

        CF::Properties getNonNilConfigureProperties();
        CF::Properties getNonNilConstructProperties();
        CF::Properties getNonNilNonExecConstructProperties();
        CF::Properties getConfigureProperties();
        CF::Properties getConstructProperties();
        CF::Properties getOptions();
        CF::Properties getAffinityOptions();
        CF::Properties getAffinityOptionsWithAssignment();
        CF::Properties getExecParameters();
        CF::Properties getPopulatedExecParameters();

        void load(CF::FileSystem_ptr fileSystem,
                  CF::FileSystem_ptr depFileSys );

        ComponentDescriptor scd;
        ossie::Properties prf;

    static ProgramProfile *LoadProfile(CF::FileSystem_ptr fileSystem, 
				       const char* _SPDFile,
				       CF::FileSystem_ptr depFileSys );

    static std::unique_ptr<ProgramProfile> LoadProgramProfile(CF::FileSystem_ptr fileSystem, 
							    const char* _SPDFile,
							    CF::FileSystem_ptr depFileSys );

    protected:

        void process_overrides(CF::Properties* props, const char* id, const CORBA::Any &value);
        bool _isAssemblyController;
        bool _isConfigurable;
        bool _isScaCompliant;
        bool isNamingService;

        std::string usageName;
        std::string identifier;
        std::string instantiationId;
        std::string namingServiceName;
        std::string nicAssignment;

        ossie::Properties _affinity_prf;
        LoggingConfig     loggingConfig;

        CF::Properties configureProperties;
        CF::Properties ctorProperties;
        CF::Properties options;
        CF::Properties factoryParameters;
        CF::Properties execParameters;
        CF::Properties affinityOptions;
        
        std::vector<std::string> resolved_softpkg_dependencies;

    private:
        ProgramProfile &operator=(const ProgramProfile&);
        ProgramProfile ( const ProgramProfile&);
        ProgramProfile ( ProgramProfile&);
	
    };




  };



};

namespace local_spd = ossie::SpdSupport;

#endif
