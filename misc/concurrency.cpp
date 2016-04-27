/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2016 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>

using namespace mango;

void exampleConcurrency1()
{
    // MANGO(tm) ThreadPool implementation never puts work to the pool
    // directly. The work is enqueued using work queues, which keep
    // track of tasks the user has submitted.
    ConcurrentQueue q;

    for (int i = 0; i < 10; ++i)
    {
        // Create tasks for the queue
        q.enqueue([] {
            computeSomethingExpensive();
        });
    }

    // The ThreadPool will run the tasks concurrently and they will
    // complete eventually. If we want to block current thread until
    // the queue is drained we can issue this call:
    q.wait();

    // When we get here all of the issued tasks are guaranteed to be completed.
}


void exampleConcurrency2()
{
    ConcurrentQueue q;

    for (int i = 0; i < 20; ++i)
    {
        q.enqueue([=] {
            computeSomethingExpensive(i);
        });
    }

    // Insert execution barrier; tasks issued after the barrier will NOT
    // be processed by the ThreadPool until ALL tasks in the queue BEFORE
    // the barrier are completed.
    q.barrier();

    q.enqueue([] {
        computeSomethingDependingOnPreviousStuff();
    });

    // Insert another barrier but for a different reason...
    q.barrier();

    // A good time to read about std::atomic variables and what they do :)
    std::atomic<bool> done { false };

    // Submit a task which sets the boolean
    q.enqueue([&done] {
        done = true;
    });

    while (!done) {
        // This loop will waste CPU cycles until done equals true. This
        // condition is satisfied ONLY after the task setting it true is
        // executed in the ThreadPool. This won't happen until the tasks
        // BEFORE the second barrier are complete. So, when this loop is
        // finally done we know for a FACT that the tasks in the queue
        // are complete.
    }

    // OK; synchronization like this is pointless on it's own since we can
    // get the same effect with q.wait(). However, this pattern can be used
    // as a building-block for greater things. The multicore-programming
    // wisdom is that when your thread will block, don't put it into a pool.

    // It would be very bad to block a pool with a sleeping thread.
    // What you want to do is to launch free-standing thread for any
    // activity that will block a thread. When a thread is being blocked
    // for any reason (waiting for I/O, waiting for work to complete, etc.)
    // this thread should be sleeping. A sleeping thread does not consume
    // CPU time or energy. A sleeping thread is good.

    // There are many ways to go about this. One way is to use std::promise
    // and std::future which allow to wait-for-result, in practise, sleep
    // until the result is available. This is pretty nice way to pair the
    // MANGO ThreadPool with free-standing threads. The downside is that
    // the communication "channel", if you will, is the heap. This implies
    // dynamic memory allocation. This might be an issue for some so keep
    // that in mind. It's the way of the modern computing world so whatever.

    // The other approach is to use condition variables. These allow the
    // result collector to sleep until notified that the results are
    // available. Both work; which one is better choise depends on many
    // factors which are outside the scope of this small code example.

    // Some Rules of Thumb:
    // - Don't block "main" threads in critical path
    // - If you have to block, do it in isolated thread, this is O.K.
    // - Know what you are doing (above only applies most of the time)
    // - Better Rules of Thumb to communicate the intent better :)
}

void exampleConcurrency3()
{
    // SerialQueue has a special property: each task has automatic
    // dependency to previously issued task. This means that the tasks
    // are executed in well defined order and the results of previous
    // task are always visible to the next task.
    SerialQueue a;
    SerialQueue b;

    for (int i = 0; i < 10; ++i)
    {
        a.enqueue([] {
            computeStuff(i);
        });

        // The tasks in queue b run in complete isolation from queue a.
        // There is no dependency between the two queues. The dependency
        // and serialization is always unique for each queue.

        // This means that one SerialQueue cannot consume more than one
        // worker thread in the ThreadPool at any given time since only
        // one task is allowed to execute at a time.

        // In this example we have two serial queues which means only
        // two worker threads, at best, can be utilized.

        // Concurrent queues consume all available pool workers since
        // concurrent tasks don't have any dependencies, except those which
        // are artificially created with execution barriers.

        b.enqueue([] {
            computeOtherStuff(i);
        });
    }
}
