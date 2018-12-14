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


#ifndef PORT_IMPL_H
#define PORT_IMPL_H

#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <boost/thread/mutex.hpp>

#include "CF/cf.h"
#include "ossie/Autocomplete.h"
#include "ossie/logging/rh_logger.h"

namespace _seqVector {

template<typename _Tp>
    class seqVectorAllocator {
    public:
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef _Tp* pointer;
        typedef const _Tp* const_pointer;
        typedef _Tp& reference;
        typedef const _Tp& const_reference;
        typedef _Tp value_type;

        template<typename _Tp1>
        struct rebind {
            typedef seqVectorAllocator<_Tp1> other;
        };

        seqVectorAllocator() throw() {
        }

        seqVectorAllocator(const seqVectorAllocator&) throw() {
        }

        template<typename _Tp1>
        seqVectorAllocator(const seqVectorAllocator<_Tp1>&) throw() {
        }

        ~seqVectorAllocator() throw() {
        }

        pointer address(reference __x) const {
            return &__x;
        }

        const_pointer address(const_reference __x) const {
            return &__x;
        }

        // NB: __n is permitted to be 0.  The C++ standard says nothing
        // about what the return value is when __n == 0.
        pointer allocate(size_type __n, const void* = 0) {
            if (__builtin_expect(__n > this->max_size(), false))
                std::__throw_bad_alloc();
            return (_Tp*) new _Tp[__n];
        }

        // __p is not permitted to be a null pointer.
        void deallocate(pointer __p, size_type) {
            ::operator delete[](__p);
        }

        size_type max_size() const throw() {
            return size_t(-1) / sizeof(_Tp);
        }

        // _GLIBCXX_RESOLVE_LIB_DEFECTS
        // 402. wrong new expression in [some_] allocator::construct
        void construct(pointer __p, const _Tp& __val) {
            ::new (__p) _Tp(__val);
        }

        void destroy(pointer __p) {
            __p->~_Tp();
        }
    };

    template<typename _Tp>
    inline bool operator==(const seqVectorAllocator<_Tp>&,
            const seqVectorAllocator<_Tp>&) {
        return true;
    }

    template<typename _Tp>
    inline bool operator!=(const seqVectorAllocator<_Tp>&,
            const seqVectorAllocator<_Tp>&) {
        return false;
    }
} // namespace _seqVector

namespace redhawk {

    class PortCallError : public std::runtime_error {

    public:
        PortCallError( const std::string &msg, const std::vector<std::string> &connectionids );

        ~PortCallError() throw ();

    private:
        static std::string makeMessage(const std::string& msg, const std::vector<std::string>& connectionids);
    };

}

class Port_impl
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
: public virtual POA_CF::Port
#endif
{
public:
    Port_impl();
    ~Port_impl();
    void connectPort(CORBA::Object_ptr connection, const char* connectionId);
    void disconnectPort(const char* connectionId);
};

template <class PortType, class ComponentType>
class Port_Uses_impl
{
public:
    Port_Uses_impl(ComponentType* _parent, std::string port_name);
    ~Port_Uses_impl();
    void connectPort(CORBA::Object_ptr connection, const char* connectionId);
    void disconnectPort(const char* connectionId);
    void setActiveStatus(bool active_flag);
    void releasePort();
    std::vector< std::pair<class PortType::_var_type, std::string> > get_ports();
    // Return whether this Port is connected to another Port
    bool isActive();
    // Return the Port name
    std::string getName();

protected:
    // Pointer to the Component or Device that owns this Port
    ComponentType* parent;
    // Vector of all outgoing connections
    std::vector < std::pair<class PortType::_var_type, std::string> > outPorts;
    bool active;
    std::string name;
    boost::mutex updatingPortsLock;
    bool refreshSRI;
};

template <class PortType, class ComponentType>
Port_Uses_impl<PortType, ComponentType>::Port_Uses_impl(ComponentType* _parent, std::string port_name)
{
    parent = _parent;
    active = false;
    name = port_name;
};

template <class PortType, class ComponentType>
Port_Uses_impl<PortType, ComponentType>::~Port_Uses_impl()
{
};

template <class PortType, class ComponentType>
void Port_Uses_impl<PortType, ComponentType>::connectPort(CORBA::Object_ptr connection, const char* connectionId)
{
    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
    class PortType::_var_type port = PortType::_narrow(connection);
    outPorts.push_back(std::make_pair(port, connectionId));
    active = true;
    refreshSRI = true;
};

