
from ossie.cf import CF

def codeToString(_code):
    if _code == CF.LogLevels.OFF:
        return 'OFF'
    elif _code == CF.LogLevels.FATAL:
        return 'FATAL'
    elif _code == CF.LogLevels.ERROR:
        return 'ERROR'
    elif _code == CF.LogLevels.WARN:
        return 'WARN'
    elif _code == CF.LogLevels.INFO:
        return 'INFO'
    elif _code == CF.LogLevels.DEBUG:
        return 'DEBUG'
    elif _code == CF.LogLevels.TRACE:
        return 'TRACE'
    elif _code == CF.LogLevels.ALL:
        return 'ALL'
    else:
        raise Exception("Invalid code")

def stringToCode(_codeString):
    _newString = _codeString.upper()
    if _newString == 'OFF':
        return CF.LogLevels.OFF
    elif _newString == 'FATAL':
        return CF.LogLevels.FATAL
    elif _newString == 'ERROR':
        return CF.LogLevels.ERROR
    elif _newString == 'WARN':
        return CF.LogLevels.WARN
    elif _newString == 'INFO':
        return CF.LogLevels.INFO
    elif _newString == 'DEBUG':
        return CF.LogLevels.DEBUG
    elif _newString == 'TRACE':
        return CF.LogLevels.TRACE
    elif _newString == 'ALL':
        return CF.LogLevels.ALL
    else:
        raise Exception("Invalid code string. Options: 'OFF', 'FATAL', 'ERROR', 'WARN', 'INFO', 'DEBUG', 'TRACE', 'ALL'")

