#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
#
# REDHAWK core is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

try:
    from os.path import relpath
except ImportError:
    # The relpath() function was added in Python 2.6. If it is not available,
    # define a substitute that is sufficient for our usage.
    def relpath(path, start):
        newpath = ['..'] * len(start.split('/')) + [path]
        return '/'.join(newpath)

from jinja2.filters import environmentfilter

from testparser import TestParser

def do_codealign(value):
    """
    Aligns a section of text based on basic code scoping rules. Lines within a
    scope are indented to align with the first character after the scope
    opener. Scopes may be nested.
    """
    scope_start = '({['
    scope_end = ']})'
    string_delim = '\'"'
    lines = []
    indent = []
    quoted = None
    for line in value.split('\n'):
        if indent and line and not quoted:
            # Only apply indent on non-empty, non-quoted lines
            line = indent[-1] + line.strip()

        lines.append(line)

        # Adjust indentation based on scopes opened or closed on line.
        # NB: Scopes are not checked to ensure they match
        for ii in xrange(len(line)):
            current = line[ii]
            if current in string_delim:
                # Naive tracking of string literal entry and exit--each quote
                # mark toggles the "quoted" state (exit must match). Escaping
                # is not handled, but it works in most cases for C++ and Python
                # (except that triple-quotes are not treated as a single token,
                # so embedded quotes can toggle quoted state).
                if quoted:
                    if current == quoted:
                        quoted = None
                else:
                    quoted = current
            elif quoted:
                continue
            elif current in scope_start:
                indent.append(' '*(ii+1))
            elif current in scope_end:
                try:
                    indent.pop()
                except IndexError:
                    # Unbalanced scope closing character; usually, this should
                    # only occur within a string literal, such as a property
                    # description. Otherwise, the output code is malformed.
                    pass

    return '\n'.join(lines)

def do_lines(value):
    """
    Splits text block 'value' into a list of lines.
    """
    return value.split('\n')

def do_relpath(path, base):
    """
    If 'path' is a string, returns the relative path of 'path' to 'base'.
    Otherwise, 'path' is assumed to be a list, and the relative path
    operation is applied to each element.
    """
    if isinstance(path, basestring):
        return relpath(path, base)
    else:
        return (relpath(p,base) for p in path)

def do_quote(value, quote='"'):
    """
    If 'value' is a string, returns 'value' surrounded by quote type 'quote'.
    Otherwise, 'value' is assumed to be a sequence, and a generator yielding
    each item quoted is returned.
    """
    if isinstance(value, basestring):
        return quote+value+quote
    else:
        return (quote+v+quote for v in value)

def do_prepend(values, prefix):
    """
    Prepends 'prefix' to each item in 'values'.
    """
    return [prefix+v for v in values]

def do_append(values, suffix):
    """
    Appends 'suffix' to each item in 'values'.
    """
    return [v+suffix for v in values]

def do_unique(values, keep_blank_lines=False):
    """
    Filters out repeated occurrences of items in the given sequence. If
    'keep_blank_lines' is True, lines containing only whitespace will always
    be returned.
    """
    seen = set()
    for value in values:
        if keep_blank_lines and not value.strip():
            # Line contains no non-whitespace characters
            yield value
        elif not value in seen:
            yield value
            seen.add(value)

@environmentfilter
def do_filter(environment, values, test):
    """
    Returns only the items from 'values' for which 'test' evaluates true.
    'test' may be a unary test, or any boolean combination of unary tests,
    e.g.:

      filter('callable or mapping')
    """
    test = TestParser(environment, test).parse()
    return (v for v in values if test(v))

@environmentfilter
def do_test(environment, values, test):
    """
    Returns the results of 'test' for each item in 'values'. 'test' may be a
    unary test, or any boolean combination of unary tests, e.g:

      test('string and not upper')
    """
    test = TestParser(environment, test).parse()
    return (test(v) for v in values)
