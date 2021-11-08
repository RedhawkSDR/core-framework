md_files := $(shell find md -name '*.md')  # get list of markdown files
img_dirs := $(shell find md -name img)  # get list of image directories

all:    tree-of-files.html \
        $(patsubst md/%img, html/%img, $(img_dirs)) \
        $(patsubst md/%.md, html/%.html, $(md_files))
	$(RM) tree-of-files.html

# Always generate tree-of-files.html.
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

pdf:
	./build-pdf.sh

clean:
	$(RM) -r html pdf
	$(RM) RedhawkManual.pdf raw.html tree-of-files.html 

