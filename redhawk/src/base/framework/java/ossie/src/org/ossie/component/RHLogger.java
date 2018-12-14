/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

package org.ossie.component;

import org.apache.log4j.Logger;
import org.apache.log4j.Level;
import org.apache.log4j.spi.LoggerRepository;
import java.util.Enumeration;
import java.util.ArrayList;
import java.util.List;

import org.ossie.logging.logging;

/**
  RHLogger

  Class extends the log4j logger with some utility functions

 */

public class RHLogger {

    private Logger l4logger;
    private String logname;
    static public String USER_LOGS = "user";
    static public RHLogger _rootLogger;

    public RHLogger ( String logName ) {
        this.l4logger = Logger.getLogger(logName);
        this.logname = logName;
        this._rootLogger = null;
    }

    public Logger getL4Logger() {
        return this.l4logger;
    }

    static public RHLogger getRootLogger() {
        if ( _rootLogger == null ) {
            _rootLogger = new RHLogger("");
        }
        return _rootLogger;
    }

    static public RHLogger getLogger( String name ) {
        RHLogger ret;
        if ( !name.isEmpty() ) {
            ret = new RHLogger( name );
        } else {
            ret = RHLogger.getRootLogger();
        }
        return ret;
    }

    public RHLogger getResourceLogger( String name ) {
        return this.getLogger( name );
    }

    public RHLogger getChildLogger( String name ) {
        String ns = "user";
        String _ns = ns;
        if (logname.contains(".")) {
            _ns = "";
        }
        String _full_name;
        if (!_ns.isEmpty() && !name.contains("."+USER_LOGS+"."))
            _full_name = logname+"."+_ns+"."+name;
        else
            _full_name = logname+"."+name;
        return getResourceLogger(_full_name);
    }

    public RHLogger getChildLogger( String name, String ns ) {
        String _full_name;
        String _ns = ns;
        if (_ns.equals("user")) {
            if (logname.contains(".")) {
                _ns = "";
            }
        }
        if (!_ns.isEmpty() && ((!_ns.equals(USER_LOGS)) || ((_ns.equals(USER_LOGS)) && (!name.contains("."+USER_LOGS+".")))))
            _full_name = logname+"."+_ns+"."+name;
        else
            _full_name = logname+"."+name;
        return getResourceLogger(_full_name);
    }

    public boolean isLoggerInHierarchy(String search_name) {
        LoggerRepository repo = Logger.getRootLogger().getLoggerRepository();
        Enumeration list = repo.getCurrentLoggers();
        for (Enumeration loggerEnumeration = repo.getCurrentLoggers() ; loggerEnumeration.hasMoreElements() ; ) {
            Logger logger = (Logger)loggerEnumeration.nextElement();
            String _name = logger.getName();
            if (_name.startsWith(logname)) {
                if (_name.length() > logname.length()) {
                    if (_name.charAt(logname.length()) != '.') {
                        continue;
                    }
                }
                if (!_name.startsWith(search_name)) {
                    continue;
                }
                if (_name.length() > search_name.length()) {
                    if ((!search_name.isEmpty()) && (_name.charAt(search_name.length()) != '.')) {
                        continue;
                    }
                }
                return true;
            }
        }
        return false;
    }

    public String[] getNamedLoggers() {
        List<String> ret = new ArrayList<String>();
        LoggerRepository repo = Logger.getRootLogger().getLoggerRepository();
        Enumeration list = repo.getCurrentLoggers();
        for (Enumeration loggerEnumeration = repo.getCurrentLoggers() ; loggerEnumeration.hasMoreElements() ; ) {
            Logger logger = (Logger)loggerEnumeration.nextElement();
            String _name = logger.getName();
            if (_name.startsWith(logname)) {
                if (_name.length() > logname.length()) {
                    if (_name.charAt(logname.length()) != '.') {
                        continue;
                    }
                }
                ret.add(_name);
            }
        }
        return ret.toArray(new String[ret.size()]);
    }

    public void setLevel(Level level) {
        this.l4logger.setLevel(level);
    }

    public Level getEffectiveLevel() {
        return this.l4logger.getEffectiveLevel();
    }

    public Level getLevel() {
        Level retval = this.l4logger.getLevel();
        if (retval == null) {
            retval = this.getEffectiveLevel();
        }
        return retval;
    }

    public void trace(Object message) {
        this.l4logger.trace(message);
    }
    public void debug(Object message) {
        this.l4logger.debug(message);
    }
    public void info(Object message) {
        this.l4logger.info(message);
    }
    public void warn(Object message) {
        this.l4logger.warn(message);
    }
    public void error(Object message) {
        this.l4logger.error(message);
    }
    public void fatal(Object message) {
        this.l4logger.fatal(message);
    }

    public void trace(Object message, Throwable t) {
        this.l4logger.trace(message, t);
    }
    public void debug(Object message, Throwable t) {
        this.l4logger.debug(message, t);
    }
    public void info(Object message, Throwable t) {
        this.l4logger.info(message, t);
    }
    public void warn(Object message, Throwable t) {
        this.l4logger.warn(message, t);
    }
    public void error(Object message, Throwable t) {
        this.l4logger.error(message, t);
    }
    public void fatal(Object message, Throwable t) {
        this.l4logger.fatal(message, t);
    }
}
