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

#include <ossie/GCThread.h>
#include <ossie/GCContext.h>

using namespace ossie;

boost::mutex GCThread::instanceMutex_;
GCThread* GCThread::instance_ = 0;

GCThread::GCThread():
    sleepTime_(boost::posix_time::seconds(1)),
    thread_(&GCThread::run, this)
{
}

GCThread::~GCThread()
{
}

void GCThread::sweep()
{
    boost::mutex::scoped_lock lock(contextsMutex_);
    for (std::list<GCContext*>::iterator iter = contexts_.begin(); iter != contexts_.end(); ++iter) {
        (*iter)->gc_sweep();
    }
}

void GCThread::run()
{
    while (true) {
        sweep();
        boost::this_thread::sleep(sleepTime_);
    }
}

void GCThread::stop()
{
    thread_.interrupt();
    thread_.join();
}

GCThread* GCThread::instance()
{
    boost::mutex::scoped_lock lock(instanceMutex_);
    if (!instance_) {
        instance_ = new GCThread();
    }
    return instance_;
}

void GCThread::add(GCContext* context)
{
    instance()->addContext(context);
}

void GCThread::release(GCContext* context)
{
    instance()->releaseContext(context);
}

void GCThread::shutdown()
{
    boost::mutex::scoped_lock lock(instanceMutex_);
    if (!instance_) {
        return;
    }
    instance_->stop();
    delete instance_;
    instance_ = 0;
}

void GCThread::addContext(GCContext* context)
{
    boost::mutex::scoped_lock lock(contextsMutex_);
    contexts_.push_back(context);
}

void GCThread::releaseContext(GCContext* context)
{
    boost::mutex::scoped_lock lock(contextsMutex_);
    contexts_.remove(context);
}
