///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#include <catch/catch.hpp>

#include <atomic>
#include <chrono>
#include <thread>

#include <gsl/gsl_assert>
#include <gsl/gsl_thread>

using std::chrono::steady_clock;
using std::this_thread::sleep_for;

const auto t_100ms = std::chrono::milliseconds(100);

TEST_CASE("joining_thread_tests")
{
    SECTION("joining_thread_same_scope_clock_test")
    {
        auto start_time = steady_clock::now();

        gsl::joining_thread t{[&]{ sleep_for(t_100ms); }};

        auto end_time = steady_clock::now();

        CHECK(end_time - start_time < t_100ms);
    }

    SECTION("joining_thread_different_scope_clock_test")
    {
        auto start_time = steady_clock::now();

        {
            gsl::joining_thread t{[&]{ sleep_for(t_100ms); }};
        }

        auto end_time = steady_clock::now();

        CHECK(end_time - start_time >= t_100ms);
    }

    SECTION("joining_thread_ctor_test")
    {
        auto start_time = steady_clock::now();

        gsl::joining_thread t1{[&]{ sleep_for(t_100ms); }};

        {
            gsl::joining_thread t2{std::move(t1)};
        }

        auto end_time = steady_clock::now();

        CHECK(end_time - start_time >= t_100ms);
    }

    SECTION("joining_thread_assign_test")
    {
        auto start_time = steady_clock::now();

        gsl::joining_thread t1{[&]{ sleep_for(t_100ms); }};

        {
            gsl::joining_thread t2;
            t2 = std::move(t1);
        }

        auto end_time = steady_clock::now();

        CHECK(end_time - start_time >= t_100ms);
    }

    SECTION("joining_thread_std_thread_ctor_test")
    {
        auto start_time = steady_clock::now();

        std::thread t1{[&]{ sleep_for(t_100ms); }};

        {
            gsl::joining_thread t2{std::move(t1)};
        }

        auto end_time = steady_clock::now();

        CHECK(end_time - start_time >= t_100ms);
    }

    SECTION("joining_thread_std_thread_assign_test")
    {
        auto start_time = steady_clock::now();

        std::thread t1{[&]{ sleep_for(t_100ms); }};

        {
            gsl::joining_thread t2;
            t2 = std::move(t1);
        }

        auto end_time = steady_clock::now();

        CHECK(end_time - start_time >= t_100ms);
    }

    SECTION("joining_thread_swap_test")
    {
        gsl::joining_thread t1{[&]{ sleep_for(t_100ms); }};
        gsl::joining_thread t2{[&]{ sleep_for(t_100ms); }};

        auto id1 = t1.get_id();
        auto id2 = t2.get_id();

        std::swap(t1, t2);

        CHECK(t1.get_id() == id2);
        CHECK(t2.get_id() == id1);

        t1.swap(t2);

        CHECK(t1.get_id() == id1);
        CHECK(t2.get_id() == id2);
    }
}


