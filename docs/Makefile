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
	#cp -dr css font html img js md README.md ${OSSIEHOME}/docs/
	(test -e RedhawkManual.pdf && cp RedhawkManual.pdf ${OSSIEHOME}/docs/) || true

pdf: FORCE
	py/generate_pdf.py

clean:
	$(RM) -r html pdf
	$(RM) RedhawkManual.pdf raw.html tree-of-files.html 

