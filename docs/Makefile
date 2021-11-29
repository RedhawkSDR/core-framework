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

md_files := $(shell find md -name '*.md')  # get list of markdown files
img_dirs := $(shell find md -name img)  # get list of image directories

# Bare invocation of `make`, handles html.
most:    tree-of-files.html \
        $(patsubst md/%img, html/%img, $(img_dirs)) \
        $(patsubst md/%.md, html/%.html, $(md_files))
	$(RM) raw.html tree-of-files.html

# `make all` or `make pdf` will handle html and pdf.
all:  most pdf

# Always generate tree-of-files.html.  It compares filesystem to ordered list of files.
tree-of-files.html: FORCE
	py/generate_tree_of_files.html.py

# Create a symlink to the main image directory.
html/img: md/img
	mkdir -p $(@D)
	cp -d $< $@

# Create other symlinks to the main image directory.
html/%img: md/%img
	mkdir -p $(@D)
	cp -d $< $@

# Convert .md files to .html
html/%.html: md/%.md py/convert_md_to_html.py py/generate_tree_of_files.html.py
	py/convert_md_to_html.py $< $@

FORCE:

.PHONY: checkossiehome

checkossiehome:
	if [ ! -d "${OSSIEHOME}" ]; then \
		echo "OSSIEHOME must be defined before 'make install'"; \
		exit 1; \
	fi

install: checkossiehome
	mkdir -p ${OSSIEHOME}/docs
	cp -dr css font img js Makefile md py README.md ${OSSIEHOME}/docs/
	cd ${OSSIEHOME}/docs && make

pdf: FORCE
	py/generate_pdf.py

clean:
	$(RM) -r html pdf
	$(RM) RedhawkManual.pdf raw.html tree-of-files.html 

