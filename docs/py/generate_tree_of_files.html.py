#!/usr/bin/env python3

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

Verify that the files in the filesystem match fpaths_ordered.
Else, exit so the user can fix that.

Create html to display links to the files.  The links should:
- be in the same order as `fpaths_ordered`,
- be indented to show the directory structure,
- use the h1 content of the file as the text for the link.
"""

import os
import shlex
import subprocess

from ordered_markdown_file_list import fpaths_ordered

indent_code = '    '
top_level_nodes = []

def get_node(path):
    """Return Node with path==path if found; else None."""
    for tln in top_level_nodes:
        node = tln.get(path)
        if node:
            return node

class Node():
    """A file or dir at the top level of the tree."""
    def __init__(self, path):
        self.path = path
        self.name = path.split('/')[-1]
        self.depth = len(path.split('/'))
        if self.depth == 1:
            self.depth_class = 'depth0'
        elif self.depth == 2:
            self.depth_class = 'depth1'
        elif self.depth == 3:
            self.depth_class = 'depth2'
        elif self.depth == 4:
            self.depth_class = 'depth3'
        elif self.depth == 5:
            self.depth_class = 'depth4'

    def get(self, path):
        """Return Node with path==path if found; else None."""
        if self.path == path:
            return self
        if isinstance(self, Dir):
            for child in self.children:
                node = child.get(path)
                if node:
                    return node

class Dir(Node):
    """A directory in the tree."""
    def __init__(self, path):
        super().__init__(path)
        self.children = []

    def append(self, node):
        self.children.append(node)

    def html(self):
        html = [indent_code * self.depth + '<div class="tree-of-files-dir {}">'.format(self.depth_class)]
        html.append(indent_code * (self.depth + 1) + '<div class="ui-accordion-header-icon ui-icon ui-icon-triangle-1-e"></div>')
        html.append(indent_code * (self.depth + 1) + self.name.replace('-', ' '))
        html.append(indent_code * (self.depth + 1) + '<div style="clear: left;"></div>')
        html.append(indent_code * self.depth + '</div>  <!-- end tree-of-files-dir -->')
        html.append(indent_code * (self.depth) + '<div class="tree-of-files-children dontshow">')
        for child in self.children:
            html.extend(child.html())
        html.append(indent_code * (self.depth) + '</div>  <!-- end tree-of-files-children -->')
        return html

class File(Node):
    """A file in the tree."""
    def __init__(self, path):
        super().__init__(path)
        self.title = self._read_title()

    def _read_title(self):
        fpath = os.path.join(dpath_doc_root, self.path)
        for line in open(fpath).readlines():
            if line.startswith('# '):
                return line[2:].strip()
        print('error:  no title found for {}'.format(fpath))
        exit(1)

    def html(self):
        href = os.path.join('file://', dpath_doc_root.replace('/md', '/html'), self.path.replace('.md', '.html'))
        clickable = '<a class="save-tree" href={0} title="{1}">{1}</a>'.format(href, self.title)
        html = [indent_code * (self.depth) + '<div class="tree-of-files-child {}">'.format(self.depth_class)]
        html.append(indent_code * (self.depth + 1) + '<div class="ui-accordion-header-icon ui-icon ui-icon-blank"></div>')
        html.append(indent_code * (self.depth + 1) + clickable)
        html.append(indent_code * (self.depth + 1) + '<div style="clear: left;"></div>')
        html.append(indent_code * (self.depth) + '</div>  <!-- end tree-of-files-child -->')
        return html

def ingest_path(fpath):
    if not fpath.endswith('.md'):
        return
    path_segments = fpath.split('/')
    num_path_segments = len(path_segments)
    path_segment_no = 1
    parent_path = ''
    parent_node = None
    # Handle the directories, if any.
    while path_segment_no < num_path_segments:
        name = path_segments[path_segment_no - 1]
        path = os.path.join(parent_path, name)
        node = get_node(path)
        if not node:
            node = Dir(path)
            if parent_node:
                parent_node.append(node)
            else:
                top_level_nodes.append(node)
        # Update vars for next loop, or for the following section.
        parent_node = node
        parent_path = os.path.join(parent_path, name)
        path_segment_no += 1
    # Handle the .md file.
    name = path_segments[path_segment_no - 1]
    path = os.path.join(parent_path, name)
    node = File(path)
    if parent_node:
        parent_node.append(node)
    else:
        top_level_nodes.append(node)

def verify_files():
    """Exit with error if:
    - an .md file in the list is not on the filesystem
    - an .md file in the filesystem is not in the list
    """
    not_in_fs = []
    not_in_list = []
    cmd = shlex.split('find md -name "*.md"')
    completed_process = subprocess.run(cmd, stdout=subprocess.PIPE)
    # The `[3:]` removes the `md/` from the paths found by `find`.
    fpaths = [b.decode('ascii')[3:] for b in completed_process.stdout.strip().split(b'\n')]
    for f in fpaths:
        if f not in fpaths_ordered:
            not_in_list.append(f)
    for f in fpaths_ordered:
        if f not in fpaths:
            not_in_fs.append(f)
    if not_in_list:
        print('`.md` files on filesystem that are not in internal list:')
        for f in not_in_list:
            print('    ' + f)
    if not_in_fs:
        print('`.md` files in internal list that are not in filesystem:')
        for f in not_in_fs:
            print('    ' + f)
    if not_in_list or not_in_fs:
        exit(1)

def write_html():
    html = ['<div id="tree-of-files">']
    for node in top_level_nodes:
        html.extend(node.html())
    html.append('</div>  <!-- end div id=tree-of-files -->')

    with open('tree-of-files.html', 'w') as fp:
        for html_line in html:
            fp.write(html_line + '\n')

if __name__ == '__main__':
    """pwd is the location of Makefile."""
    dpath_doc_root = os.path.join(os.path.abspath(os.getcwd()), 'md')

    verify_files()

    for fpath in fpaths_ordered:
        ingest_path(os.path.join(fpath))

    write_html()

