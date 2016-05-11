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

#ifndef PLACEMENT_H
#define PLACEMENT_H

#include <string>
#include <vector>

#include "applicationSupport.h"

namespace ossie {

    class PlacementPlan
    {
    public:
        typedef std::vector<ComponentInfo*> ComponentList;

        PlacementPlan();
        PlacementPlan(const std::string& id, const std::string& name);

        const std::string& getId() const;
        const std::string& getName() const;

        const ComponentList& getComponents() const;

        void addComponent(ComponentInfo* component);

        ComponentInfo* getComponent(const std::string& instantiationId);

    protected:
        std::string id;
        std::string name;
        ComponentList components;
    };

    class ApplicationPlacement
    {
    public:
        typedef std::vector<PlacementPlan*> PlacementList;

        ApplicationPlacement();
        ~ApplicationPlacement();

        void addPlacement(PlacementPlan* placement);

        const PlacementList& getPlacements() const;

        ComponentInfo* getAssemblyController();

        ComponentInfo* getComponent(const std::string& instantiationId);

    protected:
        PlacementList placements;
    };

}

#endif // PLACEMENT_H
