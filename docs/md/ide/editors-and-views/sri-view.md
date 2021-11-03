# SRI View

The Signal Related Information (SRI) view enables the user to view the [SRI data](../../connections/bulkio/sri.html) from a particular <abbr title="See Glossary.">port</abbr>.  The following procedure explains how to open the SRI view.

1.  To open the SRI view, right-click the port of a started <abbr title="See Glossary.">component</abbr> and select **Display SRI** from the context menu:
    ![Port Context Menu with Display SRI Selected](../images/SRIMenu.png)

    The SRI view is opened:
    ![SRI View](../images/SRIView.png)

2.  The following options are available in the SRI view.

      - To clear the display of specific SRI data once an End of Stream (EOS) has been received, click **Clear Selected SRI**.
      - To clear the display of all SRI data once the respective EOSs have been received, click **Clear All SRIs**.
      - To pause the SRI data, click **Pause Incoming SRI Data**.
      - To receive notification when new SRI data is displayed, click **Notify on receiving new Push SRI**.
      - To change the active SRI data stream between different active components, click **Change Active Stream**.
      - To generate both an `.xml` and a `.SRI` file for each stream being monitored, click **Save SRI Data to file**.


> **TIP**  
> If a component is deleted from the Chalkboard, the SRI view closes automatically.  
