# Data List and Statistics Views

The Data List and Statistics views are designed for simple runtime debugging of <abbr title="See Glossary.">component</abbr> <abbr title="See Glossary.">ports</abbr>.

1.  To open the Data List view, right-click a port of a started component.
    ![Port Context Menu](img/DataListImg1.png)

2.  From the context menu, select **Data List**.

    The Data List view is opened:
    ![Data List View](img/DataListView.png)

3.  Select the preferred capture type:

      - **Number of Samples**: Select a sample size
      - **Indefinitely**: Collect until the user stops the process.

4.  Select the **Number of Dimensions** of the sample data:

      - **Real**
      - **Complex**
      - any positive integer number of dimensions

5.  Click **Start Acquire**.

    After all desired samples have been acquired and displayed in the Data List table, two additional options are displayed:

      - **Save**
      - **Chart**

6.  To open a wizard and write the data to a Midas BLUE file or a binary file, click **Save**.

7.  To open the Statistics view, which features a histogram and basic statistics of the sample data, click **Chart**:
    ![Statistics View](img/DataListImg2.png)

8.  There are two ways to change the dimensions displayed in the Statistics view:

      - In the Data List view, click the column headers.
      - In the Statistics view, from the View menu, select **Settings**. This opens the **Chart Options** dialog. In the **Chart Options** dialog, change the categories displayed in the chart:
        ![Statistics View Chart Options Dialog](img/DataListImg3.png)

        The chart and basic statistics refresh with each new collection of data in the Data List view.
