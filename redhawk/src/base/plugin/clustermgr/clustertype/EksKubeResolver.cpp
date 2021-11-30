#include "EksKubeResolver.h"

#include "systemCallLib.h"
//#include <fstream>

EksKubeResolver::EksKubeResolver (std::string namespace_name) : ClusterManagerResolver(namespace_name) {
    timeout = 30.0;
    deletedAlready = false;
	char* ossiehome;
	ossiehome = getenv("OSSIEHOME");
	if (ossiehome == NULL) {
		throw "OSSIEHOME not found";
	}

    std::string cluster_file = std::string(ossiehome) + "/cluster.cfg";

	boost::property_tree::ptree pt;
	boost::property_tree::ini_parser::read_ini(cluster_file, pt);

	registry =  pt.get<std::string>("EksKube.registry");
	tag = pt.get<std::string>("EksKube.tag");
	dockerconfigjson = pt.get<std::string>("EksKube.dockerconfigjson");

	std::string user = std::getenv("USER");
	std::string docker_login = "aws --profile="+user+" ecr get-login-password --region us-gov-west-1 | docker login --username AWS --password-stdin " + registry;

	/* Start k8s YAML */
	// Configure registry secret
        std::string name = "waveform-";
	int full_name = std::string(name).length() + namespace_name.length();
	if (full_name > 63) {
	    RH_NL_WARN("Cluster", "The name was longer than 63 characters now truncating")
	    int skip = (full_name - 63);
	    namespace_name = namespace_name.substr(skip);
	}
        wave_namespace = name +namespace_name;
	std::transform(wave_namespace.begin(), wave_namespace.end(), wave_namespace.begin(), ::tolower);
	wave_namespace.erase(std::remove(wave_namespace.begin(), wave_namespace.end(), ':'), wave_namespace.end());
	wave_namespace.erase(std::remove(wave_namespace.begin(), wave_namespace.end(), '_'), wave_namespace.end());
	wave_namespace.erase(std::remove(wave_namespace.begin(), wave_namespace.end(), '.'), wave_namespace.end());

        configureNamespace();
}

void EksKubeResolver::configureRegistrySecret() {
	yaml << YAML::BeginMap;

	yaml << YAML::Key << "apiVersion" << YAML::Value << "v1";

	yaml << YAML::Key << "data";
	yaml << YAML::Value << YAML::BeginMap;
	yaml << YAML::Key << ".dockerconfigjson";
	yaml << YAML::Value << dockerconfigjson;
	yaml << YAML::EndMap;

	yaml << YAML::Key << "kind" << YAML::Value << "Secret";

	yaml << YAML::Key << "metadata";
	yaml << YAML::Value << YAML::BeginMap;
	yaml << YAML::Key << "name" << YAML::Value << "regcred";
	yaml << YAML::Key << "namespace" << YAML::Value << wave_namespace;
	yaml << YAML::EndMap;

	yaml << YAML::Key << "type" << YAML::Value << "kubernetes.io/dockerconfigjson";

	yaml << YAML::EndMap;

}

EksKubeResolver::~EksKubeResolver() {
}

void EksKubeResolver::configureNamespace() {
	yaml << YAML::BeginMap;

	yaml << YAML::Key << "apiVersion" << YAML::Value << "v1";

	yaml << YAML::Key << "kind" << YAML::Value << "Namespace";

	yaml << YAML::Key << "metadata";
	yaml << YAML::Value << YAML::BeginMap;
	yaml << YAML::Key << "name" << YAML::Value << wave_namespace;
	yaml << YAML::Key << "labels" << YAML::BeginMap;
	yaml << YAML::Key << "name" << YAML::Value << wave_namespace;
	yaml << YAML::EndMap;
	yaml << YAML::EndMap;

	yaml << YAML::EndMap;
}

