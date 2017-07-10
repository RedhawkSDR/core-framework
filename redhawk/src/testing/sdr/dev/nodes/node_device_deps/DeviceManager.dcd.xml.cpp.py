<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE deviceconfiguration PUBLIC "-//JTRS//DTD SCA V2.2.2 DCD//EN" "deviceconfiguration.dtd">
<deviceconfiguration id="node_dev_cpp_py" name="node_dev_cpp_py">
  <devicemanagersoftpkg>
    <localfile name="/mgr/DeviceManager.spd.xml"/>
  </devicemanagersoftpkg>
  <componentfiles>
    <componentfile id="devpy_file_1" type="SPD">
      <localfile name="/devices/devpy/devpy.spd.xml"/>
    </componentfile>
    <componentfile id="devcpp_file_1" type="SPD">
      <localfile name="/devices/devcpp/devcpp.spd.xml"/>
    </componentfile>
  </componentfiles>
  <partitioning>
    <componentplacement>
      <componentfileref refid="devcpp_file_1"/>
      <componentinstantiation id="devcpp_1">
        <usagename>devp_1</usagename>
      </componentinstantiation>
    </componentplacement>
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

