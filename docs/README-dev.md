# readme for developers

## requirements

`yum install pandoc`

Centos 7 needed (some of?) these to make pandoc produce pdfs:
```
texlive-collection-latex
texlive-ec
texlive-bidi
texlive-changepage
texlive-cmap
texlive-euenc
texlive-fancybox
texlive-fncychap
texlive-framed
texlive-ifmtarg
texlive-iftex
texlive-makecmds
texlive-mdwtools
texlive-multirow
texlive-parskip
texlive-placeins
texlive-polyglossia
texlive-threeparttable
texlive-titlesec
texlive-ucs
texlive-upquote
texlive-wrapfig
texlive-xifthen
```

## usage

### produce html
1. Edit the files in `md/`.  See `man pandoc_markdown` for syntax rules.
1. Edit `ordered_markdown-file_list.py`.  The order of the files listed there determines the order in which they will occur in `index.html`.
1. Run `make`.

#### also produce pdf
1. Run `make pdf`.

### use the documentation
1. Load `html/index.html` in a browser.  It contains a map to all the other files.
1. In a terminal, use the `md/` directory for search.

## installed files
These are installed to `$OSSIEHOME/docs/` by `make install`:
```
css/
font/
html/
img/
js/
md/  # for search with grep
README.MD  # readme for users
RedhawkManual-<ver>.pdf  #  optional
```

## fonts

This [site](https://google-webfonts-helper.herokuapp.com/fonts) was used to get font files and the css code to use them.


## Tricks

### generate pdf

The code that handles md-->html checks, and warns, for new or missing files.  
The code that handles md-->pdf does not.  We need to manually check that all the md files are in `buildp-pdf.sh`.

### change/refresh the file tree

The contents of the leftmost portion of the html contains a file tree.  
It is handled in a non-obvious way.  
You may have difficulty updating it in the browser after a change.  
Explanation:

The file tree is inserted into each `.html` file.  That way, if a user browses to any `.html` file, they will also get the file tree contents.  This is useful for bookmarks, or in the case of crashes.

The file tree is also kept in session storage, and passed from page to page during navigation.  This allows the open-or-closed state of the directories to be maintained within a browser session.

For a developer who wants to update and refresh the file-tree, you might need to remove both the content in the `.html` files and in the session storage.

The easy way:
```
make clean
make
```
then `ctrl-shift-`refresh the browser.

A quicker way:
```
rm html/<the-file-in-the-browser>
make
```
then `ctrl-shift-`refresh the browser.

