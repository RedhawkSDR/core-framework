# Runtime Environment Inspection

During runtime, there are a large number of operations that are handled under the hood by the REDHAWK Core Framework (CF). At times though, it becomes necessary to take a closer look at these underlying parts to ensure that they are working properly or to inspect what kind of a state they are currently in. REDHAWK provides tools to help accomplish this task.

## REDHAWK Module

A Python module called REDHAWK is provided with the capability to interact with running <abbr title="See Glossary.">domains</abbr>, <abbr title="See Glossary.">devices</abbr>, <abbr title="See Glossary.">waveforms</abbr> and <abbr title="See Glossary.">components</abbr>. This allows for individual control and assessment over all aspects of a runtime environment.

In order to use the REDHAWK utility module, begin a Python session from a terminal and enter the following command:

```python
from ossie.utils import redhawk
```

The REDHAWK module is built on the same foundation as the <abbr title="See Glossary.">sandbox</abbr>, and provides a compatible interface for domain objects. Sandbox objects, including plots, can be dynamically connected to devices, waveforms and components running in the domain.

Refer to [Working with Components, Devices, and Services](../Sandbox/Python/working_with_components.html) for more information.

### Attach

The module provides the ability to attach to a running domain.

This allows the user access to the underlying API of the <abbr title="See Glossary.">Domain Manager</abbr> in addition to other useful functionality:
  - `createApplication()` - install/create a particular waveform
  - `removeApplication()` - release a particular waveform

To attach to an existing domain, the name must be passed as an argument.

Assuming the domain name is `MY_DOMAIN`, start a Python script and enter:

```python
>>> domain = redhawk.attach("MY_DOMAIN")
```

Note that if there is only 1 Domain visible, no argument is needed for the attach call:

```python
>>> domain = redhawk.attach()
```

Once attached to the running domain, waveforms that are installed in the `$SDRROOT` can easily be launched using the `createApplication()` function:

```python
>>> wave = domain.createApplication("/waveforms/wave/wave.sad.xml")
```

Upon success, the above call returns an `Application` object which gives access to the external resource API. This allows for manual operation of the <abbr title="See Glossary.">application</abbr>. In addition, functions exist to allow the user to connect and disconnect <abbr title="See Glossary.">ports</abbr>. Finally, in order to inspect the current conditions of the waveform, an API function call is available. This shows any external ports, components that are in the application, and their <abbr title="See Glossary.">properties</abbr>.

```python
>>> wave
<ossie.utils.redhawk.core.App object at 0x2bfb350>
>>> wave.api()
Waveform [wave_025_090314360_1]
---------------------------------------------------
External Ports ==============
Provides (Input) Ports ==============
Port Name       Port Interface
---------       --------------
external_in     IDL:BULKIO/dataChar


Uses (Output) Ports ==============
Port Name       Port Interface
---------       --------------
external_out    IDL:BULKIO/UsesPortStatisticsProvider


Components ==============
1. Sink
2. Source (Assembly Controller)


Properties ==============
Property Name    (Data Type)      [Default Value]   Current Value
-------------    -----------      ---------------   -------------
sample_size      (long/SL/32t)    None              None
```
Once finished, the waveform needs to be removed from the domain by using the `removeApplication()` method:

```python
>>> domain.removeApplication(wave)
```

### Starting a Domain from within a Python session

Normally, the REDHAWK Python package is used to either interact with a running domain, or to launch some [sandbox](../Sandbox/_index.html) components. However, sometimes it may be required to launch a domain from a Python script.

To help in such a scenario, the REDHAWK Python package includes some helper functions. The `kickDomain` feature allows for an easy way to launch domain and <abbr title="See Glossary.">Device Managers</abbr> from within a Python script. With no arguments, the function launches and returns the Domain Manager that is installed in `$SDRROOT`. Additionally, all Device Managers in `$SDRROOT/dev/nodes` are started.

Other arguments to the function exist to control different aspects to how the domain is started. A list of specific Device Managers to start can be passed if the user does not want to start all available. If the `$SDRROOT` that the user wishes to use is not in the standard location, a path can be supplied to direct the function to the desired place in the file system. Standard out and logging to a file can also be set.

```python
>>> domain = redhawk.kickDomain()
INFO:DomainManager - Starting Domain Manager
INFO:DeviceManager - Starting Device Manager with
    /nodes/DevMgr_localhost.localdomain/DeviceManager.dcd.xml
INFO:DomainManager - Starting ORB!
INFO:DeviceManager_impl - Connecting to Domain Manager
    MY_DOMAIN/MY_DOMAIN
INFO:DeviceManager - Starting ORB!
>>> INFO:DCE:9d9bcc38-d654-43b1-8b74-1dc024318b6f:Registering
    Device
INFO:DeviceManager_impl - Registering device GPP_localhost_
    localdomain on
    Device Manager DevMgr_localhost.localdomain
INFO:DeviceManager_impl - Initializing device GPP_localhost_
    localdomain on
    Device Manager DevMgr_localhost.localdomain
INFO:DeviceManager_impl - Registering device GPP_localhost_
    localdomain on
    Domain Manager

>>> domain
<ossie.utils.redhawk.core.Domain object at 0x2da6710>
```

The ability to search for domains is also available through the scan function which searches the <abbr title="See Glossary.">Naming Service</abbr>.

```python
>>> redhawk.scan()
['MY_DOMAIN']
```

### Domain State

