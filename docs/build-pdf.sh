#!/bin/bash

# The order of files/sections in this script are based on the file
# py/ordered_markdown_file_list.py  If that order is changed, this
# file must be updated to match.

# In each directory we will build the individual .md files into an intermediate
# .md file.  If there are multiple intermediate files generated for a directory,
# then those intermediate files will be combined into a single final intermediate
# file for the directory.

cp -dr md build-pdf
PDF=$(readlink -f build-pdf)

cd $PDF
echo "-->  $(pwd)"
pandoc -t markdown -o indexMD.md index.md

cd $PDF/getting-started
echo "-->  $(pwd)"
files="introduction.md process-management.md basic-sandbox-example.md basic-ide-example.md further-reading.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/gettingStarted.md $files
# Insert a top level heading on the first line of the generated .md file.
sed -i '1s/^/# Getting Started\n/' $PDF/gettingStarted.md

cd $PDF/installation
echo "-->  $(pwd)"
files="_index.md redhawk-yum.md dependencies.md ide.md source-install.md troubleshooting.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/indexInstallation.md $files
sed -i '1s/^/# Installation\n/' $PDF/indexInstallation.md

cd $PDF/components
echo "-->  $(pwd)"
files="components.md redhawk-core-assets.md creating-a-component.md"
files+=" octave-components.md running-a-component.md sandbox.md hello-world-component.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/componentsDoc.md $files
sed -i '1s/^/# Components\n/' $PDF/componentsDoc.md

cd $PDF/component-structure
echo "-->  $(pwd)"
files="_index.md generated-files.md generated-methods.md base-component-members.md"
files+=" component-implementations.md java-version.md manage-properties.md events.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/componentStructure.md $files
sed -i '1s/^/# Component Structure\n/' $PDF/componentStructure.md

cd $PDF/shared-libraries
echo "-->  $(pwd)"
files="_index.md creating-a-shared-library-project.md using-a-shared-library-project.md"
files+=" packaging-shared-libraries.md manually-including-external-libraries.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/sharedLibraries.md $files
sed -i '1s/^/# Shared Libraries\n/' $PDF/sharedLibraries.md

cd $PDF/connections
echo "-->  $(pwd)"
files="_index.md the-connection-process.md why-ports.md port-access.md dynamic-connections.md"
files+=" standardized-data-interfaces.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/connections.md $files
cd $PDF/connections/bulkio
echo "-->  $(pwd)"
files="_index.md stream-api.md sri.md attachablestreams.md data-transfers.md bit-data.md multiout-ports.md working-with-complex-data.md time-stamps.md port-statistics.md examples.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/connectionsBulkio.md $files
cd $PDF/connections/burstio
echo "-->  $(pwd)"
files="_index.md data-transfers.md sri.md multiout-ports.md working-with-complex-data.md"
files+=" time-stamps.md port-statistics.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/connectionsBurstio.md $files
cd $PDF/connections/messaging
echo "-->  $(pwd)"
files="_index.md message-producer.md message-consumer.md viewing-messages.md connecting-producers-consumers.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/connectionsMessaging.md $files
cd $PDF/connections
echo "-->  $(pwd)"
files="connection-callbacks.md custom-idl-interfaces.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/connections2.md $files
# Combine the connections files.
pandoc -t markdown -o $PDF/connectionsFinal.md $PDF/connections.md $PDF/connectionsBulkio.md $PDF/connectionsBurstio.md $PDF/connectionsMessaging.md $PDF/connections2.md
sed -i '1s/^/# Connections\n/' $PDF/connectionsFinal.md

cd $PDF/waveforms
echo "-->  $(pwd)"
files="_index.md waveform-editor.md sample-waveform.md deployment-resources.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/waveforms.md $files
sed -i '1s/^/# Waveforms\n/' $PDF/waveforms.md

cd $PDF/services
echo "-->  $(pwd)"
files="_index.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/services.md $files
sed -i '1s/^/# Services\n/' $PDF/services.md

cd $PDF/devices
echo "-->  $(pwd)"
files="_index.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/devicesIndex.md $files
cd $PDF/devices/interacting-with-hardware
echo "-->  $(pwd)"
files="_index.md creating-fei-device-ide.md interacting-fei-device-python-package.md using-fei-device-ide.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/devicesHardware.md $files
cd $PDF/devices
echo "-->  $(pwd)"
files="associating-a-waveform.md running-components.md interface-with-fpgas.md fei-data-structures.md misc.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/devices.md $files
# Combine the devices files.
pandoc -t markdown -o $PDF/devicesFinal.md $PDF/devicesIndex.md $PDF/devicesHardware.md $PDF/devices.md
sed -i '1s/^/# Devices\n/' $PDF/devicesFinal.md

cd $PDF/nodes
echo "-->  $(pwd)"
files="_index.md running-a-node.md creating-a-new-node.md distributed-computing-and-rf-devices.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/nodes.md $files
sed -i '1s/^/# Nodes\n/' $PDF/nodes.md

