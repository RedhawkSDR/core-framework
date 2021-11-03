# Miscellaneous

#### Saving and Loading Waveforms

The <abbr title="See Glossary.">components</abbr> making up a <abbr title="See Glossary.">waveform</abbr> can be loaded into the <abbr title="See Glossary.">workspace</abbr> by passing the path and name of the waveform's Software Assembly Descriptor (SAD) XML file to the `loadSADFile()` method. Note that <abbr title="See Glossary.">*usesdevice* relationships</abbr> are ignored when loading a SAD file onto the <abbr title="See Glossary.">sandbox</abbr>.

```python
>>> sb.loadSADFile("/path/to/sad/file/waveform.sad.xml")
```

The instantiated components and their associated connections can also be saved as a waveform. To perform this operation, pass the desired waveform name to the `generateSADXML()` method.

This method returns an XML string representing the contents of the SAD file; this string may then be written to a file:

```python
>>> sadString = sb.generateSADXML("waveform_name")
>>> fp=open("/path/to/sad/file/waveform_name.sad.xml","w")
>>> fp.write(sadString)
>>> fp.close()
```

#### Debug Statements and Standard Out

Standard out and standard error can be redirected to a file:

```python
>>> sb.redirectSTDOUT("/path/to/file/file.txt")
```

Debug statements can be set explicitly.

To set the debug output status, pass `True` or `False` to the `setDEBUG()` method:

```python
>>> sb.setDEBUG(True)
```

To get the current state of the debug output, use the `getDEBUG()` method:

```python
>>> sb.getDEBUG()
True
```

#### Processing Components from the Command Line

To process individual components from the command line, use the `proc()` function. The following command is an example of the `proc()` function call with sample arguments:

```python
>>>sb.proc("my_component","input_file",sink="output_file",sourceFmt="16t",sinkFmt="8u",sampleRate=10000,execparams={"execprop1":5},configure={"prop2":4},providesPortName="input",usesPortName="output",timeout=10)
```

##### `proc()` Function Arguments
| **Name**            | **Description**                                                                                                                                |
| :------------------ | :--------------------------------------------------------------------------------------------------------------------------------              |
| `<first argument>`  | Specifies the name of the component to run (required).                                                                                         |
| `<second argument>` | Specifies the name of the input file (required).                                                                                               |
| `sink`              | Specifies the name of the output file (optional). If this argument is omitted, exit the `proc()` function with Ctrl+C.                         |
| `sourceFmt`         | Specifies the format for the input file (optional).                                                                                            |
|                     | \- `8/16/32/64` bit resolution                                                                                                                 |
|                     | \- `u/t`: unsigned/signed                                                                                                                      |
|                     | \- `c`: complex (real if omitted)                                                                                                              |
|                     | \- `r`: big endian (little endian if omitted)                                                                                                  |
| `sinkFmt`           | Specifies the format for the output file (optional).                                                                                           |
| `sampleRate`        | Specifies the sampling rate for the input file (optional).                                                                                     |
| `execparams`        | Specifies the dictionary describing properties (of kind <abbr title="See Glossary.">`property`</abbr> with `commandline` set to true) to be passed as command-line arguments (optional). |
| `configure`         | Specifies the dictionary describing properties of kind `property` to be overridden (optional).                                   |
| `providesPortName`  | Specifies the component input <abbr title="See Glossary.">port</abbr> name (optional).                                                                                            |
| `usesPortName`      | Specifies the component output port name (optional).                                                                                           |
| `timeout`           | Specifies how long the `proc()` function runs before exiting (optional).                                                                       |
