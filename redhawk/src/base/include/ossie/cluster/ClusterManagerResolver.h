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
#ifndef __RH_CLUSTER_CONFIG_RESOLVER_H__
#define __RH_CLUSTER_CONFIG_RESOLVER_H__
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <string>
#include <map>
#include <unistd.h>
#include <stdlib.h>

#include <ossie/PropertyMap.h>

#include "yaml-cpp/yaml.h"

namespace ossie {

	namespace cluster {

		/*
		 * Pure virtual parent class that holds all the functions that REDHAWK expects to find in the plugin.
		 * 
		 * REDHAWK uses the following functions to launch, delete, and check on components. The plugins must inherit this class 
		 * and fill in the logic for the various required REDHAWK functionalities. When compiling the user must put `MAKE_CLUSTER_FACTORY(<class name>);` 
		 * at the end of their cpp file that is including their class. This will make it so that the plugin loader knows that the user wants to use this 
		 * cluster technology.  
		 */
		class ClusterManagerResolver  {
			public:

				virtual ~ClusterManagerResolver() {};

				/*
				 * Launches a component or yaml file of multiple components into the cluster
				 * @param app_id the redhawk code passes the plugin the application ID (can be used for namespaces for example)
				 * @return An integer representing a pid. A negative pid will throw an error while a pid 0 and greater will succeed
				 */
				virtual int launchComponent(std::string app_id) = 0;

				/*
				 * Polls the component and waits to see that it is active and running (equivalent to REDHAWKs native pid check but for clusters)
				 * @param comp_id the key that is used on validNamesMap to find the name of the component that is being checked to be active
				 * @return Boolean where true means the component is active and false means the component is not. REDHAWK prints an error to logs if false
				 */
				virtual bool pollStatusActive(std::string comp_id) = 0;

				/*
				 * Polls the component and waits for terminatation (in clusters this might mean CrashLoopBackoff, Completed, etc...)
				 * @param comp_id the key that is used on validNamesMap to find the name of the component that is being checked to be terminated
				 * @return Boolean where true means the component is terminated and false means the component is not. REDHAWK prints an error to logs if false
				 */
				virtual bool pollStatusTerminated(std::string comp_id) = 0;

				/*
				 * Deletes a component or multiple components in a yaml file from the namespace
				 * @param comp_id the key that is used on validNamesMap to find the name of the component that is being checked to be deleted
				 */
				virtual void deleteComponent(std::string comp_id) = 0;

				/*
				 * One-off check for if a component has terminated (in clusters this might mean CrashLoopBackoff, Completed, etc...)
				 * @param comp_id the key that is used on validNamesMap to find the name of the component that is being checked to be terminated
				 * @return true if terminated and false if not. Throws a ComponentTerminated exception if false on start up
				 */
				virtual bool isTerminated(std::string comp_id) = 0;

				/*
				 * Adds a component to the yaml file so that all cluster type components are in the file before launching. This is also the location where non-cluster type deployments would run executables (see DockerResolver.cpp)
				 * @param execParameters The parameters given to the component to be able to execute it. These parameters can instead be baked into the yaml file for the cluster technology to launch itself 
				 *        (i.e /path/to/executable NAMING_CONTEXT_IOR <nameing_context_ior> PROFILE_NAME <profile_name> NAME_BINDING <name_binding> COMPONENT_IDENTIFIER <component_identifier> DEBUG_LEVEL <debug_level>). Other params include:
				 *        NIC
				 *        RH::GPP::MODIFIED_CPU_RESERVATION_VALUE
				 * @param entryPoint The path to the executable (i.e /var/redhawk/sdr/dom/components/rh/SigGen/cpp/SigGen)
				 * @param image The image name that was attached to the entrypoint in the spd.xml file (i.e in the spd <\entrypoint>/path/to/executable::image<\entrypoint>). 
				 *              This is not the full path to a AWS directory. That full path will instead be found in /usr/local/redhawk/core/aws/cluster.cfg.
				 */
				virtual void openComponentConfigFile(redhawk::PropertyMap execParameters, std::string entryPoint, std::string image) = 0;

				/*
				 * Closes the yaml file that was being written to
				 * @param app_id the application ID is given so that when the file is saved out it can be unique
				 */
				virtual void closeComponentConfigFile(std::string app_id) = 0;



				void open_tmp_file() {
					RH_NL_TRACE("Cluster", "filename");
					filename = "/tmp/k8s_waveform.XXXXXX";
					char char_array[filename.length()+1];
					strcpy(char_array, filename.c_str()); // template for our file. 
					RH_NL_TRACE("Cluster", "mkstemp");       
					fd = mkstemp(char_array);
					filename = std::string(char_array);
				}

				void close_tmp_file() {
					if (fd != -1) {
					   close(fd);
					   fd = -1;
					   char char_array[filename.length()+1];
					   strcpy(char_array, filename.c_str());
					   remove(char_array);
					}
				}
			protected:

				/*
				 * Contructor for the parent cluster manager
				 * @param app_id the redhawk code passes the plugin the application ID (can be used for namespaces for example)
				 */
				ClusterManagerResolver(std::string app_id){};

				/// The yaml file that is used to launch multiple components at once in various cluster technologies (moved here because both k8s and docker swarm use it)
				YAML::Emitter yaml;

				/// The path to the file that is being written to (written as "/tmp/k8s-" and combined with the app_id and extension to create a file name file_prefix+app_id+extension)
				std::string file_prefix;
				
				/// A map to track the component ids and names of components of cluster types. This is used to check if specific components in a list are active, terminated, or deleted (i.e name = validNamesMap[comp_id])
				std::map<std::string, std::string> validNamesMap;
				
				/// The time to wait for polling
				double timeout;

				int fd;
				std::string filename;

			private:
				ClusterManagerResolver( const ClusterManagerResolver &src) {};

		};

		typedef boost::shared_ptr<ClusterManagerResolver>  ClusterManagerResolverPtr;
		typedef ClusterManagerResolverPtr   (*ClusterFactory)(std::string app_id);

	};

};



// the class factories

#define MAKE_CLUSTER_FACTORY(CLAZZ) \
\
  class CLAZZ;  \
\
extern "C" { \
  ossie::cluster::ClusterManagerResolverPtr cluster_factory(std::string app_id) {	\
    return ossie::cluster::ClusterManagerResolverPtr( new CLAZZ(app_id) );	\
  }; \
\
  void destroy( ossie::cluster::ClusterManagerResolverPtr &p) {	\
  p.reset(); \
}; \
};

#endif
