
package bulkio;

import BULKIO.PrecisionUTCTime;

public class SampleTimestamp
{
    /**
        * @brief  Constructor.
        * @param time       Time stamp.
        * @param offset     Sample offset.
        * @param synthetic  False if @p time was received, true if
        *                   interpolated.
        */
    public SampleTimestamp(final BULKIO.PrecisionUTCTime in_time, int in_offset, boolean in_synthetic) {
        time = in_time;
        offset = in_offset;
        synthetic = in_synthetic;
    };

    /// @brief  The time at which the referenced sample was created.
    public BULKIO.PrecisionUTCTime time;

    /// @brief  The 0-based index of the sample at which @a time applies.
    public int offset;

    /// @brief  Indicates whether @a time was interpolated.
    public boolean synthetic;
};
