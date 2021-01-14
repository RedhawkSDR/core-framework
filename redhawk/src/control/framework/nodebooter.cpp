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

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <exception>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <map>
#include <vector>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/cerrno.hpp>

namespace fs = boost::filesystem;

#include <ossie/SoftPkg.h>
#include <ossie/Properties.h>
#include <ossie/DomainManagerConfiguration.h>
#include <ossie/DeviceManagerConfiguration.h>
#include <ossie/prop_utils.h>
#include <ossie/ossieSupport.h>
#include <ossie/debug.h>
#include <ossie/logging/loghelpers.h>

using namespace std;


#define ENABLE_PERSISTENCE (ENABLE_BDB_PERSISTENCE || ENABLE_GDBM_PERSISTENCE || ENABLE_SQLITE_PERSISTENCE)

CREATE_LOGGER(nodebooter);

// Track the DomainManager and DeviceManager pids, if using fork-and-exec.
static pid_t domPid = 0;
static pid_t devPid = 0;

typedef std::map<std::string, std::string> ExecParams;

/*
 * Iterate through all of the implementations and find the first implementation
 * that matches the operating system and processor requirements specified in
 * deviceProps.
 *
 * Return 0 if no match is found.
 */
const ossie::SPD::Implementation* matchImplementation (
    const std::vector<ossie::SPD::Implementation>& implementations,
    const std::vector<const ossie::Property*>&     deviceProps)
{
    std::vector<ossie::SPD::Implementation>::const_iterator implIter;
    for ( implIter = implementations.begin();
          implIter != implementations.end();
          ++implIter) { // iterate through each implementation

        if (ossie::checkOs(implIter->getOsDeps(), deviceProps) &&
            ossie::checkProcessor(implIter->getProcessors(), deviceProps)) {
            // If the operating system and processor of the implementation
            // match the requirements specified in deviceProps.
            return &(*implIter);
        }

    }

    return 0;
}

/*
 * If filePath is absolute (i.e., starts with "/"), return:
 *
 *      sdrroot/filePath
 *
 * else (i.e., relative path), return:
 *
 *      currentPath/filePath
 */
std::string getLocalPath (
    const std::string& filePath,
    const fs::path&    sdrRoot,
    const fs::path&    currentPath)
{
    fs::path localPath;
    if (filePath.find('/') == 0) {
        // Absolute path (within SDR directory).
        localPath = sdrRoot;
    } else {
        // Relative path (to current directory).
        localPath = currentPath;
    }
    localPath /= filePath;
    return localPath.string();
}

/*
 * Iterate through the properties tagged as execparams; add simple, execparam
 * properties with default values to execParams.
 */
void loadPRFExecParams (const std::string& prfFile, ExecParams& execParams)
{
    if (!fs::exists(prfFile)) {
        LOG_WARN(nodebooter, "PRF file does not exist: " << prfFile);
        return;
    }

    std::ifstream prfStream(prfFile.c_str());
    if (!prfStream) {
        LOG_WARN(nodebooter, "Unable to read PRF file: " << prfFile);
        return;
    }

    ossie::Properties prf;
    try {
        prf.load(prfStream);
    } catch (const ossie::parser_error& ex) {
        std::string parser_error_line = ossie::retrieveParserErrorLineNumber(ex.what());
        LOG_ERROR(nodebooter, "Failed to parse PRF file " << prfFile << ". " << parser_error_line << "The XML parser returned the following error: " << ex.what());
        exit(EXIT_FAILURE);
    }
    prfStream.close();
    LOG_TRACE(nodebooter, "Loaded PRF file: " << prfFile);

    const std::vector<const ossie::Property*>& execProps = prf.getExecParamProperties();
    std::vector<const ossie::Property*>::const_iterator prop;

    for ( prop = execProps.begin(); prop != execProps.end(); ++prop) {
        const ossie::SimpleProperty* simpleProp;
        simpleProp = dynamic_cast<const ossie::SimpleProperty*>(*prop);
        if (!simpleProp) {
            LOG_WARN(nodebooter, "Only exec params of type \"simple\" supported");
            continue;
        } else if (!simpleProp->getValue()) {
            continue;
        }
        execParams[simpleProp->getID()] =  simpleProp->getValue();
    }

    const std::vector<const ossie::Property*>& propertyProps = prf.getConstructProperties();

    for ( prop = propertyProps.begin(); prop != propertyProps.end(); ++prop) {
        const ossie::SimpleProperty* simpleProp;
        simpleProp = dynamic_cast<const ossie::SimpleProperty*>(*prop);
        if (!simpleProp) {
            // property properties that are not simples cannot be commandline, so no warning is needed
            continue;
        } else if (!simpleProp->getValue()) {
            continue;
        }
        if (not simpleProp->isCommandLine())
            continue;
        execParams[simpleProp->getID()] =  simpleProp->getValue();
    }
}


