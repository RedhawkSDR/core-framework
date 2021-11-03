markdown := $(shell find md -name '*.md')  # get list of markdown files
images := $(shell find md -name images)  # get list of image directories

all:    tree-of-files.html \
        $(patsubst md/%.md, html/%.html, $(markdown)) \
        $(patsubst md/%images, html/%images, $(images))
	$(RM) tree-of-files.html

# Convert .md files to .html
html/%.html: md/%.md py/convert_md_to_html.py py/generate_tree_of_files.html.py
	py/convert_md_to_html.py $< $@

# Link to the md/images directory.
html/images: md/images
	mkdir -p $(@D)
	ln -frs $< $@

# Link to the other images directories.
html/%images: md/%images
	mkdir -p $(@D)
	ln -frs $< $@

# Always generate tree-of-files.html.
tree-of-files.html: FORCE
	py/generate_tree_of_files.html.py

FORCE:

clean:
	$(RM) -r html
	$(RM) tree-of-files.html raw.html

