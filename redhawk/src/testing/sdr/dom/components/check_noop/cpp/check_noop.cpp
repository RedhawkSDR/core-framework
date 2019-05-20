/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "check_noop.h"

PREPARE_LOGGING(check_noop_i)

check_noop_i::check_noop_i(const char *uuid, const char *label) :
    check_noop_base(uuid, label)
{
    // Avoid placing constructor code here. Instead, use the "constructor" function.

}

check_noop_i::~check_noop_i()
{
}

void check_noop_i::constructor()
{
    /***********************************************************************************
     This is the RH constructor. All properties are properly initialized before this function is called 
    ***********************************************************************************/
	iteration_number = -1;
}

int check_noop_i::serviceFunction()
{
    if (this->evaluate == "go") {
        if (iteration_number == -1) {
                if (this->noop_delay != -1)
                        this->serviceThread->updateDelay(this->noop_delay);
                clock_gettime(CLOCK_MONOTONIC, &ts_start);
        }
        iteration_number +=1;
        if (iteration_number == this->iterations) {
                this->evaluate = "done";
                clock_gettime(CLOCK_MONOTONIC, &ts_end);
                this->average_delay = (((float)ts_end.tv_sec*1e9+(float)ts_end.tv_nsec)-((float)ts_start.tv_sec*1e9+(float)ts_start.tv_nsec))/1e9;
                this->average_delay = this->average_delay/iteration_number;
                iteration_number = -1;
        }
    }

    return NOOP;
}

