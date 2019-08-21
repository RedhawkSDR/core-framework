#ifndef TRANSPORT_API_TEST_H
#define  TRANSPORT_API_TEST_H

#include <algorithm>
#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include <ossie/debug.h>
#include <bulkio/bulkio.h>

void setUp_custom_priority(const std::string &priority="0" );
void setUp_disable_custom();
void setUp_transport_priority(const std::string &transport, const std::string &priority);
void setUp_disable_transport(const std::string &transport);
void setUp_disable_env( const std::string &envv);
void setUp_disable_cfg();
void setUp_bulkio_test();
void setUp_config( const std::string &cfgname="custom.udp.cfg");
void tearDown_reset_env();


class CustomOutFloatPort : public bulkio::OutFloatPort {
    public:
            virtual ~CustomOutFloatPort() {};
            CustomOutFloatPort(std::string port_name)
              : bulkio::OutFloatPort(port_name) {};

            virtual redhawk::UsesTransport* _createLocalTransport(PortBase* port, CORBA::Object_ptr object, const std::string& connectionId) {
                return 0;
            }
    };


namespace {

  template <typename E, typename A>
    bool arrayEquals (const E &expArray, size_t expIdx, size_t expLen,
		      const A &actArray, size_t actIdx, size_t actLen, std::string &msg ) {

      if ( (expLen-expIdx) != (actLen-actIdx) ){
          std::ostringstream os;
          os  << "Match ranges do not match exp: " << (expLen-expIdx) << " actual " << (actLen-actIdx);
          msg = os.str();
        return false;
      }
      else if (expArray.data() == actArray.data()) {
        return true;
      }
      else {
	for (size_t i = 0; i < expLen; i++) {
	  if (expArray[i+expIdx] != actArray[i+actIdx]) {
          std::ostringstream os;
          os  << "Match failed at exp("<< i+expIdx << ") " << expArray[i+expIdx] << " != act("<< i+actIdx << ") " << actArray[i+actIdx];
          msg = os.str();
	    return false;
          }
        }
      }
      return true;
    }


  template <typename E, typename A>
  bool arrayEquals ( const E &expArray, const A &actArray, std::string &msg ) {
      return arrayEquals(expArray, 0, expArray.size(), actArray, 0, actArray.size(), msg);
    }

    template <class T>
    bool overlaps(const redhawk::shared_buffer<T>& lhs, const redhawk::shared_buffer<T>& rhs)
    {
        const T* end;
        const T* ptr;
        if (lhs.data() < rhs.data()) {
            end = lhs.data() + lhs.size();
            ptr = rhs.data();
        } else {
            end = rhs.data() + rhs.size();
            ptr = lhs.data();
        }
        return (ptr < end);
    }
}


#define TEST_EACH_NUMERIC_PORT_TYPE(x) \
  x(Char,Char);			     \
  x(Octet,Octet);		     \
  x(Short,Short);		     \
  x(Ushort,UShort);		     \
  x(Long,Long);			     \
  x(Ulong,ULong);		     \
  x(LongLong,LongLong);		     \
  x(UlongLong,ULongLong);	     \
  x(Float,Float);		     \
  x(Double,Double);

#define TEST_EACH_PORT_TYPE(x)     \
    TEST_EACH_NUMERIC_PORT_TYPE(x) \
    x(File,File);		 \
    x(XML,XML);

#endif
