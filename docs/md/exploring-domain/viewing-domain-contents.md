# Viewing the Contents of the Domain in the REDHAWK Explorer View

After the <abbr title="See Glossary.">domain</abbr> connection is established, the file system visible to the <abbr title="See Glossary.">Domain Manager</abbr> and its attached <abbr title="See Glossary.">Device Managers</abbr> is displayed in the <abbr title="See Glossary.">**REDHAWK Explorer** view</abbr>. Detailed information about each item is available in the <abbr title="See Glossary.">**Properties** view</abbr>.

The Domain Manager's root contains the following folders:

  - **Device Managers**: Displays the currently connected Device Managers. More than one Device Manager may be connected to the domain. Each Device Manager entry consists of a single <abbr title="See Glossary.">node</abbr>, and each node may contain multiple <abbr title="See Glossary.">devices</abbr>. Right-click devices to monitor <abbr title="See Glossary.">port</abbr> information, plot port output, and play audio.

    ![Device Managers Folder](img/devman.png)

  - **Event Channels**: Displays the <abbr title="See Glossary.">event channels</abbr> in the domain. Right-click a channel to display the **Refresh** and **Listen to Event Channel** options. Select **Listen to Event Channel** to open the [**Event Viewer** view](../ide/editors-and-views/event-viewer-view.html).

    ![Event Channels Folder](img/eventchannel.png)

  - <abbr title="See Glossary.">**File Manager**</abbr>: Displays the file systems in the domain. It contains references to all <abbr title="See Glossary.">components</abbr>, devices, <abbr title="See Glossary.">waveforms</abbr>, and the device and Domain Managers' configuration and executable files.

    ![Example Domain File System](img/REDHAWK_Domain_File_System_1.png)

  - **Waveforms**: Displays the <abbr title="See Glossary.">applications</abbr> in the domain. When applications launch, they are displayed and can be expanded to show each of the running components within the application. These components can be expanded to show the device on which they are executing and port information.

    ![Example Waveforms Folder](img/wavedom.png)
