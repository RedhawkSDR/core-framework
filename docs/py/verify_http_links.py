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
Check internal `file://` links that are manually entered.
Exclude the generated ones in the filetree.
    Currently, these are skipped because they are not in double-quotes.
Exclude external refs.
This would be easier w requests and BeautifulSoup, but avoiding dependencies.

NOTE:  this operates on the generated .html files, not on the .md files.
"""
import os
import re
import urllib.request

from ordered_markdown_file_list import fpaths_ordered

def get_hrefs(fpath):
    hrefs = []
    text = open(fpath, encoding='utf-8').read()
    pre = r'(href=")'
    url = r'(.*?)?'
    post = r'(")'
    regexp = pre + url + post
    for m in re.finditer(regexp, text):
        if m.group(2).startswith('http') or m.group(2).startswith('file:'):
            continue
        hrefs.append(m.group(2))
    return hrefs

def check_fragment(fpath, fragment):
    #print('--> check_fragment({}, {})'.format(fpath, fragment))
    text = open(fpath, encoding='utf-8').read()
    pre = r'(<h[1-8] id=")'
    url = r'({})'.format(fragment)
    post = r'(")'
    regexp = pre + url + post
    for m in re.finditer(regexp, text):
        return True
    return False

def check_href(href, fpath_src):
    #print('--> check_href({})'.format(fpath_src))
    os.chdir(dpath)
    split = href.split('#')
    path = split[0]
    if path:
        fpath_target = os.path.abspath(path)
    else:
        fpath_target = fpath_src
    fragment = None
    if len(split) == 2:
        fragment = split[1]
    elif len(split) > 2:
        print('error:  href has more than 2 parts (path and fragment)')
        exit(1)
    if not os.path.exists(fpath_target):
        print('error:  no such relative file:  {}'.format(path))
        print('    referenced from: {}'.format(fpath_src))
    # The target file exists.  Check that the fragment refers to a heading.
    if fragment and not check_fragment(fpath_target, fragment):
        print('error:  fragment not found:  {}'.format(fragment))
        print('    in file:  {}'.format(fpath_target))
        print('    referenced from: {}'.format(fpath_src))

if __name__ == '__main__':
    """pwd is the location of Makefile."""
    dpath_root = os.path.join(os.path.abspath(os.getcwd()))
    for rfpath in fpaths_ordered:
        fpath = os.path.abspath(os.path.join(dpath_root, 'html', rfpath.replace('.md', '.html')))
        dpath = os.path.dirname(fpath)
        hrefs = get_hrefs(fpath)
        for href in hrefs:
            check_href(href, fpath)

