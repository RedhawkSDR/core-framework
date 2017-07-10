<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE deviceconfiguration PUBLIC "-//JTRS//DTD SCA V2.2.2 DCD//EN" "deviceconfiguration.dtd">
<deviceconfiguration id="node_devj" name="nd_devj">
  <devicemanagersoftpkg>
    <localfile name="/mgr/DeviceManager.spd.xml"/>
  </devicemanagersoftpkg>
  <componentfiles>
    <componentfile id="devj_file_1" type="SPD">
      <localfile name="/devices/devj/devj.spd.xml"/>
    </componentfile>
  </componentfiles>
  <partitioning>
    <componentplacement>
      <componentfileref refid="devj_file_1"/>
      <componentinstantiation id="devj_1">
        <usagename>devj_1</usagename>
      </componentinstantiation>
    </componentplacement>
  </partitioning>
  <domainmanager>
    <namingservice name="REDHAWK_DEV/REDHAWK_DEV"/>
  </domainmanager>
</deviceconfiguration>
