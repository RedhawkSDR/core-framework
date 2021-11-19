# FrontEnd Interfaces - old

FrontEnd Interfaces (FEI) is a module containing interfaces designed to standardize the interaction between <abbr title="See Glossary.">applications</abbr> and radio hardware. This appendix specifies the requirements of an FEI 2.0 compatible <abbr title="See Glossary.">device</abbr>, explains best-practices, provides advice for development, and describes related data structures. This appendix is not intended to be an API reference for FEI or an exhaustive description of the Interface Description Language (IDL). This appendix is intended to provide an additional resource for developers.

## Theory of Operations

FEI were developed to standardize the allocation, operation, and development of tuner devices within the REDHAWK Core Framework (CF). Tuner devices in this context may consist of Radio Frequency (RF), Intermediate Frequency (IF), or purely digital tuning equipment or software. Explicit [types for tuners](#types-of-tuners) have been defined, so that devices can be easily categorized by the capabilities they provide.

Tuner devices can provide individual tuners to other REDHAWK entities through tuner allocation. To allocate an individual tuner, the `allocateCapacity()` function of a device is called with an appropriate allocation structure as the only argument. Devices then allocate physical resources and, once a valid connection has been made, begin flowing data out of the device.  

## Common FEI Terminology

The following table describes the FEI terminology used for tuner devices.

##### FEI Terminology
| **Terminology**  | **Description**                                                                                                                                |
| :--------------- | :--------------------------------------------------------------------------------------------------------------------------------------------- |
| device           | A REDHAWK device.                                                                                                                              |
| FEI device       | Devices that have a device_kind of FRONTEND and implement one of the FEI IDLs. Typically, GPS and navigation devices fall into this category. |
| tuner            | A specific tuner capability in an FEI device.                                                                                                  |
| FEI tuner device | Devices that have a device_kind of FRONTEND::TUNER. These devices must implement the TunerControl IDL and contain tuners for allocation.      |

## Required Properties for an FEI Tuner

The following table describes the required properties for an FEI tuner device.

##### Required Properties for an FEI Tuner
| **Name** / **ID**                                        | **Description**                                                                                                      |
| :------------------------------------------------------- | :------------------------------------------------------------------------------------------------------------------- |
| device_kind / DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d  | Must be set to FRONTEND or FRONTEND::TUNER.                                                                          |
| device_model / DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb | Used to specify the model of the hardware device.                                                                    |
| frontend_tuner_status / FRONTEND::tuner_status        | A struct sequence where each struct in the sequence represents a single tuner. The structure is defined further in [Status Elements](#status-elements). |

## Types of Tuners

Tuner types are defined so that device developers and users can categorize the basic behavior of disparate hardware. Behavior of each of these types is described below and must be adhered to during development to allow for interoperability between different hardware baselines.

Physical devices often need to be split into multiple logical REDHAWK tuners to fully describe their functionality. Splitting physical devices often involves multiple tuners of the same, or mixed, types. Common pairings are described in each of the type descriptions.

### RX Tuner

A simple receiver, or `RX` tuner, is an RF to IF conversion only. This conversion implies an analog to analog translation, typically down-converting an RF signal to a new IF. Devices often have multiple RX devices corresponding to each of the independent analog channels provided. Single channels that have selectable RF input <abbr title="See Glossary.">ports</abbr> should be represented as a single `RX` tuner and utilize an [RF Flow ID](#rf-flow-id) to select between the input options.

RX devices have an analog output port. RX devices that output digital-IF (SDDS, VITA-49, etc.) are considered RX_DIGITIZERS, which are a distinct tuner type and should be classified as such.

### RX_DIGITIZER Tuner

An `RX_DIGITIZER` tuner is an RX device that also samples the analog data and provides it as a digitized stream. The stream can be either real samples (typically referred to as digital-IF) or complex baseband data. Multiple `RX_DIGITIZER` channels in a singular physical device should be treated like RX devices, including using the RF Flow ID to differentiate between which external RF source to use.

`RX_DIGITIZER` tuners can also optionally provide access to the analog-IF output. If the device in question provides access to the analog-IF output, an additional analog output port should be added to the device. Users can optionally connect to and use that port, although the existence of the port is not guaranteed for all `RX_DIGITIZER` tuners.

### CHANNELIZER Tuner

A `CHANNELIZER` tuner takes a digital wideband input and provides tuned, filtered, and decimated narrowband output. A `CHANNELIZER` device acts as a token to allocate and control the wideband input, which then can have individual narrowband channels allocated as well. Each of these narrowband tuners is its own tuner of type Digital Down Converter (DDC).

Allocating a `CHANNELIZER` establishes control over the input to a `CHANNELIZER` and allows users to understand that they have the ability to attach a new stream to the wideband input. Typical operation is to allocate a `CHANNELIZER` to gain control over the input prior to connecting a data stream to the input, though the order is not mandatory.

Allocating a `DDC` provides the tuned, filtered, and decimated narrowband output. A `DDC` is allocated by calling `allocateCapacity()` with the tuner type set to `DDC` and also specifying the frequency, bandwidth, sample rate, and any other desired aspects of the resulting `DDC`. Optionally, the RF Flow ID is used during allocation to specify the wideband input connection and `CHANNELIZER` desired. Allocation of a DDC succeeds only if the information specified during allocation can be supported by an allocated `CHANNELIZER` with a successful matching input connection. Changing the `CHANNELIZER` input can cause attached `DDC` tuners to be dropped.

### DDC Tuner

`DDC` tuners provide a narrowband output from an existing `CHANNELIZER` capability. These narrowband channels are typically selectable in terms of the center frequency and bandwidth/sample rate within the constraints of the wideband input to the `CHANNELIZER`.

Allocating a `DDC` tuner against a device with multiple `CHANNELIZER` (or `RX_DIGITIZER_CHANNELIZER`) tuners can specify which tuner to use in two different ways. One method is to specify an RF Flow ID of the specific `CHANNELIZER` to be allocated against. The other method is to allow the device to match by using the requested center frequency and/or bandwidth/sample rate. The device matches the DDC to the first channelizer capability that meets those criteria.

### RX_DIGITIZER_CHANNELIZER Tuner

The `RX_DIGITIZER_CHANNELIZER` tuner is a combination of an `RX_DIGITIZER` and `CHANNELIZER` capability into a single tuner type. Input is through an analog-RF input port, and output is through `DDC` tuners.

`RX_DIGITIZER_CHANNELIZER` tuners have the optional ability to output both analog-IF as well as digital-IF data. The analog-IF ports are represented just like the output of an `RX` or `RX_DIGITIZER` tuner. The digital-IF data is accessed by allocating a listener to the `RX_DIGITIZER_CHANNELIZER`, which then starts flowing digital data just like an `RX_DIGITIZER`.


> **NOTE**  
> The primary `RX_DIGITIZER_CHANNELIZER` allocation does not put out wideband digital data by default. These are optional ports, and each may or may not be present on each device.  

### RX_SCANNER_DIGITIZER Tuner

The `RX_SCANNER_DIGITIZER` tuner is an `RX_DIGITIZER` that also has a scanning capability (usually a built-in hardware capability). `RX_SCANNER_DIGITIZER` tuners can follow a scan plan. This plan may be a list of discrete frequencies to dwell on or a regular pattern that is followed. The transition between frequencies is controlled by time or number of samples generated.


> **NOTE**  
> An `RX_SCANNER_DIGITIZER` allocation's `allocateCapacity()` call does not stand on its own. The call requires both a `FRONTEND::scanner_allocation` and a `FRONTEND::tuner_allocation` allocation.  

### TX Tuner

Although the exact functionality of the `TX` tuner is not yet defined, it is reserved for transmitter devices.

## Tuner Allocation

Allocation is the process where specific tuners are requested for use, and initial setup of the tuner is performed. Allocation of FrontEnd devices always occurs by calling `allocateCapacity()` on the device with the argument of the `allocateCapacity()` call being a FrontEnd allocation <abbr title="See Glossary.">property</abbr>.

### ID

A number of different IDs are used in REDHAWK and FEI devices. The most important types of IDs are described in this section.

#### Allocation ID

The Allocation ID is a string used as an unique ID for each allocated FEI tuner. The Allocation ID is passed in as part of the `allocateCapacity()` call and is used as identification for all subsequent interaction. Allocation IDs are used to identify tuners in the FEI Status structure and in the TunerControl IDL.

Allocation IDs can be any unique string, but typically UUID values are used. If human readable Allocation IDs are required, the preferred approach is to append the UUID onto a base human-readable string. With this approach, allocations for non-coordinated devices are not likely to create identical allocations.

#### Stream ID

In all REDHAWK Bulk Input/Output (BulkIO) ports, Stream ID is used to separate unique data streams that are passed in a single BulkIO port. This methodology is also utilized in FEI devices to allow devices to pass the output of multiple tuners through a single BulkIO port. REDHAWK does not mandate any uniqueness requirements on Stream IDs, but it is recommended that developers attempt to make the Stream IDs unique by appending a UUID.

Stream IDs can only be changed after an End of Stream (EOS) is sent, and in FEI devices, an EOS is only sent when deallocating or disabling the tuner.


> **NOTE**  
> An EOS should only be sent on an externally commanded tuner disable and not one that is handled internally to the device. Therefore, if the device must temporarily disable output to tune, an EOS should not be sent.  

#### Connection ID

In all REDHAWK BulkIO connections, there is a specific string ID for the connection. For FEI devices, the connection to the output port of the device must have a Connection ID, which can be assigned as described in [Connect Wizard](../ide/connect-wizard.html), that is equal to the Allocation ID of the tuner to be accessed. Because the Connection ID is equal to the Allocation ID, the output port can be constructed as a [multi-out port](#multi-output).

#### RF Flow ID

RF Flow IDs are used to differentiate between RF input streams, similarly to how Stream IDs are used to differentiate between BulkIO streams. For FrontEnd devices, these RF Flow IDs are used to match allocation requests to specific RF sources.

Unlike Allocation IDs and Stream IDs, RF Flow IDs are often human readable. Typically, they describe specific inputs to a device or feed names. RF Flow IDs for a device can be set as a device property, which is set as part of the <abbr title="See Glossary.">node</abbr> configuration. System developers can create a node that sets the individual FrontEnd device with the RF Flow IDs that are appropriately set to the current state of the physical RF connections.

#### Device Group ID

Device Group ID is similar to RF Flow ID in that it is used as an additional ID to allow allocations against a specific pool of devices. In this case, each FEI device can have its Group ID set, and individual allocations can request specific Group IDs. Devices with the same Group ID are considered to be in the same group, and only devices that are part of that group allow those allocations. Group IDs are strings that are often human readable. A device typically has its Group IDs set as a property, so it can be configured as part of a node.

### Tuner Allocation Overview

FrontEnd tuners are allocated by calling the device's `allocateCapacity()` method with a particular FrontEnd Allocation structure. The call acts as a request to the device for allocation of a tuner that meets the specifications in that structure. If the request is able to be met by that device, then the tuner is allocated, and True is returned by the `allocateCapacity()` call. If the tuner cannot be allocated, then an appropriate exception is thrown. Returns and exceptions are shown in [Allocation Return Types](#allocation-return-types).

Two different FrontEnd Allocation structures can be used. The `FRONTEND::tuner_allocation` structure is primarily used for allocating new tuners of an FEI device. The `FRONTEND::listener_allocation` structure is used for allocating listener tuners that piggyback on existing control tuners. Listener tuners are logical tuners that simply mirror the output of existing control tuners. Therefore, FrontEnd devices can keep track of how many consumers are attached to each of the tuner outputs and handle requests appropriately. The `FRONTEND::tuner_allocation` structure is described in [Tuner Allocation Properties](#tuner-allocation-properties), and the `FRONTEND::listener_allocation` structure is described in [Listener Allocation Properties](#listener-allocation-properties).

### Allocation Return Types
The following table describes the values returned and exceptions thrown by the `allocateCapacity()` method.

##### Allocation Return Types
| **Return Type**                 | **Description**              | **Notes**                                           |
| :------------------------------ | :--------------------------- | :------------------------------------------------- |
| `True` |  Tuner was successfully allocated.	  | Returned if the tuner was successfully allocated. Indicates that the tuner has been added to the status structure, the underlying hardware/software has been set up, and the tuner is now enabled. |
| `CF::Device::InvalidCapacity` | Capacity request was malformed.  |Indicates that there was an error in parsing the capacity (FrontEnd Allocation Structure) that was passed in. The error is typically the result of a missing Allocation ID or other critical field but can also indicate a duplicate Allocation ID request.|
| `CF::Device::InvalidState`  | Device is in an invalid state for allocation.  | Returned when the device is in an Error or Disabled state. |
| `False`  | Tuner was not successfully allocated.	| Indicates the device is unable to meet the request in the allocation structure. |

### Tuner Allocation Properties

Each `allocateCapacity()` call passes in a single `FRONTEND::tuner_allocation` structure. If multiple tuners are allocated, each tuner needs an independent `allocateCapacity()` call. Each field in the allocation structure needs to be matched against the available tuners in the device. Each field must correctly match for the allocation to be successful. If any of the properties cannot be met, the allocation throws an exception.

The following table describes the tuner elements of the `FRONTEND::tuner_allocation` structure and how to handle requests.

##### Tuner Allocation Properties
| **Name**          | **Type**       | **Notes**                 | **Description** |
| :-----------------| :--------------| :-------------------------| :---------------|
| `tuner_type`  |  `string` | 	Type of tuner requested.  | The [tuner type](#types-of-tuners) must match exactly and cannot be a super set of the request (e.g., an RX_DIGITIZER is not a match for an RX request).  |
| `allocation_id`   |  `string` | Used by the caller to reference the tuner uniquely.  | Reject any requests with an Allocation ID already in use with the InvalidCapacity exception.  |
| `center_frequency`   |  `double` | Requested center frequency in Hz.  | It is up to the device developer to determine the error tolerance of the true tuned center frequency. For CHANNELIZER tuners only, this is the expected input frequency of the stream that is provided.  |
| `bandwidth`   | `double`  | Requested Bandwidth in Hz.  | The minimum bandwidth that must be provided to allocate the channel. See tolerance for the upper bound of bandwidth. A value of zero indicates any bandwidth is acceptable.  |
| `bandwidth_tolerance`   | `double`   | Allowable Percent above requested bandwidth. | The acceptable amount of excess bandwidth used to allocate the channel. This amount is defined as a percentage of the requested bandwidth. For example, if a 1kHz bandwidth was required, and the tolerance was set to 50%, then any bandwidth between 1-1.5kHz is acceptable. For the CHANNELIZER only, this should be the expected bandwidth of the provided input stream.  |
| `sample_rate`   | `double`  | Requested complex sample rate in Hz.  | The minimum sample rate that must be provided to allocate the channel expressed in terms of complex sample rate. Refer to the tolerance for the upper bound of sample rate. A value of zero indicates any sample rate is acceptable. The sample_rate value can be ignored for such devices as analog tuners, which do not provide digital (sampled) data. For the CHANNELIZER only, this is the expected sample rate of the provided input stream. |
| `sample_rate_tolerance`   | `double`  | Allowable percent above requested sample rate.  | The amount of excess sample rate that is acceptable to allocate the channel as a percentage of the requested sample rate.  |
| `device_control`   | `boolean`  | Indicates if this tuner has modification control.  | Describes if the requested tuner has control over the tuner and can make changes to tuner parameters. Setting `device_control` to `false` indicates that the device should attempt to find an existing channel and add the allocated tuner as a listener. If no suitable tuners exist, the request fails. For CHANNELIZER tuners, `device_control` must always be set to true and, if not, an `InvalidCapacity` exception is thrown.  |
| `group_id`   | `string`  | Unique ID that specifies a group of devices.  | Must match `group_id` on the device; otherwise, allocation fails. The matching must be explicit, but a blank string is typically used to indicate a default group.  |
| `rf_flow_id`   | `string`  | Specifies a certain RF flow to allocate against.	  | Must match `rf_flow_id` on the device input used; otherwise, allocation fails. A blank string indicates that no channel is requested.  |

After successfully matching the allocation properties, the FrontEnd device needs to perform a number of tasks prior to returning from the `allocateCapacity()` call. First, the actual hardware or software should then be allocated with the values requested. Then, the internal [status properties](#status-elements) need to be updated to show the new channel. Once the status is set, the `allocateCapacity()` call can return.

After a successful allocation, the tuner is enabled and data is flowing though the device and out the output port. It is not required that the `allocateCapacity()` call return prior to the first data packet being pushed through the device.

In the event that a device has an RFInfo input port, the center frequency for allocation should be the true RF frequency of the signal even if the RFInfo port indicates that some frequency translation has occurred before input to the device. In this case, the tuner allocation frequency and status frequency will be different than the actual tuned frequency to which the hardware device was tuned. The center frequency and bandwidth of the RFInfo packet should be used to validate the requested center frequency. The requested center frequency should fit within the bandwidth of the incoming analog signal as described by the RFInfo packet. However, the requested bandwidth from the FEI allocation should be interpreted as the requested bandwidth of the receiver or receiver digitizer and can include bandwidth that is outside the bandwidth specified by the RFInfo packet.

### Scanner Allocation Properties

To allocate the scanning functionality in a tuner that supports automatic scanning, the `FRONTEND::scanner_allocation` property must be used. This property cannot be used on its own, it needs to be passed in the same `allocateCapacity()` call with the `FRONTEND::tuner_allocation` property described above.

The following table describes the scanning elements and how to handle requests.

##### Scanner Allocation Properties
| **Name**          | **Type**       | **Notes**                 | **Description** |
| :-----------------| :--------------| :-------------------------| :---------------|
| `min_freq`  |  `double` |   Requested lower edge of the scanning band.  | The center frequency of the scanning plan cannot go below this frequency. |
| `max_freq`  |  `double` |   Requested upper edge of the scanning band.  | The center frequency of the scanning plan cannot go above this frequency. |
| `mode`   |  `enum string` | SPAN_SCAN or DISCRETE_SCAN.  | Use SPAN_SCAN for a regularly-spaced set of frequencies and DISCRETE_SCAN for a discrete list of frequencies |
| `control_mode`   |  `enum string` | TIME_BASED or SAMPLE_BASED.  | Use TIME_BASED when the re-tune decision is based on the dwell time in seconds and SAMPLE_BASED when the re-tuning decision is made based on the number of samples produced. |
| `control_limit`   | `double`  | Limits on the control of the scanning device.  | Use TIME_BASED to establish the fastest hop rate and SAMPLE_BASED for the fewest number of samples that the scanner is expected to output before re-tuning. |

In the case of a scanning device, allocation is insufficient for defining the scan. The allocation data structure provides the bounds for the scanning strategy (e.g.: minimum and maximum frequency, fastest re-tuning rate). To define the scanning strategy such that the device will execute it, the strategy needs to be set through the `setScanStrategy` function in the `ScanningTuner` control port, as described in [Scanning Tuner Control Functions](#additional-scanner-tuner-control-functions).

### Listener Allocation Properties

There are two methods used to perform listener allocations. The first method is to use a standard FrontEnd Allocation Structure as shown in [Tuner Allocation Properties](#tuner-allocation-properties) with the control property set to False. Setting the control property to False causes the device to look for existing tuners that meet the required properties in the rest of the FrontEnd Allocation Structure and assign a new listener tuner to that master tuner. If no tuners already exist, the request fails.

The second way of allocating a listener tuner is by using a `FrontEnd::listener_allocation` structure in the `allocateCapacity()` call. Passing in this `FrontEnd::listener_allocation` structure causes the device to create a new listener tuner that is attached to the tuner with the Allocation ID given in the structure.  The following table describes the `FrontEnd::listener_allocation` structure.

##### FrontEnd Listener Allocation Structure
| **Name**                 | **Type** | **Description**                           | **Notes**                                                                                                              |
| :----------------------- | :------- | :---------------------------------------- | :--------------------------------------------------------------------------------------------------------------------- |
| `existing_allocation_id` | string   | Allocation ID for an existing allocation. | The Allocation ID of the tuner to which the new listener should attach. The tuner can be either a control or listener. |
| `listener_allocation_id` | string   | New Listener ID                           | The Allocation ID of the new listener tuner. All typical Allocation ID requirements apply.                             |

The requirements for tuner setup of a listener tuner are the same as those for a control tuner. The status structure must be updated, and the output must be enabled. These actions are expected to be performed prior to returning from `allocateCapacity()`.

## Tuner Deallocation

Tuners are deallocated using the `deallocateCapacity()` call. There is no return from the `deallocateCapacity()` function, but exceptions are thrown if deallocation is unsuccessful. The only exceptions that can be thrown are `InvalidCapacity`, which indicates that the Allocation ID provided is not valid, or `InvalidState`, indicating the device is in an error state.

The `deallocateCapacity()` call can accept either a `FRONTEND::tuner_allocation` structure or a `FRONTEND::listener_allocation` structure as only the Allocation ID field is utilized. All other fields are ignored and have no impact on the deallocation. The concept is that the same allocation structure provided to `allocateCapacity()` can be sent to `deallocateCapacity()` to remove the tuner.

Tuners that are deallocated need to have their status entries removed, the underlying hardware/software disabled, and all output stopped. A final BulkIO packet containing an EOS Signal Related Information (SRI) flag is sent prior to the return from the `deallocateCapacity()` call. More information on the BulkIO SRI information can be found in [FrontEnd-specific Keywords](#sri-and-keywords). No more data from that tuner flows though the BulkIO port.

### Automatic Listener Deallocation

When a control tuner is deallocated, its attached listeners are expected to also be deallocated. Therefore, both status cleanup and EOS information flows over the BulkIO port.

## Command and Control

Command and Control of existing allocated tuners is performed through the `DigitalTuner` or `AnalogTuner` port on the FEI device. These commands allow external users to get and set specific settings for each of the tuners. Each FEI tuner device must have a `DigitalTuner` port named `DigitalTuner_in` (or `AnalogTuner_in` for an `AnalogTuner` port) that allows for command and control. All of the functions in the tuner control interface need to be implemented even if only to report that the capability is not supported. Each of these tuner control interface functions uses the Allocation ID to uniquely identify the tuners.


> **NOTE**  
> An output control port used to control an FEI device follows the same connectivity rules explained in [Custom IDL Interfaces](../connections/custom-idl-interfaces.html).  

### Tuner Control Interface

The tuner control interface describes two interfaces for control. The first is the `AnalogTuner`, which describes all of the functions common for Digital and Analog tuners. `DigitalTuner` inherits `AnalogTuner` and adds `setOutputSampleRate()` and `getOutputSampleRate()`.

#### Tuner Control Functions

The following table describes the functions that can be accessed via the Tuner Control IDL.

##### Tuner Control Functions
| **Function Prototype**                 | **Description** |
| :----------------------- | :------- |
|`string getTunerType(in string id)`   | Get the type of tuner (e.g., RX or DDC) associated with this Allocation ID.  |
|`boolean getTunerDeviceControl(in string id)`   | Returns whether this Allocation ID has control (modification privileges) over the tuner.  |
|`string getTunerGroupId(in string id)`   | Retrieves the Group ID (may be empty) for this Allocation ID.  |
|`string getTunerRfFlowId(in string id)`   | Retrieves the RF Flow ID (may be empty) for this Allocation ID.  |
|`CF::Properties getTunerStatus(in string id)`   | Key/Value pair of entire tuner status structure. Note: The return is a sequence of simple properties, not a single struct property.  |
|`void setTunerCenterFrequency(in string id,  in double freq)`   | Set the current center frequency in Hz.  |
|`double getTunerCenterFrequency(in string id)`   | Get the current center frequency in Hz.  |
|`void setTunerBandwidth(in string id, in double bw)`   | Set the current bandwidth in Hz.  |
|`double getTunerBandwidth(in string id)`   | 	Get the current bandwidth in Hz.  |
|`void setTunerAgcEnable(in string id, in boolean enable)`  | 	Enable or disable the Auto Gain Control (AGC). True indicates that the AGC should be enabled.  |
|`boolean getTunerAgcEnable(in string id)` | 	Get the current status of AGC. True indicates enabled. |
|`void setTunerGain(in string id, in float gain)` | Set tuner gain in dB.  |
|`float getTunerGain(in string id)`| Get current tuner gain in dB.  |
|`void setTunerReferenceSource(in string id, in long source)`   | Set the tuner reference source. Zero is defined as internal and one is defined as external.  |
|`long getTunerReferenceSource(in string id)`   | Get the current tuner reference source.  |
|`void setTunerEnable(in string id,  in boolean enable)`   | Set the output enable state of the tuner. True indicates output is enabled.  |
|`boolean getTunerEnable(in string id)`   | Get the current output enable state of the tuner. True indicates output is enabled.  |
|`void setTunerOutputSampleRate(in string id, in double sr)`   | Set the output sample rate in samples/sec.  |
|`double getTunerOutputSampleRate(in string id)`  | Get the output sample rate in samples/sec.  |

#### Scanner Tuner Control Functions

The following table describes additional tuner control functions, which are present when the device is of type RX_SCANNER_DIGITIZER.

##### Scanner Tuner Control Functions
| **Function Prototype**                 | **Description** |
| :----------------------- | :------- |
|`ScanStatus getScanStatus(in string id)`   | Get the current scanner status. The return structure is of type FRONTEND::ScanningTuner::ScanStatus, which contains information on the scanning strategy, the scheduled start time, the list of center frequencies that the plan will execute, and whether or not the scan has started  |
|`void setScanStartTime(in string id, in BULKIO::PrecisionUTCTime start_time)`   | Schedule when a scan plan should start (in epoch time). Setting the time to 0 or a previous time with the tcstatus flag set to true starts a scan immediately. To disable the scan, set the start_time's tcstatus flag to false. |
|`void setScanStrategy(in string id, in ScanStrategy scan_strategy)`   | Provide a plan for what frequencies the scanner will cover and how it will cover them. |

#### Tuner Control Exceptions

The following table describes exceptions that may occur during calls to tuner control functions.

##### Tuner Control Exceptions
| **Exception**           | **Description**                | **Notes**                                                                                                                                  |
| :---------------------- | :----------------------------- | :----------------------------------------------------------------------------------------------------------------------------------------- |
| `BadParameterException` | Parameter provided is invalid. | Indicates the value provided is out of bounds for the capability of the device or that the value was invalid (e.g., a negative frequency). |
| `NotSupportedException` | Capability is not supported.   | Indicates the tuner does not support the setting (or getting) of this capability.                                                          |
| `FrontendException`     | Generic FrontEnd exception.    | Indicates there is a FrontEnd issue preventing the command, often because the Allocation ID does not match any currently allocated tuners. |

### Scanning Interface

A scanning tuner requires a definition of how it should scan.  This is provided with the `ScanStrategy` data structure.

#### ScanStrategy Description

The following table describes the members of the `ScanStrategy` data structure.

##### ScanStrategy Description
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`scan_mode` | `ScanMode`  | ScanMode is an enumerated type that can be set to MANUAL_SCAN, SPAN_SCAN, or DISCRETE_SCAN  |
|`scan_definition` | `ScanModeDefinition` | ScanModeDefinition is a union that provides the mode-specific information: `double center_frequency` for MANUAL_SCAN, `ScanSpanRanges freq_scan_list` for SPAN_SCAN, and `Frequencies discrete_freq_list` for DISCRETE_SCAN |
|`control_mode` | `OutputControlMode` | OutputControlMode is an enumerated type that can be set to TIME_BASED or SAMPLE_BASED |
|`control_value` | `double` | This is the value for control_mode. The unit is seconds for TIME_BASED and samples for SAMPLE_BASED |

#### ScanSpanRange Description

The data structures `Frequencies` and `ScanSpanRanges` provide more detail for their respective modes. `Frequencies` is a sequence of doubles. `ScanSpanRanges` is a sequence of `ScanSpanRange`.  The following table describes the members of `ScanSpanRange`.

##### ScanSpanRange Description
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`begin_frequency` | `double`  | The beginning center frequency for the scan in Hz  |
|`end_frequency` | `double` | The ending center frequency for the scan in Hz |
|`step` | `double` | The change in center frequency in Hz |

### GPS Interface

The GPS IDL provides an interface to retrieve GPS information from a device that is GPS-enabled. The GPS interface is composed of two attributes and no functions. However, the data structures returned by these attributes contain a large amount of detail.


> **NOTE**  
> This interface's attributes are read/write, therefore, this interface can be used to read GPS information or to receive GPS information.  

#### GPS Attributes

The GPS attributes are listed in the following table.

##### GPS Attributes
| **Attribute Prototype**                 | **Description** |
| :----------------------- | :------- |
|`GPSInfo gps_info`   | Get a detailed description of the GPS information on the device.  |
|`GpsTimePos gps_time_pos`   | Return the position generated by GPS as well as the timestamp for that position estimate.  |

#### GPSInfo

The GPSInfo descriptions are listed in the following table.

##### GPSInfo Description
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`source_id` | `string`  | Device identifier for the device that generated the GPS location report (this device's ID if accessing the hardware directly) |
|`rf_flow_id` | `string` | Identifier for the RF source (antenna) |
|`mode` | `string` | "Locked" if the GPS Receiver has locked onto the signal, and "Unlocked" if the GPS Receiver has not locked onto the signal. Use "Tracking" if the GPS Receiver has found but not locked onto the signal (if available). |
|`fom` | `long` | Position figure-of-merit (refer to the Figure of Merit table) |
|`tfom` | `long` | Time figure-of-merit (refer to the Time Figure of Merit table) |
|`datumID` | `long` | Identifier for the reference ellipsoid (datum). Use 47 for WGS 1984, the GPS datum |
|`time_offset` | `double` | Receiver oscillator's most recent time offset (seconds). Usually 0 |
|`freq_offset` | `double` | Receiver's center frequency offset (Hz) |
|`time_variance` | `double` | Receiver oscillator's time offset variance (seconds**2). Usually 0 |
|`freq_variance` | `double` | Receiver's center frequency offset variance (Hz**2) |
|`satellite_count` | `short` | Number of satellites visible to the receiver |
|`snr` | `float` | GPS receiver's reported signal to noise ratio. The definition of this value is not standardized and varies by manufacturer. |
|`status_message` | `string` | Device-specific status message |
|`timestamp` | `BULKIO::PrecisionUTCTime` | Timestamp for the GPS information |
|`additional_info` | `CF::Properties` | Device-specific additional information |

#### Figure of Merit

The figure of merit (fom) provides the expected position error (EPE).  The following table describes the possible values of the `fom`.

##### Figure of Merit
| **Value**       | **Error (meters)**          |
| :----------------------- | :------- |
| 1 | less than 25  |
| 2 | less than 50  |
| 3 | less than 75  |
| 4 | less than 100  |
| 5 | less than 200  |
| 6 | less than 500  |
| 7 | less than 1000  |
| 8 | less than 5000  |
| 9 | greater than or equal to 5000  |

#### Time Figure of Merit

The time figure of merit (tfom) provides the expected time error (ETE).  The following table describes the possible values of the `tfom`.

##### Time Figure of Merit
| **Value**       | **Error (nanoseconds)**          |
| :----------------------- | :------- |
| 1 | less than 1 |
| 2 | less than 10 |
| 3 | less than 100 |
| 4 | less than 1e3 |
| 5 | less than 1e4 |
| 6 | less than 1e5 |
| 7 | less than 1e6 |
| 8 | less than 1e7 |
| 9 | greater than or equal to 1e7 |

#### GPS Interface Helpers

The GPS Interface helpers identify whether the GPS receiver has locked in on a signal or only found the receiver.  The helper details are listed in the table below.

##### Helper Definitions
| **Member**       | **Value**          | **Description** |
| :----------------------- | :------- | :------- |
|`GPS_MODE_LOCKED` | `Locked`  | GPS Receiver has locked onto the signal |
|`GPS_MODE_UNLOCKED` | `Unlocked` | GPS Receiver has not locked onto the signal |
|`GPS_MODE_TRACKING` | `Tracking` | GPS Receiver has found, but not locked onto the signal (optional) |

#### GpsTimePos

The GPSTimePos reports the location and the timestamp associated with the location report.  The GPSTimePos details are listed in the table below.

##### GpsTimePos Description
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`position` | `PositionInfo`  | Position report |
|`timestamp` | `BULKIO::PrecisionUTCTime` | Timestamp for the location report |

#### GPS Support Types

The following table lists the type used by GpsTimePos for communicating position information.

##### PositionInfo
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`valid` | `boolean`  | The report is valid |
|`datum` | `string` | Reference ellipsoid. Set to DATUM_WGS84 |
|`lat` | `double` | Latitude (degrees) |
|`lon` | `double` | Longitude (degrees) |
|`alt` | `double` | Altitude (meters) |

### NavData Interface

This interface provides basic navigational information such as position, velocity, and so forth. The NavData interface is composed of one attribute and no functions. While the interface returns a single attribute, that attribute contains multiple positional structures, some of which may not be supported by the navigation data source. Structures contain a "valid" flag, allowing the hardware to indicate whether or not the source provides that specific data set.


> **NOTE**  
> This interface's attributes are read/write, therefore, this interface can be used to read navigation information or to receive navigation information.  

#### NavData Attribute

The NavData attribute is used to retrieve additional detailed navigation information from devices.

##### NavData Attribute
| **Attribute Prototype**                 | **Description** |
| :----------------------- | :------- |
|`NavigationPacket nav_packet`   | Get detailed navigational information from the device.  |

#### NavigationPacket

The NavigationPacket provides all of the detail required for communicating navigational information for a device.

##### NavigationPacket Description
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`source_id` | `string`  | Device identifier for the device that generated the navigation report (this device's ID if accessing the hardware directly) |
|`rf_flow_id` | `string` | Identifier for the RF source (antenna) |
|`position` | `PositionInfo` | Location information in lat/long/altitude |
|`cposition` | `CartesianPositionInfo` | Location information in x/y/z |
|`velocity` | `VelocityInfo` | Velocity vector |
|`acceleration` | `AccelerationInfo` | Acceleration vector |
|`attitude` | `AttitudeInfo` | Attitude info (pitch/yaw/roll) |
|`timestamp` | `BULKIO::PrecisionUTCTime` | Timestamp for the navigation data information |
|`additional_info` | `CF::Properties` | Device-specific additional information |

#### NavData Support Types

The following tables list the types that are used by the `NavigationPacket` for communicating position, velocity, acceleration, and attitude information.

##### PositionInfo
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`valid` | `boolean`  | The report is valid |
|`datum` | `string` | Reference ellipsoid. Set to DATUM_WGS84 |
|`lat` | `double` | Latitude (degrees) |
|`lon` | `double` | Longitude (degrees) |
|`alt` | `double` | Altitude (meters) |

##### CartesianPositionInfo
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`valid` | `boolean`  | The report is valid |
|`datum` | `string` | Reference ellipsoid. Set to DATUM_WGS84 |
|`x` | `double` | X-axis (meters) |
|`y` | `double` | Y-axis (meters) |
|`z` | `double` | Z-axis (meters) |

##### VelocityInfo
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`valid` | `boolean`  | The report is valid |
|`datum` | `string` | Reference ellipsoid. Set to DATUM_WGS84 |
|`coordinate_system` | `string` | CS_ECF (Earth-Centered Earth-Fixed), CS_ENU (East, North, Up), CS_NED (North, East, Down) |
|`x` | `double` | X-axis (meters/second) |
|`y` | `double` | Y-axis (meters/second) |
|`z` | `double` | Z-axis (meters/second) |

##### AccelerationInfo
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`valid` | `boolean`  | The report is valid |
|`datum` | `string` | Reference ellipsoid. Set to DATUM_WGS84 |
|`coordinate_system` | `string` | CS_ECF (Earth-Centered Earth-Fixed), CS_ENU (East, North, Up), CS_NED (North, East, Down) |
|`x` | `double` | X-axis (meters/second^2) |
|`y` | `double` | Y-axis (meters/second^2) |
|`z` | `double` | Z-axis (meters/second^2) |

##### AttitudeInfo
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`valid` | `boolean` | The report is valid |
|`pitch` | `double` | Pitch (degrees) |
|`yaw`   | `double` | Yaw (degrees) |
|`roll`  | `double` | Roll (degrees) |

### RFInfo Interface

The RFInfo interface describes the contents of an RF feed (antenna/feed); the most direct analogy is that RFInfo describes a wire connecting an antenna to a tuner.


> **NOTE**  
> This interface's attributes are read/write, therefore, this interface can be used to send RF feed information or to describe the RF feed information being received.  

#### RFInfo Attributes

The RFInfo interface implements the attributes listed in the table below.

##### RFInfo Attributes
| **Attribute Prototype**                 | **Description** |
| :----------------------- | :------- |
|`string rf_flow_id`   | A string that uniquely describes the feed |
|`RFInfoPkt rfinfo_pkt`   | A description of the RF feed |

#### Support Types

The following tables list the types that are used by the RFInfo for communicating RF, sensor, antenna, feed, and frequency, path delay, and frequency range information.

##### RFInfoPkt
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`rf_flow_id` | `string` | The string that uniquely describes the feed |
|`rf_center_freq` | `double` | Center frequency for the RF Feed (the physical antenna) |
|`rf_bandwidth`   | `double` | Bandwidth for the RF Feed (the physical antenna) |
|`if_center_freq`  | `double` | Center frequency for the IF that is output by the feed (if the hardware performs a down-conversion) |
|`spectrum_inverted` | `boolean` | Set to true if the spectrum for the feed output is inverted |
|`sensor` | `SensorInfo` | A description of the antenna/feed combination |
|`ext_path_delays`   | `sequence PathDelay` | Frequency-selective path delays |
|`capabilities`  | `RFCapabilities` | Potential operating frequency range for the antenna/feed combination |
|`additional_info` | `CF::Properties` | Device-specific additional information |

##### SensorInfo
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`mission` | `string` | Name for the mission |
|`collector` | `double` | Name for the collector |
|`rx`   | `double` | Name for the rx element |
|`antenna`  | `AntennaInfo` | Description of the antenna's RF characteristics |
|`feed` | `FeedInfo` | Description of the feed's RF characteristics |

##### AntennaInfo
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`name` | `string` | Name for the antenna |
|`type` | `string` | Type of antenna |
|`size`   | `string` | Size of the antenna |
|`description`  | `string` | Description of the antenna installation |

##### FeedInfo
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`name` | `string` | Name for the feed |
|`polarization` | `string` | Polarization (for example, "vertical", "horizontal") |
|`freq_range`   | `FreqRange` | Operating range for the RF feed |

##### FreqRange
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`min_val` | `double` | Minimum frequency |
|`max_val` | `double` | Maximum frequency |
|`values`   | `sequence double` | List of specific center frequencies that are available (if not continuous) |

##### PathDelay
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`freq` | `double` | Frequency where this delay applies |
|`delay_ns` | `double` | Delay at the given frequency (nanoseconds) |

##### RFCapabilities
| **Member**       | **Type**          | **Description** |
| :----------------------- | :------- | :------- |
|`freq_range` | `FreqRange` | Minimum to Maximum center frequency |
|`bw_range` | `FreqRange` | Minimum to Maximum operating bandwidth |

### RFSource Interface

The RFSource interface describes a device that contains multiple RF feeds and any one can be selected at any one time. This is equivalent to an RF switch connected to multiple antenna subsystems.


> **NOTE**  
> This interface's attributes are read/write, therefore, they are meant to be used to control the RF source (RF switch) to select the appropriate RF stream to feed to the tuner.  

#### RFSource Attributes

The RFSource interface implements the attributes listed in the table below.

##### RFSource Attributes
| **Attribute Prototype**                 | **Description** |
| :----------------------- | :------- |
|`RFInfoPktSequence available_rf_inputs`   | The list of RF feeds that can be selected. Each RF feed is described by an RFInfoPkt structure |
|`RFInfoPkt current_rf_input`   | The RF feed that is currently streaming out of the device |

## Data Flow

By default, FEI devices can provide BulkIO data. Data flow into FEI devices is typically constrained to digital-IF data for CHANNELIZER tuners. Digital-IF data is consumed regardless of Stream ID and it is the responsibility of the CHANNELIZER allocator to make the connection from the source into the FrontEnd device.

Output data flow is managed by the FEI device and multiple tuners can have output through a single BulkIO port. In general, there should only be one BulkIO port for each data type (int, float, etc.). All streams of any data type must pass through this BulkIO port. The allocator of each tuner is responsible for connecting to the device output port. The Connection ID of the BulkIO connection must match the Allocation ID of the tuner for data to flow through that connection. Thus, the output port must be a multi-out port, one that only passes data packets to specific connections rather than all connections.

### Multi-Output

Multi-out ports are a specific implementation of a BulkIO output port. The defining characteristic is that multi-out ports are selective in which connection each piece of data is sent to rather than having each piece of data broadcast to every connection. This behavior is typically implemented by creating a custom BulkIO port implementation that handles data routing for each connection.

Constructing the custom port is easiest if the internal `pushPacket()` call does not need to be modified, which is why Stream ID is the easiest field on which to differentiate. As long as the Stream ID is equal to the Allocation ID,  the Stream ID can simply be matched to the Connection ID to make a multi-out port. Hence, a REDHAWK best practice is to maintain a one-to-one relationship between Stream and Allocation IDs.

If the IDs are the same, there are only two required changes to the standard port implementation. The first change is that as the port iterates over the list of connections, a simple check needs to be performed to only do the remote `pushPacket()` or `pushSRI()` call on connections where the Connection ID matches the Allocation ID associated with the current packet. The second change is that the internal storage of previous SRI states needs to become a vector rather than a single element so that a new connection gets the correct previous SRI for its stream rather than just the most recent stream that has had data sent.

### SRI and Keywords

FrontEnd devices use both the basic BulkIO SRI fields and the Keywords field for additional items. All standard SRI fields are filled out as appropriate in accordance with the existing BulkIO IDL descriptions.  The following table describes the specific keywords for FrontEnd devices.

##### FrontEnd-Specific Keywords
| **Name**               | **Type** | **Description**                   | **Notes**                                                                                                                                                                                                                                                  |
| :--------------------- | :------- | :-------------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `COL_RF`               | double   | Collector center frequency in Hz. | Center frequency of the collector, which is typically thought of as the center frequency of the wideband receiver used to generate the IF data. In the case of a DDC tuner, the value of `COL_RF` is the center frequency of the input to the CHANNELIZER. |
| `CHAN_RF`              | double   | Channel center frequency in Hz.   | The center frequency of the stream. The value of `CHAN_RF` is equal to the COL_RF for RX and RX_DIGITIZER tuners but should still be included.                                                                                                           |
| `FRONTEND::BANDWIDTH`  | double   | Effective bandwidth in Hz.        | The effective bandwidth of the stream.                                                                                                                                                                                                                     |
| `FRONTEND::RF_FLOW_ID` | string   | RF Flow ID of data.               | Always include even if the RF Flow ID is blank.                                                                                                                                                                                                            |
| `FRONTEND::DEVICE_ID`  | string   | The ID of the device.             | <abbr title="See Glossary.">Component</abbr> ref ID,  which allows downstream users to gain a reference to the device that created the data.                                                                                                                                                   |

## Status

Recall that `frontend_tuner_status` is a sequence of structs.  Each FEI tuner device reports its status in one of these structs.  This includes both allocated and un-allocated tuners so users can see tuners available for allocation.

### Status Elements

Each struct in the `frontend_tuner_status` sequence contains all of the required elements listed below, and may contain any number of the optional elements.  Additional elements are permitted in the structures if a suitable element is not already defined.

#### Required Status Elements

The following table describes the required status elements for each tuner of an FEI tuner device. In some cases, null, zero, or blank values may be used to indicate that a value is not set for this device.

##### Required Status Elements of an FEI Tuner Device
| **Name**            | **Type** | **Description**                                  | **Notes**                                                                                                                                                                                                                                               |
| :------------------ | :------- | :----------------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `tuner_type`        | string   | Type description of tuner.                       | Defined in [Types of Tuners](#types-of-tuners)                                                                                                                                                                                                                     |
| `allocation_id_csv` | string   | Comma separated list of current Allocation IDs.  | Contains a list of both control and listener Allocation IDs. In effect, the length of the `allocation_id_csv` list is the number of independent consumers of the tuner output. The control Allocation ID must be the first in the comma separated list. |
| `center_frequency`  | double   | Current center frequency in Hz.                  | Actual tuned frequency rather than the desired frequency (if those values are not the same).                                                                                                                                                            |
| `bandwidth`         | double   | Current bandwidth in Hz                          | Actual bandwidth rather than the desired bandwidth (if those values are not the same).                                                                                                                                                                  |
| `sample_rate`       | double   | Current sample rate in Hz.                       | Actual sample rate rather than the desired sample rate (if those values are not the same). Can be ignored for such devices as analog tuners                                                                                                             |
| `group_id`          | string   | Unique ID that specifies a group of devices.     | Actual Group ID,  regardless whether it was requested in the tuner allocation or not.                                                                                                                                                                    |
| `rf_flow_id`        | string   | Specifies a certain RF flow to allocate against. | Actual RF Flow ID,  regardless whether it was requested in the tuner allocation or not.                                                                                                                                                                  |
| `enabled`           | boolean  | Indicates if tuner is enabled.                   | Enabled refers to the output state not any internal hardware/software state.                                                                                                                                                                            |

#### Additional Required Status Elements of an FEI Scanner Device

The following table describes additional fields required in the tuner status structure when a device is of type `RX_SCANNER_DIGITIZER`.

##### Additional Required Status Elements of an FEI Scanner Device
| **Name**            | **Type** | **Description**                                  | **Notes**                                                                                                                                                                                                                                               |
| :------------------ | :------- | :----------------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `scan_mode_enabled`        | boolean   | Describes whether or not a scan plan is running on this tuner.                       |  |
| `supports_scan`            | boolean   | Describes whether or not this tuner can support a scan plan.  | Scan plans may not necessarily be available to all tuners in a device. |

#### Optional Status Elements

 The following table describes the optional status elements for each tuner of an FEI tuner device. In some cases, null, zero, or blank values may be used to indicate that a value is not set for an individual tuner.  Note that all tuners of the same device must have the same set of properties in their status structure.

##### Optional Status Elements of an FEI Tuner Device
| **Name**            | **Type** | **Description**                                  | **Notes**                                                                                                                                                                                                                                               |
| :------------------ | :------- | :----------------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
|`bandwidth_tolerance`   | `double`  | Allowable percentage over requested bandwidth.	  | Tolerance provided by the requester.  |
|`sample_rate_tolerance`   |  `double` | Allowable percentage over requested sample rate.	  | Tolerance provided by the requester.  |
|`complex`   | `boolean`  | Indicates if the output data is complex.	  | True for complex; False for real.  |
|`gain`   | `double`  | Current gain in dB.	  | N/A  |
|`agc`   | `boolean` | Indicates if the tuner has AGC enabled.  | Even if AGC is enabled, the device still reports the current gain in the gain property.  |
|`valid`   | `boolean`  | Indicates if the tuner is in a valid state.	  | When the tuner is of type DDC, False indicates that the DDC channel is no longer able to tune to the appropriate frequency because the CHANNELIZER it is attached to has been moved.  |
|`available_frequency`   | `string`  | Valid potential center frequencies for the tuner in Hz. | In range(XX-YY) or csv (X,Y,Z) format.  |
|`available_bandwidth`   | `string`  | Valid potential bandwidth for the tuner in Hz.	  | In range(XX-YY) or csv (X,Y,Z) format.  |
|`available_gain`   | `string`  | Valid potential gain for the tuner in dB.	  | In range(XX-YY) or csv (X,Y,Z) format. |
|`available_sample_rate`   | `string`  | Valid potential sample rates for the tuner.	  | In range(XX-YY) or csv (X,Y,Z) format.  |
|`reference_source`   | `long`  | Indicates internal vs external reference source.	  | 0 = internal reference; 1 = external reference.  |
|`output_format`   | `string` | Indicates current output data format.	  | Uses the SDDS digraph format.  |
|`output_multicast`   | `string`  | Multicast address for SDDS output.	  |  Multicast address in dotted quad notation (e.g., "192.168.0.1"). |
|`output_vlan`   | `long`  | vlan number for SDDS output.	  | If there is no vlan used, indicate that with a zero.  |
|`output_port`   | `long`  | port number for SDDS output.	  | N/A  |
|`decimation`   | `long`  | Current decimation of tuner.	  | Decimation values for DDC tuners. Defined as the ratio of input sample rate to output sample rate regardless of data format.  |
|`tuner_number`   | `short`  | Physical tuner ID.	  | Tuner ID number within device. May represent physical tuner ordering or virtual ordering in software.  |

### RFSource Interface

FrontEnd devices that have `RFInfo` output ports and that flow RF metadata to other FrontEnd devices also have an `RFSource` interface. Such devices include antennas and RF distribution/switches. The `RFSource` interface is used to determine the currently selected RF Flow and all possible RF flows that a device can provide. The interface has two attributes:

  - `RFInfoPktSequence available_rf_inputs` - A list of all possible RF inputs to which this source could switch.

  - `RFInfoPkt current_rf_input` - The currently selected source that is being output.

Typical consumers of the `RFInfo` can read these values to know if other RF flows are possible. A device with the `RFSource` interface may get the information about its RF flows in multiple ways. The device may get the information through a non-REDHAWK interface to the actual switch, it may be stored as a configuration item or a property, or it may be set through the `RFSource` interface by another entity in the system with knowledge of the current RF configuration. If an external entity has knowledge of the RF configuration and sets the two attributes on a device, the device should push out an updated `RFInfoPacket` to any connected users.

#### RFSource Allocation Property

The `RFSource rf_flow_id` property is used in the `allocateCapacity()` interface of a device to request a particular RF flow. Allocation is performed by setting the `FRONTEND::RFSource::rf_flow_id` allocation property equal to the value of the RF flow requested. The following table describes the `RFSource` allocation property.

##### RFSource Allocation Property
| **Name**                         | **Type** | **Description**         | **Notes**                                           |
| :------------------------------- | :------- | :---------------------- | :-------------------------------------------------- |
| `FRONTEND::RFSource::rf_flow_id` | string   | Requested `rf_flow_id`. | Will return true if source can satisfy the request. |

#### allocationCount Property

If the requested RF flow can be switched to the output, the `RFSource` device will make the switch and the `allocateCapacity()` call will return true. If the device cannot provide the RF flow or has already been allocated to another RF flow, the `allocateCapacity()` call will return false. If the device is already allocated to the requested RF flow, the `allocateCapacity()` call will return true and increment the count of the total number of allocations received for that `rf_flow_id`. Once allocated, the device will not switch RF flows until the number of `deallocateCapacity()` calls received is equal to the total number of allocations for that `rf_flow_id`. The total number of allocations for the selected RF flow is stored in a readonly property named `allocationCount`.  The following table describes the `allocationCount` property.

##### allocationCount Property

| **Name**          | **Type** | **Description**                                                    | **Notes**                              |
| :---------------- | :------- | :----------------------------------------------------------------- | :------------------------------------- |
| `allocationCount` | long     | Total number of successful allocations against the current output. | Always >0 if the device is allocated. |

Even when the `allocationCount` is zero and no switch allocations are allocated, the  `current_rf_input` and the `RFInfoPacket` should reflect the current state of the RF Flow. For example, even without allocation, if real RF data is still being output, the status should reflect that. If no data is being output, then the `current_rf_input` attribute should be empty. Setting the `current_rf_input` attribute does not imply allocation or a request to switch an input. It assumes that this would only be set if the input had already switched to that new value from outside of the device. If the device `allocationCount` is not zero, it is not recommended to change the switch configuration and thus set the `current_rf_input`.
