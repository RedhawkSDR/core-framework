# Plugin Class
REDHAWK offers an extensible plugin class for running waveforms on container orchestration technologies.
These capabilities have been integrated into core-framework through modifications to the Application Factory and Sandbox.
It is still possible to use the standard Domain Manager and GPP combo to run native Executable and SharedLibrary type components on your REDHAWK systems.

Each cluster technology has its own ways to launch, configure, and delete containers.
The plugin system allows users to develop their own plugins that Domain Manager dynamically loads at run-time to spawn waveforms on a cluster according configuration file `$OSSIEHOME/cluster.cfg`.

The parent plugin class can be used to inherit from and form the skeleton of your own plugin. It can be found at `core-framework/src/base/include/ossie/cluster/ClusterManagerResolver.h`.  The class' public methods are those that your derived class can overwrite for you orchestration technology's way of handling containers.

## Public Methods
<br>

`launchComponent(std::string app_id)`
  : * Launches a component or yaml file of multiple components into the cluster
    * `@param app_id` the redhawk code passes the plugin the application ID (can be used for namespaces for example)
    * `@return An integer` representing a pid. A negative pid will throw an error while a pid 0 and greater will succeed

`pollStatusActive(std::string app_id)`
  : * Polls the component and waits to see that it is active and running (equivalent to REDHAWKs native pid check but for clusters)
    * `@param comp_id` the key that is used on validNamesMap to find the name of the component that is being checked to be active
    * `@return Boolean` where true means the component is active and false means the component is not. REDHAWK prints an error to logs if false

`pollStatusTerminated(std::string app_id)`
  : * Polls the component and waits for terminatation (in clusters this might mean CrashLoopBackoff, Completed, etc...)
    * `@param comp_id` the key that is used on validNamesMap to find the name of the component that is being checked to be terminated
    * `@return Boolean` where true means the component is terminated and false means the component is not. REDHAWK prints an error to logs if false

`deleteComponent(std::string comp_id)`
  : * Deletes a component or multiple components in a yaml file from the namespace
    * `@param comp_id` the key that is used on validNamesMap to find the name of the component that is being checked to be deleted

`isTerminated(std::string app_id)`
  : * One-off check for if a component has terminated (in clusters this might mean CrashLoopBackoff, Completed, etc...)
    * `@param comp_id` the key that is used on validNamesMap to find the name of the component that is being checked to be terminated
    * `@return true` if terminated and false if not. Throws a ComponentTerminated exception if false on start up

`openComponentConfigFile(redhawk::PropertyMap execParameters, std::string entryPoint, std::string image)`
  : Adds a component to the yaml file so that all cluster type components are in the file before launching. This is also the location where non-cluster type deployments would run executables (see DockerResolver.cpp)

    `execParameters` The parameters given to the component to be able to execute it. These parameters can instead be baked into the yaml file for the cluster technology to launch itself (eg `/path/to/executable NAMING_CONTEXT_IOR <nameing_context_ior> PROFILE_NAME <profile_name> NAME_BINDING <name_binding> COMPONENT_IDENTIFIER <component_identifier> DEBUG_LEVEL <debug_level>`).
    Other params include:
    * `NIC`
    * `RH::GPP::MODIFIED_CPU_RESERVATION_VALUE`

    `entryPoint` The path to the executable (eg `/var/redhawk/sdr/dom/components/rh/SigGen/cpp/SigGen`)

    `image` The image name that was attached to the entrypoint in the spd.xml file (eg in the spd `<\entrypoint>/path/to/executable::image<\entrypoint>`).
    This is not the fully qualified path to the image. The registry path will instead be found in `/usr/local/redhawk/core/cluster.cfg` and combined with this `image` parameter to yield the fully qualified image path.

`closeComponentConfigFile(std::string app_id)`
  : * Closes the yaml file that was being written to
    * `@param app_id` the application ID is given so that when the file is saved out it can be unique

