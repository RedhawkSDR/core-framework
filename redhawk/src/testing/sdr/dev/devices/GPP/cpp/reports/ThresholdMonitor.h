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

#include "utils/Updateable.h"
#include "utils/EventDispatcher.h"
#include "Reporting.h"
#include "struct_props.h"

//class ThresholdMonitor :  public Reporting, public EventDispatcherMixin<threshold_event_struct>
class ThresholdMonitor :  public Updateable, public EventDispatcherMixin<threshold_event_struct>
{
public:
 ThresholdMonitor( const std::string& message_class, const std::string& resource_id, const bool enableDispatch=true):
  _enable_dispatch( enableDispatch),
    resource_id_(resource_id),
    message_class_(message_class)
    {}

 ThresholdMonitor( const std::string& source_id, const std::string& resource_id, const std::string& message_class, const bool enableDispatch=true):
  _enable_dispatch(enableDispatch),
    source_id_(source_id),
    resource_id_(resource_id),
      message_class_(message_class)
    {}

    virtual void update() = 0;
    //void report(){ update(); }

    virtual std::string get_threshold() const = 0;
    virtual std::string get_measured() const = 0;
    virtual bool is_threshold_exceeded() const = 0;
    void    enable_dispatch() { _enable_dispatch=true;}
    void    disable_dispatch() { _enable_dispatch=false;}
    std::string get_source_id() const{ return source_id_; }
    std::string get_resource_id() const{ return resource_id_; }
    std::string get_message_class() const{ return message_class_; }

protected:
    void dispatch_message() const
    {
      if ( !_enable_dispatch )   return;

        threshold_event_struct message;
        message.source_id = get_source_id();
        message.resource_id = get_resource_id();
        message.threshold_class = get_message_class();
        message.type = get_message_type();
        message.threshold_value = get_threshold();
        message.measured_value = get_measured();
        message.message = get_message_string();
        message.timestamp = time(NULL);

        dispatch(message);
    }
    std::string get_message_type() const
    {
        return is_threshold_exceeded() ? "THRESHOLD_EXCEEDED" : "THRESHOLD_NOT_EXCEEDED";
    }
    std::string get_message_string() const
    {
        std::stringstream sstr;
        std::string exceeded_or_not( is_threshold_exceeded() ? "" : "not " );

        sstr << get_message_class() << " threshold " << exceeded_or_not << "exceeded "
             << "(resource_id=" << get_resource_id()
             << " threshold_value=" << get_threshold()
             << " measured_value=" << get_measured() << ")";

        return sstr.str();
    }

    bool  _enable_dispatch;

private:
    const std::string source_id_;
    const std::string resource_id_;
    const std::string message_class_;

};

template<class DATA_TYPE, class COMPARISON_FUNCTION = std::less<DATA_TYPE> >
class GenericThresholdMonitor : public ThresholdMonitor
{
public:
    typedef DATA_TYPE DataType;
    typedef boost::function< DataType() > QueryFunction;

public:
 GenericThresholdMonitor( const std::string& message_class, const std::string& resource_id, QueryFunction threshold, QueryFunction measured, const bool enableDispatch=true ):
ThresholdMonitor(message_class, resource_id, enableDispatch),
    threshold_(threshold),
    measured_(measured),
    threshold_value_( threshold() ),
    measured_value_( measured() ),
    prev_threshold_exceeded_(false)
    {
    }

 GenericThresholdMonitor( const std::string& source_id, const std::string& resource_id, const std::string& message_class, QueryFunction threshold, QueryFunction measured, const  bool enableDispatch=true ):
ThresholdMonitor(source_id, resource_id, message_class, enableDispatch ),
    threshold_(threshold),
    measured_(measured),
    threshold_value_( threshold() ),
    measured_value_( measured() ),
    prev_threshold_exceeded_(false)
    {
    }

    void update()
    {
        threshold_value_ = threshold_();
        measured_value_ = measured_();
        if( prev_threshold_exceeded_ != is_threshold_exceeded() )
        {
            dispatch_message();
        }

        prev_threshold_exceeded_ = is_threshold_exceeded();
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
    QueryFunction threshold_;
    QueryFunction measured_;

    DataType threshold_value_;
    DataType measured_value_;
    bool prev_threshold_exceeded_;
};



#endif
