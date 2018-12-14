
from pydoc import pager as _pager

__all__ = [ 'PagerWithHeader', 
            'Pager'
           ]

def PagerWithHeader( src_generator,  num_lines=25, header=None, repeat_header=None):
    """
    Apply simple pager controller to the ouptput of a specified generator.
    
    src_generator - a generator object that will yield lines to display
    num_lines - number of lines to display before requesting manual input
    header - a header row to display, this can be multi-line object
    repeat_header - number of lines to repeat between header blocks
    """
    for index,line in enumerate(src_generator):
       if header and repeat_header:
           if (index % repeat_header) == 0 :
               if type(header) == list:
                   for x in header:
                       print x
               else:
                   print header
       if index % num_lines == 0 and index:
           input=raw_input("Hit any key to continue press q to quit ")
           if input.lower() == 'q':
               break
       else:
           print line


def Pager( doc ):
    _pager(doc)
