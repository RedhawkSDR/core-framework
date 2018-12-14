/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef THRESHOLD_MONITOR_H_
#define THRESHOLD_MONITOR_H_

#include <string>

#include <boost/shared_ptr.hpp>

#include <ossie/callback.h>

#include "utils/Updateable.h"

class ThresholdMonitor : public Updateable
{
public:
    ThresholdMonitor(const std::string& resource_id, const std::string& threshold_class);

    const std::string& get_resource_id() const;
    const std::string& get_threshold_class() const;

    bool is_enabled() const;
    virtual void enable();
    virtual void disable();

    void update();

    bool is_threshold_exceeded() const;

    template <class Target, class Func>
    void add_listener(Target target, Func func)
    {
        notification_.add(target, func);
    }

protected:
    virtual void update_threshold() = 0;
    virtual bool check_threshold() const = 0;

    const std::string resource_id_;
    const std::string threshold_class_;
    bool enabled_;
    bool prev_threshold_exceeded_;

    ossie::notification<void (ThresholdMonitor*)> notification_;
};

class FunctionThresholdMonitor : public ThresholdMonitor
{
public:
    template <class Func>
    FunctionThresholdMonitor(const std::string& resource_id, const std::string& threshold_class, Func func) :
        ThresholdMonitor(resource_id, threshold_class),
        callback_(func),
        exceeded_(false)
    {
    }

    template <class Target, class Func>
    FunctionThresholdMonitor(const std::string& resource_id, const std::string& threshold_class,
                             Target target, Func func) :
        ThresholdMonitor(resource_id, threshold_class),
        callback_(target, func),
        exceeded_(false)
    {
    }

private:
    virtual void update_threshold();
    virtual bool check_threshold() const;

    redhawk::callback<bool (ThresholdMonitor*)> callback_;
    bool exceeded_;
};

class ThresholdMonitorSet : public ThresholdMonitor
{
public:
    ThresholdMonitorSet(const std::string& resource_id, const std::string& threshold_class);

    void add_monitor(const boost::shared_ptr<ThresholdMonitor>& monitor);

    virtual void enable();
    virtual void disable();

private:
    virtual void update_threshold();
    virtual bool check_threshold() const;

    typedef std::vector< boost::shared_ptr<ThresholdMonitor> > MonitorList;
    MonitorList monitors_;
};

#endif