void EksKubeResolver::openComponentConfigFile(redhawk::PropertyMap execParameters, std::string entryPoint, std::string image) {

	RH_NL_TRACE("Cluster", "Adding to Yaml file")
	std::string naming_context_ior = execParameters["NAMING_CONTEXT_IOR"].toString();
	std::string name_binding = execParameters["NAME_BINDING"].toString();
	std::string comp_id = execParameters["COMPONENT_IDENTIFIER"].toString();
	std::string profile_name = execParameters["PROFILE_NAME"].toString();

	// Begin yaml
	yaml << YAML::BeginMap;

	yaml << YAML::Key << "apiVersion" << YAML::Value << "v1";
	yaml << YAML::Key << "kind" << YAML::Value << "Pod";

	yaml << YAML::Key << "metadata" << YAML::Value;
	yaml << YAML::BeginMap;
	std::string name = comp_id + "-container";
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	name.erase(std::remove(name.begin(), name.end(), ':'), name.end());
	name.erase(std::remove(name.begin(), name.end(), '_'), name.end());
	name.erase(std::remove(name.begin(), name.end(), '.'), name.end());
	yaml << YAML::Key << "name" << YAML::Value << name;
	yaml << YAML::Key << "namespace" << YAML::Value << wave_namespace;
        validNamesMap.insert(std::pair<std::string, std::string>(comp_id, name));
	yaml << YAML::EndMap;

	yaml << YAML::Key << "spec" << YAML::Value;
	yaml << YAML::BeginMap;
	yaml << YAML::Key << "containers" << YAML::Value;
	yaml << YAML::BeginSeq << YAML::BeginMap;

	std::string full_image = registry + "/" + image + ":" + tag;
	yaml << YAML::Key << "image" << YAML::Value << full_image;
	std::string image_name = name_binding + "-pod";
	std::transform(image_name.begin(), image_name.end(), image_name.begin(), ::tolower);
	image_name.erase(std::remove(image_name.begin(), image_name.end(), ':'), image_name.end());
	image_name.erase(std::remove(image_name.begin(), image_name.end(), '_'), image_name.end());
	image_name.erase(std::remove(image_name.begin(), image_name.end(), '.'), image_name.end());
	yaml << YAML::Key << "name" << YAML::Value << image_name;

	if (entryPoint.find("python") != std::string::npos) {
		RH_NL_TRACE("Cluster", "Python type component detected")
		yaml << YAML::Key << "args" << YAML::Value;
		yaml << YAML::BeginSeq;
		std::stringstream ss;
		std::string full_entry = "/var/redhawk/sdr/dom" + entryPoint + " ";
		ss << full_entry;
		ss << " NAMING_CONTEXT_IOR ";
		ss << naming_context_ior;
		ss << " PROFILE_NAME ";
		ss << profile_name;
		ss << " NAME_BINDING ";
		ss << name_binding;
		ss << " COMPONENT_IDENTIFIER ";
		ss << comp_id;
		yaml << ss.str();
		yaml << YAML::EndSeq;
	}
	else {
		yaml << YAML::Key << "command" << YAML::Value;
		yaml << YAML::BeginSeq;
		std::string full_entry = "/var/redhawk/sdr/dom" + entryPoint;
		yaml << full_entry;
		yaml << "NAMING_CONTEXT_IOR";
		yaml << naming_context_ior;
		yaml << "PROFILE_NAME";
		yaml << profile_name;
		yaml << "NAME_BINDING";
		yaml << name_binding;
		yaml << "COMPONENT_IDENTIFIER";
		yaml << comp_id;
		yaml << YAML::EndSeq;
	}
	yaml << YAML::EndMap << YAML::EndSeq;

	yaml << YAML::Key << "imagePullSecrets" << YAML::Value;
	yaml << YAML::BeginSeq << YAML::BeginMap;
	yaml << YAML::Key << "name" << YAML::Value << "regcred";
	yaml << YAML::EndMap << YAML::EndSeq;
	yaml << YAML::EndMap;

	// End yaml
	yaml << YAML::EndMap;
}

void EksKubeResolver::closeComponentConfigFile(std::string identifier) {
	RH_NL_TRACE("Cluster", "Closing file");
	yaml << YAML::EndMap;
        
	open_tmp_file();

	RH_NL_TRACE("Cluster", filename);
	std::ofstream k8sFile;
	k8sFile.open(filename.c_str());
	k8sFile << yaml.c_str() << std::endl;
	k8sFile.close();
}

