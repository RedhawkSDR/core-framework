# Troubleshooting

This section explains how to troubleshoot and resolve REDHAWK installation issues.

## Setting Host Architecture/Computer Processor Name

When installing RPMs, if the host architecture (computer processor) cannot be determined, the following error is displayed:
```
Cannot determine processor name.
```
Run the following command with the appropriate processor name (for example, x86_64):
```
./gpp_setup --gppcfg --processorname <processor name>
```
In the event of a 'Permission Denied' error, change the permissions with the following command and rerun the gpp_setup command:
```
sudo chmod g+rw /var/redhawk/sdr/dev/devices/GPP/GPP.prf.xml
```

To provide a processor name, run the following command, where `<processor name>` is the host architecture (for example, `x86_64`):

```bash
./gpp_setup --gppcfg --processorname <processor name>
```

In the event of a `Permission Denied` error, run the following command (requires root permissions) to change the permissions for the file, `GPP.prf.xml`, which stores the processor name.

```bash
sudo chmod g+rw /var/redhawk/sdr/dev/devices/GPP/GPP.prf.xml
```
