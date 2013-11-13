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

#include <com/ubuntu/property.h>

#include <gtest/gtest.h>

TEST(Property, default_construction_yields_default_value)
{
    com::ubuntu::Property<int> p1;
    EXPECT_EQ(int{}, p1.get());

    static const int new_default_value = 42;
    com::ubuntu::Property<int>::set_default_value(new_default_value);
    com::ubuntu::Property<int> p2;

    EXPECT_EQ(new_default_value, p2.get());
}

TEST(Property, copy_construction_yields_correct_value)
{
    com::ubuntu::Property<int>::set_default_value(0);

    static const int default_value = 42;
    com::ubuntu::Property<int> p1{default_value};
    com::ubuntu::Property<int> p2{p1};

    EXPECT_EQ(default_value, p2.get());
}

TEST(Property, assignment_operator_for_properties_works)
{
    com::ubuntu::Property<int>::set_default_value(0);

    static const int default_value = 42;
    com::ubuntu::Property<int> p1{default_value};
    com::ubuntu::Property<int> p2;
    p2 = p1;

    EXPECT_EQ(default_value, p2.get());
}

TEST(Property, assignment_operator_for_raw_values_works)
{
    com::ubuntu::Property<int>::set_default_value(0);

    static const int default_value = 42;
    com::ubuntu::Property<int> p1;
    p1 = default_value;

    EXPECT_EQ(default_value, p1.get());
}

TEST(Property, equality_operator_for_properties_works)
{
    com::ubuntu::Property<int>::set_default_value(0);

    static const int default_value = 42;
    com::ubuntu::Property<int> p1{default_value};
    com::ubuntu::Property<int> p2;
    p2 = p1;

    EXPECT_EQ(p1, p2);
}

TEST(Property, equality_operator_for_raw_values_works)
{
    com::ubuntu::Property<int>::set_default_value(0);

    static const int default_value = 42;
    com::ubuntu::Property<int> p1{default_value};

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
    com::ubuntu::Property<int>::set_default_value(0);

    static const int default_value = 42;
    com::ubuntu::Property<int> p1;
    Expectation<int> expectation{default_value};

    p1.changed().connect([&expectation](int value) { expectation.triggered = true; expectation.current_value = value; });

    p1.set(default_value);

    EXPECT_TRUE(expectation.satisfied());
}

TEST(Property, signal_changed_is_emitted_with_correct_value_for_assignment)
{
    com::ubuntu::Property<int>::set_default_value(0);

    static const int default_value = 42;
    com::ubuntu::Property<int> p1;

    Expectation<int> expectation{42};

    p1.changed().connect([&expectation](int value) { expectation.triggered = true; expectation.current_value = value; });

    p1 = default_value;

    EXPECT_TRUE(expectation.satisfied());
}

TEST(Property, signal_changed_is_emitted_with_correct_value_for_update)
{
    com::ubuntu::Property<int>::set_default_value(0);

    static const int default_value = 42;
    com::ubuntu::Property<int> p1;

    Expectation<int> expectation{default_value};

    p1.changed().connect([&expectation](int value) { expectation.triggered = true; expectation.current_value = value; });
    p1.update([](int& value) { value = default_value; return true; });

    EXPECT_TRUE(expectation.satisfied());
}