int EksKubeResolver::launchComponent(std::string identifier) {
	std::string cmd = std::string("kubectl apply -f " + filename);
	RH_NL_TRACE("Cluster", cmd);

	std::string output = GetStdoutFromCommand(cmd.c_str());
	output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
	RH_NL_INFO("Cluster", output);

	// Unable to use return code of system call to track pod status; successful kubectl invocation not indicative of pod status
	// must call kubectl get pod SOMEHOW. Best option is to have it be triggered by successful CORBA registration. Upon registration,
	// make the call and record and set pod status.
	return 9999;
}

bool EksKubeResolver::pollStatusActive(std::string name) {
        std::string validName = validNamesMap.find(name)->second;
	RH_NL_TRACE("Cluster", "Check active " + validName);

        bool all_active = true;
        RH_NL_TRACE("Cluster", "Check activation");
        //std::string cmd = std::string("kubectl get pod  " + name + " -o=jsonpath={.status.phase}");
        std::string cmd = std::string("kubectl get pod  " + validName + " -n " + wave_namespace + " | grep -v NAME | awk '{ print $3 }'");
        std::string expected_statuses [] = {"Running"};
        std::string status = "";
        if(!pollComponent(validName, expected_statuses, 1, cmd, timeout, status)) {
            all_active = false;
        }
	return all_active;
}

void EksKubeResolver::deleteComponent(std::string name) {
        if (deletedAlready) {
            RH_NL_TRACE("Cluster", "This component " + name + " is already being deleted");
            return;
        }
	// if component is a ClusterType, call kubectl delete pod
	RH_NL_TRACE("Cluster", "releaseObject is attempting to delete pod " + filename);
	std::string cmd = std::string("kubectl delete --ignore-not-found -f " + filename + " &");
	RH_NL_TRACE("Cluster", cmd.c_str());

	std::string output = GetStdoutFromCommand(cmd.c_str(), false);
	output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
	RH_NL_INFO("Cluster", output);
        deletedAlready = true;

	close_tmp_file();
}

bool EksKubeResolver::pollStatusTerminated (std::string name) {
        std::string validName = validNamesMap.find(name)->second;
	RH_NL_TRACE("Cluster", "Check termination " + validName);

        bool all_terminated = true;
	//std::string cmd = std::string("kubectl get pod  " + name + " -o=jsonpath={.status.phase}");
	std::string cmd = std::string("kubectl get pod --ignore-not-found " + validName + " -n " + wave_namespace + " | grep -v NAME | awk '{ print $3 }'");
	std::string expected_statuses [] = {"Terminating", "Completed", "CrashLoopBackOff", "not found", ""};
        std::string status = "";
	if(pollComponent(validName, expected_statuses, 5, cmd, timeout, status)) {
            all_terminated = false;
        }
	return all_terminated;
}

bool EksKubeResolver::isTerminated(std::string name) {
	// Check that the Status is something that indicates it is at least trying to come up
        std::string validName = validNamesMap.find(name)->second;
	RH_NL_TRACE("Cluster", "Status with an output of "+ validName)
        std::string cmd = std::string("kubectl get pod --ignore-not-found " + validName + " -n " + wave_namespace + " | grep -v NAME | awk '{ print $3 }'");

	std::string processStatus = GetStdoutFromCommand(cmd.c_str());
	processStatus.erase(std::remove(processStatus.begin(), processStatus.end(), '\n'), processStatus.end());
	RH_NL_INFO("Cluster", processStatus);
	if (processStatus == "Terminating" || processStatus == "Completed" || processStatus == "CrashLoopBackOff" || processStatus.find("not found") != std::string::npos || processStatus.empty()) {
	    RH_NL_TRACE("Cluster", "Termination reporting 'true'");
	    return true;
	}
	else {
	    RH_NL_TRACE("Cluster", "Termination reporting 'false'");
	    return false;
	}

}
