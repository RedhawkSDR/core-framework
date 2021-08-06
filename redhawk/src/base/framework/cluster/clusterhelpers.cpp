#include <dlfcn.h>

#include <ossie/cluster/clusterhelpers.h>
#include <ossie/cluster/ClusterManagerResolver.h>
#include <ossie/PropertyMap.h>

namespace ossie {
	namespace cluster {

		class _EmptyClusterManager : public ClusterManagerResolver {
			public:
				_EmptyClusterManager(std::string app_id) : ClusterManagerResolver(app_id) {};
				
				virtual ~_EmptyClusterManager() {};

				virtual int launchComponent(std::string comp_id) { return 0; }

				virtual bool pollStatusActive(std::string name) { return false; }
				virtual bool pollStatusTerminated(std::string name) { return false; }

				virtual void deleteComponent(std::string name) {}

				virtual bool isTerminated(std::string name) { return false; }

				virtual void openComponentConfigFile(redhawk::PropertyMap execParameters, std::string entryPoint, std::string image) { return; }

				virtual void closeComponentConfigFile(std::string file_name) {}
		};

		struct ClusterManagerHolder {
			void *library;
			ClusterFactory factory;
			ClusterManagerHolder() : library(NULL), factory(NULL){};
			~ClusterManagerHolder() {
				close();
			}
			void close() {
				if (library) dlclose(library);
				library = NULL;
			}
		};

		static ClusterManagerResolverPtr	_cluster_resolver;
		static ClusterManagerHolder		_clusterMgr;

		void _LoadClusterManagerLibrary () {
			if (_clusterMgr.library) _clusterMgr.close();

			try {
				void* cluster_library = dlopen("libossiecluster.so", RTLD_LAZY);
				if (!cluster_library) {
					throw 1;
				}
				_clusterMgr.library=cluster_library;
				// reset errors
				dlerror();
			
				// load the symbols
				ossie::cluster::ClusterFactory cluster_factory = (ossie::cluster::ClusterFactory) dlsym(cluster_library, "cluster_factory");
				const char* dlsym_error = dlerror();
				if (dlsym_error) {
				RH_NL_ERROR( "ossie.clustermgr", "Cannot find cluster_factory symbol in libossicluster.so library: " << dlsym_error);
				throw 2;
				}
				_clusterMgr.factory=cluster_factory;

				RH_NL_DEBUG( "ossie.clustermgr",  "ossie.clustermgr: Found libossiecluster.so for LOGGING_CONFIG_URI resolution." );
			}
			catch( std::exception &e){
				RH_NL_ERROR( "ossie.clustermgr", "Error: Exception on cluster load " << e.what());
			}
			catch( int e){
				RH_NL_ERROR( "ossie.clustermgr", "Error: Exception on cluster load with integer output " << e);
				if ( e == 2 ) { // library symbol look up error
				if ( _clusterMgr.library ) dlclose(_clusterMgr.library); 
				_clusterMgr.library = 0;
				}
				
			}

		}

		ClusterManagerResolverPtr   GetClusterManagerResolver(std::string app_id) {
			if (_cluster_resolver) {
				return _clusterMgr.factory(app_id);
			}
			
			_LoadClusterManagerLibrary();

			// If the factory does not exist a new class was not found and a default 
			// cluster manager will be used instead
			if (!_clusterMgr.factory) {
				RH_NL_ERROR("ossie.clustermgr", "Could not find cluster factory. This means a default cluster manager will be used instead!");
				return ClusterManagerResolverPtr ( new _EmptyClusterManager(app_id) );
			}
			else  {
				RH_NL_TRACE("ossie.clustermgr", "No cluster resolver. It will now be set by the Cluster Manager Factory");
				_cluster_resolver = _clusterMgr.factory(app_id);
				return _cluster_resolver;
			}
		}
	};
};
