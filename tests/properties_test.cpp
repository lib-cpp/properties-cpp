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

#include <core/property.h>

#include <gtest/gtest.h>

TEST(Property, default_construction_yields_default_value)
{
    core::Property<int> p1;
    EXPECT_EQ(int{}, p1.get());

    static const int new_default_value = 42;
    core::Property<int> p2{new_default_value};

    EXPECT_EQ(new_default_value, p2.get());
}

TEST(Property, copy_construction_yields_correct_value)
{
    static const int default_value = 42;
    core::Property<int> p1{default_value};
    core::Property<int> p2{p1};

    EXPECT_EQ(default_value, p2.get());
}

TEST(Property, assignment_operator_for_properties_works)
{
    static const int default_value = 42;
    core::Property<int> p1{default_value};
    core::Property<int> p2;
    p2 = p1;

    EXPECT_EQ(default_value, p2.get());
}

TEST(Property, assignment_operator_for_raw_values_works)
{
    static const int default_value = 42;
    core::Property<int> p1;
    p1 = default_value;

    EXPECT_EQ(default_value, p1.get());
}

TEST(Property, equality_operator_for_properties_works)
{
    static const int default_value = 42;
    core::Property<int> p1{default_value};
    core::Property<int> p2;
    p2 = p1;

    EXPECT_EQ(p1, p2);
}

TEST(Property, equality_operator_for_raw_values_works)
{
    static const int default_value = 42;
    core::Property<int> p1{default_value};

    EXPECT_EQ(default_value, p1);
}

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

TEST(Property, signal_changed_is_emitted_with_correct_value_for_set)
{
    static const int default_value = 42;
    core::Property<int> p1;
    Expectation<int> expectation{default_value};

    p1.changed().connect([&expectation](int value) { expectation.triggered = true; expectation.current_value = value; });

    p1.set(default_value);

    EXPECT_TRUE(expectation.satisfied());
}

TEST(Property, signal_changed_is_emitted_with_correct_value_for_assignment)
{
    static const int default_value = 42;
    core::Property<int> p1;

    Expectation<int> expectation{42};

    p1.changed().connect([&expectation](int value) { expectation.triggered = true; expectation.current_value = value; });

    p1 = default_value;

    EXPECT_TRUE(expectation.satisfied());
}

TEST(Property, signal_changed_is_emitted_with_correct_value_for_update)
{
    static const int default_value = 42;
    core::Property<int> p1;

    Expectation<int> expectation{default_value};

    p1.changed().connect([&expectation](int value) { expectation.triggered = true; expectation.current_value = value; });
    p1.update([](int& value) { value = default_value; return true; });

    EXPECT_TRUE(expectation.satisfied());
}

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
    
    tf.cursor_position.changed().connect(
        [&position](int value) 
        { 
            position = value; 
        });

    tf.move_cursor_to(22);

    EXPECT_EQ(22, position);
}

TEST(Property, chaining_properties_works)
{
    core::Property<int> p1, p2;

    p1 | p2;

    p1.set(42);

    EXPECT_EQ(42, p2.get());
}

TEST(Property, getter_is_invoked_for_get_operations)
{
    bool invoked = false;
    auto getter = [&invoked]()
    {
        invoked = true;
        return 42;
    };

    core::Property<int> prop;
    prop.install(getter);

    EXPECT_EQ(42, prop.get());
    EXPECT_TRUE(invoked);
}

TEST(Property, setter_is_invoked_for_set_operations)
{
    int value = 0;
    auto setter = [&value](int new_value)
    {
        value = new_value;
    };

    core::Property<int> prop;
    prop.install(setter);

    prop.set(42);
    EXPECT_EQ(42, value);
}
