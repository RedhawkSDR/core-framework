/*#
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
 #*/
//% extends "pull/resource_base.h"
/*{% block defines %}*/
${super()}
#define BOOL_VALUE_HERE 0
/*{% endblock %}*/

/*{% block extendedPublic %}*/
/*{%   if component.hasmultioutport %}*/
        void matchAllocationIdToStreamId(const std::string allocation_id, const std::string stream_id, const std::string port_name="");
        void removeAllocationIdRouting(const size_t tuner_id);
        void removeStreamIdRouting(const std::string stream_id, const std::string allocation_id="");
/*{%   else %}*/
        void removeAllocationIdRouting(const size_t tuner_id);
/*{%   endif %}*/
/*{%   if 'FrontendTuner' in component.implements %}*/

        virtual CF::Properties* getTunerStatus(const std::string& allocation_id);
        virtual void assignListener(const std::string& listen_alloc_id, const std::string& allocation_id);
        virtual void removeListener(const std::string& listen_alloc_id);
        void frontendTunerStatusChanged(const std::vector<frontend_tuner_status_struct_struct>* oldValue, const std::vector<frontend_tuner_status_struct_struct>* newValue);
/*{%   endif %}*/
/*{% endblock %}*/

/*{% block baseProtectedMembers %}*/
${super()}
/*{% if 'FrontendTuner' in component.implements %}*/
        std::map<std::string, std::string> listeners;
/*{% endif %}*/
/*{% endblock %}*/

/*{% block extendedProtected%}*/
/*{% endblock %}*/
