# Connections

This section explains how to diagnose connection issues.

## Diagnosing Problems Using the `rh_net_diag` Script

The `rh_net_diag` script is a Python script used to diagnose various omniORB-related problems and to perform other system checks to diagnose potential problems that may impact a REDHAWK installation. To run this script, enter the following command:

```bash
rh_net_diag
```

By default, the `--ns` (<abbr title="See Glossary.">Naming Service</abbr>), `--dom` (<abbr title="See Glossary.">Domain Manager</abbr>), and `--dev` (<abbr title="See Glossary.">Device Manager</abbr>) options are enabled when the script is executed. These options assume that `omniNames`, the Domain Manager, and the Device Manager are all running on the host executing the script. For help with `rh_net_diag`, enable the `-h` option. For more detailed output, enable the `-v` option. The four test categories include:

  - Standard tests performed every time `rh_net_diag` is executed.
      - Check if `OSSIEHOME`, `SDRROOT`, and `/etc/omniORB.cfg` are readable.
      - Check that omniORB is installed.
      - If `/etc/omniORB.cfg` is not readable or if omniORB is not installed, the script terminates.

  - Tests that diagnose potential problems with the Naming Service. They are performed if `--ns` is enabled.
      - Check if `omniNames` is running.
      - Check if `omniEvents` is running and if it is running locally. If it is not running at all, refer to [Performing a Hard Reset Using the cleanonmi Script](../../appendices/troubleshooting/omni.html#performing-a-hard-reset-using-the-cleanomni-script). If it is not running locally, assume it is running on another host.
      - Retrieve entries currently stored in the Naming Service. Refer to [Common Causes for `omniNames` Failure](../../appendices/troubleshooting/omni.html#common-causes-for-omninames-failure) for further assistance.
      - Check `/etc/omniORB.cfg` to ensure that all defined endPoints are correct.

> **NOTE**  
> If all the above checks pass, but the Domain Manager and Device Manager still cannot communicate with the Naming Service, manually check the firewall settings on the host running `omniNames`. Confirm there is a firewall rule that allows for new connections between hosts. For example, the following `iptables` rule allows new TCP connections from subnet 192.168.1.0:
>
> `INPUT -s 192.168.1.0/24 -p tcp -m state --state NEW,ESTABLISHED -j ACCEPT`

  - Tests that diagnose potential problems with the Domain Manager. They are performed if `--dom` is enabled.
      - Attempt to retrieve entries currently stored in the Naming Service.
      - Check if `omniEvents` is running and if it is running locally. If it is not running at all, refer to [Performing a Hard Reset Using the cleanonmi Script](../../appendices/troubleshooting/omni.html#performing-a-hard-reset-using-the-cleanomni-script). If it is not running locally, assume it is running on another host.
      - Check `/etc/omniORB.cfg` to ensure all defined endPoints are correct.

  - Tests that diagnose potential problems with the Device Manager. They are performed if `--dev` is enabled.
      - Check if `InitRef` was overwritten with the `rh_net_diag` script `--ORBInitRef` option or if we are using the `InitRef` specified in `/etc/omniORB.cfg`.
      - If `InitRef` is valid, attempt to retrieve entries currently stored in the Naming Service. Refer to [Common Causes for `omniNames` Failure](../../appendices/troubleshooting/omni.html#common-causes-for-omninames-failure) for further assistance.
      - Check if `omniEvents` is running and if it is running locally. If it is not running at all, refer to [Performing a Hard Reset Using the cleanonmi Script](../../appendices/troubleshooting/omni.html#performing-a-hard-reset-using-the-cleanomni-script). If it is not running locally, assume it is running on another host.
      - Try to connect to the Domain Manager if one exists in the Naming Service.
      - Check that the IP address for the host running this script is listed in `ifconfig`. If there is no matching entry with the Device Manager, then the Java components fail on initialization.
