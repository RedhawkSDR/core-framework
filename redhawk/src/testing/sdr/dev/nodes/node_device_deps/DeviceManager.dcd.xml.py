<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE deviceconfiguration PUBLIC "-//JTRS//DTD SCA V2.2.2 DCD//EN" "deviceconfiguration.dtd">
<deviceconfiguration id="node_devpy" name="node_devpy">
  <devicemanagersoftpkg>
    <localfile name="/mgr/DeviceManager.spd.xml"/>
  </devicemanagersoftpkg>
  <componentfiles>
    <componentfile id="devpy_file_1" type="SPD">
      <localfile name="/devices/devpy/devpy.spd.xml"/>
    </componentfile>
  </componentfiles>
  <partitioning>
    <componentplacement>
      <componentfileref refid="devpy_file_1"/>
      <componentinstantiation id="devpy_1">
        <usagename>devpy_1</usagename>
      </componentinstantiation>
    </componentplacement>
  </partitioning>
  <domainmanager>
    <namingservice name="REDHAWK_DEV/REDHAWK_DEV"/>
  </domainmanager>
</deviceconfiguration>
