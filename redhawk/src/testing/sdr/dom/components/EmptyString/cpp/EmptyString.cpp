/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "EmptyString.h"

PREPARE_LOGGING(EmptyString_i)

EmptyString_i::EmptyString_i(const char *uuid, const char *label) :
    EmptyString_base(uuid, label)
{
    // Avoid placing constructor code here. Instead, use the "constructor" function.
    estr="ctor-value";

}

EmptyString_i::~EmptyString_i()
{
}

void EmptyString_i::constructor()
{
}

int EmptyString_i::serviceFunction()
{

    return NOOP;
}
