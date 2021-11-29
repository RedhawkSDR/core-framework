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

Verify that the files in build-pdf.sh match fpaths_ordered.
Else, exit so the user can fix that.

"""

import os

from ordered_markdown_file_list import fpaths_ordered


def read_pdf():
    dir_ = ''
    pdf_files
    fpath = os.path.join(dpath_root, 'build-pdf.sh')
    for line in open(fpath).readlines():
        l = line.strip()
        if 'end of individual files' in l:
            break
        if l.startswith('cd $PDF'):
            dir_ = l[len('cd $PDF/'):]
            #print('dir:  {}'.format(dir_))
        if l.startswith('files'):
            files = l[l.find('"') + 1:]  # remove beginning to "
            #print('after rm beginning:  {}'.format(files))
            files = files[:files.find('"')]  # remove " to end
            files = files.split()
            for f in files:
                pdf_files.append(os.path.join(dir_, f))

def verify_files():
    """Exit with error describing the first mismatch."""
    fi = 0
    pi = 0
    while fi < len(fpaths_ordered) and pi < len(pdf_files):
        f = fpaths_ordered[fi]
        if '-orig' in f or '-old' in f:
            fi += 1
            continue
        p = pdf_files[pi]
        if f != p:
            print('mismatch at fpaths_ordered[{}], pdf_files[{}]:'.format(fi, pi))
            print('    fpaths ordered:  {}'.format(f))
            print('    pdf files     :  {}'.format(p))
            exit(1)
        else:
            print('{} {}    {} {}'.format(fi, f, pi, p))
        fi += 1
        pi += 1
    if fi == len(fpaths_ordered) and pi == len(pdf_files):
        print('all files checked')
    else:
        print('some files not checked')

if __name__ == '__main__':
    """pwd is the location of Makefile."""
    dpath_root = os.path.join(os.path.abspath(os.getcwd()))

    pdf_files = []
    read_pdf()
    verify_files()

