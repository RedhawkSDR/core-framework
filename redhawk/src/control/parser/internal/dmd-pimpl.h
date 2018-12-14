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

#ifndef CXX___XML_XSD_DMD_PIMPL_H
#define CXX___XML_XSD_DMD_PIMPL_H

#include "dmd-pskel.h"
#include <ossie/logging/rh_logger.h>

namespace dmd
{
  extern rh_logger::LoggerPtr parserLog;

  class domainmanagerconfiguration_pimpl: public domainmanagerconfiguration_pskel
  {
    public:
    virtual void
    pre ();

    virtual void
    description (const ::std::string&);

    virtual void
    domainmanagersoftpkg (const ::std::string& softpkg);

    virtual void
    services ();

    virtual void
    id (const ::std::string&);

    virtual void
    name (const ::std::string&);

    virtual std::auto_ptr<ossie::DomainManagerConfiguration::DMD>
    post_domainmanagerconfiguration ();

    private:
    std::auto_ptr<ossie::DomainManagerConfiguration::DMD> _data;

  };

  class domainmanagersoftpkg_pimpl: public virtual domainmanagersoftpkg_pskel
  {
    public:
    virtual void
    pre ();

    virtual void
    localfile (const ::std::string&);

    virtual ::std::string
    post_domainmanagersoftpkg ();
    private:
    std::string _localfile;
  };

  class localfile_pimpl: public virtual localfile_pskel
  {
    public:
    virtual void
    pre ();

    virtual void
    name (const ::std::string&);

    virtual ::std::string
    post_localfile ();
    private:
    std::string _name;
  };

  class services_pimpl: public virtual services_pskel
  {
    public:
    virtual void
    pre ();

    virtual void
    service ();

    virtual void
    post_services ();
  };

  class service_pimpl: public virtual service_pskel
  {
    public:
    virtual void
    pre ();

    virtual void
    usesidentifier (const ::std::string&);

    virtual void
    findby ();

    virtual void
    post_service ();
  };

  class findby_pimpl: public virtual findby_pskel
  {
    public:
    virtual void
    pre ();

    virtual void
    namingservice ();

    virtual void
    stringifiedobjectref (const ::std::string&);

    virtual void
    domainfinder ();

    virtual void
    post_findby ();
  };

  class namingservice_pimpl: public virtual namingservice_pskel
  {
    public:
    virtual void
    pre ();

    virtual void
    name ();

    virtual void
    post_namingservice ();
  };

  class domainfinder_pimpl: public virtual domainfinder_pskel
  {
    public:
    virtual void
    pre ();

    virtual void
    type (const ::std::string&);

    virtual void
    name (const ::std::string&);

    virtual void
    post_domainfinder ();
  };
}

#endif // CXX___XML_XSD_DMD_PIMPL_H
