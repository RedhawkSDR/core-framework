#ifndef BASE_PROGRAMMABLE_ENTRY_POINTS_H
#define BASE_PROGRAMMABLE_ENTRY_POINTS_H

// ************* AGREED UPON METHOD TO INSTANTIATE DEVICE FROM SHARED OBJECT *************
//
//   The following typedef defines how the shared object should be constructed.  Any
//   additional parameters may be passed in as long as each persona device supports the
//   construct method signature.  Any updates to the ConstructorPtr typedef requires a
//   change to the 'generateResource' method to pass in the additional parameters
//   defined in the 'ConstructorPtr' typedef.
//
//     IE: If a specific api/interface object is to be shared with each persona device,
//         the following changes are required
//
//          Within this entry point file:
//          typedef Device_impl* (*ConstructorPtr)(int, char*[], SharedAPIObjectType*)
//
//          Within 'generatePersona' method located in the resource cpp file:
//          Device_impl* personaPtr = personaEntryPoint(argc, argv, SharedAPIObject);
//
typedef Device_impl* (*ConstructorPtr)(int, char*[], Device_impl* parentDevice);

#endif // BASE_PROGRAMMABLE_ENTRY_POINTS_H
