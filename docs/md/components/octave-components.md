# Creating Octave Components

REDHAWK provides the ability to auto-generate a REDHAWK <abbr title="See Glossary.">component</abbr> given an Octave M function. Assuming M functions are set up to input/output data vectors rather than relying on file or terminal-based input/output, these components are seamlessly deployable on runtime systems.

Octave version 3.4 or greater, with development support, must be installed on the development and deployment systems. This requirement can be met by either installing Octave from source or installing the octave-devel RPM.

The generated REDHAWK C++ code utilizes the Octave C++ programming interface to:

  - Translate incoming REDHAWK data to the available Octave data formats.
  - Call the M function using Octave's `feval()` C++ function.
  - Translate the resulting Octave data to REDHAWK data formats.

In many cases, the Octave component can be created without any C++ programming by the developer and without an in-depth understanding of REDHAWK. Developers with a more in-depth understanding of C++ programming and REDHAWK have the option of leveraging more advanced REDHAWK features by modifying the generated C++ code. Furthermore, Octave components can be composed into <abbr title="See Glossary.">waveforms</abbr> with components written in other languages (Java, C++, and Python).

### The createOctaveComponent Script

Octave components can be generated using a command line tool (`createOctaveComponent`) or using the [REDHAWK IDE](../ide/octave-wizard.html). The help string for the command line tool can be accessed by entering the following command:

```bash
createOctaveComponent --help
```

In the most simple case, the command line tool is passed a list of M files with no additional flags. Function arguments that have a default value are treated as properties and function arguments without default values are treated as <abbr title="See Glossary.">ports</abbr>.

Below is an example of a basic M function defined in a file named `addConst.m`:

```octave
function myOutput = addConst(myInput, myConst=0)
    myOutput = myInput + myConst
```

To generate the component code, use the following command:
```bash
createOctaveComponent addConst.m
```

Refer to the `createOctaveComponent` help string for flags to:

  - Automatically compile and install the component.
  - Automatically create an RPM for the component.
  - Enable buffering, which causes the component to wait for an End of Stream (EOS) before processing data.
  - Enable the Octave diary, which writes Octave's standard out and standard error to a file.
  - Point to shared `.m` and `.oct` files.
  - Specify an output directory.

## Design Considerations

There are a few design considerations to keep in mind when creating an M file to be used in REDHAWK:

  - Data must be passed as row vectors or n-by-m matrices. All values must be doubles.
  - Configuration <abbr title="See Glossary.">property</abbr> values may be doubles, double vectors, or strings. All numerical properties are treated as complex.
  - The `serviceFunction` of the component is auto-generated and should not be hand-modified. To manipulate data before or after the `feval()` call to Octave, modify the `inputPackets` and `outputPackets` maps in the `preProcess` and `postProcess` methods.
