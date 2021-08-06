#include "DockerSwarmResolver.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "systemCallLib.h"

DockerSwarmResolver::DockerSwarmResolver (std::string app_id) : ClusterManagerResolver(app_id) {
	char* ossiehome;
	ossiehome = getenv("OSSIEHOME");
	if (ossiehome == NULL) {
		throw "OSSIEHOME not found";
	}
	std::string cluster_file = "/usr/local/redhawk/core/cluster.cfg";
	boost::property_tree::ptree pt;
	boost::property_tree::ini_parser::read_ini(cluster_file, pt);

	std::string name =  pt.get<std::string>("CLUSTER.name");
	registry =  pt.get<std::string>(name+".registry");
	tag = pt.get<std::string>(name+".tag");
	key = pt.get<std::string>(name+".key");
	user = pt.get<std::string>(name+".user");
	ip = pt.get<std::string>(name+".ip");
    docker_login_cmd = pt.get<std::string>(name+".docker_login_cmd");

	_longest_name = 0;
	timeout = 120.0;
	ssh_cmd = "ssh -i " + key + " "+user+"@"+ip;
	scp_cmd = "scp -i " + key;
	docker_login = ssh_cmd + " " + docker_login_cmd;

    // Run docker_login_cmd on the Manager Node over SSH
	std::string output = GetStdoutFromCommand(docker_login.c_str());
	output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
	RH_NL_INFO("Cluster", output);

	// Begin yaml
	yaml << YAML::BeginMap;

	// Version information
	yaml << YAML::Key << "version";
	yaml << YAML::SingleQuoted << "3";


	yaml << YAML::Key << "networks";
	yaml << YAML::Value << YAML::BeginMap;
	yaml << YAML::Key << "outside";
	yaml << YAML::Value << YAML::BeginMap;
	yaml << YAML::Key << "external";
	yaml << YAML::Value << YAML::BeginMap;
	yaml << YAML::Key << "name";
	yaml << YAML::Value << "host";
	yaml << YAML::EndMap;
	yaml << YAML::EndMap;
	yaml << YAML::EndMap;

	// Services
	yaml << YAML::Key << "services";
	yaml << YAML::Value;

	yaml << YAML::BeginMap;
}


