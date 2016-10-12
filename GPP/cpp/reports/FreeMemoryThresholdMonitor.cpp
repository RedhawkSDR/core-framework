#include "FreeMemoryThresholdMonitor.h"
#include "utils/ReferenceWrapper.h"

FreeMemoryThresholdMonitor::FreeMemoryThresholdMonitor( const std::string& source_id, QueryFunction threshold, QueryFunction measured ):
GenericThresholdMonitor<int64_t>(source_id, GetResourceId(), GetMessageClass(), threshold, measured )
{

}
