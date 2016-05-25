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


#ifndef APPLICATIONPROFILE_H
#define APPLICATIONPROFILE_H

#include <string>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>

#include "applicationSupport.h"

namespace ossie {

    class ApplicationVisitor;

    class Placement
    {
    public:
        virtual ~Placement() { }
        virtual void accept(ApplicationVisitor* visitor) = 0;
    };

    class SinglePlacement : public Placement
    {
    public:
        SinglePlacement(const ComponentInstantiation* instantiation,
                        const SoftPkg* softpkg);

        virtual void accept(ApplicationVisitor* visitor);

        const ComponentInstantiation* getComponentInstantiation() const;

        const SoftPkg* getComponentProfile();

    protected:
        const ComponentInstantiation* instantiation;
        const SoftPkg* softpkg;
    };

    class CollocationPlacement : public Placement
    {
    public:
        typedef std::vector<Placement*> PlacementList;

        CollocationPlacement(const std::string& id, const std::string& name);

        virtual void accept(ApplicationVisitor* visitor);

        const std::string& getId() const;
        const std::string& getName() const;

        const PlacementList& getPlacements() const;

        void addPlacement(SinglePlacement* placement);

    protected:
        const std::string id;
        const std::string name;
        PlacementList placements;
    };

    /* Base class to contain data for applications
     *  - Used to store information about about:
     *       -> ExternalPorts
     *       -> External Properties
     *       -> UsesDevice relationships
     */
    class ApplicationProfile
    {
        ENABLE_LOGGING;

    public:
        typedef std::vector<Placement*> PlacementList;

        ApplicationProfile();
        ~ApplicationProfile();

        void accept(ApplicationVisitor* visitor);

        const std::string& getIdentifier() const;

        void load(CF::FileSystem_ptr fileSystem, const SoftwareAssembly& sad);

        const PlacementList& getPlacements() const;

        const SoftPkg* getSoftPkg(const std::string& filename) const;

    protected:
        typedef boost::ptr_vector<SoftPkg> ProfileList;

        const SoftPkg* loadProfile(CF::FileSystem_ptr fileSystem, const std::string& filename);

        SinglePlacement* buildComponentPlacement(CF::FileSystem_ptr fileSystem,
                                                 const SoftwareAssembly& sad,
                                                 const ComponentPlacement& placement);

        std::string identifier;
        ProfileList profiles;
        PlacementList placements;
    };

    class ApplicationVisitor
    {
    public:
        virtual ~ApplicationVisitor() { }

        virtual void visitApplication(ApplicationProfile* application)
        {
        }

        virtual void visitComponentPlacement(SinglePlacement* placement)
        {
        }

        virtual void visitHostCollocation(CollocationPlacement* collocation)
        {
        }

    };

}

#endif // APPLICATIONPROFILE_H
