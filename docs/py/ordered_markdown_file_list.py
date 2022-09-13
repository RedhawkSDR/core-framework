# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK.
#
# REDHAWK is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.

"""
The order of the files in `fpaths_ordered` determines how they will be
organized in the browser.
"""
fpaths_ordered = [
    'index.md',

    'Introduction/introduction.md',
    'Introduction/process-management.md',
    'Introduction/basic-sandbox-example.md',
    'Introduction/basic-ide-example.md',
    'Introduction/further-reading.md',

    'Install/install-from-rpm.md',
    'Install/install-from-source.md',
    'Install/external-dependencies.md',
    'Install/redhawk-yum-repo.md',
    'Install/cpp.md',
    'Install/java-and-jacorb.md',
    'Install/python.md',
    'Install/ide.md',
    'Install/troubleshooting.md',

    'Components/components.md',
    'Components/redhawk-core-assets.md',
    'Components/creating-a-component.md',
    'Components/octave-components.md',
    'Components/running-a-component.md',
    'Components/sandbox.md',
    'Components/hello-world-component.md',

    'Component-Structure/_index.md',
    'Component-Structure/generated-files.md',
    'Component-Structure/generated-methods.md',
    'Component-Structure/base-component-members.md',
    'Component-Structure/component-implementations.md',
    'Component-Structure/java-version.md',
    'Component-Structure/manage-properties.md',
    'Component-Structure/events.md',

    'Shared-Libraries/_index.md',
    'Shared-Libraries/creating-a-shared-library-project.md',
    'Shared-Libraries/using-a-shared-library-project.md',
    'Shared-Libraries/packaging-shared-libraries.md',
    'Shared-Libraries/manually-including-external-libraries.md',

    'Connections/_index.md',
    'Connections/the-connection-process.md',
    'Connections/why-ports.md',
    'Connections/port-access.md',
    'Connections/dynamic-connections.md',
    'Connections/standardized-data-interfaces.md',

    'Connections/Bulkio/_index.md',
    'Connections/Bulkio/stream-api.md',
    'Connections/Bulkio/sri.md',
    'Connections/Bulkio/attachablestreams.md',
    'Connections/Bulkio/data-transfers.md',
    'Connections/Bulkio/bit-data.md',
    'Connections/Bulkio/multiout-ports.md',
    'Connections/Bulkio/working-with-complex-data.md',
    'Connections/Bulkio/time-stamps.md',
    'Connections/Bulkio/port-statistics.md',
    'Connections/Bulkio/examples.md',

    'Connections/Burstio/_index.md',
    'Connections/Burstio/data-transfers.md',
    'Connections/Burstio/sri.md',
    'Connections/Burstio/multiout-ports.md',
    'Connections/Burstio/working-with-complex-data.md',
    'Connections/Burstio/time-stamps.md',
    'Connections/Burstio/port-statistics.md',

    'Connections/Messaging/_index.md',
    'Connections/Messaging/message-producer.md',
    'Connections/Messaging/message-consumer.md',
    'Connections/Messaging/viewing-messages.md',
    'Connections/Messaging/connecting-producers-consumers.md',

    'Connections/connection-callbacks.md',
    'Connections/custom-idl-interfaces.md',
    'Connections/add-transport.md',

    'Waveforms/_index.md',
    'Waveforms/waveform-editor.md',
    'Waveforms/sample-waveform.md',
    'Waveforms/deployment-resources.md',

    'Services/_index.md',

    'Devices/_index.md',
    'Devices/fei.md',
    'Devices/Interacting-with-Hardware/_index.md',
    'Devices/Interacting-with-Hardware/creating-fei-device-ide.md',
    'Devices/Interacting-with-Hardware/interacting-fei-device-python-package.md',
    'Devices/Interacting-with-Hardware/using-fei-device-ide.md',
    'Devices/associating-a-waveform.md',
    'Devices/running-components.md',
    'Devices/interface-with-fpgas.md',
    'Devices/fei-data-structures.md',
    'Devices/misc.md',
    'Devices/persona-device-pattern.md',

    'Nodes/_index.md',
    'Nodes/running-a-node.md',
    'Nodes/creating-a-new-node.md',
    'Nodes/distributed-computing-and-rf-devices.md',
    'Nodes/gpp-plugins.md',

    'Shared-Address/components-howto.md',
    'Shared-Address/component-model.md',
    'Shared-Address/component-examples.md',

    'Shared-Memory/shared-memory.md',
    'Shared-Memory/shared-memory-ipc.md',
    'Shared-Memory/metrics.md',
    'Shared-Memory/tuning.md',

    'Sandbox/_index.md',
    'Sandbox/Python/_index.md',
    'Sandbox/Python/working_with_components.md',
    'Sandbox/Python/helpers.md',
    'Sandbox/Python/devices.md',
    'Sandbox/Python/example-interaction.md',
    'Sandbox/Python/sources-and-sinks.md',
    'Sandbox/Python/sdds-data.md',
    'Sandbox/Python/plotting-data.md',
    'Sandbox/Python/misc.md',
    'Sandbox/IDE/_index.md',

    'Runtime-Environment/_index.md',
    'Runtime-Environment/launching-a-domain.md',
    'Runtime-Environment/domain-manager.md',
    'Runtime-Environment/file-system.md',
    'Runtime-Environment/applications.md',
    'Runtime-Environment/application-factory.md',
    'Runtime-Environment/device-manager.md',
    'Runtime-Environment/allocation-manager.md',
    'Runtime-Environment/connection-manager.md',
    'Runtime-Environment/events.md',
    'Runtime-Environment/inspection.md',

    'Logging/_index.md',
    'Logging/logging-structure.md',
    'Logging/configuring-logging-capabilities.md',
    'Logging/adjusting-logging-at-runtime.md',
    'Logging/logging-within-a-resource.md',
    'Logging/viewing-logging-events.md',
    'Logging/logging-config-plugin.md',

    'Containers/containers.md',
    'Containers/in-sandbox.md',
    'Containers/in-a-domain.md',
    'Containers/plugin-class.md',
    'Containers/containerized-components.md',

    'IDE-DEPRECATED/_index.md',
    'IDE-DEPRECATED/launching-the-ide.md',
    'IDE-DEPRECATED/pydev-overview.md',
    'IDE-DEPRECATED/workbench.md',
    'IDE-DEPRECATED/Editors-and-Views/_index.md',
    'IDE-DEPRECATED/Editors-and-Views/softpkg-editor.md',
    'IDE-DEPRECATED/Editors-and-Views/waveform-editor.md',
    'IDE-DEPRECATED/Editors-and-Views/node-editor.md',
    'IDE-DEPRECATED/Editors-and-Views/nextmidas-plot-editor.md',
    'IDE-DEPRECATED/Editors-and-Views/redhawk-explorer-view.md',
    'IDE-DEPRECATED/Editors-and-Views/redhawk-plot-view.md',
    'IDE-DEPRECATED/Editors-and-Views/plot-settings-dialog.md',
    'IDE-DEPRECATED/Editors-and-Views/event-viewer-view.md',
    'IDE-DEPRECATED/Editors-and-Views/data-list-statistics-view.md',
    'IDE-DEPRECATED/Editors-and-Views/port-monitor-view.md',
    'IDE-DEPRECATED/Editors-and-Views/sri-view.md',
    'IDE-DEPRECATED/Editors-and-Views/console-view.md',
    'IDE-DEPRECATED/Editors-and-Views/properties-view.md',
    'IDE-DEPRECATED/creating-redhawk-projects.md',
    'IDE-DEPRECATED/namespaces.md',
    'IDE-DEPRECATED/debugging.md',
    'IDE-DEPRECATED/deploying-projects.md',
    'IDE-DEPRECATED/snapshot-tool.md',
    'IDE-DEPRECATED/connect-wizard.md',
    'IDE-DEPRECATED/octave-wizard.md',
    'IDE-DEPRECATED/plot-port-wizard.md',
    'IDE-DEPRECATED/exploring-sdrroot.md',

    'Exploring-Domain/_index.md',
    'Exploring-Domain/connecting-to-a-domain.md',
    'Exploring-Domain/viewing-domain-contents.md',
    'Exploring-Domain/working-with-waveforms.md',
    'Exploring-Domain/plotting-bulkio-ports.md',
    'Exploring-Domain/increasing-bulkio-connection-bandwidth.md',
    'Exploring-Domain/getting-error-condition-details.md',

    'Appendices/optimization.md',
    'Appendices/sharing-projects.md',
    'Appendices/troubleshoot-connections.md',
    'Appendices/troubleshoot-omni.md',

    'glossary.md',
    'acronyms.md',
]
