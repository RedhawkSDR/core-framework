
#include "DockerResolver.h"
#include "systemCallLib.h"

DockerResolver::DockerResolver (std::string app_id) : ClusterManagerResolver(app_id) {
}

void DockerResolver::openComponentConfigFile(redhawk::PropertyMap execParameters, std::string entryPoint, std::string image) {
    redhawk::PropertyMap _execParameters = execParameters;
	std::string _entryPoint = entryPoint;
	std::string _image = image;

	std::transform(_image.begin(), _image.end(), _image.begin(), ::tolower);

    std::string comp_id = _execParameters["COMPONENT_IDENTIFIER"].toString();
	std::string name = comp_id;
	name.erase(std::remove(name.begin(), name.end(), ':'), name.end());
	name.erase(std::remove(name.begin(), name.end(), '_'), name.end());
	name.erase(std::remove(name.begin(), name.end(), '.'), name.end());
        validNamesMap.insert(std::pair<std::string,std::string>(comp_id, name));

	RH_NL_DEBUG("Cluster", "Attempting to Execute " << _entryPoint);
	std::string cmd = std::string("docker run --rm -d --network host -P --name " + name + " " + _image + " '/var/redhawk/sdr/dom" + _entryPoint + " NAMING_CONTEXT_IOR " + _execParameters["NAMING_CONTEXT_IOR"].toString() +
		        " PROFILE_NAME " + _execParameters["PROFILE_NAME"].toString() + " NAME_BINDING " + _execParameters["NAME_BINDING"].toString() + " COMPONENT_IDENTIFIER " + _execParameters["COMPONENT_IDENTIFIER"].toString() + " DEBUG_LEVEL 5" + "'");

	RH_NL_INFO("Cluster", cmd.c_str());

	std::string output = GetStdoutFromCommand(cmd.c_str());
	output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
	RH_NL_INFO("Cluster", output);

	return;
}

void DockerResolver::closeComponentConfigFile(std::string identifier) {}

int DockerResolver::launchComponent(std::string comp_id){
	return 9999;
}

bool DockerResolver::pollStatusActive(std::string name) {
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

        //docker container ls --filter "name=HardLimit1testalltypes0671422368981" --format '{{json .Status}}'
        std::string cmd = std::string("docker container ls --filter \"name=" + validName + "\" --format '{{json .Status}}'");
	double timeout = 30.00;
	time_t start;
	time_t now;
	RH_NL_TRACE("Cluster", "Container " +  validName + "is registered: Checking its status");
	start = std::time(NULL);
	now = std::time(NULL);
	while (now-start <= timeout) {
	        std::string status = GetStdoutFromCommand(cmd);
	        RH_NL_TRACE("Cluster", cmd << " resulted in " << status)
	        if (status.find("Up") != std::string::npos) {
	            return true;
	        }
	        now = std::time(NULL);
	}
	return false;
}

bool DockerResolver::pollStatusTerminated (std::string name) {
        std::map<std::string,std::string>::iterator valuePair;
	std::string validName;
        if ((valuePair = validNamesMap.find(name)) != validNamesMap.end()) {
            validName = validNamesMap.find(name)->second;
        }
        else {
            RH_NL_TRACE("Cluster", "The name "+name+" has not been registered before with docker");
            return false;
        }
	RH_NL_TRACE("Cluster", "Check termination " + name + " " + validName);

	std::string cmd = std::string("docker container ls --filter \"name=" + validName + "\" --format '{{json .Status}}'");
	double timeout = 30.00;
	time_t start;
	time_t now;
	RH_NL_TRACE("Cluster", "Pod " + validName + "is registered: Checking its status");
	start = std::time(NULL);
	now = std::time(NULL);
	while (now-start <= timeout) {
	        std::string status = GetStdoutFromCommand(cmd);
	        RH_NL_TRACE("Cluster", cmd << " resulted in " << status)
	        if (status == "") {
	            return true;
	        }
	        now = std::time(NULL);
	}
	return false;
}

void DockerResolver::deleteComponent(std::string name) {
        std::map<std::string,std::string>::iterator valuePair;
	std::string validName;
        if ((valuePair = validNamesMap.find(name)) != validNamesMap.end()) {
            validName = validNamesMap.find(name)->second;
        }
        else {
            RH_NL_TRACE("Cluster", "The name "+name+" has not been registered before with docker");
            return;
        }

        RH_NL_TRACE("Cluster", "delete component " + name + " " + validName);

	std::string cmd = std::string("docker container rm -f \"" + validName + "\"");
	RH_NL_TRACE("Cluster", cmd)

	std::string output = GetStdoutFromCommand(cmd.c_str());
	output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
	RH_NL_INFO("Cluster", output);
}

bool DockerResolver::isTerminated(std::string name) {
        std::map<std::string,std::string>::iterator valuePair;
	std::string validName;
        if ((valuePair = validNamesMap.find(name)) != validNamesMap.end()) {
            validName = validNamesMap.find(name)->second;
        }
        else {
            RH_NL_TRACE("Cluster", "The name "+name+" has not been registered before with docker");
            return false;
        }
	RH_NL_TRACE("Cluster", "Check terminated " + name + " " + validName);

	std::string cmd = std::string("docker container ls --filter \"name=" + validName + "\" --format '{{json .Status}}'");
	std::string status = GetStdoutFromCommand(cmd);

	RH_NL_TRACE("Cluster", "Status with an output of \""+status + "\"")
	if (status == "") {
	    return true;
	}
	return false;
}

