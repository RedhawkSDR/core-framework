# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of Docker REDHAWK.
#
# Docker REDHAWK is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# Docker REDHAWK is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

VERSION := $(or $(VERSION), $(VERSION), 2.2.8)

image_prefix := geontech/redhawk
base := $(image_prefix)-base
omni := $(image_prefix)-omniserver
runtime := $(image_prefix)-runtime
redhawk_images := \
	$(image_prefix)-development \
	$(image_prefix)-domain \
	$(image_prefix)-gpp \
    $(image_prefix)-usrp \
	$(image_prefix)-rtl2832u \
	$(image_prefix)-bu353s4
redhawk_webserver := $(image_prefix)-webserver
all_images := $(base) $(omni) $(runtime) $(redhawk_images) $(redhawk_webserver)
reversed := $(redhawk_webserver) $(redhawk_images) $(runtime) $(omni) $(base)

linked_scripts := omniserver domain sdrroot login gpp rhide volume-manager webserver usrp rtl2832u bu353s4 show-log

# Default REST-python server and branch
REST_PYTHON := https://github.com/GeonTech/rest-python.git
REST_PYTHON_BRANCH := master

# Macros for querying an image vs. building one.
image_dockerfile = $(shell echo "$(subst $(image_prefix)-,,$1).Dockerfile")
image_tmpl = $(shell echo "./templates/$(call image_dockerfile,$1)")
image_def = $(shell echo "./Dockerfiles/$(call image_dockerfile,$1)")
image_patch = $(shell sed -E "s/(geontech.+\:)(.+)/\1"$(VERSION)"/g" $(call image_tmpl,$1) > $(call image_def,$1))
image_check = $(strip $(shell docker images -q $1))
image_build = docker build --rm \
		$2 \
		-f $(call image_def,$1) \
		-t $1:$(VERSION) \
		./Dockerfiles \
		&& \
	docker tag $@:$(VERSION) $@:latest

.PHONY: all build deliver clean $(all_images)

# Everything, pulled
all: $(linked_scripts)
	@for image in $(all_images) ; do \
		docker pull $$image:$(VERSION) ; \
		docker tag $$image:$(VERSION) $$image:latest ; \
	done

# Rebuild from scratch
build: $(all_images) $(linked_scripts)

deliver: $(all_images)
	$(eval result := $(foreach image,$(all_images),\
		$(shell docker push $(image):$(VERSION)) \
		$(shell docker push $(image):latest))\
		)

# Image building targets
$(base):
	@sed -E "s/(RH_VERSION=)(.+)/\1"$(VERSION)"/g" $(call image_tmpl,$@) > $(call image_def,$@)
	$(call image_build,$@)

$(omni) $(runtime): $(base)
	$(call image_patch,$@)
	$(call image_build,$@)

$(redhawk_images): $(runtime)
	$(call image_patch,$@)
	$(call image_build,$@)

$(redhawk_webserver): $(runtime)
	$(call image_patch,$@)
	$(eval BUILD_ARGS = --build-arg REST_PYTHON=$(REST_PYTHON) --build-arg REST_PYTHON_BRANCH=$(REST_PYTHON_BRANCH))
	@echo Build Arguments: ${BUILD_ARGS}
	$(call image_build,$@,$(BUILD_ARGS))

# Launcher/helper script targets
helper_scripts: $(linked_scripts)
$(linked_scripts):
	@ln -s scripts/$@.sh ./$@
	@chmod a+x ./$@

# Cleaning
remove_container = $(shell docker rm -f $1)
remove_image = $(shell docker rmi $1)
list_containers = $(shell docker ps -qa --filter="ancestor=$1")
for_each_container = $(foreach container,$(call list_containers,$1),\
	$(call remove_container,$(container)) \
	$(info --> Removed $(container)) \
	)
for_each_image = $(foreach image,$1,\
	$(info Checking $(image):$(VERSION)...) \
	$(if $(call image_check,$(image):$(VERSION)),\
		$(call for_each_container,$(image)) \
		$(call remove_image,$(image):$(VERSION)) \
		$(call remove_image,$(image):latest) \
		$(info Removed with $(image):$(VERSION) and latest), \
		$(info Nothing to do for $(image):$(VERSION)) \
		)\
	)

clean:
	@$(eval result := $(call for_each_image,$(reversed)))
	@rm -f $(linked_scripts)
