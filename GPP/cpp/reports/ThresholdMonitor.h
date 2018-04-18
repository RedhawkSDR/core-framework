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
        prev_threshold_exceeded_(false)
    {}

    virtual void update()
    {
        update_threshold();
        if (prev_threshold_exceeded_ != is_threshold_exceeded()) {
            notification_(this);
        }
        prev_threshold_exceeded_ = is_threshold_exceeded();
    }

    virtual std::string get_threshold() const = 0;
    virtual std::string get_measured() const = 0;
    virtual bool is_threshold_exceeded() const = 0;
    std::string get_resource_id() const{ return resource_id_; }
    std::string get_message_class() const{ return message_class_; }

    template <class Target, class Func>
    void add_listener(Target target, Func func)
    {
        notification_.add(target, func);
    }

protected:
    virtual void update_threshold() = 0;

    const std::string resource_id_;
    const std::string message_class_;
    bool prev_threshold_exceeded_;

    ossie::notification<void (ThresholdMonitor*)> notification_;
};

template<class DATA_TYPE, class COMPARISON_FUNCTION = std::less<DATA_TYPE> >
class GenericThresholdMonitor : public ThresholdMonitor
{
public:
    typedef DATA_TYPE DataType;
    typedef boost::function< DataType() > QueryFunction;

public:
    GenericThresholdMonitor(const std::string& message_class, const std::string& resource_id, QueryFunction threshold, QueryFunction measured):
        ThresholdMonitor(message_class, resource_id),
        threshold_(threshold),
        measured_(measured),
        threshold_value_( threshold() ),
        measured_value_( measured() )
    {
    }

    std::string get_threshold() const{ return boost::lexical_cast<std::string>(threshold_value_); }
    std::string get_measured() const{ return boost::lexical_cast<std::string>(measured_value_); }

    bool is_threshold_exceeded() const
    {
        return COMPARISON_FUNCTION()( get_measured_value(), get_threshold_value() );
    }

    DataType get_threshold_value() const { return threshold_value_; }
    DataType get_measured_value() const { return measured_value_; }

private:
    void update_threshold()
    {
        threshold_value_ = threshold_();
        measured_value_ = measured_();
    }

    QueryFunction threshold_;
    QueryFunction measured_;

    DataType threshold_value_;
    DataType measured_value_;
};



#endif
