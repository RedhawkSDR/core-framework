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
#ifndef SYSTEM_CALL_LIB
#define SYSTEM_CALL_LIB

/*
 *
 */
std::string GetStdoutFromCommand(std::string cmd, bool daemon = false) {

	std::string data;
	FILE * stream;
	const int max_buffer = 256;
	char buffer[max_buffer];
	if (daemon) {
		cmd.append(" 2>&1 &");
	}
	else {
		cmd.append(" 2>&1");
	}

	if (daemon) {
		stream = popen(cmd.c_str(), "r");
	}
	else {	
		stream = popen(cmd.c_str(), "r");
		RH_NL_TRACE("systemCallLib", "Getting cmd " << cmd)
		if (stream) {
		    while (!feof(stream))
		    if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
		    pclose(stream);
		}
	}
	return data;
}

/*
 *
 */
bool pollComponent(std::string name, std::string *expected_status, int len, std::string cmd, double timeout, std::string& status) {
	time_t start;
	time_t now;
	RH_NL_TRACE("systemCallLib", "Pod " + name + "is registered: Checking its status");
	start = std::time(NULL);
	now = std::time(NULL);
	while (now-start <= timeout) {
		status = GetStdoutFromCommand(cmd);
		status.erase(std::remove(status.begin(), status.end(), '\n'), status.end());
		RH_NL_TRACE("systemCallLib", "Pod " + name + " status: " + status);
		for (int i = 0; i < len; i++) {
		    RH_NL_TRACE("systemCallLib", "Checking expected status " + expected_status[i] + " for " + name);
		    if ((!expected_status[i].empty() && status.find(expected_status[i]) != std::string::npos) || (expected_status[i].empty() && status.empty())) {
		        return true;
		    }
		}
		now = std::time(NULL);
	}
	return false;
}

#endif
