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

class inherit_doc(object):
    """
    Decorator to allow an object to inherit a docstring from another. If the
    decorated object has a docstring, it is appended to the parent docstring,
    with its indentation normalized.
    """
    def __init__(self, obj):
        self.doc = obj.__doc__

    def _get_indent(self, doc):
        """
        Returns the lowest indent level of the docstring.
        """
        indent = None
        for line in doc.split('\n'):
            line_indent = self._get_line_indent(line)
            if indent is None:
                indent = line_indent
            else:
                indent = min(indent, line_indent)
            if indent == 0:
                return indent
        return 0

    def _get_line_indent(self, line):
        for pos in xrange(len(line)):
            if not line[pos].isspace():
                return pos
        return 0

    def _do_normalize(self, doc, indent):
        """
        Generator function to perform line-by-line normalization of
        indentation.
        """
        source_indent = self._get_indent(doc)
        for line in doc.split('\n'):
            if line:
                line = ' '*indent + line[source_indent:]
            yield line

    def _normalize_indent(self, doc, indent):
        """
        Normalizes the indentation of a docstring to the givent base
        indentation level.
        """
        return '\n'.join(self._do_normalize(doc, indent))

    def __call__(self, obj):
        if self.doc is not None:
            indent = self._get_indent(self.doc)
            original_doc = obj.__doc__
            obj.__doc__ = self.doc
            if original_doc is not None:
                obj.__doc__ += self._normalize_indent(original_doc, indent)
        return obj
