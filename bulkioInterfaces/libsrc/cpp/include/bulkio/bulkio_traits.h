/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef __bulkio_traits_h
#define __bulkio_traits_h

#include <vector>

#include <ossie/Port_impl.h> // for _seqVector

#include "BULKIO_Interfaces.h"
#include "bulkio_base.h"
#include "bulkio_datatransfer.h"

namespace bulkio {

template < typename TT, typename AT=_seqVector::seqVectorAllocator< TT > > 
  class DataTransferBuffer 
  {
  private:
  DataTransferBuffer(void) {};
  public:
  typedef TT                                            TransportType;
  typedef AT                                            AllocatorType;
  typedef std::vector< TransportType, AllocatorType >   Type;
};


//
// DataTransferTraits
//
// Traits template definition used to define input and output types used the port
// classes
//
template < typename PST, typename TT, typename NDT=TT, class DBT=std::vector< NDT > >
struct DataTransferTraits {
  typedef PST   PortSequenceType;                           // Port Sequence type used by middleware
  typedef TT    TransportType;                              // Transport Type contained in the Port Sequence container
  typedef NDT   NativeDataType;                             // Native c++ mapping of Transport Type
  typedef DBT   DataBufferType;                             // Container defintion to hold data from Input port
  typedef typename DBT::allocator_type AllocatorType;
};


typedef DataTransferTraits< PortTypes::CharSequence, CORBA::Char, Int8 >      CharDataTransferTraits;
typedef DataTransferTraits< CF::OctetSequence, CORBA::Octet >                 OctetDataTransferTraits;
typedef DataTransferTraits< PortTypes::ShortSequence, CORBA::Short >          ShortDataTransferTraits;
typedef DataTransferTraits< PortTypes::UshortSequence, CORBA::UShort >        UShortDataTransferTraits;
typedef DataTransferTraits< PortTypes::LongSequence, CORBA::Long >            LongDataTransferTraits;
typedef DataTransferTraits< PortTypes::UlongSequence, CORBA::ULong >          ULongDataTransferTraits;
typedef DataTransferTraits< PortTypes::LongLongSequence, CORBA::LongLong >    LongLongDataTransferTraits;
typedef DataTransferTraits< PortTypes::UlongLongSequence, CORBA::ULongLong >  ULongLongDataTransferTraits;
typedef DataTransferTraits< PortTypes::FloatSequence, CORBA::Float >          FloatDataTransferTraits;
typedef DataTransferTraits< PortTypes::DoubleSequence, CORBA::Double >        DoubleDataTransferTraits;
typedef DataTransferTraits< Char *, Char, Char, std::string >                 StringDataTransferTraits;

//
// PortTraits
// This template defines the set of traits used by Input and Output port template classes
//
//  POA = Portable Object Adapter Class
//  PT - BULKIO Port Type
//  DTT  DataTransferTraits  associated with port type
//     TransportType - TransportType defined by middleware
//     NativeType    - TransportType mapped to native type
//     PortSequenceType - Data container used by middleware to transfer TransportType objects
//     DataBufferType  - Data Container of the DataTransfer object returned from getPacket
//

template < typename POA, typename PT, typename DTT >
struct PortTraits {
  typedef POA POAPortType;
  typedef PT  PortType;
  typedef DTT  DataTransferTraits;
  typedef typename PortType::_var_type      PortVarType;
  typedef typename DTT::TransportType       TransportType;
  typedef typename DTT::NativeDataType      NativeType;
  typedef typename DTT::PortSequenceType    SequenceType;
  typedef typename DTT::DataBufferType      DataBufferType;
};


typedef PortTraits< POA_BULKIO::dataChar, BULKIO::dataChar, CharDataTransferTraits >                  CharPortTraits;
typedef PortTraits< POA_BULKIO::dataOctet, BULKIO::dataOctet, OctetDataTransferTraits >               OctetPortTraits;
typedef PortTraits< POA_BULKIO::dataShort, BULKIO::dataShort, ShortDataTransferTraits >               ShortPortTraits;
typedef PortTraits< POA_BULKIO::dataUshort, BULKIO::dataUshort, UShortDataTransferTraits >            UShortPortTraits;
typedef PortTraits< POA_BULKIO::dataLong,    BULKIO::dataLong,  LongDataTransferTraits >              LongPortTraits;
typedef PortTraits< POA_BULKIO::dataUlong,    BULKIO::dataUlong,  ULongDataTransferTraits >           ULongPortTraits;
typedef PortTraits< POA_BULKIO::dataLongLong, BULKIO::dataLongLong, LongLongDataTransferTraits >      LongLongPortTraits;
typedef PortTraits< POA_BULKIO::dataUlongLong, BULKIO::dataUlongLong, ULongLongDataTransferTraits >   ULongLongPortTraits;
typedef PortTraits< POA_BULKIO::dataFloat,   BULKIO::dataFloat,   FloatDataTransferTraits >           FloatPortTraits;
typedef PortTraits< POA_BULKIO::dataDouble,  BULKIO::dataDouble,  DoubleDataTransferTraits >          DoublePortTraits;

typedef PortTraits< POA_BULKIO::dataFile, BULKIO::dataFile,     StringDataTransferTraits >            URLPortTraits;
typedef PortTraits< POA_BULKIO::dataFile, BULKIO::dataFile,     StringDataTransferTraits >            FilePortTraits;
typedef PortTraits< POA_BULKIO::dataXML, BULKIO::dataXML,       StringDataTransferTraits >            XMLPortTraits;


typedef CharPortTraits     Int8PortTraits;
typedef OctetPortTraits    UInt8PortTraits;
typedef ShortPortTraits    Int16PortTraits;
typedef UShortPortTraits   Unt16PortTraits;
typedef LongPortTraits     Int32PortTraits;
typedef ULongPortTraits    Unt32PortTraits;
typedef LongLongPortTraits     Int64PortTraits;
typedef ULongLongPortTraits    Unt64PortTraits;

}  // end of bulkio namespace


#endif