static pid_t launchSPD (
    const std::string&                         spdFile,
    const fs::path&                            sdrRoot,
    const ExecParams&                          overrideExecParams,
    const ExecParams&                          optionParams,
    const std::vector<const ossie::Property*>& deviceProps,
    bool                                       doFork)
{
    fs::path spdPath = sdrRoot / spdFile;
    if (!fs::exists(spdPath)) {
        throw runtime_error("SPD file " + spdFile + " does not exist");
    }

    std::ifstream spdStream(spdPath.string().c_str());
    if (!spdStream) {
        throw std::runtime_error("Could not read SPD file " + spdFile);
    }

    ossie::SoftPkg spd;
    try {
        spd.load(spdStream, spdFile);
    } catch (const ossie::parser_error& ex) {
        std::string parser_error_line = ossie::retrieveParserErrorLineNumber(ex.what());
        LOG_ERROR(nodebooter, "Failed to parse SPD file " << spdFile << ". " << parser_error_line << "The XML parser returned the following error: " << ex.what());
        exit(EXIT_FAILURE);
    }
    spdStream.close();

    LOG_DEBUG(nodebooter, "Loaded SPD file " << spdFile);
    LOG_DEBUG(nodebooter, "SPD Id:   " << spd.getSoftPkgID());
    LOG_DEBUG(nodebooter, "SPD Name: " << spd.getName());

    // Find an implementation that we can run.
    const ossie::SPD::Implementation* impl = matchImplementation(spd.getImplementations(), deviceProps);
    if (!impl) {
        throw runtime_error("No matching implementation found");
    }

    LOG_TRACE(nodebooter, "Implementation: " << impl->getID());
    if (impl->getCodeType()) {
        LOG_TRACE(nodebooter, "Code type:   " << impl->getCodeType());
    }
    LOG_TRACE(nodebooter, "Code file:   " << impl->getCodeFile());
    LOG_TRACE(nodebooter, "Entry point: " << impl->getEntryPoint());

    ExecParams execParams;

    // If the SoftPkg refers to a PRF file, parse it to get the default execparams.
    if (spd.getPRFFile()) {
        std::string prfFile = getLocalPath(spd.getPRFFile(), sdrRoot, spdPath.parent_path());
        LOG_TRACE(nodebooter, "Loading SoftPkg PRF: " << prfFile);
        loadPRFExecParams(prfFile, execParams);
    }

    // Update the execparams with implementation-specific values.
    if (impl->getPRFFile()) {
        std::string prfFile = getLocalPath(impl->getPRFFile(), sdrRoot, spdPath.parent_path());
        LOG_TRACE(nodebooter, "Loading implementation-specific PRF: " << prfFile);
        loadPRFExecParams(prfFile, execParams);
    }
    
    execParams["SPD"] = spdFile;

    // Update the execparams with the user-supplied overrides.
    for (ExecParams::const_iterator param = overrideExecParams.begin(); param != overrideExecParams.end(); ++param) {
        execParams[param->first] = param->second;
    }

    // Locate the executable file and verify its existence.
    std::string exePath = getLocalPath(impl->getEntryPoint(), sdrRoot, spdPath.parent_path());
    if (!fs::exists(exePath)) {
        throw runtime_error("Could not find executable file" + exePath);
    }
    LOG_TRACE(nodebooter, "Running executable file " << exePath);

    // Create a C string array of the arguments to the executable from the code file name (0th argument)
    // and execparams. Note the importance of the final NULL, which terminates the array.
    std::vector<const char *> argv;
    argv.push_back(impl->getCodeFile().c_str());
    for (ExecParams::const_iterator param = execParams.begin(); param != execParams.end(); ++param) {
        LOG_TRACE(nodebooter, "EXEC_PARAM: " << param->first << "=\"" << param->second << "\"");
        argv.push_back(param->first.c_str());
        argv.push_back(param->second.c_str());
    }

    // push options as command line args
    for (ExecParams::const_iterator param = optionParams.begin(); param != optionParams.end(); ++param) {
        LOG_TRACE(nodebooter, "EXEC_PARAM (option): " << param->first );
        argv.push_back(param->first.c_str());
    }

    argv.push_back(NULL);


    if (doFork) {
        pid_t pid = fork();
        if (pid < 0) {
            std::ostringstream err;
            err << "fork(): " << strerror(errno);
            throw runtime_error(err.str());
        } else if (pid > 0) {
            return pid;
        }
    }

    // Execute in place; by definition, if execution continues past this point,
    // an error must have occurred, so checking the return status is redundant.
    execvp(exePath.c_str(), const_cast<char* const*>(&argv[0]));
    std::ostringstream err;
    err << "Could not execute " << exePath << ": " << strerror(errno);
    LOG_ERROR(nodebooter, err.str());
    throw runtime_error(err.str());
}

static void setOwners(const std::string& user, const std::string& group)
{
    bool group_set = false;

    if (user.empty() && group.empty()) {
        return;
    }

    // Checks for command line specified group
    if (!group.empty()){
        std::vector<std::string> groups;
        if (group.find(',') == std::string::npos) {
            groups.push_back(group);
        } else {
            size_t current_pos = 0;
            while (group.find(',', current_pos) != std::string::npos) {
                std::string _group = group.substr(current_pos, (group.find(',', current_pos))-current_pos);
                groups.push_back(_group);
                current_pos = group.find(',', current_pos) + 1;
            }
            groups.push_back(group.substr(current_pos, group.find(',', current_pos)-current_pos));
        }
        gid_t gids[groups.size()];
        for (unsigned int i=0; i<groups.size(); i++) {
            const struct group *gr = getgrnam(groups[i].c_str());
            if (gr == NULL){
                std::cout<<"... error: "<<errno<<" "<<strerror(errno)<<std::endl;
                throw std::runtime_error("Invalid group '" + group + "' specified");
            }
            gids[i] = gr->gr_gid;
        }
        if (setgroups(groups.size(), gids)){
            std::ostringstream err;
            err << "Cannot set group ID to one of [ ";
            for (unsigned int gr_idx=0; gr_idx < groups.size(); gr_idx++) {
                err << gids[gr_idx] << " ";
            }
            err << "]: " << strerror(errno);
            throw std::runtime_error(err.str());
        }
        setgid(gids[0]);

        std::cout << "Running as group: [ ";
        for (unsigned int gr_idx=0; gr_idx < groups.size(); gr_idx++) {
            std::cout << groups[gr_idx] <<  "(gid=" << gids[gr_idx] << ") ";
        }
        std::cout << "]" << std::endl;
        group_set = true;
    }

    // Checks for command line specified user
    if (!user.empty()){
        const struct passwd *pwent = getpwnam(user.c_str());

        if (pwent == NULL) {
            throw std::runtime_error("Invalid user '" + user + "' specified");
        }

        // Use the users primary group if another group is specified in command line
        uid_t uid = pwent->pw_uid;

        if (getuid() != uid) {
            // Only set the group if another group hasn't already been specified
            if (not group_set) { // when root sets the user id, group id's are not set
                int ngroups = 1024;
                gid_t groups[ngroups];
                struct passwd *pw;
                pw = getpwnam(user.c_str());
                if (getgrouplist(user.c_str(), pw->pw_gid, &groups[0], &ngroups) == -1) {
                    std::ostringstream err;
                    err << "Cannot retrieve group list for user ID " << uid << ": " << strerror(errno);
                    throw std::runtime_error(err.str());
                } else {
                    setgroups(ngroups, groups);
                }
                setgid(groups[0]);
                std::cout << "Running as group: [ ";
                for (unsigned int gr_idx=0; gr_idx < ngroups; gr_idx++) {
                    std::cout << getgrgid(groups[gr_idx])->gr_name <<  "(gid=" << groups[gr_idx] << ") ";
                }
                std::cout << "]" << std::endl;
            }

            if (setuid(uid)) {
                std::ostringstream err;
                err << "Cannot set user ID to " << uid << ":" << strerror(errno);
                throw std::runtime_error(err.str());
            }

            std::cout << "Running as user: " << user << "(uid=" << uid << ")" << std::endl;
        }
    }
}

