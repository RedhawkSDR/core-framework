# Increasing the Bandwidth of BulkIO Connections

In the presence of high data rates, plots of Bulk Input/Output (BulkIO) <abbr title="See Glossary.">ports</abbr> may not be able to keep up with the data stream. To increase the bandwidth of BulkIO CORBA connections, it is possible to connect using native omniORB libraries. This ability is currently disabled by default. The following procedure explains how to enable this ability from within the IDE:

1.  Select **Window  >  Preferences**.

    The **Preferences** dialog is displayed:

    ![Preferences Dialog](images/bulkioprefs.png)

2.  Expand **REDHAWK**.

3.  Select **BulkIO**.

4.  Set **Port Factory** to `omnijni`.

5.  Click **OK**.


> **NOTE**  
> This option should only be enabled if the <abbr title="See Glossary.">domain</abbr> matches the version of the IDE, and your <abbr title="See Glossary.">application</abbr> requires the increased performance of `omnijni`.  