template <class PortType, class ComponentType>
void Port_Uses_impl<PortType, ComponentType>::disconnectPort(const char* connectionId)
{
    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
    for (unsigned int i = 0; i < outPorts.size(); i++) {
        if (outPorts[i].second == connectionId) {
            outPorts.erase(outPorts.begin() + i);
            break;
        }
    }

    if (outPorts.size() == 0) {
        active = false;
    }
};

template <class PortType, class ComponentType>
bool Port_Uses_impl<PortType, ComponentType>::isActive()
{
    return active;
};

template <class PortType, class ComponentType>
void Port_Uses_impl<PortType, ComponentType>::setActiveStatus(bool active_flag)
{
    active = active_flag;
};

template <class PortType, class ComponentType>
void Port_Uses_impl<PortType, ComponentType>::releasePort()
{
};

template <class PortType, class ComponentType>
std::vector< std::pair<class PortType::_var_type, std::string> > Port_Uses_impl<PortType, ComponentType>::get_ports()
{
    return outPorts;
};

template <class PortType, class ComponentType>
std::string Port_Uses_impl<PortType, ComponentType>::getName()
{
    return name;
};




template <class PortType, class ComponentType>
class Port_Provides_impl
{
public:
    Port_Provides_impl(ComponentType* _parent, std::string port_name);
    ~Port_Provides_impl();
    // Return this Port's name
    std::string getName();

protected:
    ComponentType* parent;
    std::string name;
};

template <class PortType, class ComponentType>
Port_Provides_impl<PortType, ComponentType>::Port_Provides_impl(ComponentType* _parent, std::string port_name)
{
    parent = _parent;
    name = port_name;
};

template <class PortType, class ComponentType>
Port_Provides_impl<PortType, ComponentType>::~Port_Provides_impl()
{
};

template <class PortType, class ComponentType>
std::string Port_Provides_impl<PortType, ComponentType>::getName()
{
    return name;
};


class PortBase
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
: public virtual PortableServer::ServantBase
#endif
{
public:
    PortBase (const std::string& name) :
        name(name), description("")
    {
    }

    virtual ~PortBase ()
    {
    }

    virtual void setDescription(const std::string& desc)
    {
        description = desc;
    }

    virtual void startPort ()
    {
    }

    virtual void stopPort ()
    {
    }

    // Called when the Port is first activated by a PortSupplier; subclasses
    // may override to implement virtual constructor behavior
    virtual void initializePort()
    {
    }

    virtual void releasePort()
    {
    }

    // Return the Port name
    virtual std::string getName ()
    {
        return name;
    }

    // Return the Port description
    virtual std::string getDescription ()
    {
        return description;
    }

    // Return the interface that this Port supports
    virtual std::string getRepid () const
    {
        return "IDL:CORBA/Object:1.0";
    }

    // Return the direction (uses/provides) for this Port
    virtual std::string getDirection() const 
    {
        return "Direction";
    }

    LOGGER getLogger();
    void setLogger(LOGGER newLogger);

protected:
    std::string name;
    std::string description;
    LOGGER _portLog;
};

class Port_Uses_base_impl : public PortBase
{
public:
    Port_Uses_base_impl(std::string port_name) :
        PortBase(port_name)
    {
        active = false;
    }

    virtual ~Port_Uses_base_impl()
    {
    }

    virtual void connectPort(CORBA::Object_ptr connection, const char* connectionId)
    {
    }

    virtual void disconnectPort(const char* connectionId)
    {
    }

    virtual void setActiveStatus(bool active_flag)
    {
        active = active_flag;
    }

    // Return true if this Port is connected to another Port
    virtual bool isActive()
    {
        return active;
    }

    // Return the direction (uses/provides) for this Port
    virtual std::string getDirection () const 
    {
        return CF::PortSet::DIRECTION_USES;
    }

protected:
    bool active;
    boost::mutex updatingPortsLock;
    bool refreshSRI;
};

class Port_Provides_base_impl : public PortBase
{
public:
    Port_Provides_base_impl(std::string port_name) :
        PortBase(port_name)
    {
    }

    virtual ~Port_Provides_base_impl()
    {
    }

    // Return the direction (uses/provides) for this Port
    virtual std::string getDirection () const 
    {
        return CF::PortSet::DIRECTION_PROVIDES;
    }
};


#endif                                            /*  */
