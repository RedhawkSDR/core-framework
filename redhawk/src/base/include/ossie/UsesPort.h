#ifndef OSSIE_USESPORT_H
#define OSSIE_USESPORT_H

#include <string>
#include <vector>

#include <omniORB4/CORBA.h>

#include "CF/QueryablePort.h"

#include "Autocomplete.h"
#include "Port_impl.h"
#include "callback.h"

namespace redhawk {
    class BasicTransport
    {
    public:
        BasicTransport(CORBA::Object_ptr objref);
        virtual ~BasicTransport() { }

        bool isAlive() const;
        void setAlive(bool alive);

        CORBA::Object_ptr objref() const;

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

        // Register the member function 'func' to be called on class instance
        // 'target' when a new connection is made. The function receives one
        // argument, the connection ID:
        //
        //   void Target::func(const std::string&);
        //
        template <class Target, class Func>
        void addConnectListener (Target target, Func func)
        {
            _portConnected.add(target, func);
        }

        // Unregister the member function 'func' on class instance 'target'
        // from further connection notifications. If the pair has not been
        // registered previously, it is ignored.
        template <class Target, class Func>
        void removeConnectListener (Target target, Func func)
        {
            _portConnected.remove(target, func);
        }

        // Register the member function 'func' to be called on class instance
        // 'target' when an existing connection is broken. The function
        // receives one argument, the connection ID:
        //
        //   void Target::func(const std::string&);
        //
        template <class Target, class Func>
        void addDisconnectListener (Target target, Func func)
        {
            _portDisconnected.add(target, func);
        }

        // Unregister the member function 'func' on class instance 'target'
        // from further disconnection notifications. If the pair has not been
        // registered previously, it is ignored.
        template <class Target, class Func>
        void removeDisconnectListener (Target target, Func func)
        {
            _portDisconnected.remove(target, func);
        }

        virtual void connectPort(CORBA::Object_ptr connection, const char* connectionId);

        virtual void disconnectPort(const char* connectionId);

        virtual ExtendedCF::UsesConnectionSequence* connections();

    protected:
        typedef std::pair<std::string,BasicTransport*> transport_entry;
        typedef std::vector<transport_entry> transport_list;

        virtual void _validatePort(CORBA::Object_ptr object);

        transport_list::iterator _findTransportEntry(const std::string& connectionId);
        void _addTransportEntry(const std::string& connectionId, BasicTransport* transport);

        virtual BasicTransport* _createTransport(CORBA::Object_ptr object, const std::string& connectionId);
        virtual void _transportDisconnected(const std::string& connectionId, BasicTransport* transport);

        transport_list _transports;

    private:
        ossie::notification<void (const std::string&)> _portConnected;
        ossie::notification<void (const std::string&)> _portDisconnected;
    };
}

#endif // OSSIE_USESPORT_H
