
#include "EmptyResolver.h"

EmptyResolver::EmptyResolver (std::string app_id) : ClusterManagerResolver(app_id) {
}

void EmptyResolver::openComponentConfigFile(redhawk::PropertyMap execParameters, std::string entryPoint, std::string image) {
	return;
}

void EmptyResolver::closeComponentConfigFile(std::string identifier) {}

int EmptyResolver::launchComponent(std::string comp_id){
	return 9999;
}

bool EmptyResolver::pollStatusActive(std::string name) {
	return false;
}

bool EmptyResolver::pollStatusTerminated (std::string name) {
	return false;
}

void EmptyResolver::deleteComponent(std::string name) {
}

bool EmptyResolver::isTerminated(std::string name) {
	return false;
}

