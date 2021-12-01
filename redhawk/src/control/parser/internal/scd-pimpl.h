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
// Not copyrighted - public domain.
//
// This sample parser implementation was generated by CodeSynthesis XSD,
// an XML Schema to C++ data binding compiler. You may use it in your
// programs without any restrictions.
//

#ifndef CXX___XML_XSD_SCD_PIMPL_H
#define CXX___XML_XSD_SCD_PIMPL_H

#include "scd-pskel.h"

// Create an scd namespace because there may be conflicts
// with parsers for other files (for instance the localfile_pimpl)
namespace scd {
    class softwarecomponent_pimpl: public softwarecomponent_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      corbaversion (const ::std::string&);

      virtual void
      componentrepid ();

      virtual void
      componenttype (const ::std::string&);

      virtual void
      componentfeatures ();

      virtual void
      interfaces ();

      virtual void
      propertyfile ();

      virtual std::unique_ptr<ossie::ComponentDescriptor::SCD>
      post_softwarecomponent ();

      private:
      std::unique_ptr<ossie::ComponentDescriptor::SCD> _data;
    };

    class propertyFile_pimpl: public virtual propertyFile_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      localfile ();

      virtual void
      type (const ::std::string&);

      virtual void
      post_propertyFile ();
    };

    class localFile_pimpl: public virtual localFile_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      name (const ::std::string&);

      virtual void
      post_localFile ();
    };

    class componentRepId_pimpl: public virtual componentRepId_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      repid (const ::std::string&);

      virtual void
      post_componentRepId ();
    };

    class componentFeatures_pimpl: public virtual componentFeatures_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      supportsinterface ();

      virtual void
      ports ();

      virtual void
      post_componentFeatures ();
    };

    class supportsInterface_pimpl: public virtual supportsInterface_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      repid (const ::std::string&);

      virtual void
      supportsname (const ::std::string&);

      virtual void
      post_supportsInterface ();
    };

    class ports_pimpl: public virtual ports_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      provides ();

      virtual void
      uses ();

      virtual void
      post_ports ();
    };

    class provides_pimpl: public virtual provides_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      description (const ::std::string&);

      virtual void
      porttype ();

      virtual void
      repid (const ::std::string&);

      virtual void
      providesname (const ::std::string&);

      virtual void
      post_provides ();
    };

    class uses_pimpl: public virtual uses_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      description (const ::std::string&);

      virtual void
      porttype ();

      virtual void
      repid (const ::std::string&);

      virtual void
      usesname (const ::std::string&);

      virtual void
      post_uses ();
    };

    class portType_pimpl: public virtual portType_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      type ();

      virtual void
      post_portType ();
    };

    class interfaces_pimpl: public virtual interfaces_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      interface ();

      virtual void
      post_interfaces ();
    };

    class interface_pimpl: public virtual interface_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      inheritsinterface ();

      virtual void
      repid (const ::std::string&);

      virtual void
      name (const ::std::string&);

      virtual void
      post_interface ();
    };

    class inheritsInterface_pimpl: public virtual inheritsInterface_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      repid (const ::std::string&);

      virtual void
      post_inheritsInterface ();
    };

    class type_pimpl: public virtual type_pskel,
      public ::xml_schema::nmtoken_pimpl
    {
      public:
      virtual void
      pre ();

      virtual void
      post_type ();
    };
}
#endif // CXX___XML_XSD_SCD_PIMPL_H
