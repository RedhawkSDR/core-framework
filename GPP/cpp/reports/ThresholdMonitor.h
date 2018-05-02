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
#include <functional>
#include <sstream>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

#include <ossie/callback.h>

#include "utils/Updateable.h"
#include "utils/ConversionWrapper.h"

class ThresholdMonitor : public Updateable
{
public:
    ThresholdMonitor(const std::string& message_class, const std::string& resource_id):
        resource_id_(resource_id),
        message_class_(message_class),
        enabled_(true),
        prev_threshold_exceeded_(false)
    {}

    virtual void update()
    {
        if (enabled_) {
            update_threshold();
        }
        if (prev_threshold_exceeded_ != is_threshold_exceeded()) {
            notification_(this);
        }
        prev_threshold_exceeded_ = is_threshold_exceeded();
    }

    void enable()
    {
        enabled_ = true;
        update();
    }

    void disable()
    {
        enabled_ = false;
        update();
    }

    bool is_enabled() const
    {
        return enabled_;
    }

    virtual bool is_threshold_exceeded() const
    {
        if (!enabled_) return false;
        return check_threshold();
    }

    std::string get_resource_id() const{ return resource_id_; }
    std::string get_message_class() const{ return message_class_; }

    template <class Target, class Func>
    void add_listener(Target target, Func func)
    {
        notification_.add(target, func);
    }

protected:
    virtual void update_threshold() = 0;
    virtual bool check_threshold() const = 0;

    const std::string resource_id_;
    const std::string message_class_;
    bool enabled_;
    bool prev_threshold_exceeded_;

    ossie::notification<void (ThresholdMonitor*)> notification_;
};

class FunctionThresholdMonitor : public ThresholdMonitor
{
public:
    template <class Func>
    FunctionThresholdMonitor(const std::string& message_class, const std::string& resource_id, Func func) :
        ThresholdMonitor(message_class, resource_id),
        callback_(func),
        exceeded_(false)
    {
    }

    template <class Target, class Func>
    FunctionThresholdMonitor(const std::string& message_class, const std::string& resource_id, Target target, Func func) :
        ThresholdMonitor(message_class, resource_id),
        callback_(target, func),
        exceeded_(false)
    {
    }

private:
    virtual void update_threshold()
    {
        exceeded_ = callback_(this);
    }

    virtual bool check_threshold() const
    {
        return exceeded_;
    }

    redhawk::callback<bool (ThresholdMonitor*)> callback_;
    bool exceeded_;
};

class ThresholdMonitorSet : public ThresholdMonitor
{
public:
    ThresholdMonitorSet(const std::string& message_class, const std::string& resource_id) :
        ThresholdMonitor(message_class, resource_id)
    {
    }

    void add_monitor(const boost::shared_ptr<ThresholdMonitor>& monitor)
    {
        monitors_.push_back(monitor);
    }

    virtual void enable()
    {
        std::for_each(monitors_.begin(), monitors_.end(), boost::bind(&ThresholdMonitor::enable, _1));
        ThresholdMonitor::enable();
    }

    virtual void disable()
    {
        std::for_each(monitors_.begin(), monitors_.end(), boost::bind(&ThresholdMonitor::disable, _1));
        ThresholdMonitor::disable();
    }

private:
    virtual void update_threshold()
    {
    }

    virtual bool check_threshold() const
    {
        if (monitors_.empty()) {
            return false;
        }

        for (MonitorList::const_iterator monitor = monitors_.begin(); monitor != monitors_.end(); ++monitor) {
            if (!((*monitor)->is_threshold_exceeded())) {
                return false;
            }
        }
        return true;
    }

    typedef std::vector< boost::shared_ptr<ThresholdMonitor> > MonitorList;
    MonitorList monitors_;
};

#endif
