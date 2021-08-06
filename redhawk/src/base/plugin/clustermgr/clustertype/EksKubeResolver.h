#ifndef EKS_KUBE
#define EKS_KUBE

#include <ossie/cluster/ClusterManagerResolver.h>
#include <iostream>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <stdlib.h>
#include <map>

#include "yaml-cpp/yaml.h"

class EksKubeResolver : public ossie::cluster::ClusterManagerResolver {
        public:
            EksKubeResolver (std::string app_id);

            virtual ~EksKubeResolver();

            virtual void openComponentConfigFile(redhawk::PropertyMap execParameters, std::string entryPoint, std::string image);
            virtual void closeComponentConfigFile(std::string identifier);

            virtual int launchComponent(std::string identifier);
            virtual void deleteComponent(std::string name);

            virtual bool pollStatusActive(std::string name);
            virtual bool pollStatusTerminated (std::string name);

            virtual bool isTerminated(std::string name);

        protected:
            std::string registry;
            std::string tag;
            std::string dockerconfigjson;
            std::string wave_namespace;
	    boost::property_tree::ptree pt;

            bool deletedAlready;

            void configureRegistrySecret();
            void configureNamespace();

};

#endif
