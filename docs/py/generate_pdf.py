#!/usr/bin/env python3
"""
"""

import os
import shlex
import subprocess
import sys

from ordered_markdown_file_list import fpaths_ordered

def combine_indiv_mds(fpaths_in):
    dpath_in = os.path.dirname(fpaths_in[0])
    title = dpath_in.replace('-', ' ').replace('/', '-')
    fpath_out = dpath_in.replace('/', '_')
    num_files_w_name = len([f for f in os.listdir(dpath_pdf) if f.startswith(fpath_out)])
    fpath_out += '_{}.MD'.format(num_files_w_name)
    fpath_out = os.path.join(dpath_pdf, fpath_out)
    print('{} <-- {}'.format(fpath_out, ' '.join([os.path.basename(f) for f in fpaths_in])))
    fpaths_modify = [os.path.join(dpath_pdf, f) for f in fpaths_in]
    # Decrease heading levels.
    cmd = ['sed', '-i', r's,\(^#\)\([# ]\),#\1\2,g']
    cmd.extend([f for f in fpaths_modify])
    subprocess.run(cmd)
    # Combine.
    cmd = 'pandoc -t markdown -o {} {}'.format(fpath_out, ' '.join(fpaths_modify))
    cmd = shlex.split(cmd)
    subprocess.run(cmd)
    # Insert heading level 1.
    cmd = ['sed', '-i', r'1s/^/# {}\n/'.format(title), fpath_out]
    subprocess.run(cmd)
    return fpath_out

def make_pdf(mds):
    fname = 'RedhawkManual'
    if version:
        fname += '-{}'.format(version)
    fname += '.pdf'
    cmd = 'pandoc -f markdown -t latex -V geometry:margin=0.8in --toc-depth=3 --toc -o {} {}'.format(fname, ' '.join(mds))
    cmd = shlex.split(cmd)
    subprocess.run(cmd)

def main():
    """Collect lists of paths, each with the same directory.
    Send each list to be combined into a single .md file.
    """
    combined_mds = []
    dpath_prev = None
    fpaths = []  # A list of consecutive file paths with the same directory.
    for fpath_in in fpaths_ordered:
        dpath, _ = os.path.split(fpath_in)
        if '-old' in dpath or '-orig' in dpath:
            continue
        if not dpath:
            if fpaths:
                combined_mds.append(combine_indiv_mds(fpaths))
                fpaths = []
            combined_mds.append(os.path.join(dpath_pdf, fpath_in))
        elif dpath == dpath_prev:
            fpaths.append(fpath_in)
        else:
            if fpaths and dpath_prev is not None:
                combined_mds.append(combine_indiv_mds(fpaths))
            dpath_prev = dpath
            fpaths = [fpath_in]
    make_pdf(combined_mds)

if __name__ == '__main__':
    """pwd is the location of Makefile."""
    dpath_pdf = 'pdf'
    cmd = shlex.split('rm -rf {}'.format(dpath_pdf))
    subprocess.run(cmd)
    cmd = shlex.split('cp -dr md pdf')
    subprocess.run(cmd)
    version = ''
    if len(sys.argv) > 1:
        version = sys.argv[1]
    main()
