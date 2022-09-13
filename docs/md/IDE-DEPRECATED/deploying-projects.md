# Deploying Projects to the SDRROOT

The following methods may be used to deploy a REDHAWK project into the target `SDRROOT`.

  - Drag-and-drop from the **Project Explorer**:

    1.  In the <abbr title="See Glossary.">Project Explorer view</abbr>, drag the top-level REDHAWK project onto the **<abbr title="See Glossary.">Target SDR</abbr>** in the <abbr title="See Glossary.">REDHAWK Explorer view</abbr>.
    2.  In the REDHAWK Explorer view, expand **Target SDR**, then expand the appropriate sub-area: **<abbr title="See Glossary.">Components</abbr>**, **<abbr title="See Glossary.">Devices</abbr>**, **<abbr title="See Glossary.">Nodes</abbr>**, **<abbr title="See Glossary.">Services</abbr>**, or **<abbr title="See Glossary.">Waveforms</abbr>**, to display the deployed project.

  - From the **Project** menu:

    1.  In the Project Explorer view, select the top-level REDHAWK project.
    2.  From the **Project** menu, select **Export to SDR**.

  - From the Context menu:

    1.  In the Project Explorer view, right-click the top-level REDHAWK project.
    2.  Select **Export to SDR**.

  - From the Overview page of the Software Package Descriptor (SPD) Editor:

    1.  Open the project's `spd.xml` file.
    2.  From the Overview tab of the <abbr title="See Glossary.">SoftPkg Editor</abbr>, in the **Exporting** section, click **Export Wizard**.
    3.  In the **Export Wizard**, from the **Available REDHAWK Projects** section, check the desired projects to deploy.
    4.  The **Destination** directory is prepopulated with the **Target SDR**.
    5.  Click **Finish** to deploy selected projects into the **Target SDR**.

  - From the **File** menu:

    1.  From the **File** menu, select **Export**.
    2.  From the **Export Wizard**, expand **REDHAWK Development** and select **Deployable REDHAWK Project**.
    3.  Click **Next**.
    4.  From the **Available REDHAWK Projects** section, check the desired projects to deploy.
    5.  The **Destination** directory is prepopulated with the **Target SDR**.
    6.  Click **Finish** to deploy the selected projects into the **Target SDR**.
