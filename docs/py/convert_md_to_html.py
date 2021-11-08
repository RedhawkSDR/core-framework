#!/usr/bin/env python3

import os
import shlex
import shutil
import subprocess
import sys
import time

def reformat_toc(toc_in):
    # Get each element on its own line.
    toc_in = '\n'.join(t.strip() for t in toc_in)
    toc_in = toc_in.replace('<ul>', '\n<ul>\n').replace('</ul>', '\n</ul>\n')
    toc_in = toc_in.replace('<li>', '\n<li>\n').replace('</li>', '\n</li>\n')
    toc_in = toc_in.split('\n')
    toc_in = [l.strip() + '\n' for l in toc_in if l]

    # Indent the link names.
    level = -1
    toc = []
    for line in toc_in:
        if '<ul>' in line:
            level += 1
            continue
        elif '</ul>' in line:
            level -= 1
            continue
        if '<li>' in line or '</li>' in line:
            continue
        elif 'href' not in line:
            toc.append(line)
        else:
            gt = line.find('>')
            #indent = level * 4 * '&nbsp;'
            #item = ' ' * level * 4 + line[:gt + 1] + indent + line[gt + 1:]
            item = ' ' * level * 4  # indent the source code
            depth = 'depth{}'.format(level)
            item += line[:gt] + ' class="{}"'.format(depth)  #  add class="{level}" inside the <a>
            item += line[gt:]
            toc.append(item)
    return toc

def extract_title(title_line):
    pos_gt = title_line.find('>')
    pos_lt = title_line.rfind('<')
    title = title_line[pos_gt + 1:pos_lt]
    return title

def parse_html():
    html = []
    toc = []
    put_in_toc = False
    title_line = ''
    fp_raw = open('raw.html', 'w')  # delete me
    for line in open(fpath_html).readlines():
        fp_raw.write(line.strip() + '\n')  # delete me
        if line.startswith('<nav id="TOC">') or line.startswith('<div id="TOC">'):
            put_in_toc = True
            toc.append(line.replace('nav', 'div'))
        elif put_in_toc:
            if line.startswith('</nav>') or line.startswith('</div>'):
                put_in_toc = False
                toc.append(line.replace('nav', 'div').rstrip() + '  <!-- end div id=TOC -->\n')
            else:
                toc.append(line)
        elif not put_in_toc:
            html.append(line)

        if line.startswith('<h1 '):
            title_line = line
    toc = reformat_toc(toc)
    title = extract_title(title_line)
    return html, toc, title

def rewrite_html(html, toc, title):
    fp = open(fpath_html, 'w')
    for line in html:
        if line.strip().startswith('<title>'):
            fp.write('  <title>{}</title>\n'.format(title))
            dotdots = '../' * (fpath_md.count('/') - os.getcwd().count('/') - 1)
            fp.write('<link rel="shortcut icon" href="{}md/img/favicon.png" type="image/x-icon">\n'.format(dotdots))
            # add js files before css files
            for js_file in js_files:
                js_line = '  <script src="{}{}"></script>'.format(dotdots, js_file)
                fp.write('{}\n'.format(js_line))
            for css_file in css_files:
                css_line = '  <link href="{}{}" rel="stylesheet">'.format(dotdots, css_file)
                fp.write('{}\n'.format(css_line))
            fp.write('  <script>\n')
            #fp.write(open('js/default.js').read())
            fp.write('  </script>\n')
        elif line.startswith('<body>'):
            fp.write(line)
            fp.write('<div id="navigation">\n');
            fp.write(open('tree-of-files.html').read())
            for t in toc:
                fp.write(t)
            fp.write('</div>  <!-- end div id=navigation -->\n');
            fp.write('<div id="content">\n')
        elif line.startswith('</body>'):
            fp.write('</div>  <!-- end div id=content -->\n')
            fp.write(line)
        else:
            fp.write(line)
    fp.close()

def convert_md_to_html():
    #        --from markdown+superscript
    cmd = '''pandoc {}
             --from markdown_github+definition_lists-hard_line_breaks
             --to html
             --standalone
             --toc
             --toc-depth=4
             --highlight-style=pygments
             --metadata pagetitle="tmp"
             --output={}
          '''.format(fpath_md, fpath_html).strip()
    try:
        os.makedirs(os.path.dirname(fpath_html))
    except FileExistsError:
        pass
    cmd = shlex.split(cmd)
    subprocess.call(cmd)

def handle_file_markdown():
    convert_md_to_html()
    html, toc, title = parse_html()
    rewrite_html(html, toc, title)

if __name__ == '__main__':
    css_files = [
        'js/jquery-ui-1.13.0.custom/jquery-ui.css',
        'css/default.css',
    ]
    js_files = [
        'js/jquery-3.6.0.js',  # keep jquery before the others
        'js/jquery-ui-1.13.0.custom/jquery-ui.js',
        'js/default.js',
    ]

    if len(sys.argv) != 3:
        print('usage:  {} <md-file-in> <html-file-out>'.format(sys.argv[0]))
        exit(1)
    fpath_md = os.path.abspath(sys.argv[1])
    fpath_html = os.path.abspath(sys.argv[2])
    handle_file_markdown()