The domain proxy tracks the current state of the domain. For aspects of the domain state that may take a long time to process, scanning is deferred until the first access. If the Outgoing Domain Management (ODM) Channel is available, devices, Device Managers, and waveforms are tracked as they are added to and removed from the domain. Otherwise, the domain is re-scanned on access to ensure that the state is accurate.

#### Applications

The domain proxy provides a list of waveforms currently running via the `apps` attribute:

```python
>>> domain.apps
[<ossie.utils.redhawk.core.App object at 0x<hex address>>, ...]
```

The `App` class supports many of the sandbox interfaces, such as properties, `connect()` and `api()`.

The components within the waveform are available via the `comps` attribute:

```python
>>> app = domain.apps[0]
>>> app.comps
[<ossie.utils.redhawk.component.Component object at 0x<hex address>>, ...]
```

The REDHAWK module `component` class supports the same interfaces as sandbox components. Much like in the sandbox, ports, properties, and any other feature of a component is equally accessible from the Python environment, regardless of how the component was deployed.

For example, if one were to want to plot the output of a component running on the Domain:

```python
>>> from ossie.utils import redhawk, sb
>>> dom=redhawk.attach()
>>> for app in dom.apps:
...  if app.name == "my_app":
...   break

>>> for comp in app.comps:
...  if comp.name == "my_comp":
...   break

>>> plot = sb.LinePlot()
>>> plot.start()
>>> comp.connect(plot)
```

#### Device Managers

The domain proxy provides a list of Device Managers registered with the domain via the `devMgrs` attribute:

```python
>>> domain.devMgrs
[<ossie.utils.redhawk.core.DeviceManager object at 0x<hex address>>, ...]
```

Each Device Manager maintains a list of its devices, accessible via the `devs` attribute:

```python
>>> domain.devMgrs[0].devs
[<ossie.utils.redhawk.device.ExecutableDevice object at 0x<hex address>>, ...]
```

#### Devices

The domain proxy provides a list of all devices registered with the domain via the `devices` attribute:

```python
>>> domain.devices
[<ossie.utils.redhawk.device.ExecutableDevice object at 0x<hex address>>, ...]
```

The device list is the concatenation of the devices for all Device Managers.

The REDHAWK module `device` class supports the same interfaces as sandbox devices.

#### Event Channels

<abbr title="See Glossary.">Event channels</abbr> can be accessed through the `eventChannels` member of the domain object. Each of the returned objects contains the name of the channel and a reference to the channel object.

```python
>>> evt = dom.eventChannels
>>> evt[0].name
ODM_Channel
>>> evt[0].ref
<CosEventChannelAdmin._objref_EventChannel object at 0x1608550>
```

Event channels can be created through the <abbr title="See Glossary.">Event Channel Manager</abbr>:

```python
>>> channel = dom.eventChannelMgr.create("TestChan")
```

Subscribers and publishers to event channels can also be created in the Python sandbox. They can be created as entities in the Python environment, with the constructor argument being the channel that they are to publish or subscribe to.

```python
>>> from ossie.events import Subscriber, Publisher
>>> def my_callback(data):
       print data
>>> sub=Subscriber(evt[0], dataArrivedCB=callback)
>>> pub=Publisher(evt[0])
>>> pub.push(data)
```

#### Managing Allocations

The domain has an <abbr title="See Glossary.">Allocation Manager</abbr> that allows the developer to offload some of the bookkeeping tasks associated with capacity allocation. Interactions with the Allocation Manager are exercised through requests and responses. The Allocation Manager responds to requests by searching through all available devices to find Devices that can satisfy the request. An example of interactions with the Allocation Manager follows:

```python
>>> am = dom.allocationMgr
>>> from ossie.utils import allocations
>>> prop =  allocations.createProps({'s_prop':{'s_prop::a':'hello','s_prop::b':5}})
>>> request = am.createRequest('request id', prop)
>>> response = am.allocate([request])
>>> am.listAllocations(CF.AllocationManager.LOCAL_ALLOCATIONS, 100)
[...]
>>> am.deallocate([response[0].allocationID])
```

#### Managing Connections

While it is simple to establish point-to-point connections, sometimes it is desirable for a connection to be established as domain objects come and go. To manage these types of connections, the domain contains a <abbr title="See Glossary.">Connection Manager</abbr>. Helpers have been created to make it easy to create endpoints from Python objects connected to domain objects.

```python
>>> from ossie.utils import rhconnection
>>> endpoint_1 = rhconnection.makeEndPoint(dom.apps[0], dom.apps[0].ports[0].name)
>>> endpoint_2 = rhconnection.makeEndPoint(dom.apps[1], '')
>>> cm = dom.connectionMgr
>>> cm.connect(endpoint_1, endpoint_2)
```

### Using the Sandbox

As shown briefly in section [Applications](#applications), sandbox and domain objects are inter-operable and can be connected together. This allows for inspection of different parts of the domain and more sophisticated testing of components.

For example, to use a sandbox plot to view the data coming from a device, enter the following commands:

```python
>>> from ossie.utils import sb
>>> plot = sb.LinePlot()
>>> domain.devices[1].connect(plot)
>>> plot.start()
```

Use the following commands to capture an approximately one second cut from a waveform to a file:

```python
>>> import time
>>> sink = sb.FileSink("/data/tuner_out.dat")
>>> domain.apps[0].connect(sink, usesPortName="tuner_out")
>>> sink.start()
>>> time.sleep(1.0)
>>> sink.stop()
```

When the sandbox exits, any connections made between sandbox objects and domain objects are broken to limit interference with normal domain operation.

For more details on using the sandbox, refer to [Sandbox](../Sandbox/_index.html).
