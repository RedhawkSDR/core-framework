# Component Examples

Transforming Component
----------------------

A component that takes some input stream(s), and produces output data. E.g., TuneFilterDecimate, AmFmPmBasebandDemod, etc.

### Service Function

The following example is from the modified FM demodulator I'm using to do resource utilization measurement in shared address space components, versus a comparable 2.0 application. This is the basic flow I'm expecting for new components that are doing some sort of data transformation; other than the use of the shared buffer data types, this is almost a valid 2.0 flow as well–much of it is intrinsic to the BulkIO stream API. The one thing missing in 2.0 is the getStream() method on
output ports. You have to track the output streams yourself in 2.0, which was an oversight.

```
int FmDemod_i::serviceFunction()
{
    // InFloatPort::getCurrentStream() returns the first input stream that has data available to be read,
    // blocking if none are currently ready.
    bulkio::InFloatStream input = dataFloat_in->getCurrentStream();
    if (!input) {
        // No streams are available (typically due to a stop).
        return NOOP;
    }

    // If an output stream has already been created, OutFloatPort::getStream() finds it by streamID.
    bulkio::OutFloatStream output = dataFloat_out->getStream(input.s
 treamID());
    if (!output) {
        // Otherwise, create a new stream.
        output = dataFloat_out->createStream(input.streamID());
        // configureStream() is a user-defined method to apply any transformations from the input SRI to the
        // output SRI
        configureStream(output, input.sri());
    }

    // Blocking read from the input stream. The default behavior returns exactly one packet worth of data,
    // but a sample count and consume length (for overlap) can be provided as well.
    bulkio::FloatDataBlock block = input.read();
    if (!block) {
        // There are two reasons why InFloatStream::read() might return a "null" block:
        if (input.eos()) {
            // 1. End-of-stream; close the output stream.
            LOG_DEBUG(FmDemod_i, "Stream " << input.streamID() << " got an EOS");
            output.close();
            return NORMAL;
        } else {
            // 2. The component was stopped.
            return NOOP;
        }
    }

    // Handle an input queue flush. Depending on your algorithm, this may require resetting state, etc.
    if (block.inputQueueFlushed()) {
        LOG_WARN(FmDemod_i, "Input queue flushed");
    }

    // Handle SRI changes. The configureStream() method is also used on output stream creation; the SRI change flag
    // is not set for a new stream.
    // Optionally, FloatDataBlock::sriChangeFlags() returns a set of bit flags to check which parts of the SRI changed,
    // if only certain fields are relevant.
    if (block.sriChanged()) {
        LOG_INFO(FmDemod_i, "SRI changed");
        configureStream(output, block.sri());
    }

    // Update any internal variables or properties. This a user-defined method, and the behavior is up to the
    // implementer. In this case, the algorithm depends on the sample rate of the input SRI.
    updateParameters(block.sri());

    // Run the algorithm here (FM demodulation, in this case), taking into account whether the input is complex or
    // real. In this example, the output is always real regardless of the input, but this can differ on a per-component
    // basis. The actual work is done in a private templatized member function, doFmDemod().
    redhawk::buffer<float> buffer;
    if (block.complex()) {
        buffer = doFmDemod(block.cxbuffer());
    } else {
        buffer = doFmDemod(block.buffer());
    }

    // Write the processed data to the output stream. After calling OutFloatStream::write(), the buffer is now shared
    // with an unknown number of consumers. No more modifications should be made.
    // Regarding the BULKIO::PrecisionUTCTime, a block can contain multiple timestamps, if you explicitly request
    // a read size; here, we are just taking the data in the block sizes it comes in, so we can just use the starting
    // timestamp for the entire block. Algorithms that affect the time basis should compute a new block time.
    output.write(buffer, block.getStartTime());
    return NORMAL;
}
```

### Algorithm

Here's what `doFmDemod()` looks like. You wouldn't necessarily have to
do it this way, it's just an example of using functional programming to
do your algorithm. By making the method templatized, the same code can
be used (inline) for both real and complex input.

```
template <class T>
redhawk::shared_buffer<float> FmDemod_i::doFmDemod(const redhawk::shared_buffer<T>& input)
{
    redhawk::buffer<float> output(input.size());
    dsp::process(input.begin(), input.end(), output.begin(), demod);
    return output;
}
```

It's not taking different streams into account, but that's relatively
easy to fix–you could keep a map of stream IDs to demodulator objects.

Quick implementation notes:

-   `dsp::process()` is basically `std::transform()`, but it allows the
    `demod` object to be stateful (i.e., it remembers the last phase)
-   `demod` is a small functor that is applied to each input, storing
    the result to the output

Generative Component
--------------------

A component that creates BulkIO output data based on parameters or
files, instead of BulkIO input. E.g., SigGen, FileReader.

### Service Function

Generative components can be simpler, because there is no input to be
concerned with. The following example is for a component that generates
a waveform (like SigGen, but more trivial).

```
int Waveform_i::serviceFunction()
{
    // Update any internal variables or properties. This a user-defined method, and the behavior is up to the
    // implementer. In this case, the waveform parameters could be copied from properties to shadow variables
    // while holding the properties lock (propertySetAccess).
    updateParameters();

    // For this example, the output stream is a member variable on the Waveform_i class. If it's not already
    // created, do it here. Another option would be to create it inthe constructor() method.
    if (!_outputStream) {
        _outputStream = dataFloat_out->createStream(_streamID);
        _sriChanged = true;
    }

    // If the SRI needs to be updated, based on the properties (or some other reason), one option is to set an
    // internal flag; another possibility might be to update the SRI in updateParameters()--if the stream is only
    // used from within serviceFunction(), there are no threading concerns.
    // configureStream() is a user-defined method to apply any changes to the output stream SRI.
    if (_sriChanged) {
        configureStream(_outputStream);
    }

    // Create a buffer of the desired size. The redhawk::buffer class is the mutable version, and it gracefullly
    // "degrades" to the immutable redhawk::shared_buffer class.
    redhawk::buffer<float> data(1024);

    // The implementation of the generator function is left as an exercise to the reader.
    doWaveform(data.begin(), data.end());

    // This example keeps a running BULKIO::PrecisionUTCTime for the first sample of each block. There are overloaded
    // arithmetic operators to make it easier to modify.
    _currentTimestamp += _outputStream.xdelta() * data.size();

    // After calling OutFloatStream::write(), the buffer is now shared with an unknown number of consumers. No more
    // modifications should be made.
    _outputStream.write(data, _currentTimestamp);

    return NORMAL;
}
```
