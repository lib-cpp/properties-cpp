/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#include <core/signal.h>

#include <gtest/gtest.h>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace
{
template<typename T>
struct Expectation
{
    Expectation(const T& expected_value) : expected_value(expected_value)
    {
    }

    bool satisfied() const
    {
        return triggered && current_value == expected_value;
    }

    bool triggered = false;
    T expected_value;
    T current_value;
};
}

TEST(Signal, emission_works)
{
    Expectation<int> expectation{42};

    core::Signal<int> s;
    s.connect([&expectation](int value) { expectation.triggered = true; expectation.current_value = value; });

    s(42);

    EXPECT_TRUE(expectation.satisfied());
}

TEST(Signal, disconnect_results_in_slots_not_invoked_anymore)
{
    Expectation<int> expectation{42};

    core::Signal<int> s;
    auto connection = s.connect(
                [&expectation](int value)
                {
                    expectation.triggered = true;
                    expectation.current_value = value;
                });
    connection.disconnect();
    s(42);

    EXPECT_FALSE(expectation.satisfied());
}

TEST(Signal, disconnect_via_scoped_connection_results_in_slots_not_invoked_anymore)
{
    Expectation<int> expectation{42};

    core::Signal<int> s;
    auto connection = s.connect(
                [&expectation](int value)
                {
                    expectation.triggered = true;
                    expectation.current_value = value;
                });
    {
        core::ScopedConnection sc{connection};
    }
    s(42);

    EXPECT_FALSE(expectation.satisfied());
}

TEST(Signal, a_signal_going_out_of_scope_disconnects_from_slots)
{
    auto signal = std::make_shared<core::Signal<int>>();

    auto connection = signal->connect([](int value) { std::cout << value << std::endl; });

    signal.reset();

    core::Connection::Dispatcher dispatcher{};

    EXPECT_NO_THROW(connection.disconnect());
    EXPECT_NO_THROW(connection.dispatch_via(dispatcher));
}

#include <queue>

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
                [&dispatcher, dispatcher_thread_id](int value, double)
                {
                    EXPECT_EQ(dispatcher_thread_id,
                              std::this_thread::get_id());                    

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
