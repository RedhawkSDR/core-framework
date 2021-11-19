# The Workbench

The Eclipse introductory screen displays a **<abbr title="See Glossary.">Workbench</abbr>** button that takes the user to the IDE's development environment: the workbench. The workbench is made up of multiple, smaller windows, which are referred to as views in the Eclipse context.

At the center of the IDE workbench is the editor window, which is empty at initial startup. The editor is the primary window used when developing code within the REDHAWK IDE. An Eclipse editor is a context-sensitive window within the workbench; the language of opened files dictates the type of editor that is opened, impacting editing features such as syntax highlighting.

For a more detailed understanding of the Eclipse environment and nomenclature, consult the online Eclipse documentation at <http://help.eclipse.org/> or the embedded documentation within the REDHAWK IDE by selecting **Help > Search**.

### Perspectives

The views that makeup the workbench, along with the particular layout of those views, are referred to as a <abbr title="See Glossary.">perspective</abbr>. By changing perspectives throughout the development process, a developer may optimize his/her work environment based on the requirements of the particular task at hand. The default perspective in the REDHAWK IDE is the REDHAWK perspective, which is discussed in the following section. A user may switch from the REDHAWK perspective to any other perspective whenever needed.

There are two primary methods for changing perspectives:

  - Click **Open Perspective** from the top right of the workbench.

    ![Open Perspective](img/REDHAWK_Open_Perspective.png)

  - Select **Window > Open Perspective > Other**.

A view may be resized, moved, and closed within a given perspective to allow for personal customization.

To reset the current perspective to its default state:

  - Click **Window > Reset Perspective...**

### The REDHAWK Perspective

The REDHAWK perspective is comprised of seven views and the editor window. Five of these views are provided by Eclipse IDE, while the remaining two views are REDHAWK-specific.

The following five Eclipse-provided views are in the REDHAWK perspective:

  - <abbr title="See Glossary.">Project Explorer view</abbr>: Provides a hierarchical view of the resources in the Workbench.
  - <abbr title="See Glossary.">Outline view</abbr>: Displays an outline of a structured file that is currently open in the editor area.
  - <abbr title="See Glossary.">Properties view</abbr>: Displays property names and basic properties of a selected resource.
  - <abbr title="See Glossary.">Problems view</abbr>: Automatically logs problems, errors, or warnings when working with various resources in the workbench.
  - <abbr title="See Glossary.">Console view</abbr>: Displays a variety of console types depending on the type of development and the current set of user settings.

The following two REDHAWK-specific views are in the REDHAWK perspective:

  - <abbr title="See Glossary.">REDHAWK Explorer view</abbr>: Allows a user to [navigate the contents of a REDHAWK domain](../exploring-domain/_index.html). It provides capabilities for viewing the contents of the <abbr title="See Glossary.">domain</abbr>, configuring instantiated resources, and launching <abbr title="See Glossary.">applications</abbr> in a <abbr title="See Glossary.">Target SDR</abbr> environment. It also provides access to the <abbr title="See Glossary.">sandbox</abbr>, which is an environment for running <abbr title="See Glossary.">components</abbr> and applications without a <abbr title="See Glossary.">Domain Manager</abbr> or a <abbr title="See Glossary.">Device Manager</abbr>.
    ![REDHAWK Explorer View](img/REDHAWK_Explorer_View.png)

  - <abbr title="See Glossary.">CORBA Name Browser</abbr> view: Maps names to specific CORBA Servants. The CORBA Name Browser view is used to examine the current contents of the <abbr title="See Glossary.">Naming Service</abbr> as well as perform basic manipulation of that context. The view displays all currently bound name contexts (folders) and objects.
    ![CORBA Name Browser View](img/REDHAWK_Name_Browser.png)

### Programming Language Specific Perspectives

While the REDHAWK perspective combines views that are commonly used while viewing domain objects, creating REDHAWK resources, and launching applications, many other perspectives are available that are optimized for code development. Because the REDHAWK IDE is built on top of the Eclipse platform, it takes advantage of standard Eclipse, as well as third party, IDE perspectives for the purpose of supporting language-specific development. Specifically, the IDE contains perspectives that support C/C++, Java, and Python development.

For example, the Java perspective combines views that are commonly used while editing Java source files, while the Debug perspective contains the views that are used while debugging Java programs.

For more information on perspectives, particularly the Eclipse default and the programming language-specific perspectives packaged with the REDHAWK IDE, refer to <http://help.eclipse.org/>.