void DockerSwarmResolver::openComponentConfigFile(redhawk::PropertyMap execParameters, std::string entryPoint, std::string image) {

	RH_NL_TRACE("Cluster", "Adding to Yaml file")
	std::string naming_context_ior = execParameters["NAMING_CONTEXT_IOR"].toString();
	std::string name_binding = execParameters["NAME_BINDING"].toString();
	std::string comp_id = execParameters["COMPONENT_IDENTIFIER"].toString();
	std::string profile_name = execParameters["PROFILE_NAME"].toString();

	std::string usage = name_binding;
	std::transform(usage.begin(), usage.end(), usage.begin(), ::tolower);
	usage.erase(std::remove(usage.begin(), usage.end(), ':'), usage.end());
	usage.erase(std::remove(usage.begin(), usage.end(), '_'), usage.end());
	usage.erase(std::remove(usage.begin(), usage.end(), '.'), usage.end());

	// Update the _stack each time if it is too short
	// This way it will be a good length when finally applied
	if (usage.length() > _longest_name) {
	    _longest_name = usage.length();
	}

	yaml << YAML::Key << usage << YAML::Value;

        validNamesMap.insert(std::pair<std::string, std::string>(comp_id, usage));
	//validNames.push_back(usage);

	// Begin Service map
	yaml << YAML::BeginMap;

	// Add image info
	yaml << YAML::Key << "deploy";
	yaml << YAML::Value << YAML::BeginMap;
	yaml << YAML::Key << "placement";
	yaml << YAML::Value << YAML::BeginMap;
	yaml << YAML::Key << "constraints";
	yaml << YAML::BeginSeq;
	yaml << YAML::Value << "node.role == worker";
	yaml << YAML::EndSeq;
	yaml << YAML::EndMap;
	yaml << YAML::EndMap;

	if (entryPoint.find("python") != std::string::npos) {
	    yaml << YAML::Key << "command";
	    yaml << YAML::Value;
	    std::stringstream ss;
	    yaml << YAML::BeginSeq;
	    ss << "/var/redhawk/sdr/dom" + entryPoint;
	    ss << " NAMING_CONTEXT_IOR ";
	    ss << naming_context_ior;
	    ss << " PROFILE_NAME ";
	    ss << profile_name;
	    ss << " NAME_BINDING ";
	    ss << name_binding;
	    ss << " COMPONENT_IDENTIFIER " ;
	    ss << comp_id;
	    yaml << ss.str();
	    yaml << YAML::EndSeq;
	}
	else {
	    yaml << YAML::Key << "entrypoint";
	    yaml << YAML::Value;
	    yaml << YAML::BeginSeq;
	    yaml << "/var/redhawk/sdr/dom" + entryPoint;
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

	yaml << YAML::Key << "image";
	std::string full_image = registry + "/" + image;
	yaml << YAML::Value << full_image;

	yaml << YAML::Key << "networks";
	yaml << YAML::Value;
	yaml << YAML::BeginSeq;
	yaml << "outside";
	yaml << YAML::EndSeq;

	yaml << YAML::EndMap;
}

void DockerSwarmResolver::closeComponentConfigFile(std::string identifier) {
	open_tmp_file();

	RH_NL_TRACE("Cluster", filename);
	yaml << YAML::EndMap;

	// End
	yaml << YAML::EndMap;

	std::ofstream swarmFile;
	swarmFile.open(filename.c_str());
	swarmFile << yaml.c_str() << std::endl;
	swarmFile.close();
}

int DockerSwarmResolver::launchComponent(std::string comp_id) {


	_stack = comp_id;
	_stack.erase(std::remove(_stack.begin(), _stack.end(), ':'), _stack.end());
	_stack.erase(std::remove(_stack.begin(), _stack.end(), '_'), _stack.end());
	_stack.erase(std::remove(_stack.begin(), _stack.end(), '.'), _stack.end());

	int full_stack_name = _longest_name + 1 + _stack.length();
	if (full_stack_name > 63) {
	    RH_NL_WARN("Cluster", "The name was longer than 63 characters now truncating")
	    int skip = (full_stack_name - 63);
	    _stack = _stack.substr(skip);
	}
	std::string cmd = "";
	if (/*entryPoint.find("python") != std::string::npos*/ false) { //If a python component
	    RH_NL_DEBUG("Cluster", "Python type component detected");
	    /*cmd = std::string(ssh_cmd + " \"docker service create -q --network host --name " + stack + " --with-registry-auth " + image_full + " '/var/redhawk/sdr/dom" + entryPoint + " NAMING_CONTEXT_IOR " + execParameters["NAMING_CONTEXT_IOR"].toString() +
		    " PROFILE_NAME " + execParameters["PROFILE_NAME"].toString() + " NAME_BINDING " + execParameters["NAME_BINDING"].toString() + " COMPONENT_IDENTIFIER " + execParameters["COMPONENT_IDENTIFIER"].toString() + " DEBUG_LEVEL 5'\"");*/
	}
	else { // if a C++ component
	    cmd = std::string(ssh_cmd + " \"docker stack deploy --compose-file "+filename+" "+ _stack +"  --with-registry-auth\"");
	}

	std::string copy_file = scp_cmd + " " + filename + " " + user + "@" + ip + ":/tmp/";
	std::string copy_output = GetStdoutFromCommand(copy_file.c_str());
	copy_output.erase(std::remove(copy_output.begin(), copy_output.end(), '\n'), copy_output.end());
	RH_NL_INFO("Cluster", copy_output);

	RH_NL_TRACE("Cluster", cmd)

	std::string output = GetStdoutFromCommand(cmd.c_str());
	output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
	RH_NL_INFO("Cluster", output);

	// Unable to use return code of system call to track stack status
	return 9999;
}

bool DockerSwarmResolver::pollStatusActive(std::string name) {
        std::map<std::string,std::string>::iterator valuePair;
	std::string validName;
        if ((valuePair = validNamesMap.find(name)) != validNamesMap.end()) {
            validName = validNamesMap.find(name)->second;
        }
        else {
            RH_NL_TRACE("Cluster", "The name "+name+" has not been registered before with docker");
            return false;
        }
	RH_NL_TRACE("Cluster", "Check active " + validName);

	//arguments =  SSH_CMD + ' "" $SSH_CMD "docker stack ps -f \"name=${STACK}_$COMPNAME\" $STACK --format '{{.CurrentState}}'"
	std::string cmd = ssh_cmd + std::string(" \"docker stack ps -f \"name=" + _stack + "_" + validName + "\" "+ _stack + " --format \'{{.CurrentState}}\'\"");
	std::string expected_statuses [] = {"Running"};
        std::string status = "";
	return pollComponent(_stack, expected_statuses, 1, cmd, timeout, status);
}

bool DockerSwarmResolver::pollStatusTerminated (std::string name) {
        std::map<std::string,std::string>::iterator valuePair;
	std::string validName;
        if ((valuePair = validNamesMap.find(name)) != validNamesMap.end()) {
            validName = validNamesMap.find(name)->second;
        }
        else {
            RH_NL_TRACE("Cluster", "The name "+name+" has not been registered before with docker");
            return false;
        }
	RH_NL_TRACE("Cluster", "Check termination " + validName);

	std::string cmd = ssh_cmd + std::string(" \"docker stack ps -f \"name=" + _stack + "_" + validName + "\" "+ _stack + " --format \'{{.CurrentState}}\'\"");
	std::string expected_statuses [] = {"Complete", "Failed", "Rejected", "nothing found", "Shutdown"};
        std::string status = "";
	return pollComponent(_stack, expected_statuses, 5, cmd, timeout, status);
}

void DockerSwarmResolver::deleteComponent(std::string name) {
        std::map<std::string,std::string>::iterator valuePair;
	std::string validName;
        if ((valuePair = validNamesMap.find(name)) != validNamesMap.end()) {
            validName = validNamesMap.find(name)->second;
        }
        else {
            RH_NL_TRACE("Cluster", "The name "+name+" has not been registered before with docker");
            return;
        }
	RH_NL_TRACE("Cluster", "Delete " + validName);

	// if component is a ClusterType, call kubectl delete pod
	RH_NL_TRACE("Cluster", "releaseObject is attempting to delete stack " + _stack);
	std::string cmd = ssh_cmd + std::string(" \"docker stack rm " + _stack +"\" &");
	RH_NL_TRACE("Cluster", cmd.c_str());

	std::string output = GetStdoutFromCommand(cmd.c_str());
	output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
	RH_NL_INFO("Cluster", output);

	close_tmp_file();
}

bool DockerSwarmResolver::isTerminated(std::string name) {
        std::map<std::string,std::string>::iterator valuePair;
	std::string validName;
        if ((valuePair = validNamesMap.find(name)) != validNamesMap.end()) {
            validName = validNamesMap.find(name)->second;
        }
        else {
            RH_NL_TRACE("Cluster", "The name "+name+" has not been registered before with docker");
            return false;
        }
	RH_NL_TRACE("Cluster", "Check is terminated " + validName);

	std::string cmd = ssh_cmd + std::string(" \"docker stack ps -f \"name=" + _stack + "_" + validName + "\" "+ _stack + " --format \'{{.CurrentState}}\'\"");
	std::string status = GetStdoutFromCommand(cmd);
	status.erase(std::remove(status.begin(), status.end(), '\n'), status.end());
	// Check that the Status is something that indicates it is at least trying to come up
	RH_NL_TRACE("Cluster", "Executing "+cmd+" with an output of "+status)
	if (status.find("Complete") != std::string::npos || status.find("Rejected") != std::string::npos || status.find("Fail") != std::string::npos || status.find("no such service") != std::string::npos || status.find("nothing found") != std::string::npos) {
	    RH_NL_DEBUG("Cluster", "Termination reporting 'true'")
	    return true;
	}
	else {
	    RH_NL_DEBUG("Cluster", "Termination reporting 'false'")
	    return false;
	}

}
