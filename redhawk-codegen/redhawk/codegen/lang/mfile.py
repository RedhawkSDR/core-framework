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

class mFunctionParameters:
    """
    A simple struct for storing off inputs, output, and function mame
    of an m function.

    """
    def __init__(self, outputs, inputs, functionName, defaults={}):
        self.outputs      = outputs
        self.inputs       = inputs
        self.functionName = functionName
        self.defaults     = defaults
        if "__sampleRate" in self.defaults:
            del self.defaults["__sampleRate"]

def stringToList(stringInput):
    """
    Convert a string of the form:

        "[1,2,3]"

    To:

        ["1", "2", "3"]

    """

    stringInput = stringInput[1:-1]
    stringInput = stringInput.replace(" ", "")  # remove whitespace
    return stringInput.split(",")

def parseDefaults(inputs):
    """
    Parse out default values that have been specified in the input parameters
    of the function declaration.

    Example:

        inputs = ["var1", "var2=5"]

    will return:

        inputs = ["var1", "var2"]
        defaults = {"var2" = "5"}

    """

    defaults = {}
    for index in range(len(inputs)):
        if inputs[index].find("=") != -1:
            splits = inputs[index].split("=")
            inputs[index] = splits[0]
            defaults[inputs[index]] = splits[1]
            if defaults[inputs[index]].find("[") != -1:
                defaults[inputs[index]] = stringToList(defaults[inputs[index]])

    return inputs, defaults

def getInputArguments(inputString):
    """
    Get arguments within a string between "(" and ")".  This is a special
    sub-case of getArguments, as it can handle nested lists (i.e., sequence
    properties).  Also, it can be assumed that the input arguments are comma-
    separated.

    See also getArguments.

    """
    def _splitFirstArg(args):
        """
        Given an input "a,b,c", return ["a", "b,c"].  Honors brackets (e.g.,
        "a=[1,2],b,c" will return ["a=[1,2]","b,c"].
        """

        openBracket = args.find("[")
        closeBracket = args.find("]")
        commaLocation = args.find(",")
        if commaLocation == -1:
            # last argument
            retVal = [args, None]
        elif commaLocation < openBracket or openBracket == -1:
            # this is not a sequence
            retVal = [args[0:commaLocation], args[commaLocation+1:]]
        else:
            # this is a sequence
            retVal = [args[0:closeBracket+1], args[closeBracket+2:]]

        return retVal

    inputString = inputString.replace(" ", "")

    args = inputString[inputString.find("(")+1:
                       inputString.find(")")]

    if args == "":
        # Special case: no input arguments.  This will prevent us from
        # returning an input argument with a blank name.
        return []

    args = _splitFirstArg(args)
    prevLength = -1
    while args[-1] != None:
        retVal = _splitFirstArg(args[-1])
        args = args[:-1] # strip off last val
        args.extend(retVal)

    args = args[:-1] # strip off last val

    return args

def getArguments(inputString, openDelimiter, closeDelimiter):
    """
    Get arguments within a string.  For example, if openDelimiter="(", 
    closeDelimiter=")", and inputString = "function [a] = foo(b, c, d)",
    this function will return ["b","c","d"].

    """

    args = inputString[inputString.find(openDelimiter)+1: 
                       inputString.find(closeDelimiter)]
    if args.find(",") != -1:
        if openDelimiter != '[' and closeDelimiter != ']':
            raise SystemExit('ERROR: When a function returns more than 1 field, their declaration must be placed between brackets')
        args = args.split(',')
    else:
        args = args.split()

    args = [x.replace(" ","") for x in args] # get rid of extra whitespace

    # TODO: remove comments from strings
    return args

def parse(filename):
    """
    Parse file specified by filename to get inputs, outputs, and function
    name of the first function defined in the m file.

    """
    # Get the contents of the file as a single string
    file  = open(filename)
    fileLines = file.readlines()
    file.close()

    fileStringNoComments = ""
    for line in fileLines:
        if line.find("#") != 0 and line.find("%") != 0:
            # get a copy of the m file with no comments
            fileStringNoComments += line

    # the declaration consists of all content up to the first ")"
    declaration  = fileStringNoComments[0:fileStringNoComments.find(")")+1]

    # get output arguments
    bracket1 = declaration.find("[")
    if declaration.find("=") == -1 or declaration.find("=") > declaration.find("("):
        # no output arguments, either because no equals sign was found or
        # because no equals sign was found before the input arguments.
        outputs = []
    elif bracket1 != -1 and declaration.find("=") > bracket1:
        # if a bracket is found before the first equals sign, parse
        # the output argument that are between the first set of brackets
        outputs = getArguments(declaration, "[", "]")
    else:
        # list of output arguments without brackets
        outputs = getArguments(declaration, " ", "=")

    # get input arguments
    inputs = getInputArguments(declaration)
    inputs, defaults = parseDefaults(inputs)

    # get function name
    if declaration.find("=") == -1 or declaration.find("=") > declaration.find("("):
        # if there are no outputs
        functionName = getArguments(declaration, "function", "(")[0]
    else:
        # If there are output arguments (output variables present)
        functionName = getArguments(declaration, "=", "(")[0]

    # store off outputs, inputs and function name in a struct
    return mFunctionParameters(outputs      = outputs, 
                               inputs       = inputs, 
                               functionName = functionName,
                               defaults     = defaults)
