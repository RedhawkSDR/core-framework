#ifndef DOCKER_SWARM
#define DOCKER_SWARM

#include <ossie/cluster/ClusterManagerResolver.h>
#include <iostream>
#include <sstream>
#include <map>

#include "yaml-cpp/yaml.h"

class DockerSwarmResolver : public ossie::cluster::ClusterManagerResolver {
        public:
            DockerSwarmResolver (std::string app_id);

            virtual ~DockerSwarmResolver(){}

            virtual void openComponentConfigFile(redhawk::PropertyMap execParameters, std::string entryPoint, std::string image);
            virtual void closeComponentConfigFile(std::string identifier);

            virtual int launchComponent(std::string comp_id);
            virtual void deleteComponent(std::string name);

            virtual bool pollStatusActive(std::string name);
            virtual bool pollStatusTerminated (std::string name);

            virtual bool isTerminated(std::string name);
        private:
            std::string _stack;
            size_t _longest_name;

            std::string user;
            std::string key;
            std::string ip;
            std::string registry;
            std::string tag;

            std::string ssh_cmd;
            std::string scp_cmd;
            std::string docker_login;
            std::string docker_login_cmd;

};

#endif
