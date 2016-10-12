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
/*{%extends "pull/resource_base.h"%}*/
/*{%block license%}*/
/*{% from "gpl.cpp" import gplHeader%}*/
${gplHeader(component)}
/*{%endblock%}*/

/*{% block includeExtentions %}*/

// The octave imports make this program GPL.
#include <octave/oct.h>
#include <octave/parse.h>
#include <octave/octave.h>
#include <octave/toplev.h>

#include <boost/filesystem.hpp>
/*{% endblock %}*/

/*{% block extendedPublic %}*/
        int serviceFunction();
        void setCurrentWorkingDirectory(std::string& cwd);
        const octave_value_list _feval(const std::string function, const octave_value_list &functionArguments);
/*{% endblock %}*/

/*{% block extendedPrivate %}*/
        ENABLE_LOGGING
        std::map<std::string, std::vector<double> > inputBuffers;
/*{%if component.ports%}*/
        int buffer(std::string portName, bulkio::InDoublePort* port);
        void populateOutputPacket(
            bulkio::InDoublePort::DataTransferType* outputPacket,
            const octave_value_list&                result,
            const int                               resultIndex);
/*{%endif%}*/
        std::string getLogDir();
        void createDirectoryTree(std::string target_dir);
        void setDiary();
        void flushDiary();
        std::string _diaryFile;
/*{% endblock %}*/

/*{% block extendedProtected%}*/

/*{%if component.ports%}*/
        std::map<std::string, bulkio::InDoublePort::DataTransferType*> inputPackets;
        std::map<std::string, bulkio::InDoublePort::DataTransferType*> outputPackets;

        bulkio::InDoublePort::DataTransferType* createDefaultDataTransferType(std::string&);

/*{%endif%}*/
        virtual int preProcess();
        virtual int postProcess();

        int         _serviceFunctionReturnVal;
        std::string _sriPort;
        double      __sampleRate;
/*{% endblock %}*/
