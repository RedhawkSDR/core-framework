# omniNames and omniEvents

This section explains how to troubleshoot and resolve `omniNames` and `omniEvents` issues.

## Performing a Hard Reset Using the `cleanomni` Script

The `cleanomni` script is used to perform a hard reset of `omniNames` and `omniEvents` and delete their associated log files. To run this script, enter the following command:

```bash
sudo $OSSIEHOME/bin/cleanomni
```

The `cleanomni` script performs the following:

- Stops the `omniNames` service
- Stops the `omniEvents` service
- Removes all the old log files from `omniNames` and `omniEvents`
- Starts `omniNames`
- Starts `omniEvents`

## Performing a Soft Reset of `omniNames` and `omniEvents`

If the runtime-error indicates a <abbr title="See Glossary.">Naming Service</abbr> failure, enter the following command to attempt a soft reset on `omniNames`:

```bash
# CentOS 7 command
sudo systemctl restart omniNames

# CentOS 6 command
sudo /sbin/service omniNames restart
```

This process first performs a stop and then performs a start. If the stop process fails, `omniNames` was never started, stopped due to an error condition, or is in a non-recoverable state. If the start process fails, `omniNames` is either misconfigured or already running (i.e., `omniNames` was not stopped).


> **NOTE**  
> `omniNames` can potentially report a successful start and then fail soon after. If the `omniNames` service appears to fail after reporting a successful start, a reconfiguration and hard reset of `omniNames` may be necessary. For more information, refer to [Common Failures](#common-causes-for-omninames-failure) and [Using `cleanomni`](#performing-a-hard-reset-using-the-cleanomni-script).  

A restart of `omniEvents` may be necessary when restarting `omniNames`. To perform a soft reset of `omniEvents`, enter the following commands:

```bash
# CentOS 7 command
sudo systemctl restart omniEvents

# CentOS 6 command
sudo /sbin/service omniEvents restart
```

## Setting Omni Log Levels

When diagnosing `omniNames/omniEvents` problems, it is often useful to set the omni logging levels. Use the following procedure to set the omni logging levels (requires root permissions):

1.  Open the `/etc/omniORB.cfg` file.
2.  Set a traceLevel value. For example:
    ```bash
    traceLevel = 10
    ```

Details on the available trace levels can be found in Chapter 4 of the omniORB User's Guide (http://omniorb.sourceforge.net/omni41/omniORB/omniORB004.html) (CentOS 6) and (http://omniorb.sourceforge.net/omni42/omniORB/omniORB004.html) (CentOS 7) or on your local system at <file:///usr/share/doc/omniORB-devel-4.1.6/doc/omniORB/omniORB004.html> (CentOS 6) and at <file:///usr/share/doc/omniORB-devel-4.2.0/doc/omniORB/omniORB004.html> (CentOS 7).

For the changes to take effect, restart `omniNames/omniEvents`.

Log messages are displayed in the terminal and in the files contained in `/var/log/omniORB` and `/var/lib/omniEvents`.

## Common Causes for `omniNames` Failure

This section identifies the most common causes for omniNames failures.

### IP Version 6 Conflicts

Certain combinations of IP Version 6 (IPv6) configurations and `/etc/omniORB.cfg` configurations can cause `omniNames` failures.

Specifically, if the `InitRef` section of `/etc/omniORB.cfg` is set to point to `localhost` rather than pointing explicitly to `127.0.0.1`, the operating system may resolve `localhost` to `::1` (the IPv6 localhost) and not to `127.0.0.1` (the IPv4 localhost). If this occurs, `omniNames` fails. There are three options for preventing this failure condition:

  - Explicitly set `127.0.0.1` in the `InitRef` section instead of using `localhost`.
  - Disable IPv6 in the operating system (refer to operating system documentation).
  - Modify the `/etc/hosts` file to prevent `localhost` from being resolved as `::1`.

#### Preventing IPv6 `localhost` Resolution

Below is an example `/etc/hosts` file from an older CentOS distribution:

```/etc/hosts
127.0.0.1   localhost.localdomain     localhost
::1         localhost6.localdomain6   localhost6
```

Below is an example `/etc/hosts` file from a newer CentOS distribution:

```/etc/hosts
127.0.0.1 localhost localhost.localdomain localhost4 localhost4.localdomain4
::1       localhost localhost.localdomain localhost6 localhost6.localdomain6
```

In the older `/etc/hosts file`, `localhost` resolves unambiguously to `127.0.0.1`. In the newer `/etc/hosts` file, `localhost` can resolve to either `127.0.0.1` or `::1` (where resolving to `::1` causes an `omniNames` failure).

The newer `/etc/hosts` file can be modified to read:

```/etc/hosts
127.0.0.1 localhost localhost.localdomain localhost4 localhost4.localdomain4
::1       localhost6 localhost6.localdomain6
```

Alternatively, `localhost4` can be used in the `InitRef` section of `/etc/omniORB.cfg`.

The line pertaining to IPv6 can also be completely removed from the file; however, some operating systems, depending on IPv6 configurations, may automatically repopulate IPv6 localhost settings on reboot.

### Invalid IP Addresses in `/etc/hosts`

Invalid entries in the `/etc/hosts` file may induce an `omniNames` failure. Invalid entries may be in the form of an IP address that cannot be reached or in the form of an entry that is not valid according to the `/etc/hosts` grammar.

> **NOTE**  
> Firewall IP and port settings on both the server and client side may cause the target `omniNames` service to be unreachable.  
