# readme for developers

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
These files will be installed to `$OSSIEHOME`:
```
docs/
    css/
    fonts/
    html/
    js/
    md/  # to allow users to search with grep
    README.MD  # readme for users
```
The rest of the files are for internal development.

## fonts

This [site](https://google-webfonts-helper.herokuapp.com/fonts) was used to get font files and the css code to use them.


## texlive installs

I needed (some of) these extra installs on my centos 7 dev box to make pandoc produce pdfs.

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

