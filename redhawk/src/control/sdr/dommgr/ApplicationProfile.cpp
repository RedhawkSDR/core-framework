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

#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

#include <ossie/FileStream.h>

#include "ApplicationProfile.h"

using namespace ossie;

/**
 * SinglePlacement
 */
SinglePlacement::SinglePlacement(const ComponentInstantiation* instantiation,
                                 const boost::shared_ptr<SoftPkg>& softpkg) :
    instantiation(instantiation),
    softpkg(softpkg)
{
}

void SinglePlacement::accept(ApplicationVisitor* visitor)
{
    visitor->visitComponentPlacement(this);
}

const ComponentInstantiation* SinglePlacement::getComponentInstantiation() const
{
    return instantiation;
}

const boost::shared_ptr<SoftPkg>& SinglePlacement::getComponentProfile()
{
    return softpkg;
}


CollocationPlacement::CollocationPlacement(const std::string& id, const std::string& name) :
    id(id),
    name(name)
{
}

void CollocationPlacement::accept(ApplicationVisitor* visitor)
{
    visitor->visitHostCollocation(this);
}

const std::string& CollocationPlacement::getId() const
{
    return id;
}

const std::string& CollocationPlacement::getName() const
{
    return name;
}

const CollocationPlacement::PlacementList& CollocationPlacement::getPlacements() const
{
    return placements;
}

void CollocationPlacement::addPlacement(SinglePlacement* placement)
{
    placements.push_back(placement);
}

////////////////////////////////////////////////////
/*
 * ApplicationProfile member function definitions
 */
PREPARE_LOGGING(ApplicationProfile);

ApplicationProfile::ApplicationProfile()
{
}

ApplicationProfile::~ApplicationProfile()
{
    for (PlacementList::iterator placement = placements.begin(); placement != placements.end(); ++placement) {
        delete *placement;
    }
}

void ApplicationProfile::accept(ApplicationVisitor* visitor)
{
    visitor->visitApplication(this);
}

const std::string& ApplicationProfile::getIdentifier() const
{
    return identifier;
}

void ApplicationProfile::load(CF::FileSystem_ptr fileSystem, const SoftwareAssembly& sad)
{
    identifier = sad.getID();

    // Gets uses device relationships
    const std::vector<UsesDevice>& usesDevice = sad.getUsesDevices();
    for (std::vector<UsesDevice>::const_iterator use = usesDevice.begin(); use != usesDevice.end(); ++use) {
        UsesDeviceInfo* useDev = new UsesDeviceInfo(use->getID(), use->getType(), use->getDependencies());
        addUsesDevice(useDev);
    }

    // Walk through the host collocations first
    const std::vector<SoftwareAssembly::HostCollocation>& collocations = sad.getHostCollocations();
    for (size_t index = 0; index < collocations.size(); ++index) {
        const SoftwareAssembly::HostCollocation& collocation = collocations[index];
        LOG_TRACE(ApplicationProfile, "Building component info for host collocation "
                  << collocation.getID());
        CollocationPlacement* placement = new CollocationPlacement(collocation.getID(), collocation.getName());
        placements.push_back(placement);

        const std::vector<ComponentPlacement>& components = collocations[index].getComponents();
        for (unsigned int i = 0; i < components.size(); i++) {
            SinglePlacement* component = buildComponentPlacement(fileSystem, sad, components[i]);
            placement->addPlacement(component);
        }
    }

    // Then, walk through the remaining non-collocated components
    const std::vector<ComponentPlacement>& components = sad.getComponentPlacements();
    for (unsigned int i = 0; i < components.size(); i++) {
        // ossie::ComponentInfo* component = buildComponentInfo(fileSys, sadParser, componentsFromSAD[i]);
        // if (component->getInstantiationIdentifier() == assemblyControllerRefId) {
        //     component->setIsAssemblyController(true);
        // }
        SinglePlacement* placement = buildComponentPlacement(fileSystem, sad, components[i]);
        placements.push_back(placement);
    }
}

boost::shared_ptr<SoftPkg> ApplicationProfile::loadProfile(CF::FileSystem_ptr fileSystem,
                                                           const std::string& filename)
{
    for (ProfileList::const_iterator profile = profiles.begin(); profile != profiles.end(); ++profile) {
        if ((*profile)->getSPDFile() == filename) {
            LOG_TRACE(ApplicationProfile, "Found existing profile " << filename);
            return *profile;
        }
    }

    File_stream spd_stream(fileSystem, filename.c_str());
    boost::shared_ptr<SoftPkg> spd = boost::make_shared<SoftPkg>(boost::ref(spd_stream), filename);
    spd_stream.close();

    if (spd->getPRFFile()) {
        File_stream prf_stream(fileSystem, spd->getPRFFile());
        boost::shared_ptr<Properties> prf = boost::make_shared<Properties>();
        prf->load(prf_stream);
        spd->setProperties(prf);
    }

    if (spd->getSCDFile()) {
        File_stream scd_stream(fileSystem, spd->getSCDFile());
        boost::shared_ptr<ComponentDescriptor> scd = boost::make_shared<ComponentDescriptor>();
        scd->load(scd_stream);
        spd->setDescriptor(scd);
    }

    profiles.push_back(spd);
    return spd;
}

SinglePlacement* ApplicationProfile::buildComponentPlacement(CF::FileSystem_ptr fileSystem,
                                                             const SoftwareAssembly& sad,
                                                             const ComponentPlacement& placement)
{
    const ComponentFile *componentfile = sad.getComponentFile(placement.getFileRefId());
    if (!componentfile) {
        throw std::runtime_error("componentplacement has invalid componentfileref " + placement.getFileRefId());
    }
    boost::shared_ptr<SoftPkg> softpkg = loadProfile(fileSystem, componentfile->getFileName());

    // Even though it is possible for there to be more than one instantiation
    // per component, the tooling doesn't support that, so supporting this at a
    // framework level would add substantial complexity without providing any
    // appreciable improvements. It is far easier to have multiple placements
    // rather than multiple instantiations.
    const std::vector<ComponentInstantiation>& instantiations = placement.getInstantiations();
    const ComponentInstantiation& instance = instantiations[0];

    return new SinglePlacement(&instance, softpkg);
}

const ApplicationProfile::PlacementList& ApplicationProfile::getPlacements() const
{
    return placements;
}
