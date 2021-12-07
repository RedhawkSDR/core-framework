# Creating a FrontEnd Interfaces Device

The IDE does not support the creation of FEI devices although it is possible to interact with an FEI device through the IDE.

To create an FEI, use the `fei3-generator` utility.

The creation process follows 3 command-line steps.
First, a template YAML file is created.
The developer edits the YAML with desired FEI device contents.
Second, the edited YAML template is converted to a detailed definition of the multiple devices.
The third and final step is to convert this auto-generated detailed definition into the XML and source code project.

The first step is to create an empty template:

```bash
fei3-generator create-list --output my_template.yml
```

The template file contains a list of all possible FEI device types (e.g.: ABOT, TDC).
Under each device type category that will be defined in this FEI device, add a dash followed by the class name for that device.
For example, if the user wants to Abc as RDC and Ghi as ABOT, edit the `my_template.yml` file as follows:

```yaml
Analog Input Bank of Tuners (ABOT):
 - Ghi

Analog Input Receiver/Digital Out Channel (ARDC):

Digital Input Bank of Tuners (DBOT):

Digital Receive Channel (RDC):
 - Abc
```

Next, the parent device needs to be defined.
In this case, we select Ghi as the parent, so edit `my_template.yml` as follows:

```yaml
PARENT:
 - Ghi

Analog Input Receiver (RX):
```

Once the overall structure has been defined, generate a more detailed description by running the following command:

```bash
fei3-generator devices --devices-list my_template.yml --output detailed_template.yml
```

Once the detailed template is generated, use it to generate the XML and C++ source code project:

```bash
fei3-generator xml --fei3-devices detailed_template.yml --lang cpp --codegen
```

If the desired language is Python 3, use `python` as the language.
For Java 11, use `java` as the language.

The project will be generated in a subdirectory with the same name as the parent class, in this case `Ghi`.
