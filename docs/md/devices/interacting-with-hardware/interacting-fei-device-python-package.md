# Interacting with an FEI Device with the Python Package

The Python package contains helpers that simplify the allocation/deallocation of FrontEnd Interfaces (FEI) tuners. For example, to allocate a tuner to receive, centered at 1 MHz with a bandwidth of 1kHz, a 10% tolerance in the requested values, and a sample rate of 2 kHz, the following functionality can be used:

```python
>>> import frontend
>>> allocation = frontend.createTunerAllocation(tuner_type="RX", allocation_id="someid",center_frequency=1e6, bandwidth=1e3,bandwidth_tolerance=0.1, sample_rate=2e3,sample_rate_tolerance=0.1)
>>> retval = dev.allocateCapacity(allocation)
```

where `dev` is a reference to the <abbr title="See Glossary.">device</abbr> object and `retval` is `True` if the allocation succeeded.

To connect to an allocated tuner, use the Allocation ID as the Connection ID:

```python
>>> dev.connect(comp, connectionId="someid")
```

where `comp` is a reference to the <abbr title="See Glossary.">component</abbr> being connected to an allocated tuner. If the Connection ID corresponding to an allocation is not provided and the possible sources of data are not ambiguous, the Python package will create a connection that delivers the allocated stream.

When a device contains a single allocated tuner and there is a single output Bulk Input/Output (BulkIO) <abbr title="See Glossary.">port</abbr>, it is possible to unambiguously determine what tuner to connect to and over which port to connect. In such instances, the Python package creates a listener allocation against the allocated tuner and establishes a connection between the FEI device and the destination using the listener Allocation ID as the Connection ID. The created listener enables the owner of the allocation to establish a connection to the data source with the given Allocation ID. When the connection is removed, the Python package automatically deallocates the listener allocation. In such instances, the following call can be used to connect:

```python
>>> dev.connect(comp)
```

To deallocate the tuner, use the following call:

```python
>>> dev.deallocateCapacity(allocation)
```

To allocate a listener to a specific allocated tuner, use the following call:

```python
>>> listen_alloc = frontend.createTunerListenerAllocation(allocation_id, "some ID listener")
>>> retval = dev.allocateCapacity(listen_alloc)
```

To allocate a listener to any tuner with a particular set of values, use the following call:

```python
>>> allocation=frontend.createTunerGenericListenerAllocation(tuner_type="RX", allocation_id="someidanotherlistener",center_frequency=1e6, bandwidth=1e3,bandwidth_tolerance=0.1,sample_rate=2e3, sample_rate_tolerance=0.1)
>>> retval = dev.allocateCapacity(allocation)
```

Deallocation of listeners follows the same pattern as the deallocation of tuners.

#### Scanning Tuners

Tuners that have a built-in scanning capability can also be accessed through Python helpers. Assuming that the above device is also scanning-capable, to create a scanning allocation that spans between 1 MHz and 1.1 MHz and the retuning rate will be no less than 100 ms, use the following command:

```python
>>> scan_alloc = frontend.createScannerAllocation(min_freq=1e6, max_freq=1.1e6, mode='SPAN_SCAN', control_mode='TIME_BASED', control_limit=0.1)
>>> dev.allocateCapacity([allocation, scan_alloc])
```

The allocation does not setup the scan plan, it just requests a device that will support the type of scan that is required. To create the plan, the strategy for the scanner needs to be created. In this case, the strategy will be a single span that will be scanned between 1.0 MHz and 1.1 MHz in 10 kHz increments, with a retune every 150 ms. To setup the strategy and start the scan, use the following commands:

```python
>>> from redhawk.frontendInterfaces import FRONTEND
>>> from ossie.utils import bulkio
>>> scan_spans = [FRONTEND.ScanningTuner.ScanSpanRange(1e6, 1.1e6, 1e4)]
>>> scan_strategy=FRONTEND.ScanningTuner.ScanStrategy(FRONTEND.ScanningTuner.SPAN_SCAN, FRONTEND.ScanningTuner.ScanModeDefinition(freq_scan_list=scan_spans), FRONTEND.ScanningTuner.TIME_BASED, 0.15)
>>> port = dev.getPort("DigitalScanningTuner_in")
>>> port.setScanStrategy("someid", scan_strategy)
>>> port.setScanStartTime("someid", bulkio.createCPUTimestamp())
```

Note that the set of frequencies that will be spanned can extend over multiple non-contiguous spans. To setup these non-contiguous spans, use multiple instances of ScanSpanRange in the scan_spans list.
