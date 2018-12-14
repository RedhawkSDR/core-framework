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

#include <ossie/PropertyMap.h>

using namespace redhawk;

namespace {
    template <typename Iterator>
    static Iterator find_impl(Iterator start, const Iterator end, const std::string& id) {
        for (; start != end; ++start) {
            if (id == static_cast<const char*>(start->id)) {
                return start;
            }
        }
        return end;
    }
    template <typename Iterator>
    int find_offset(Iterator start, const Iterator end, const Iterator target) {
        unsigned int idx = 0;
        for (; start != end; ++start) {
            if (start == target) {
                return idx;
            }
            idx++;
        }
        return -1;
    }
}

PropertyMap::PropertyMap() :
    CF::Properties()
{
}

PropertyMap::PropertyMap(const CF::Properties& properties) :
    CF::Properties(properties)
{
}

PropertyMap& PropertyMap::operator=(const CF::Properties& properties)
{
    CF::Properties::operator=(properties);
    return *this;
}

bool PropertyMap::contains (const std::string& id) const
{
    return find(id) != end();
}

bool PropertyMap::empty () const
{
    return length() == 0;
}

size_t PropertyMap::size() const
{
    return length();
}

PropertyType& PropertyMap::operator[] (size_t index)
{
    return *(begin() + index);
}

const PropertyType& PropertyMap::operator[] (size_t index) const
{
    return *(begin() + index);
}

Value& PropertyMap::operator[] (const std::string& id)
{
    iterator dt = find(id);
    if (dt == end()) {
        CF::DataType property;
        property.id = id.c_str();
        push_back(property);
        dt = end()-1;
    }
    return dt->getValue();
}

const Value& PropertyMap::operator[] (const std::string& id) const
{
    const_iterator dt = find(id);
    if (dt == end()) {
        throw std::invalid_argument("id '"+id+"' not found");
    }
    return dt->getValue();
}

const Value& PropertyMap::get(const std::string& id, const Value& def) const
{
    const_iterator dt = find(id);
    if (dt != end()) {
        return dt->getValue();
    } else {
        return def;
    }
}

bool PropertyMap::operator==( const redhawk::PropertyMap &other ) const
{
    //
    // perform simple matching of a property map against another map
    //
    if ( size() != other.size() ) {
        return false;
    }

    if ( size() == 0 ) {
        return true;
    }

    for ( const_iterator iter = begin(); iter != end(); ++iter) {
        std::string pid(iter->getId());
        const_iterator other_prop = other.find( pid );
        if ( other_prop == other.end() ) {
            return false;
        }
        // perform  equal match values
        std::string action("eq");
        if (  !ossie::compare_anys(iter->getValue(), other_prop->getValue(), action)  ) {
            return false;
        }
    }

    return true;
}


bool PropertyMap::operator!=( const redhawk::PropertyMap &other ) const
{
    return !(*this == other);
}

void PropertyMap::update(const CF::Properties& properties)
{
    const PropertyMap& other = cast(properties);
    for (const_iterator prop = other.begin(); prop != other.end(); ++prop) {
        (*this)[prop->getId()] = prop->getValue();
    }
}

void PropertyMap::push_back(const CF::DataType& property)
{
    ossie::corba::push_back(*this, property);
}

void PropertyMap::extend(const CF::Properties& properties)
{
    ossie::corba::extend(*this, properties);
}

PropertyMap::iterator PropertyMap::begin()
{
    return static_cast<iterator>(this->get_buffer());
}

PropertyMap::iterator PropertyMap::end()
{
    return begin() + size();
}

PropertyMap::const_iterator PropertyMap::begin() const
{
    return static_cast<const_iterator>(this->get_buffer());
}

PropertyMap::const_iterator PropertyMap::end() const
{
    return begin() + size();
}

PropertyMap::iterator PropertyMap::find(const std::string& id)
{
    return find_impl(begin(), end(), id);
}

PropertyMap::const_iterator PropertyMap::find(const std::string& id) const
{
    return find_impl(begin(), end(), id);
}

void PropertyMap::erase(const std::string& id)
{
    erase(find(id));
}

void PropertyMap::erase(iterator pos)
{
    if (pos == end()) {
        return;
    }
    erase(pos, pos + 1);
}

void PropertyMap::erase(iterator first, iterator last)
{
    // Move items after range backwards to fill the gap
    std::copy(last, end(), first);

    // Resize to remove deleted items
    length(length()-(last-first));
}

std::string PropertyMap::toString() const
{
    std::ostringstream out;
    out << *this;
    return out.str();
}

std::ostream& redhawk::operator<<(std::ostream& out, const redhawk::PropertyMap& properties)
{
    out << "{";
    bool first = true;
    for (PropertyMap::const_iterator prop = properties.begin(); prop != properties.end(); ++prop) {
        if (!first) {
            out << ", ";
        }
        first = false;
        out << prop->getId() << "=" << prop->getValue().toString();
    }
    out << "}";
    return out;
}
