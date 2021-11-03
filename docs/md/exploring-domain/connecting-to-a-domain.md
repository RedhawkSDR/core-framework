# Connecting to a Domain

In the IDE, you can launch and connect to a <abbr title="See Glossary.">domain</abbr> through the IDE interface or connect to a running domain.

You can also [launch a domain and Device Manager from the command line](../runtime-environment/domain-manager.html#launching-a-domain-manager-from-the-command-line).

### Launching and Connecting Using the IDE

The following procedure explains how to launch and connect to a domain through the IDE.

1.  In the <abbr title="See Glossary.">**REDHAWK Explorer** view</abbr>, right-click **<abbr title="See Glossary.">Target SDR</abbr>** and select **Launch Domain...**
![Launching a Domain](images/REDHAWK_Launch_domain.png)

    The **Launch Domain Manager** window is displayed:
![Launch Domain Manager Window](images/SelectDomMgr.png)

2.  In the **Domain Name** field, enter a name.

3.  Optionally, select a <abbr title="See Glossary.">Device Manager</abbr> to start.

> **NOTE**  
> A running Device Manager is required to launch <abbr title="See Glossary.">applications</abbr>.  

4.  Optionally, in either the <abbr title="See Glossary.">Domain Manager</abbr> or Device Manager section, select a **Debug Level**. By default, the **Debug Level** is set to **Info**, which displays any messages at the **Info** level or higher: (**Info**, **Warn**, **Error**, and **Fatal** message levels). If this is the first time using REDHAWK, changing the **Debug Level** from **Info** to **Debug** for both the domain and Device Manager may be helpful in the learning process.

5.  Optionally, in either the Domain Manager or Device Manager section, select **Arguments** for the `nodeBooter` process. This option is provided to advanced users who are comfortable with [command line options](../runtime-environment/_index.html).

6.  Finally, click **OK** to launch and connect to the new domain.

7.  Notice that the IDE reacts to the newly launched domain:

    1.  The new domain has been added to the **REDHAWK Explorer** view

    2.  The new domain, within a short amount of time, is connected and this connection is indicated to the right of the domain name within the **REDHAWK Explorer** view.

    3.  At least one new <abbr title="See Glossary.">Console view</abbr> (within the IDE) has been created. One contains a `nodeBooter` instance for the Domain Manager that launched and one `nodeBooter` instance for each Device Manager.
![Console Showing the Domain Manager Start Up](images/REDHAWK_Domain_Console.png)

### Connecting to a Running Domain

To connect to a running domain through the IDE use the following procedure:

1.  Click the **New Domain Connection** button (i.e. the plus symbol) in the upper right of the **REDHAWK Explorer** view.
![New Domain Connection Button](images/NewDomainConnection.png)

    The **New Domain Manager** dialog is displayed:
![New Domain Manager Dialog](images/REDHAWK_New_Domain_Wizard.png)

2.  Enter the <abbr title="See Glossary.">**Name Service**</abbr>. This is the CORBA URI of the Naming Service where the domain is registered. By default, this is populated with the value from the IDE's preferences (set by selecting **Window > Preferences**, **REDHAWK > Domain Connections**, **Default Naming Service**).

3.  To specify the domain to which you want to connect, enter a domain name or click the + button and select a running domain. The domain name is the actual name of the domain in the Naming Service.

4. Optionally, enter a different display name. The display name is used only in the IDE and does not need to match the domain name.

      - Wait for the IDE to scan the Name Service for running domains. When the button next to **Display Name** shows a + icon, click it and select a running domain from the list.

5.  Click one of the following options under **Connection Settings**:

      - **Don't connect**: This adds the domain to the **REDHAWK Explorer** view but leaves the domain in the disconnected state. When the IDE is restarted, the domain remains in the <abbr title="See Glossary.">**REDHAWK Explorer**</abbr> and is in the disconnected state. After adding a disconnected domain to the **REDHAWK Explorer** view, the domain may be connected by right-clicking the domain and selecting **Connect**.

      - **Connect Once**: This adds the domain to the **REDHAWK Explorer** view and connects the IDE with the domain. When the **REDHAWK IDE** is restarted, the domain remains in the **REDHAWK Explorer** but is in the disconnected state.

      - **Always Connect**: This adds the domain to the **REDHAWK Explorer** view and connects the IDE with the domain. When the **REDHAWK IDE** is restarted, the domain remains in the **REDHAWK Explorer** and attempts to connect with this domain.

6.  Select **Finish** to close the wizard.

7.  The domain now appears in the **REDHAWK Explorer** view. If **Connect Once** or **Always Connect** was chosen, the domain is connected. If **Don't Connect** was selected, right-click the domain and select **Connect**.

> **TIP**  
> Many of these options may be changed later through the <abbr title="See Glossary.">**Properties** view</abbr>.  

