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

#include <functional>

#include "ThresholdMonitor.h"

ThresholdMonitor::ThresholdMonitor(const std::string& resource_id, const std::string& threshold_class):
    resource_id_(resource_id),
    threshold_class_(threshold_class),
    enabled_(true),
    prev_threshold_exceeded_(false)
{
}

const std::string& ThresholdMonitor::get_resource_id() const
{
    return resource_id_;
}

const std::string& ThresholdMonitor::get_threshold_class() const
{
    return threshold_class_;
}

bool ThresholdMonitor::is_enabled() const
{
    return enabled_;
}

void ThresholdMonitor::enable()
{
    enabled_ = true;
    update();
}

void ThresholdMonitor::disable()
{
    enabled_ = false;
    update();
}

void ThresholdMonitor::update()
{
    if (enabled_) {
        update_threshold();
    }
    if (prev_threshold_exceeded_ != is_threshold_exceeded()) {
        notification_(this);
    }
    prev_threshold_exceeded_ = is_threshold_exceeded();
}

bool ThresholdMonitor::is_threshold_exceeded() const
{
    if (!enabled_) return false;
    return check_threshold();
}


void FunctionThresholdMonitor::update_threshold()
{
    exceeded_ = callback_(this);
}

bool FunctionThresholdMonitor::check_threshold() const
{
    return exceeded_;
}


ThresholdMonitorSet::ThresholdMonitorSet(const std::string& resource_id, const std::string& threshold_class) :
    ThresholdMonitor(resource_id, threshold_class)
{
}

void ThresholdMonitorSet::add_monitor(const boost::shared_ptr<ThresholdMonitor>& monitor)
{
    monitors_.push_back(monitor);
}

void ThresholdMonitorSet::enable()
{
    std::for_each(monitors_.begin(), monitors_.end(), boost::bind(&ThresholdMonitor::enable, _1));
    ThresholdMonitor::enable();
}

void ThresholdMonitorSet::disable()
{
    std::for_each(monitors_.begin(), monitors_.end(), boost::bind(&ThresholdMonitor::disable, _1));
    ThresholdMonitor::disable();
}

void ThresholdMonitorSet::update_threshold()
{
    std::for_each(monitors_.begin(), monitors_.end(), boost::bind(&ThresholdMonitor::update, _1));
}

bool ThresholdMonitorSet::check_threshold() const
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
