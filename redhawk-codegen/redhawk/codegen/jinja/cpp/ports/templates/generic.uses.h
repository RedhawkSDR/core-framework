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
//% set classname = portgen.className()
//% set vartype = portgen.interfaceClass() + '_var'
class ${classname} : public Port_Uses_base_impl, public POA_ExtendedCF::QueryablePort
{
    ENABLE_LOGGING
    public:
        ${classname}(std::string port_name, ${component.baseclass.name} *_parent);
        ~${classname}();

/*{% for op in portgen.operations() %}*/
/*{%  if op.readwrite_attr %}*/
        ${op.returns} ${op.name}();
        ${op.returns} _get_${op.name}(const std::string __connection_id__);
/*{%  else %}*/
/*{%   if op.arglist %}*/
        ${op.returns} ${op.name}(${op.arglist}, const std::string __connection_id__ = "");
/*{%   else %}*/
        ${op.returns} ${op.name}(const std::string __connection_id__ = "");
/*{%   endif %}*/
/*{%  endif %}*/
/*{% endfor %}*/

        std::vector<std::string> getConnectionIds()
        {
            std::vector<std::string> retval;
            for (unsigned int i = 0; i < outConnections.size(); i++) {
                retval.push_back(outConnections[i].second);
            }
            return retval;
        };

        void __evaluateRequestBasedOnConnections(const std::string &__connection_id__, bool returnValue, bool inOut, bool out) {
            if (__connection_id__.empty() and (this->outConnections.size() > 1)) {
                if (out or inOut or returnValue) {
                    throw redhawk::PortCallError("Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.",
                            getConnectionIds());
                }
            }
            if (this->outConnections.empty()) {
                if (out or inOut or returnValue) {
                    throw redhawk::PortCallError("No connections available.", std::vector<std::string>());
                } else {
                    if (not __connection_id__.empty()) {
                        std::ostringstream eout;
                        eout<<"The requested connection id ("<<__connection_id__<<") does not exist.";
                        throw redhawk::PortCallError(eout.str(), getConnectionIds());
                    }
                }
            }
            if ((not __connection_id__.empty()) and (not this->outConnections.empty())) {
                bool foundConnection = false;
                std::vector < std::pair < ${vartype}, std::string > >::iterator i;
                for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                    if ((*i).second == __connection_id__) {
                        foundConnection = true;
                        break;
                    }
                }
                if (not foundConnection) {
                    std::ostringstream eout;
                    eout<<"The requested connection id ("<<__connection_id__<<") does not exist.";
                    throw redhawk::PortCallError(eout.str(), getConnectionIds());
                }
            }
        }

        ExtendedCF::UsesConnectionSequence * connections() 
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
            if (recConnectionsRefresh) {
                recConnections.length(outConnections.size());
                for (unsigned int i = 0; i < outConnections.size(); i++) {
                    recConnections[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
                    recConnections[i].port = CORBA::Object::_duplicate(outConnections[i].first);
                }
                recConnectionsRefresh = false;
            }
            // NOTE: You must delete the object that this function returns!
            return new ExtendedCF::UsesConnectionSequence(recConnections);
        }

/*{% if portgen.interfaceClass() != "CF::Port"  %}*/
        void connectPort(CORBA::Object_ptr connection, const char* connectionId)
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
            ${vartype} port = ${portgen.interfaceClass()}::_narrow(connection);
            outConnections.push_back(std::make_pair(port, connectionId));
            active = true;
            recConnectionsRefresh = true;
        }

        void disconnectPort(const char* connectionId)
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
            for (unsigned int i = 0; i < outConnections.size(); i++) {
                if (outConnections[i].second == connectionId) {
                    outConnections.erase(outConnections.begin() + i);
                    break;
                }
            }

            if (outConnections.size() == 0) {
                active = false;
            }
            recConnectionsRefresh = true;
        }

/*{% endif %}*/
        std::string getRepid () const;

        std::vector< std::pair<${vartype}, std::string> > _getConnections()
        {
            return outConnections;
        }

    protected:
        ${component.userclass.name} *parent;
        std::vector < std::pair<${vartype}, std::string> > outConnections;
        ExtendedCF::UsesConnectionSequence recConnections;
        bool recConnectionsRefresh;
};
