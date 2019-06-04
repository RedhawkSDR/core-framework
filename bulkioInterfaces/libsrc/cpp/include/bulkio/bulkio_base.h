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

#ifndef __bulkio_base_h
#define __bulkio_base_h

#include <queue>
#include <list>
#include <vector>
#include <set>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/locks.hpp>
#include <ossie/debug.h>
#include <ossie/BULKIO/bio_runtimeStats.h>
#include <ossie/BULKIO/bulkioDataTypes.h>
#include "ossie/Autocomplete.h"


namespace bulkio {


  //
  // helper class to manage queue depth and back pressure
  //
  class  queueSemaphore;

  //
  // ConnectionList container,  (allows for template typedef defs to occur)
  //
 template < typename T > class Connections {
  private:
    Connections(void) {};

  public:

    /*
     * use end point definition for more descriptive error messages
    struct EndPoint {
      T            bio_port;
      std::string  cid;
      PT           cf_port;
    };

    typedef typename std::vector< EndPoint >  List;
    */

    typedef typename std::vector< std::pair< T, std::string > >  List;

  };

  //
  // Mapping of Stream IDs to SRI objects
  //
  typedef std::map< std::string, std::pair< BULKIO::StreamSRI, bool > >  SriMap;

  typedef std::vector< BULKIO::StreamSRI >                               SriList;

  //
  // Tracks an SRI and which connections have the most recent version
  //
  struct SriMapStruct {
    BULKIO::StreamSRI        sri;
    std::set<std::string>    connections;

    SriMapStruct( const BULKIO::StreamSRI &in_sri ) {
      sri = in_sri;
    };

    SriMapStruct( const SriMapStruct &src ) {
      sri = src.sri;
      connections = src.connections;
    };
  };


  // Standard struct property for multi-out support
  struct connection_descriptor_struct {
    connection_descriptor_struct ()
    {
    };

    static std::string getId() {
      return std::string("connection_descriptor");
    };

    std::string connection_id;
    std::string stream_id;
    std::string port_name;
  };

  //
  // Listing of Stream IDs for searching
  //
  typedef std::list < std::string >   StreamIDList;

  //
  //  Common Name for Mutex construct used by port classes
  //
  typedef boost::mutex                MUTEX;

  //
  //  Common Name for Condition construct used by port classes
  //
  typedef boost::condition_variable   CONDITION;

  //
  // Auto lock/unlock of mutex objects based on language scope
  //
  typedef boost::unique_lock< boost::mutex >   UNIQUE_LOCK;

  //
  // Auto lock/unlock of mutex objects based on language scope
  //
  typedef boost::mutex::scoped_lock           SCOPED_LOCK;


  //
  // Logging interface definition 
  //
  typedef LOGGER                     LOGGER_PTR;


  //
  //  Base Types used by Ports
  //
  typedef char                        Char;
  typedef int8_t                      Int8;
  typedef uint8_t                     UInt8;
  typedef int16_t                     Int16;
  typedef uint16_t                    UInt16;

  typedef int32_t                     Int32;
  typedef uint32_t                    UInt32;

  typedef int64_t                     Int64;
  typedef uint64_t                    UInt64;
  typedef float                       Float;
  typedef double                      Double;

  //
  // helper class for port statistics
  //
  class linkStatistics
  {
  public:

      linkStatistics(const std::string& portName, const int nbytes=1);

      linkStatistics();

      virtual ~linkStatistics() {};

      virtual void setEnabled(bool enableStats);

      virtual void setBitSize( double bitSize );

      virtual void update(unsigned int elementsReceived, float queueSize, bool EOS, const std::string &streamID, bool flush = false);

      StreamIDList getActiveStreamIDs(){return activeStreamIDs;};

      virtual BULKIO::PortStatistics retrieve();

      virtual uint64_t connectionErrors() { return connection_errors; };
      virtual uint64_t connectionErrors( const uint64_t n );
      virtual void     resetConnectionErrors() { connection_errors=0; };

     protected:

      struct statPoint {
        unsigned int elements;
        float queueSize;
        double secs;
        double usecs;
      };

      std::string  portName;
      bool enabled;
      int  nbytes;
      double bitSize;
      BULKIO::PortStatistics runningStats;
      std::vector< statPoint > receivedStatistics;
      StreamIDList activeStreamIDs;
      unsigned long historyWindow;
      int receivedStatistics_idx;
      uint64_t connection_errors;

      double flush_sec;                   // track time since last queue flush happened
      double flush_usec;                  // track time since last queue flush happened
  };


  class queueSemaphore {

  public:
    queueSemaphore(unsigned int initialMaxValue);

    void release();

    void setMaxValue(unsigned int newMaxValue);

    unsigned int getMaxValue(void);

    void setCurrValue(unsigned int newValue);

    void incr();

    void decr();

  private:
    unsigned int maxValue;
    unsigned int currValue;
    MUTEX mutex;
    CONDITION condition;

  };

  namespace Const {

    //
    // Maximum transfer size for middleware
    //
    const uint64_t  MAX_TRANSFER_BYTES =  omniORB::giopMaxMsgSize();

    //
    // Constant that defines if retrieval of data from a port's queue will NOT block
    //
    const  float    NON_BLOCKING = 0.0;

    //
    // Constant that defines if retrieval of data from a ports's queue will BLOCK
    //
    const  float    BLOCKING = -1.0;

    inline uint64_t        MaxTransferBytes() { return omniORB::giopMaxMsgSize(); };

  };


