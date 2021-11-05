# Snapshot Tool

The Snapshot Tool enables users to save data from any Bulk Input/Output (BulkIO) <abbr title="See Glossary.">port</abbr> to a file. The following procedure explains how to use the Snapshot Tool.

1.  To open the **Snapshot Wizard**, right-click an output port in the Chalkboard or the <abbr title="See Glossary.">REDHAWK Explorer</abbr> and select **Snapshot** from the context menu:
    ![Output Port Context Menu](img/snapshotContextMenu.png)

    The **Snapshot Wizard** is displayed:
    ![Snapshot Wizard](img/snapshotWizard.png)

2.  To specify how much data is captured, select the capture mode from the first combo box. The following capture modes are supported:

      - **Number of Samples**: Collects the number of samples specified by the text field to the right.
      - **Indefinitely**: Collects samples until the user stops the snapshot or an End of Stream (EOS) occurs.
      - **Clock Time**: Collects samples for a set period of time in real time set by the text field to the right (in seconds).
      - **Sample Time**: Collects samples for a period of time as specified by the delta and number of samples (time = delta \* number of samples), set in the text field to the right.

3.  Optionally, enter a custom Connection ID in the **Connection ID (Optional)** field.

4.  To specify the file type to use when saving data, select a file type from the **File Type** combo box. The supported file types include:

      - **Binary files (.bin & .SRI)**: Saves data from the port to a `.bin` file and saves the metadata (Signal Related Information (SRI), start time, end time, data type, and number of samples) to an `.SRI` file.
      - **Binary files (.bin & .xml)**: Saves data from the port to a `.bin` file and saves the metadata (SRI, start time, end time, data type, and number of samples) to an `.xml` file.
      - **Midas BLUE File (.tmp)**: Saves output data and metadata to a BLUE `.tmp` file.

5.  To specify where to save the file, you have two options.

      - To save the file to a location other than the <abbr title="See Glossary.">workspace</abbr>, deselect the **Save to Workspace** checkbox, click **Browse**, navigate to the desired location, and click **OK**.
      - To save the file in the workspace, select the **Save to Workspace** checkbox and select the file or directory in the displayed tree of workspace files and folders. Saving the file in the workspace automatically refreshes the project, and the files can be accessed in the IDE.
        ![Snapshot Save to Workspace Navigation Tree](img/snapshotWizard5.png)

        To add folders to the workspace, right-click the existing folder where the new folder is desired. A **New Folder** window is displayed. Enter the name of the folder and click **Finish**.
        ![Snapshot New Folder Window](img/newFolder.png)

        To delete files from the workspace, right-click the item you want to delete and select **Delete**. When prompted, verify the deletion request.

6.  To be prompted before existing files are overwritten when the new snapshot is created, select the **Confirm overwrite** checkbox. If this option is not selected, any existing files are automatically overwritten when the new snapshot is created.

7.  Click **Finish**.

    The Progress view is displayed. To stop a snapshot prematurely, click **Cancel Operation** (the red square icon) next to the job in the Progress view.
    ![Snapshot - Progress View](img/progressView.png)

    When the snapshot completes, the completed job is shown in the Progress view:

    ![Snapshot - Completed Job](img/progressViewDone.png)

8.  To view the results of the snapshot, click **Finished**. The following output message is displayed:
    ![Snapshot - Results Dialog](img/results.png)

    If only a few files are written, then the output message lists all the files created by the snapshot. If a large number of files are written, the output message lists the base name of the files and the number of each type of file made.

9.  Click **OK**.
