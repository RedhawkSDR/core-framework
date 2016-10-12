/*
 * necessary to allow FrontendTunerDevice template class to be split
 * into separate header and implementation files (.h and .cpp)
 */

#include "struct_props.h"
#include <frontend/fe_tuner_device.cpp>

template class frontend::FrontendTunerDevice<frontend_tuner_status_struct_struct>;

