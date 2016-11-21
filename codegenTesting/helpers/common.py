#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK codegenTesting.
#
# REDHAWK codegenTesting is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import os

cpp_usage = ['c++', 'C++', 'cpp', 'Cpp', 'CPP', 'cxx', 'CXX']
java_usage = ['java', 'Java', 'JAVA']
python_usage = ['python', 'Python', 'PYTHON']

def recursiveRmdir(name):
    try:
        paths = os.listdir(name)
        os.chdir(name)

        for path in paths:
            try:
                open(path)
                close(path)
                os.remove(path)
            except IOError:
                recursiveRmdir(path)
            except:
                os.remove(path)

        os.chdir('../')
        os.rmdir(name)
    except:
        pass

def recursiveClrdir(name):
    try:
        paths = os.listdir(name)
        os.chdir(name)

        for path in paths:
            try:
                open(path)
                close(path)
                os.remove(path)
            except IOError:
                recursiveRmdir(path)
            except:
                os.remove(path)
    except:
        pass

if __name__ == '__main__':
    recursiveRmdir('workspace')
