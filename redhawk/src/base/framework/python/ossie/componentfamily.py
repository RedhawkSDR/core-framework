
import ossie
import traceback
from ossie.cf import CF
from omniORB import any as _any
from ossie.utils import model

class DynamicComponent:
    def __init__(self):
        self._parentInstance = None
        self._dynamicComponents = []
        self._dynamicComponentCount = {}

    def addInstance(self, child):
        if not self._parentInstance:
            raise Exception('No parent device set, setParentInstance should have been invoked on device deployment')

        child_device = self._parentInstance.addInstance(child, self)
        return child_device

class DynamicComponentParent(DynamicComponent):
    def removeInstance(self, child):
        pass

    def addInstance(self, child, parent=None):
        if not parent:
            parent = self
        device_object = None
        try:
            self._cmdLock.acquire()
            with ossie.device.envState():
                if type(child) == str:
                    device_name = child
                else:
                    device_name = child.__name__
                parameters = []
                if not parent._dynamicComponentCount.has_key(device_name):
                    parent._dynamicComponentCount[device_name] = 0
                parent._dynamicComponentCount[device_name] += 1
                device_name_count = device_name+'_'+str(parent._dynamicComponentCount[device_name])
                device_label = parent._label+':'+device_name_count
                parameters.append(CF.DataType('IDM_CHANNEL_IOR', _any.to_any(ossie.resource.__orb__.object_to_string(self._idm_publisher.channel))))
                parameters.append(CF.DataType('DEVICE_LABEL', _any.to_any(device_label)))
                parameters.append(CF.DataType('PROFILE_NAME', _any.to_any('')))
                parameters.append(CF.DataType('DEVICE_MGR_IOR', _any.to_any(ossie.resource.__orb__.object_to_string(self._devMgr.ref))))
                parameters.append(CF.DataType('DEVICE_ID', _any.to_any(parent._id+':'+device_name_count)))
                parameters.append(CF.DataType('COMPOSITE_DEVICE_IOR', _any.to_any(ossie.resource.__orb__.object_to_string(self._this()))))

                execparams = {}
                for param in parameters:
                    if param.value.value() != None:
                        # SR:453 indicates that an InvalidParameters exception should be
                        # raised if the input parameter is not of a string type; however,
                        # version 2.2.2 of the SCA spec is less strict in its wording. For
                        # our part, as long as the value can be stringized, it is accepted,
                        # to allow component developers to use more specific types.
                        try:
                            execparams[str(param.id)] = str(param.value.value())
                        except:
                            raise CF.ExecutableDevice.InvalidParameters([param])

                mod = __import__(device_name)
                kclass = getattr(mod, device_name+'_i')
                device_object = self.local_start_device(kclass, execparams, parent_instance=self)
                parent._dynamicComponents.append(device_object)
        finally:
            self._cmdLock.release()
        return device_object

    def local_start_device(self, deviceclass, execparams, interactive_callback=None, thread_policy=None,loggerName=None, skip_run=False, parent_instance=None):
        ossie.device._checkForRequiredParameters(execparams)
        try:
            try:
                orb = ossie.resource.createOrb()
                ossie.resource.__orb__ = orb

                ## sets up backwards compat logging 
                ossie.resource.configureLogging(execparams, loggerName, orb)

                devicePOA = ossie.resource.getPOA(orb, thread_policy, "devicePOA")

                devMgr = ossie.device._getDevMgr(execparams, orb)

                parentdev_ref = ossie.device._getParentAggregateDevice(execparams, orb)

                # Configure logging (defaulting to INFO level).
                label = execparams.get("DEVICE_LABEL", "")
                id = execparams.get("DEVICE_ID", "")
                log_config_uri = execparams.get("LOGGING_CONFIG_URI", None)
                debug_level = execparams.get("DEBUG_LEVEL", None)
                if debug_level != None: debug_level = int(debug_level)
                dpath=execparams.get("DOM_PATH", "")
                category=loggerName
                try:
                    if not category and label != "": category=label.rsplit("_", 1)[0]
                except:
                    pass 
                ctx = ossie.logger.DeviceCtx( label, id, dpath )
                ossie.logger.Configure( log_config_uri, debug_level, ctx, category )

                component_Obj = deviceclass(devMgr, 
                                            execparams["DEVICE_ID"], 
                                            execparams["DEVICE_LABEL"], 
                                            execparams["PROFILE_NAME"],
                                            parentdev_ref,
                                            execparams)
                devicePOA.activate_object(component_Obj)

                idm_channel_ior=execparams.get("IDM_CHANNEL_IOR",None)
                registrar_ior=execparams.get("DEVICE_MGR_IOR",None)
                component_Obj.postConstruction( registrar_ior, idm_channel_ior )

                # set logging context for resource to supoprt CF::Logging
                component_Obj.saveLoggingContext( log_config_uri, debug_level, ctx )

                objectActivated = True
                obj = devicePOA.servant_to_id(component_Obj)
                if parent_instance:
                    component_Obj.setParentInstance(parent_instance)
                component_Obj.initialize()

                return component_Obj

            except SystemExit:
                pass
            except KeyboardInterrupt:
                pass
            except:
                traceback.print_exc()
        finally:
            pass

