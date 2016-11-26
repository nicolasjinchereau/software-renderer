/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <deque>
#include <functional>
#include "Vertex.h"
#include "Shader.h"
#include "RenderingContext.h"
#include <atomic>
using namespace std;

class spinlock {
    atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while(flag.test_and_set(memory_order_acquire)){}
    }

    void unlock() {
        flag.clear(memory_order_release);
    }
};

class RenderThread
{
    struct Task
    {
        RenderingContext* context = nullptr;
        Rect rect;

        Task(){}
        Task(const Task& t) : context(t.context), rect(t.rect){}
        Task(RenderingContext* context, const Rect& rect)
            : context(context), rect(rect){}

        operator bool() { return context != nullptr; }
    };

    thread _trd;
    Task _task;
    mutable condition_variable_any _task_cv;
    mutable condition_variable_any _busy_cv;
    mutable spinlock _spn;
    volatile bool _run = true;
    volatile bool _busy = false;
    function<bool()> exitBusyWait;
    function<bool()> exitTaskWait;
public:
    RenderThread() {
        exitBusyWait = [this]{ return !this->_busy; };
        exitTaskWait = [this]{ return !this->_run || this->_task; };
        _run = true;
        _trd = thread(bind(&RenderThread::Run, this));
    }

    ~RenderThread()
    {
        _run = false;
        _task_cv.notify_one();
        _trd.join();
    }

    bool IsBusy() const {
        lock_guard<spinlock> lk(_spn);
        return _busy;
    }

    void Wait() const {
        unique_lock<spinlock> lk(_spn);
        _busy_cv.wait(lk, exitBusyWait);
    }

    void Execute(RenderingContext* context, const Rect& rect)
    {
        lock_guard<spinlock> lk(_spn);
        if(_busy) return;
        _busy = true;
        _task = Task(context, rect);
        _task_cv.notify_one();
    }

private:
    void Run()
    {
        while(_run)
        {
            Task task(nullptr, Rect());

            {
                unique_lock<spinlock> lk(_spn);

                if(!_task)
                {
                    _busy = false;
                    _busy_cv.notify_one();
                    _task_cv.wait(lk, exitTaskWait);
                }

                if(_run)
                    swap(_task, task);
            }

            if(task)
            {
                RenderingContext* context = task.context;
                for(auto& drawCall : context->_drawCalls)
                {
                    Rect rect = task.rect;
                    auto& cverts = context->_cverts;

                    for(size_t i = drawCall.start; i < drawCall.end; i += 3)
                        context->Rasterize(rect, cverts[i], cverts[i + 1], cverts[i + 2], &drawCall);
                }

                context->Resolve(task.rect);
            }
        }
    }
};
