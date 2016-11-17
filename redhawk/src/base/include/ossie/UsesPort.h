#ifndef OSSIE_USESPORT_H
#define OSSIE_USESPORT_H

#include <string>
#include <vector>

#include <omniORB4/CORBA.h>

#include "CF/QueryablePort.h"
#include "Autocomplete.h"
#include "Port_impl.h"

namespace redhawk {
    class BasicTransport
    {
    public:
        BasicTransport(CORBA::Object_ptr objref) :
            _objref(CORBA::Object::_duplicate(objref)),
            _alive(true)
        {
        }

        virtual ~BasicTransport()
        {
        }

        bool isAlive() const
        {
            return _alive;
        }

        void setAlive(bool alive)
        {
            _alive = alive;
        }

        CORBA::Object_var objref()
        {
            if (isAlive()) {
                return CORBA::Object::_duplicate(_objref);
            } else {
                return CORBA::Object::_nil();
            }
        }

    private:
        CORBA::Object_var _objref;
        bool _alive;
    };

    class UsesPort : public Port_Uses_base_impl
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
                   , public virtual POA_ExtendedCF::QueryablePort
#endif
    {
    public:
        UsesPort(const std::string& name);

        virtual void disconnectPort(const char* connectionId);

        virtual ExtendedCF::UsesConnectionSequence* connections();

    protected:
        typedef std::pair<std::string,BasicTransport*> transport_entry;
        typedef std::vector<transport_entry> transport_list;

        transport_list::iterator _findTransportEntry(const std::string& connectionId);
        void _addTransportEntry(const std::string& connectionId, BasicTransport* transport);

        virtual void _transportDisconnected(const std::string& connectionId, BasicTransport* transport);

        transport_list _transports;
    };
}

#endif // OSSIE_USESPORT_H
