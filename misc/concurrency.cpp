/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2016 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>

using namespace mango;

void example1()
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


void example2()
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
}

void example3()
{
    // SerialQueue has a special property: each task has automatic
    // dependency to previously issued task. This means that the tasks
    // are executed in well defined order and the results of previous
    // task are always visible to the next task.
    SerialQueue a;
    ConcurrentQueue b;

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

        // Concurrent queues consume all available pool workers since
        // concurrent tasks don't have any dependencies, except those which
        // are artificially created with execution barriers.

        b.enqueue([] {
            computeOtherStuff(i);
        });
    }
}

void example4()
{
    ConcurrentQueue q;
    std::atomic<int> counter { 0 };

    for (int i = 0; i < 10; ++i)
    {
        ConcurrentQueue x;

        q.enqueue([&] {
            // enqueueuing tasks from tasks works fine and can improve
            // throughput in some situations where single thread cannot keep
            // all of the pool's worker threads busy. This situation is more common
            // when there are a lot of available workers (for example, a Xeon
            // processor with more than 6 cores) and tasks are cheap.

            // The ThreadPool is currently able to process 10,000,000 tasks
            // per second on Intel Core i7 3770K CPU. The design uses atomic
            // memory operations to implement lock-free inter-thread communication.
            // A similar design using mutex/locking can do roughly 200,000 tasks
            // per second on a similar hardware. The bottleneck is how to feed
            // the ThreadPool enough work if the tasks are really trivial. A good
            // rule-of-thumb to keep the system responsive is to keep the tasks
            // short enough but not too short (for example, incrementing a counter
            // would obviously be a bit too trivial thing to do in a ThreadPool).

            x.enqueue([&] {
                ++counter; // Oops..
            });
        });
    }
}

void example5()
{
    // Queues can be named and given a priority (LOW, NORMAL, HIGH)

    // The granularity of priorities is not very fine-grained accidentally,
    // the idea is that there is some amount of control but no incentive
    // to go full-retard when scheduling the tasks.

    // The queue names can be useful when debugging and instrumenting
    // the ThreadPool to get more detailed information what is going on.
    ConcurrentQueue queue("important queue", Priority::HIGH);

    // Dispatch tasks here.
}

/*
    Demonstrate a way to keep reference to shared state which is processed in the
    ThreadPool. The State object is used in the captured lambdas which are executed
    in the pool.
*/
struct State
{
    std::vector<float> data;

    State(size_t size)
    : data(size)
    {
        // initialize the data...
    }

    void process(size_t offset, size_t count)
    {
        assert(offset + count < data.size());
        // process a range in the data here...
    }
};

void example6()
{
    // Create the shared State in the heap and refer to it using std::shared_ptr
    std::shared_ptr<State> state = std::make_shared<State>(1024);

    ConcurrentQueue q;

    for (size_t i = 0; i < 1024; i += 64)
    {
        // NOTE! State is captued by value so that the reference counting
        // is used to keep track of state life-time. In other words, the last
        // worker thread to complete will "turn off the lights". This way work can be
        // queued in the pool w/o having to synchronize before exiting this scope.
        q.enqueue([state, i] {
            state.process(i, 64);
        });
    }

    // q.wait(); <-- won't be required as the std::shared_ptr will keep track of last
    //               shared_ptr to hold reference to the state.

    // ALWAYS KEEP THE LIFE-TIME OF QUEUED OBJECTS IN MIND!!!

    // This techniques allows us to leave the book-keeping to the reference counting
    // in the std::shared_ptr. The downside is, once again, that the object lives in
    // the heap and is dynamically allocated.

    // One nice trick to mention: the State destructor could be used to enqueue
    // more work in the pool. This allows doing processing on the results of the
    // computation w/o requiring a barrier synchronization primitive.
    // In short: have implicit synchronization based on how you feed the work!
}

void example7()
{
    // Can this code deadlock?
    ConcurrentQueue q;
    for (int i = 0; i < 200; ++i)
    {
        q.enqueue([] {
            ConcurrentQueue x;
            for (int j = 0; j < 40; ++j)
            {
                x.enqueue([] {
                    computeSomething();
                });
            }
            x.wait();
        });
    }
    q.wait();

    // At first glance, it seems possible that x.wait() could be running simultaneously
    // as many times as we have available threads in the pool. This would be a problem
    // if all of the workers were sleeping and tasks couldn't get through. We would
    // be deadlocked and our program would hang.

    // It is very easy to paint yourself into the corner like this and generally NEVER
    // sleep or wait for a signal from other tasks in a thread pool.

    // However, our queue wait is implemented as a cooperative-wait so it will help
    // executing tasks in a pool. The unfortunate downside is that this increases latency
    // slightly since the wait might still be currently processing a task not related
    // to the current queue's wait status. The upside is that the pool cannot be
    // deadlocked with wait. Even if task being processed in a wait has a wait in it,
    // that wait will be cooperative as well and we degenerate into recursion but this
    // should be extremely rare, in practise non-existent if the idea to never wait in a
    // task is followed.

    // The takeaway is that if you really, absolutely have to synchronize in a enqueued
    // task then use the queue wait. It's better to have implicit synchronization by
    // NOT issuing any tasks before the data is available (eg. trigger a task when the
    // data is known to be available). Just friendly advice, feel free to ignore and cry.
}
