# Time Stamps

Bulk Input/Output (BulkIO) uses `BULKIO::PrecisionUTCTime` time stamps that denote the time since 12:00 AM January 1, 1970 (Unix epoch) in UTC. The time stamp contains several elements. In BulkIO, a time stamp corresponds to the date of birth of the first element in the data being pushed. The following table describes the different elements making up the `BULKIO::PrecisionUTCTime` structure.

##### Elements in `BULKIO::PrecisionUTCTime`
| **Identifier** | **Value**                                                        | **Type** |
| :------------- | :--------------------------------------------------------------- | :------- |
| `tcmode`       | timecode mode                                                    | short    |
| `tcstatus`     | timecode status                                                  | short    |
| `toff`         | Fractional sample offset                                         | double   |
| `twsec`        | Number of seconds since 12:00 AM January 1, 1970 (Unix epoch)    | double   |
| `tfsec`        | Number of fractional seconds (0.0 to 1.0) to be added to `twsec` | double   |

Two of the elements described in the table above correspond to predefined values. `tcstatus` can only take two values, `TCS_INVALID` (0), and `TCS_VALID` (1), showing whether the time stamp is valid or not. Invalid time stamps do not contain valid time data and should be ignored. `tcmode` is the method by which the timestamp was obtained, but this use has since been deprecated, and this value is ignored. The default value for `tcmode` is 1.

The following code snippets provide examples of how to construct a time stamp to be sent in the `pushPacket()` call. The `now()` method returns the current time of day.

C++:

```cpp
BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();
```

Python:

```python
tstamp = bulkio.timestamp.now()
```

Java:

```Java
BULKIO.PrecisionUTCTime tstamp = bulkio.time.utils.now();
```

### Timestamp Operators (C++)

In C++, `BULKIO::PrecisionUTCTime` supports common arithmetic, comparison and stream operators.

Adding an offset to a time stamp:

```cpp
/**
 * Add 1/8th of a second to the current time
 */
BULKIO::PrecisionUTCTime time1 = bulkio::time::utils::now();
time1 += 0.125;
```

Subtracting two time stamps returns the difference in seconds:

```cpp
/**
 * Check if time2 is less than a second after time1
 */
if (time2 - time1 < 1.0) {
  ...
}
```

Comparing two time stamps:

```cpp
/**
  * Check if the second time stamp occurs before the first
 */
if (time2 < time1) {
  ...
}
```

Stream formatting (output format is "YYYY:MM:DD::HH::MM::SS.SSSSSS"):

```cpp
/**
 * Write the current time out to the console
 */
std::cout << bulkio::time::utils::now() << std::endl;
```

### Timestamp Operators (Python)

In Python, `BULKIO.PrecisionUTCTime` supports common arithmetic, comparison and string conversion operators.

Adding an offset to a time stamp:

```python
# Add 1/8th of a second to the current time
tstamp = bulkio.timestamp.now()
tstamp += 0.125
```

Subtracting two time stamps returns the difference in seconds:

```python
# Check if time2 is less than a second after time1
if (time2 - time1) < 1.0:
  ...
```

Comparing two time stamps:

```python
# Check if the second time stamp occurs before the first
if time2 < time1:
  ...
```

String formatting (output format is "YYYY:MM:DD::HH::MM::SS.SSSSSS"):

```python
# Write the current time out to the console
print str(bulkio.timestamp.now())
```

### Timestamp Helpers (Java)

In Java, the `bulkio.time.utils` class provides static helper methods for common arithmetic, comparison and string conversion operations.

Adding an offset to a time stamp with `increment()` modifies the original time stamp:

```Java
// Add 1/8th of a second to the current time
BULKIO.PrecisionUTCTime tstamp = bulkio.time.utils.now();
bulkio.time.utils.increment(tstamp, 0.125);
```

Adding an offset to a time stamp with `add()` returns a new time stamp with the result, leaving the original time stamp unmodified:

```Java
// Add a second to the current time
BULKIO.PrecisionUTCTime time1 = bulkio.time.utils.now();
BULKIO.PrecisionUTCTime time2 = bulkio.time.utils.add(time1, 1.0);
```

Calculating the difference in seconds between two time stamps:

```Java
// Check if time2 is less than a second after time1
if (bulkio.time.utils.difference(time2, time1) < 1.0) {
  ...
}
```

Comparing two time stamps:

```Java
// Check if the second time stamp occurs before the first
if (bulkio.time.utils.compare(time2, time1) < 0) {
  ...
}
```

The `compare()` method follows the same rules as `java.util.Comparator`.

String formatting (output format is "YYYY:MM:DD::HH::MM::SS.SSSSSS"):

```Java
// Write the current time out to the console
System.out.println(bulkio.time.utils.now());
```