cd $PDF/sandbox
echo "-->  $(pwd)"
files="_index.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/sandbox.md $files
cd $PDF/sandbox/python
echo "-->  $(pwd)"
files="_index.md working_with_components.md helpers.md devices.md example-interaction.md"
files+=" sources-and-sinks.md sdds-data.md plotting-data.md misc.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/sandboxPython.md $files
cd $PDF/sandbox/ide
echo "-->  $(pwd)"
files="_index.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/sandboxIde.md $files
# Combine the sandbox files.
pandoc -t markdown -o $PDF/sandboxFinal.md $PDF/sandbox.md $PDF/sandboxPython.md $PDF/sandboxIde.md
sed -i '1s/^/# Sandbox\n/' $PDF/sandboxFinal.md

cd $PDF/runtime-environment
echo "-->  $(pwd)"
files="_index.md launching-a-domain.md domain-manager.md file-system.md applications.md"
files+=" application-factory.md device-manager.md allocation-manager.md connection-manager.md events.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/runtimeEnv.md $files
sed -i '1s/^/# Runtime Environment\n/' $PDF/runtimeEnv.md

cd $PDF/runtime-inspection
echo "-->  $(pwd)"
files="_index.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/runtimeInspection.md $files
sed -i '1s/^/# Runtime Inspection\n/' $PDF/runtimeInspection.md

cd $PDF/logging
echo "-->  $(pwd)"
files="_index.md logging-structure.md configuring-logging-capabilities.md"
files+=" adjusting-logging-at-runtime.md logging-within-a-resource.md viewing-logging-events.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/logging.md $files
sed -i '1s/^/# Logging\n/' $PDF/logging.md

cd $PDF/ide
echo "-->  $(pwd)"
files="_index.md launching-the-ide.md pydev-overview.md workbench.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/ide.md $files
cd ../ide/editors-and-views
echo "-->  $(pwd)"
files="_index.md softpkg-editor.md waveform-editor.md node-editor.md nextmidas-plot-editor.md"
files=" redhawk-explorer-view.md redhawk-plot-view.md plot-settings-dialog.md event-viewer-view.md"
files+=" data-list-statistics-view.md port-monitor-view.md sri-view.md console-view.md properties-view.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/ideEditors.md $files
cd $PDF/ide
echo "-->  $(pwd)"
files="creating-redhawk-projects.md namespaces.md debugging.md deploying-projects.md"
files+=" snapshot-tool.md connect-wizard.md octave-wizard.md plot-port-wizard.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/ide2.md $files
# Combine the IDE files.
pandoc -t markdown -o $PDF/ideFinal.md $PDF/ide.md $PDF/ideEditors.md $PDF/ide2.md
sed -i '1s/^/# IDE\n/' $PDF/ideFinal.md

cd $PDF/exploring-sdrroot
echo "-->  $(pwd)"
files="_index.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/exploringSDR.md $files
sed -i '1s/^/# Exploring sdrroot\n/' $PDF/exploringSDR.md

cd $PDF/sharing-projects
echo "-->  $(pwd)"
files="_index.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/sharing.md $files
sed -i '1s/^/# Sharing Projects\n/' $PDF/sharing.md

cd $PDF/exploring-domain
echo "-->  $(pwd)"
files="_index.md connecting-to-a-domain.md viewing-domain-contents.md working-with-waveforms.md"
files+=" plotting-bulkio-ports.md increasing-bulkio-connection-bandwidth.md getting-error-condition-details.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/exploringDomain.md $files
sed -i '1s/^/# Exploring Domain\n/' $PDF/exploringDomain.md

cd $PDF/appendices
echo "-->  $(pwd)"
files="optimization.md fei.md persona-device-pattern.md shared-memory.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/appendices.md $files
cd $PDF/appendices/troubleshooting
echo "-->  $(pwd)"
files="omni.md connections.md _index.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/appendicesTroubleshooting.md $files
cd $PDF/appendices
echo "-->  $(pwd)"
files="logging-config-plugin.md"
sed -i 's,\(^#\)\([# ]\),#\1\2,g' $files  # Decrease heading levels so table of contents will be correct.
pandoc -t markdown -o $PDF/appendices3.md $files
# Combine the appendices files.
pandoc -t markdown -o $PDF/appendicesFinal.md $PDF/appendices.md $PDF/appendicesTroubleshooting.md $PDF/appendices3.md
sed -i '1s/^/# Appendices\n/' $PDF/appendicesFinal.md

cd $PDF
echo "-->  $(pwd)"
pandoc -t markdown -o $PDF/glossaryAndAcronyms.md glossary.md acronyms.md

# Build final PDF from all the intermediate output .md files
files="indexMD.md gettingStarted.md indexInstallation.md componentsDoc.md componentStructure.md"
files+=" sharedLibraries.md connectionsFinal.md waveforms.md services.md devicesFinal.md"
files+=" nodes.md sandboxFinal.md runtimeEnv.md runtimeInspection.md logging.md ideFinal.md"
files+=" exploringSDR.md sharing.md exploringDomain.md appendicesFinal.md glossaryAndAcronyms.md"
pandoc -f markdown -t latex --toc-depth=3 --toc -o ../RedhawkManual.pdf $files

# tree used during build-pdf.sh

# md/
#     images/
# out/
#     md/
#         md/
#             images/
#             gettingStarted.md
#             indexMD.md
#             indexInstallation.md
#             componentsDoc.md
#             componentStructure.md
#             sharedLibraries.md
#             connections.md
#             connectionsBulkio.md
#             connectionsBurstio.md
#             connectionsMessaging.md
#             connections2.md
#             connectionsFinal.md
#             waveforms.md
#             services.md
#             devicesIndex.md
#             devices
