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
