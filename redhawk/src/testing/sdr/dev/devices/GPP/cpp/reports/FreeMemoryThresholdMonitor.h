#ifndef FREE_MEMORY_THRESHOLD_MONITOR_H_
#define FREE_MEMORY_THRESHOLD_MONITOR_H_
#include <functional>
#include "ThresholdMonitor.h"

template< class REFERENCE_TYPE, class RETURN_TYPE=REFERENCE_TYPE, class ctype=uint64_t,  class CFUNC=std::divides< RETURN_TYPE >   >
class ConversionWrapper 
  {
  public:
  typedef REFERENCE_TYPE type;
  typedef RETURN_TYPE result_type;
  typedef void argument_type;
  typedef CFUNC      opfunc;

  explicit ConversionWrapper( type& ref, ctype cf=1048576, const opfunc &func=std::divides< result_type >() ): 
  ref_(ref), func_(func), unit_conversion_(cf) 
  {};

  result_type operator()() const { 
    return  func_( static_cast<result_type>(ref_), (result_type)unit_conversion_ );
  };

  type& get() const { return ref_; };

  private:
  type  &ref_; 
  opfunc  func_;
  ctype  unit_conversion_;

  };

class FreeMemoryThresholdMonitor : public GenericThresholdMonitor<int>
{
public:
  FreeMemoryThresholdMonitor( const std::string& source_id, QueryFunction threshold, QueryFunction measured ) ;

	static std::string GetResourceId(){ return "physical_ram"; }
	static std::string GetMessageClass(){ return "MEMORY_FREE"; }

};

#endif
