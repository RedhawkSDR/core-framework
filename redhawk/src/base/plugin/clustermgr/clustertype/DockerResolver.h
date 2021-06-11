#ifndef DOCKER_RESOLVER
#define DOCKER_RESOLVER

#include <ossie/cluster/ClusterManagerResolver.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "yaml-cpp/yaml.h"

class DockerResolver : public ossie::cluster::ClusterManagerResolver {
        public:
            DockerResolver (std::string app_id);
            virtual ~DockerResolver(){}

            virtual void openComponentConfigFile(redhawk::PropertyMap execParameters, std::string entryPoint, std::string image);
            virtual void closeComponentConfigFile(std::string identifier);

            virtual int launchComponent(std::string comp_id);
            virtual void deleteComponent(std::string name);

            virtual bool pollStatusActive(std::string name);
            virtual bool pollStatusTerminated (std::string name);
            virtual bool isTerminated(std::string name);
};

#endif