  /*
   *
   * Time Stamp Helpers
   *
   */
  namespace time {

    /*
     * PrecisionUTCTime object as defined by bulkio_dataTypes.idl, definition provided for information only
     *
     * Time code modes
     *
    const short TCM_OFF  = 0;
    const short TCM_CPU  = 1;
    const short TCM_ZTC  = 2;
    const short TCM_SDN  = 3;
    const short TCM_SMS  = 4;
    const short TCM_DTL  = 5;
    const short TCM_IRB  = 6;
    const short TCM_SDDS = 7;

    struct PrecisionUTCTime {
        short tcmode;        timecode mode 
        short tcstatus;      timecode status 
        double toff;         Fractional sample offset 
        double twsec;        J1970 GMT 
        double tfsec;        0.0 to 1.0 

    };

    **/

    namespace utils {

      /*
       * Create a time stamp object from the provided input... 
       */
      BULKIO::PrecisionUTCTime create(double wholeSecs=-1.0, double fractionalSecs=-1.0, CORBA::Short tsrc=BULKIO::TCM_CPU);

      /*
       * Create a time stamp object from the current time of day reported by the system
       */
      BULKIO::PrecisionUTCTime now();
      
      /*
       * Create a time stamp object from the current time of day reported by the system
       */
      BULKIO::PrecisionUTCTime notSet();

      /*
       * Return a new time stamp object which increments a given time stamp by numSamples*xdelta seconds
       */
      BULKIO::PrecisionUTCTime addSampleOffset( const BULKIO::PrecisionUTCTime &T, const size_t numSamples, const double xdelta  );

      /*
       * Adjust the whole and fractional portions of a time stamp object to
       * ensure there is no fraction in the whole seconds, and vice-versa
       */
      void normalize(BULKIO::PrecisionUTCTime& time);
    };


    /*
     * A default time stamp comparison method 
     */
    bool           DefaultComparator( const BULKIO::PrecisionUTCTime &a, const BULKIO::PrecisionUTCTime &b);

    /*
     * Method signature for comparing time stamp objects
     */
    typedef bool  (*Compare)( const BULKIO::PrecisionUTCTime &a, const BULKIO::PrecisionUTCTime &b);

  };


  /*
   * StreamSRI
   *
   * Convenience routines for building and working with StreamSRI objects
   *
   */
  namespace sri {

    /*
       StreamSRI object as defined by bulkio_dataTypes.idl, definition provided for information only

    struct StreamSRI {
        long hversion;     version of the StreamSRI header 
        double xstart;     start time of the stream 
        double xdelta;     delta between two samples 
        short xunits;      unit types from Platinum specification; common codes defined above 
        long subsize;      0 if the data is one dimensional; > 0 if two dimensional 
        double ystart;     start of second dimension 
        double ydelta;     delta between two samples of second dimension 
        short yunits;      unit types from Platinum specification; common codes defined above 
        short mode;        0-Scalar, 1-Complex 
        string streamID;   stream identifier 
        boolean blocking;  flag to determine whether the receiving port should exhibit back pressure
        sequence<CF::DataType> keywords;  user defined keywords 
    };
    */
 
    /*
     * Method signature for comparing sri objects
     */
    typedef bool  (*Compare)( const BULKIO::StreamSRI &a, const BULKIO::StreamSRI &b);

    // Bit flags for SRI fields
    enum {
      NONE     = 0,
      HVERSION = (1<<0),
      XSTART   = (1<<1),
      XDELTA   = (1<<2),
      XUNITS   = (1<<3),
      SUBSIZE  = (1<<4),
      YSTART   = (1<<5),
      YDELTA   = (1<<6),
      YUNITS   = (1<<7),
      MODE     = (1<<8),
      STREAMID = (1<<9),
      BLOCKING = (1<<10),
      KEYWORDS = (1<<11)
    };

    /*
     * Do a field-by-field comparison of two SRI streams
     */
    int compareFields(const BULKIO::StreamSRI& lhs, const BULKIO::StreamSRI& rhs);

    /*
     * Default comparator method when comparing SRI objects
     *
     * Performs a member wise comparision of a StreamSRI object. In addition to performing
     * this comparison, any additional key/value pairs will be compared. The key identifiers 
     * are compared in order, and their associated values are compared using the
     * equivalency method of the REDHAWK framework compare_anys method.
     */
    bool           DefaultComparator( const BULKIO::StreamSRI &a, const BULKIO::StreamSRI &b);
    
    /*
     * Zeroize an SRI stream
     */
    inline void zeroSRI(BULKIO::StreamSRI &sri) {
        sri.hversion = 1;
        sri.xstart = 0.0;
        sri.xdelta = 1.0;
        sri.xunits = 1;
        sri.subsize = 1;
        sri.ystart = 0.0;
        sri.ydelta = 1.0;
        sri.yunits = 1;
        sri.mode = 0;
        sri.streamID = "";
        sri.keywords.length(0);
    };
    
    /*
     * Zeroize a PrecisionUTCTime timestamp
     */
    inline void zeroTime(BULKIO::PrecisionUTCTime &timeTag) {
        timeTag = bulkio::time::utils::notSet();
        timeTag.tcmode = BULKIO::TCM_CPU;
    };
    
    /*
     * Create a SRI object with default parameters
     */
    BULKIO::StreamSRI create( std::string sid="defStream", const double srate = 1.0, const Int16 xunits = BULKIO::UNITS_TIME, const bool blocking=false );


  };
}  // end of bulkio namespace


#endif
