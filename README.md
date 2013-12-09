properties-cpp         {#mainpage}
===========

process-cpp is a simple header-only implementation of properties and
signals. It is meant to be used for developing low-level system
services. Its main features include:

 - Thread-safe signal invocation and observer mgmt.
 - The ability to dispatch signal invocations via arbitrary event loops.
 - Typed properties with an in-place update mechanism that avoids unneccessary deep copies.
 - Well tested and documented.

A Textfield With an Observable Cursor Position
----------------------------------------------

~~~~~~~~~~~~~{.cpp}
namespace
{
struct TextField
{
    void move_cursor_to(int new_position)
    {
        cursor_position.set(new_position);
    }
    
    core::Property<int> cursor_position;
};
}

TEST(Property, cursor_position_changes_are_transported_correctly)
{
    int position = -1;

    TextField tf;
    
    // Setup a connection to the cursor position property
    tf.cursor_position.changed().connect(
        [&position](int value) 
        { 
            position = value; 
        });

    // Move the cursor
    tf.move_cursor_to(22);

    // Check that the correct value has propagated
    EXPECT_EQ(22, position);
}
~~~~~~~~~~~~~

Integrating With Arbitrary Event Loops/Reactor Implementations
--------------------------------------------------------------
~~~~~~~~~~~~~{.cpp}
namespace
{
struct EventLoop
{
    typedef std::function<void()> Handler;

    void stop()
    {
        stop_requested = true;
    }

    void run()
    {
        while (!stop_requested)
        {
            std::unique_lock<std::mutex> ul(guard);
            wait_condition.wait_for(
                        ul,
                        std::chrono::milliseconds{500},
                        [this]() { return handlers.size() > 0; });

            while (handlers.size() > 0)
            {
                handlers.front()();
                handlers.pop();
            }
        }
    }

    void dispatch(const Handler& h)
    {
        std::lock_guard<std::mutex> lg(guard);
        handlers.push(h);
    }

    bool stop_requested = false;
    std::queue<Handler> handlers;
    std::mutex guard;
    std::condition_variable wait_condition;
};
}

TEST(Signal, installing_a_custom_dispatcher_ensures_invocation_on_correct_thread)
{
    // We instantiate an event loop and run it on a different thread than the main one.
    EventLoop dispatcher;
    std::thread dispatcher_thread{[&dispatcher]() { dispatcher.run(); }};
    std::thread::id dispatcher_thread_id = dispatcher_thread.get_id();

    // The signal that we want to dispatch via the event loop.
    core::Signal<int, double> s;

    static const int expected_invocation_count = 10000;

    // Setup the connection. For each invocation we check that the id of the
    // thread the handler is being called upon equals the thread that the
    // event loop is running upon.
    auto connection = s.connect(
                [&dispatcher, dispatcher_thread_id](int value, double d)
                {
                    EXPECT_EQ(dispatcher_thread_id,
                              std::this_thread::get_id());

                    std::cout << d << std::endl;

                    if (value == expected_invocation_count)
                        dispatcher.stop();
                });

    // Route the connection via the dispatcher
    connection.dispatch_via(
                std::bind(
                    &EventLoop::dispatch,
                    std::ref(dispatcher),
                    std::placeholders::_1));

    // Invoke the signal from the main thread.
    for (unsigned int i = 1; i <= expected_invocation_count; i++)
        s(i, 42.);

    if (dispatcher_thread.joinable())
        dispatcher_thread.join();
}
~~~~~~~~~~~~~
