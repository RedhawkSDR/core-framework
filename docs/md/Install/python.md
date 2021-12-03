# Python

REDHAWK uses Python 3, which should be installed automatically when you install REDHAWK.

Use `python3`, not `python` when you start an interpreter, for example:
```
user@host $ python3
Python 3.6.8 (default, Nov 16 2020, 16:55:22)
[GCC 4.8.5 20150623 (Red Hat 4.8.5-44)] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>>
```

If you write scripts that use REDHAWK Python code, use `python3`, not `python`, in the shebang line.
For example:
```python
#!/usr/bin/env python3
```

### Select Python 3 for IDE's Python Sandbox

The IDE's 'REDHAWK Python Sandbox' console uses a Python 2 interpreter by default.
Before using this console, set it to use Python 3:

1. At the top of the tool bar click "Window" then "Preferences"
1. In the Preferences Window expand the "PyDev" tab
1. Expand the "Interpreters" tab then select "Python Interpreters"
1. Within the Python Interpreters Window:
    *  Select New
    *  Interpreter Name: `Python 3`
    *  Interpreter Executable: `/usr/bin/python3`
    *  Click OK
    *  Add all folders to SYSTEM pythonpath
    *  Click OK
    *  Select Up in order to move `Python 3` above `Python`
        * The Python Interpreter that is highest on the list will be used by the    PyDev Console
    *  Click Apply and Close
1. Alternatively you can use the "Advanced Auto-Config" which will list all Python Interpreters installed on your local machine.