static void initializeDaemon (
    const std::string& user,
    const std::string& group,
    const std::string& pidfile)
{
    LOG_INFO(nodebooter, "Running as daemon");

    pid_t pid = fork();
    if (pid < 0) {
        LOG_FATAL(nodebooter, "fork: " << strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Wait for the initial child to exit, which should occur after it forks
        int status;
        waitpid(pid, &status, 0);
        exit(WEXITSTATUS(status));
    }

    // Become a session leader (dissociates from controlling terminal, creates
    // new process group).
    setsid();

    // Double-fork, to make sure that the daemon cannot re-acquire a controlling
    // terminal, and also to prevent zombie-ism.
    pid = fork();
    if (pid < 0) {
        LOG_FATAL(nodebooter, "fork: " << strerror(errno));
        exit(EXIT_FAILURE);
    }  else if (pid > 0) {
        if (pidfile.empty()) {
            LOG_INFO(nodebooter, "PID: " << pid);
        } else {
            std::ofstream pidfileStream(pidfile.c_str(), ios::out | ios::trunc);
            if (!pidfileStream) {
                LOG_ERROR(nodebooter, "Unable to write PID file '" << pidfile << "'");
            } else {
                pidfileStream << pid << std::endl;
                if (!pidfileStream) {
                    LOG_ERROR(nodebooter, "Unable to write PID file '" << pidfile << "'");
                }
            }
        }
        exit(EXIT_SUCCESS);
    }

    // Following the second fork, set the user. This allows the first forked
    // child to write the pid file with its initial user privileges. By doing
    // this before closing stdout/stderr, we can still display an error message
    // if it fails.
    try {
        setOwners(user, group);
    } catch (const std::runtime_error& ex) {
        LOG_FATAL(nodebooter, ex.what());
        exit(EXIT_FAILURE);
    }

    // Clear umask.
    umask(0);

    // Redirect stdin, stdout and stderr to /dev/null.
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
}

void usage()
{
    std::cerr << "Usage: " << std::endl;
    std::cerr << "    nodeBooter -D <optional dmd path> -d dcd path <optional flags>" << std::endl << std::endl;

    std::cerr << "Configuration:" << std::endl;
    std::cerr << "    --help                     Display this help and exit" << std::endl;
    std::cerr << "    -D <optional dmd path>     Start DomainManager" << std::endl;
    std::cerr << "    -d <dcd path>              Start DeviceManager" << std::endl;
    std::cerr << "    -sdrroot <abs path>        Set sdr root with absolute path to sdr directory" << std::endl;
    std::cerr << "    -sdrcache <abs path>       Set sdr cache with absolute path to cache directory" << std::endl;
    std::cerr << "    -debug                     Set the threshold used for logging, the default is 3 (5=TRACE,4=DEBUG,3=INFO,2=WARN,1=ERROR,0=FATAL)" << std::endl << std::endl;
    std::cerr << "    -logcfgfile <config file>  Pass in a logging config file uri" << std::endl;
    std::cerr << "    --useloglib                Use libossielogcfg.so to generate LOGGING_CONFIG_URI " << std::endl;
    std::cerr << "    --bindapps                 Bind application and component registrations to the Domain and not the NamingService (DomainManager only)" << std::endl;
    std::cerr << "    --dburl                    Store domain state in the following URL" << std::endl;
    std::cerr << "    --force-rebind             Overwrite any existing name binding for the DomainManager" << std::endl;
    std::cerr << "    --daemon                   Run as UNIX daemon" << std::endl;
    std::cerr << "    --pidfile <filename>       Save PID in the specified file" << std::endl;
    std::cerr << "    --user <username>          Run as the specified user" << std::endl;
    std::cerr << "    --group <groupname>        Run as the specified group" << std::endl;
    std::cerr << "    --ORBport <portnumber>     Change the port number used by the Domain Manager to listen (only valid with the -D option)" << std::endl;
    std::cerr << "    --ORBInitRef <ip address>  Change the ip address used by the Device Manager to find the naming service (only valid with the -d option)" << std::endl;
    std::cerr << "    --domainname <domain name> Override the domain name specified in the configuration file" << std::endl;
    std::cerr << "    --version                  Print the REDHAWK version and then exit" << std::endl << std::endl;
    std::cerr << std::endl;

    std::cerr << "In addition to the arguments mentioned above, there is an special argument '--'." << std::endl;
    std::cerr << "The presence of this argument tells the nodeBooter to pass all the arguments to the" << std::endl;
    std::cerr << "DomaninManager and the DeviceManager as execparams " << std::endl;
    std::cerr << std::endl;


    std::cerr << "Examples:" << std::endl;
    std::cerr << "    nodeBooter -d DeviceManager.dcd.xml" << std::endl;
    std::cerr << "    nodeBooter -D -d /nodes/MyNode/DeviceManager.dcd.xml" << std::endl;
    std::cerr << "    nodeBooter -D /domain/DomainManager.dmd.xml -d /nodes/MyNode/DeviceManager.dcd.xml" << std::endl;
    std::cerr << "    nodeBooter -D /domain/DomainManager.dmd.xml -d /nodes/MyNode/DeviceManager.dcd.xml -sdrroot /opt/sdr" << std::endl;
    std::cerr << "    nodeBooter -D /domain/DomainManager.dmd.xml -d /nodes/MyNode/DeviceManager.dcd.xml -sdrroot /opt/sdr -sdrcache /tmp" << std::endl;
    std::cerr << "    nodeBooter -D dom/domain/DomainManager.dmd.xml -d dev/nodes/DeviceManager.dcd.xml #(NOTE: see below for details)" << std::endl;
    std::cerr << "    nodeBooter -D -d DeviceManager.dcd.xml -debug 9" << std::endl;
    std::cerr << "    nodeBooter -D -d DeviceManager.dcd.xml --ORBInitRef 127.0.0.1" << std::endl;
    std::cerr << "    nodeBooter -d DeviceManager.dcd.xml --daemon --pidfile /var/run/domain.pid --user domainuser" << std::endl;
    std::cerr << "    nodeBooter --help" << std::endl << std::endl;

    std::cerr << "NOTE: When using the '-sdrroot' command line argument or the 'SDRROOT' environment variable, references to the dmd.xml or" << std::endl;
    std::cerr << "      dcd.xml files must be relative to the OSSIE::FileSystem(s) - ${SDRROOT}/dom and ${SDRROOT}/dev" << std::endl << std::endl;
    std::cerr << "      Ex: If $SDRROOT=/opt/sdr, then root for the DomainManager's FileSystem will be /opt/sdr/dom, so the reference to" << std::endl;
    std::cerr << "      the DMD file will be /domain/DomainManager.dmd.xml (assuming that this exists at /opt/sdr/dom/domain/DomainManager.dmd.xml)." << std::endl;
    std::cerr << "      And similarly, the root for the DeviceManager's FileSystem will be /opt/sdr/dev, so the reference to the DCD file will" << std::endl;
    std::cerr << "      be /nodes/MyNode/DeviceManager.dcd.xml (assuming that this exists at /opt/sdr/dev/nodes/MyNode/DeviceManager.dcd.xml)." << std::endl;
}


// System Signal Interrupt Handler will allow proper ORB shutdown
void signal_catcher( int sig )
{
    // IMPORTANT Don't call exit(...) in this function
    if ((( sig == SIGINT ) || (sig == SIGQUIT) || (sig == SIGTERM))) {
        if (domPid) {
            kill(domPid, sig);
        } else {
            kill(devPid, sig);
        }
    }
}

void startDomainManager(
    const fs::path&                            domRootPath,
    const string&                              dmdFile,
    const string&                              dcdFile,
    string&                                    domainName,
    const fs::path&                            sdrRootPath,
    const int&                                 debugLevel,
    const bool &                               bind_apps,
    const string&                              logfile_uri,
    const bool&                                use_loglib,
    const string&                              db_uri,
    const string&                              endPoint,
    const bool&                                forceRebind,
    const std::vector<string>&                 execparams,
    const std::vector<const ossie::Property*>& systemProps,
    const bool&                                doFork)
{
    if (!fs::is_directory(domRootPath)) {
        LOG_ERROR(nodebooter, "Invalid Domain Manager File System Root " << domRootPath);
        exit(EXIT_FAILURE);
    }

    fs::path dmdPath = domRootPath / dmdFile;
    std::ifstream dmdStream(dmdPath.string().c_str());
    if (!dmdStream) {
        LOG_ERROR(nodebooter, "Could not read DMD file " << dmdFile);
        exit(EXIT_FAILURE);
    }

    ossie::DomainManagerConfiguration dmd;
    try {
        dmd.load(dmdStream);
    } catch (const ossie::parser_error& ex) {
        std::string parser_error_line = ossie::retrieveParserErrorLineNumber(ex.what());
        LOG_ERROR(nodebooter, "Failed to parse DMD file " << dcdFile << ". " << parser_error_line << "The XML parser returned the following error: " << ex.what());
        exit(EXIT_FAILURE);
    }
    dmdStream.close();

    std::string spdFile = dmd.getDomainManagerSoftPkg();

    LOG_DEBUG(nodebooter, "Loaded DMD file " << dmdFile);
    LOG_DEBUG(nodebooter, "DMD Name:    " << dmd.getName());
    LOG_DEBUG(nodebooter, "DMD Id:      " << dmd.getID());
    LOG_DEBUG(nodebooter, "DMD softpkg: " << spdFile);

    if (domainName.empty()) {
        domainName = dmd.getName();
    } else {
        LOG_DEBUG(nodebooter, "Overriding domain name from DMD with \"" << domainName << "\"");
    }

    ExecParams execParams;
    ExecParams optionParams;
    execParams["DMD_FILE"] = dmdFile;
    execParams["DOMAIN_NAME"] = domainName;
    execParams["SDRROOT"] = sdrRootPath.string();
    if (debugLevel != -1) {
        std::stringstream debugLevel_str;
        debugLevel_str << debugLevel;
        execParams["DEBUG_LEVEL"] = debugLevel_str.str();
    }
    if (!logfile_uri.empty()) {
        execParams["LOGGING_CONFIG_URI"] = logfile_uri;
    }
    if (!db_uri.empty()) {
        execParams["PERSISTENCE"] = "true";
        execParams["DB_URL"] = db_uri;
    } else {
        execParams["PERSISTENCE"] = "false";
    }
    if (!endPoint.empty()) {
        execParams["-ORBendPoint"] = endPoint;
    }
    if (forceRebind) {
        execParams["FORCE_REBIND"] = "true";
    }

    if(execparams.size() > 0) {
        for(unsigned int index = 0; index < execparams.size(); index++) {
            string id = execparams[index];
            if( index + 1 >= execparams.size()) {
                LOG_WARN(nodebooter, "\nThe property: " << id
                        << " did not have a value associated with it, ignoring it!!\n");
                break;
            }
            string value = execparams[++index];
            execParams[id] = value;
         }
    }

    if (use_loglib) {
        optionParams["USELOGCFG"] =  "";
    }
    
    if (bind_apps) {
        optionParams["BINDAPPS"] =  "";
    }
    
    try {
      domPid = launchSPD(spdFile, domRootPath, execParams, optionParams, systemProps, doFork);
    } catch (const std::runtime_error& ex) {
        LOG_ERROR(nodebooter, "Unable to launch DomainManager Softpkg: " << ex.what());
        exit(EXIT_FAILURE);
    }
}

void startDeviceManager(
    const fs::path&                            devRootPath,
    const string&                              dcdFile,
    const string&                              sdrCache,
    string&                                    domainName,
    const fs::path&                            sdrRootPath,
    const int&                                 debugLevel,
    const string&                              logfile_uri,
    const bool&                                use_loglib,
    const string&                              orb_init_ref,
    const std::vector<string>&                 execparams,
    const std::vector<const ossie::Property*>& systemProps,
    const bool&                                doFork)
{

    if (!fs::is_directory(devRootPath)) {
        LOG_ERROR(nodebooter, "Invalid Device Manager File System Root " << devRootPath);
        exit(EXIT_FAILURE);
    }

    fs::path dcdPath = devRootPath / dcdFile;
    std::ifstream dcdStream(dcdPath.string().c_str());
    if (!dcdStream) {
        LOG_ERROR(nodebooter, "Could not read DCD file " << dcdFile);
        exit(EXIT_FAILURE);
    }
    ossie::DeviceManagerConfiguration dcd;
    try {
        dcd.load(dcdStream);
    } catch (const ossie::parser_error& ex) {
        std::string parser_error_line = ossie::retrieveParserErrorLineNumber(ex.what());
        LOG_ERROR(nodebooter, "Failed to parse DCD file " << dcdFile << ". " << parser_error_line << "The XML parser returned the following error: " << ex.what());
        exit(EXIT_FAILURE);
    }
    dcdStream.close();

    std::string spdFile = dcd.getDeviceManagerSoftPkg();

    LOG_DEBUG(nodebooter, "Loaded DCD file " << dcdFile);
    LOG_DEBUG(nodebooter, "DCD Name:    " << dcd.getName());
    LOG_DEBUG(nodebooter, "DCD Id:      " << dcd.getID());
    LOG_DEBUG(nodebooter, "DCD softpkg: " << spdFile);

    // locate the physical location for the device manager's cache
    string devMgrCache;
    if (!sdrCache.empty()) {
        // get location from command line
        if (sdrCache[sdrCache.length()-1] == '/') {
            devMgrCache = sdrCache.substr(0, sdrCache.length() - 1);
        } else {
            devMgrCache = sdrCache;
        }
    } else if (getenv("SDRCACHE") != NULL) {
        // get it from the env variable second
        string sdrCacheEnv = getenv("SDRCACHE");

        if (sdrCacheEnv[sdrCacheEnv.length()-1] == '/') {
            devMgrCache = sdrCacheEnv.substr(0, sdrCacheEnv.length() - 1);
        } else {
            devMgrCache = sdrCacheEnv;
        }
    } else {
        // get relative to fsDevRoot
        devMgrCache = devRootPath.string();
    }

    if (domainName.empty()) {
        std::string domainManagerName = dcd.getDomainManagerName();
        domainName.insert(0, domainManagerName, 0, domainManagerName.find("/"));
    } else {
        LOG_DEBUG(nodebooter, "Overriding domain name from DCD with \"" << domainName << "\"");
    }

    // Build up the execparams based on the DCD and command line arguments.
    ExecParams execParams;
    ExecParams optionParams;
    execParams["DCD_FILE"] = dcdFile;
    execParams["DOMAIN_NAME"] = domainName;
    execParams["SDRROOT"] = sdrRootPath.string();
    execParams["SDRCACHE"] = devMgrCache;
    if (debugLevel != -1) {
        std::stringstream debugLevel_str;
        debugLevel_str << debugLevel;
        execParams["DEBUG_LEVEL"] = debugLevel_str.str();
    }
    if (!logfile_uri.empty()) {
        execParams["LOGGING_CONFIG_URI"] = logfile_uri;
    }
    if (!orb_init_ref.empty()) {
        execParams["-ORBInitRef"] = orb_init_ref;
    }

    if(execparams.size() > 0) {
        for(unsigned int index = 0; index < execparams.size(); index++) {
            string id = execparams[index];
            if( index + 1 >= execparams.size()) {
                LOG_WARN(nodebooter, "\nThe property: " << id
                        << " did not have a value associated with it, ignoring it!!\n");
                break;
            }
            string value = execparams[++index];
            execParams[id] = value;
         }
    }

    if (use_loglib) {
        optionParams["USELOGCFG"] =  "";
    }

    try {
      devPid = launchSPD(spdFile, devRootPath, execParams, optionParams, systemProps, doFork);
    } catch (const std::runtime_error& ex) {
        LOG_ERROR(nodebooter, "Unable to launch DeviceManager Softpkg: " << ex.what());
        exit(EXIT_FAILURE);
    }
}

void logDomainManagerExit(int status)
{
    if (WIFEXITED(status)) {
        status = WEXITSTATUS(status);
        if (status) {
            LOG_WARN(nodebooter, "DomainManager exited with status " << status);
        } else {
            LOG_INFO(nodebooter, "DomainManager exited");
        }
    } else if (WIFSIGNALED(status)) {
        LOG_WARN(nodebooter, "DomainManager terminated with signal " << WTERMSIG(status));
    }
}

void logDeviceManagerExit(int status)
{
    if (WIFEXITED(status)) {
        status = WEXITSTATUS(status);
        if (status) {
            LOG_WARN(nodebooter, "DeviceManager exited with status " << status);
        } else {
            LOG_INFO(nodebooter, "DeviceManager exited");
        }
    } else if (WIFSIGNALED(status)) {
        LOG_WARN(nodebooter, "DeviceManager terminated with signal " << WTERMSIG(status));
    }
}

bool isParentPath(const fs::path& parent, const fs::path& target)
{
    // Simple string-based check whether 'target' starts with 'parent'
    return (target.string().find(parent.string()) == 0);
}

fs::path relativePath (const fs::path& parent, const fs::path& target)
{
    // Assuming that isParentPath(parent, target) is true, simply return the
    // substring of target that starts after parent
    const std::string parent_str = parent.string();
    const std::string target_str = target.string();
    return target_str.substr(parent_str.size());
}

fs::path findParentDir (const fs::path& current, const std::string& name)
{
    if (current.empty()) {
        return fs::path();
    } else if (current.filename() == name) {
        return current;
    } else {
        return findParentDir(current.parent_path(), name);
    }
}

int main(int argc, char* argv[])
{
    // parse command line options
    string dmdFile;
    string dcdFile;
    string sdrRoot;
    string sdrCache;
    string logfile_uri;
    string db_uri;
    string orb_init_ref;
    string domainName;
    string endPoint;
    int debugLevel = -1;
    bool   bind_apps=false;

    bool startDeviceManagerRequested = false;
    bool startDomainManagerRequested = false;

    bool daemonize = false;
    std::string pidfile;
    std::string user;
    std::string group;

    // enable/disable use of libossielogcfg.so to resolve LOGGING_CONFIG_URI values
    bool use_loglib = false;

    // If "--force-rebind" is asserted, the DomainManager will replace any
    // existing name binding.
    bool forceRebind = false;
    std::vector<string> execparams;
    bool parsingExecParams = false;

    // iterate through all of the execparams and parse arguments
    for( int i = 1; i < argc; i++ ) {
        // using '--' as a way to determine when the execparams start.  This is
        //            needed in case one of the execparam matches an argument
        //            used by the nodeBooter
        if ( strcmp( argv[i], "--" ) == 0 ) {
            parsingExecParams = true;
            continue;
        }
        // once the flag is set, then all the remaining arguments are execparams
        if (parsingExecParams) {
            execparams.push_back(argv[i]);
            continue;
        }

        if( strcmp( argv[i], "-D" ) == 0 ) {
            dmdFile = "/domain/DomainManager.dmd.xml";
            if( i + 1 < argc && strcmp( argv[i + 1], "--" ) != 0) {
                string tmpdmdfile = argv[i+1];
                if( tmpdmdfile.find(".dmd.xml") != string::npos ) {
                    dmdFile = tmpdmdfile;
                }
              }
            startDomainManagerRequested = true;
        } else if( strcmp( argv[i], "-d" ) == 0 ) {
            if( i + 1 < argc && strcmp( argv[i + 1], "--" ) != 0) {
                dcdFile = argv[i+1];
                if( dcdFile.find(".dcd.xml") == string::npos ) {
                    std::cerr << "[nodeBooter] ERROR: Illegal DCD profile given\n";
                    usage();
                    exit(EXIT_FAILURE);
                    }
                    startDeviceManagerRequested = true;
            } else {
                std::cerr << "[nodeBooter] ERROR: No DCD profile given\n";
                usage();
                exit(EXIT_FAILURE);
            }
        } else if( strcmp( argv[i], "-sdrroot" ) == 0 ) {
            if( i + 1 < argc && strcmp( argv[i + 1], "--" ) != 0) {
                sdrRoot = argv[i+1];
            } else {
                std::cerr << "[nodeBooter] ERROR: Illegal or no sdr root path given\n";
                usage();
                exit(EXIT_FAILURE);
            }
        } else if( strcmp( argv[i], "-sdrcache" ) == 0 ) {
            if( i + 1 < argc && strcmp( argv[i + 1], "--" ) != 0) {
                sdrCache = argv[i+1];
            } else {
                std::cerr << "[nodeBooter] ERROR: Illegal or no sdr cache path given\n";
                usage();
                exit(EXIT_FAILURE);
            }
        } else if( strcmp( argv[i], "-domainname" ) == 0 ) {
            if( i + 1 < argc && strcmp( argv[i + 1], "--" ) != 0) {
                domainName = argv[i+1];
                std::cerr << "[nodeBooter] warning: -domainname is deprecated. Please use --domainname\n";
            }
            else {
                std::cerr << "[nodeBooter] ERROR: No domain name provided with -domainname argument\n";
                usage();
                exit(EXIT_FAILURE);
            }
        } else if( strcmp( argv[i], "--domainname" ) == 0 ) {
            if( i + 1 < argc && strcmp( argv[i + 1], "--" ) != 0) {
                domainName = argv[i+1];
            }
            else {
                std::cerr << "[nodeBooter] ERROR: No domain name provided with --domainname argument\n";
                usage();
                exit(EXIT_FAILURE);
            }
        } else if (( strcmp( argv[i], "--bindapps" ) == 0 )) {
            bind_apps = true;
        } else if (( strcmp( argv[i], "-log4cxx" ) == 0 ) || ( strcmp( argv[i], "-logcfgfile" ) == 0 )) {
            if( i + 1 <argc && strcmp( argv[i + 1], "--" ) != 0) {
                logfile_uri = argv[i+1];
            } else {
                std::cerr << "[nodeBooter] ERROR: No configuration file provided\n";
                usage();
                exit(EXIT_FAILURE);
            }
        } else if (( strcmp( argv[i], "--useloglib" ) == 0 )) {
            use_loglib = true;
        } else if (( strcmp( argv[i], "-dburl" ) == 0 )) {
            if( i + 1 < argc && strcmp( argv[i + 1], "--" ) != 0) {
                std::cout << "WARNING: -dburl has been deprecated. In the future please use --dburl " << std::endl;
                db_uri = argv[i+1];
            } else {
                std::cerr << "[nodeBooter] ERROR: No DB URL provided\n";
                usage();
                exit(EXIT_FAILURE);
            }
        } else if (( strcmp( argv[i], "--dburl" ) == 0 )) {
            if( i + 1 < argc && strcmp( argv[i + 1], "--" ) != 0) {
                db_uri = argv[i+1];
            } else {
                std::cerr << "[nodeBooter] ERROR: No DB URL provided\n";
                usage();
                exit(EXIT_FAILURE);
            }
        } else if (( strcmp( argv[i], "--ORBInitRef" ) == 0 )) {
            if( i + 1 < argc && strcmp( argv[i + 1], "--" ) != 0) {
                orb_init_ref = "NameService=corbaname::";
                orb_init_ref += argv[i+1];
            } else {
                std::cerr << "[nodeBooter] ERROR: No initial reference address provided\n";
                usage();
                exit(EXIT_FAILURE);
            }
        } else if (( strcmp( argv[i], "--ORBport" ) == 0 )) {
            if( i + 1 < argc && strcmp( argv[i + 1], "--" ) != 0) {
                endPoint = "giop:tcp::";
                endPoint += argv[i+1];
            } else {
                std::cerr << "[nodeBooter] ERROR: No endpoint provided\n";
                usage();
                exit(EXIT_FAILURE);
            }
        } else if (( strcmp( argv[i], "-ORBendPoint" ) == 0 )) {
            if( i + 1 < argc && strcmp( argv[i + 1], "--" ) != 0) {
                std::cerr << "[nodeBooter] WARN: -ORBendPoint option has been deprecated. Use --ORBport instead\n";
                endPoint = argv[i+1];
            } else {
                std::cerr << "[nodeBooter] ERROR: No endpoint provided\n";
                usage();
                exit(EXIT_FAILURE);
            }
        } else if( strcmp( argv[i], "-debug" ) == 0 ) {
                if( (i + 1 < argc) && isdigit( *argv[i+1] ) ) {
                    debugLevel = atoi( argv[i+1] );
                    i++;
                } else {
                    std::cerr << "\n\t[nodeBooter] WARNING: The debug level must be an integer, using default value (3)\n" << std::endl;
                }
        } else if( strcmp( argv[i], "--help" ) == 0 ) {
            usage();
            exit(EXIT_SUCCESS);
        } else if( strcmp( argv[i], "--version" ) == 0 ) {
            std::cout<<"REDHAWK version: "<<VERSION<<std::endl;
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[i], "--nopersist") == 0) {
            std::cerr << "[nodeBooter] WARNING: --nopersist is deprecated and will be ignored" << std::endl;
        } else if (strcmp(argv[i], "--force-rebind") == 0) {
            forceRebind = true;
        } else if (strcmp(argv[i], "--daemon") == 0) {
            daemonize = true;
        } else if (strcmp(argv[i], "--pidfile") == 0) {
            if (++i >= argc) {
                std::cerr << "[nodeBooter] ERROR: No PID file specified for --pidfile" << std::endl;
                usage();
                return(EXIT_FAILURE);
            }
            pidfile = argv[i];
        } else if (strcmp(argv[i], "--user") == 0) {
            if (++i >= argc) {
                std::cerr << "[nodeBooter] ERROR: No user specified for --user" << std::endl;
                usage();
                return(EXIT_FAILURE);
            }
            user = argv[i];
        } else if (strcmp(argv[i], "--group") == 0) {
            if (++i >= argc) {
                std::cerr << "[nodeBooter] ERROR: No group specified for --group" << std::endl;
                usage();
                return (EXIT_FAILURE);
            }
            group = argv[i];
        } else {
            std::string unknown_arg(argv[i]);
            if (unknown_arg[0] == '-') {
                if (unknown_arg.find('=') != std::string::npos) {
                    std::cerr << "[nodeBooter] invalid argument format for argument: \""<<unknown_arg<<"\". Separator must be a space, not an equals sign."<< std::endl;
                    usage();
                    return (EXIT_FAILURE);
                }
            }
        }
    }  // end argument parsing

    if ((not user.empty()) or (not group.empty())) {
        if (not daemonize) {
            std::cerr << "[nodeBooter] If either group or user are specified, daemon must be set" << std::endl;
            usage();
            return (EXIT_FAILURE);
        }
    }

    // Check that there is work to do
    if (!(startDeviceManagerRequested || startDomainManagerRequested)) {
        usage();
        exit (0);
    }

    // If we are launching both a DomainManager and DeviceManager, then we must
    // fork to spawn each one.
    bool doFork = (startDomainManagerRequested && startDeviceManagerRequested);

    // We have to have a real SDRROOT
    fs::path sdrRootPath;
    if (!sdrRoot.empty()) {
        sdrRootPath = sdrRoot;
    } else {
        if (getenv("SDRROOT") != NULL) {
            sdrRootPath = getenv("SDRROOT");
        } else {
            // Fall back to CWD
            sdrRootPath = fs::initial_path();
        }
    }

    // Checks if dom path is available in SDRROOT
    // If not tries to use as a relative path
    const std::string DOM_FOLDER = "dom";
    fs::path domRootPath = sdrRootPath / DOM_FOLDER;
    if (startDomainManagerRequested) {
        // Assume relative paths are relative to the current directory, and
        // turn into an absolute path
        fs::path dmdPath = fs::system_complete(dmdFile);

        // First, check if the DMD path is relative to $SDRROOT/dom
        if (!fs::exists(domRootPath / dmdPath)) {
            // DMD path is absolute; check that it really exists
            if (!fs::exists(dmdPath)) {
                std::cerr << "[nodeBooter] ERROR: .dmd.xml file does not exist" << std::endl;
                exit(EXIT_FAILURE);
            }

            // Check if the path is within $SDRROOT/dom
            if (!isParentPath(domRootPath, dmdPath)) {
                // If it's not in $SDRROOT/dom, try to determine the effective
                // SDRROOT by backtracking in the path to find a directory
                // named "dom"
                domRootPath = findParentDir(dmdPath, DOM_FOLDER);
                if (domRootPath.empty()) {
                    std::cerr << "[nodeBooter] ERROR: can't determine domain SDRROOT" << std::endl;
                    exit(EXIT_FAILURE);
                }
                sdrRootPath = domRootPath.parent_path();
            }

            // Normalize the DMD path to be relative to $SDRROOT/dom
            dmdFile = relativePath(domRootPath, dmdPath).string();
        }
    }


    // Checks if dev path is available in SDRROOT
    // If not tries to use as a relative path
    const std::string DEV_FOLDER = "dev";
    fs::path devRootPath = sdrRootPath / DEV_FOLDER;
    if (startDeviceManagerRequested) {
        // Assume relative paths are relative to the current directory, and
        // turn into an absolute path
        fs::path dcdPath = fs::system_complete(dcdFile);

        // First, check if the DCD path is relative to $SDRROOT/dev
        if (!fs::exists(devRootPath / dcdPath)) {
            // DCD path is absolute; check that it really exists
            if (!fs::exists(dcdPath)) {
                std::cerr << "[nodeBooter] ERROR: .dcd.xml file does not exist" << std::endl;
                exit(EXIT_FAILURE);
            }

            // Check if the path is in $SDRROOT/dev
            if (!isParentPath(devRootPath, dcdPath)) {
                // If it's not in $SDRROOT/dev, try to determine the effective
                // SDRROOT by backtracking in the path to find a directory
                // named "dev"
                devRootPath = findParentDir(dcdPath, DEV_FOLDER);
                if (devRootPath.empty()) {
                    std::cerr << "[nodeBooter] ERROR: can't determine device SDRROOT" << std::endl;
                    exit(EXIT_FAILURE);
                }
                sdrRootPath = devRootPath.parent_path();
            }

            // Normalize the DCD path to be relative to $SDRROOT/dev
            dcdFile = relativePath(devRootPath, dcdPath).string();
        }
    }

    // Verify the path exists
    if (!fs::is_directory(sdrRootPath)) {
        std::cerr << "Invalid SDRROOT" << std::endl;
        exit(EXIT_FAILURE);
    }


    // Check persistence flags; only presence is tested here, not validity
    // (i.e., can the DB file be read/written) because in daemonized mode the
    // user and group are not applied until after fork()
    if (!db_uri.empty()) {
        if (startDomainManagerRequested) {
#if !ENABLE_PERSISTENCE
            // Cannot supply persistence flags unless support is compiled in
            std::cerr << "[nodeBooter] ERROR: --dburl provided, but REDHAWK was compiled without persistence support" << std::endl;
            exit(EXIT_FAILURE);
#endif
        } else {
            // Cannot supply persistence flags on DeviceManager-only runs
            std::cerr << "[nodeBooter] ERROR: DeviceManager does not support --dburl" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // Create signal handler to catch system interrupts SIGINT and SIGQUIT
    struct sigaction sa;
    sa.sa_handler = signal_catcher;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    // Associate SIGINT to signal_catcher interrupt handler
    if (sigaction(SIGINT, &sa, NULL)) {
        perror("SIGINT");
        exit(EXIT_FAILURE);
    }

    // Associate SIGQUIT to signal_catcher interrupt handler
    if (sigaction( SIGQUIT, &sa, NULL)) {
        perror("SIGQUIT");
        exit(EXIT_FAILURE);
    }

    // Associate SIGTERM to signal_catcher interrupt handler
    if (sigaction( SIGTERM, &sa, NULL)) {
        perror("SIGTERM");
        exit(EXIT_FAILURE);
    }

    // Configure logging for nodebooter strictly from the debug level; let the
    // DomainManager or DeviceManager deal with any log4cxx configuration file.
    // In the case of daemon mode, this will protect us from the possibility of
    // creating the log file(s) as root.
    ossie::logging::Configure(NULL, debugLevel);

    ///////////////////////////////////////////////////////////////////////////
    // NO LOG_ STATEMENTS ABOVE THIS POINT
    ///////////////////////////////////////////////////////////////////////////

    if (daemonize) {
        if (startDomainManagerRequested && startDeviceManagerRequested) {
            LOG_FATAL(nodebooter, "Can not start both DomainManager and DeviceManager at same time in daemon mode");
            exit(EXIT_FAILURE);
        }

        initializeDaemon(user, group, pidfile);
    }

    // Figure out what architecture we are on
    // Map i686 to SCA x86
    struct utsname un;
    if (uname(&un)) {
        perror("Unable to determine system information");
        exit(EXIT_FAILURE);
    }
    if (strcmp("i686", un.machine) == 0) {
        strcpy(un.machine, "x86");
    }
    LOG_DEBUG(nodebooter, "Machine " << un.machine);
    LOG_DEBUG(nodebooter, "Version " << un.release);
    LOG_DEBUG(nodebooter, "OS " << un.sysname);

    // Build a list of properties for this system based on the information from uname.
    ossie::SimpleProperty osProp("DCE:4a23ad60-0b25-4121-a630-68803a498f75",
                                 "os_name",
                                 "string",
                                 ossie::Property::MODE_READONLY,
                                 ossie::Property::ACTION_EQ,
                                 ossie::Property::KIND_ALLOCATION,
                                 std::string(un.sysname));
    ossie::SimpleProperty procProp("DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b",
                                   "processor_name",
                                   "string",
                                   ossie::Property::MODE_READONLY,
                                   ossie::Property::ACTION_EQ,
                                   ossie::Property::KIND_ALLOCATION,
                                   std::string(un.machine));
    std::vector<const ossie::Property*> systemProps;
    systemProps.push_back(&osProp);
    systemProps.push_back(&procProp);

    try {
        // Start Domain Manager if requested
        if (startDomainManagerRequested) {
            startDomainManager(domRootPath,
                               dmdFile,
                               dcdFile,
                               domainName,
                               sdrRootPath,
                               debugLevel,
                               bind_apps,
                               logfile_uri,
                               use_loglib,
                               db_uri,
                               endPoint,
                               forceRebind,
                               execparams,
                               systemProps,
                               doFork);
        }

        // Start Device Manager if requested
        if (startDeviceManagerRequested) {
            startDeviceManager(devRootPath,
                               dcdFile,
                               sdrCache,
                               domainName,
                               sdrRootPath,
                               debugLevel,
                               logfile_uri,
                               use_loglib,
                               orb_init_ref,
                               execparams,
                               systemProps,
                               doFork);
        }

        // If we reach this point, the DomainManager and DeviceManager were
        // spawned in child processes via fork(). Wait for the children to exit.
        while (domPid || devPid) {
            int status;
            pid_t pid = waitpid(-1, &status, 0);
            if (pid == domPid) {
                domPid = 0;
                logDomainManagerExit(status);
            } else if (pid == devPid) {
                devPid = 0;
                logDeviceManagerExit(status);
            }
        }

        LOG_INFO(nodebooter, "Domain/Device Manager processes have exited");
    } catch (const std::exception& ex) {
        LOG_ERROR(nodebooter, "Terminated with unexpected exception: " << ex.what());
        exit(EXIT_FAILURE);
    }

    LOG_DEBUG(nodebooter, "Nodebooter has ended gracefully.");
    return 0;
}
