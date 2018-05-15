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

#ifndef APPLICATIONVALIDATOR_H
#define APPLICATIONVALIDATOR_H

#include <stdexcept>

#include <ossie/debug.h>
#include <ossie/CF/cf.h>
#include <ossie/SoftwareAssembly.h>

#include "ProfileCache.h"

namespace redhawk {
    
    /**
     * @brief  An exception raised when a SoftwareAssembly is invalid
     */
    class validation_error : public std::runtime_error {
        public:
            validation_error(const std::string& what_arg) : std::runtime_error(what_arg)
        {}
    };

    class ApplicationValidator {

        ENABLE_LOGGING;

    public:
        ApplicationValidator(CF::FileSystem_ptr fileSystem, rh_logger::LoggerPtr log);

        /**
         * @brief  Validates a SoftwareAssembly
         * @param sad  a parsed SoftwareAssembly
         * @exception redhawk::validation_error  the SAD is invalid
         * @exception redhawk::invalid_profile  a SoftPkg profile used by this
         *            SAD is invalid
         *
         * Performs validation of a SoftwareAssembly, making sure that it is
         * semantically valid (at least, enough to attempt deployment). All
         * SoftPkgs that could potentially be used to deploy components are
         * checked to make sure that they are valid.
         */
        void validate(const ossie::SoftwareAssembly& sad);

    private:
        void validateExternalPorts(const std::vector<ossie::SoftwareAssembly::Port>& ports);

        void validateExternalProperties(const ossie::Properties* acProperties,
                                        const std::vector<ossie::SoftwareAssembly::Property>& properties);

        void validateHostCollocation(const ossie::SoftwareAssembly::HostCollocation& collocation);

        void validateComponentPlacement(const ossie::ComponentPlacement& placement);

        void validateSoftPkgRef(const ossie::SPD::SoftPkgRef& softpkgref);

        void validateImplementation(const ossie::SoftPkg* softpkg,
                                    const ossie::SPD::Implementation& implementation,
                                    bool executable);

        const ossie::Properties* getAssemblyControllerProperties(const ossie::SoftwareAssembly& sad);

        bool endsWith(const std::string& filename, const std::string& suffix);
        bool fileExists(const std::string& filename);

        std::string _relativePath(const ossie::SoftPkg* softpkg, const std::string& path);

        CF::FileSystem_var fileSystem;
        redhawk::ProfileCache cache;
        rh_logger::LoggerPtr _appFactoryLog;
    };
}

#endif // APPLICATIONVALIDATOR_H
