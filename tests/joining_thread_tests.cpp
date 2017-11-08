///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
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
#include <gsl/joining_thread>
#include <string>

TEST_CASE("joining_thread construction")
{
    gsl::joining_thread t1{[]() {}};
    std::string text{"joining_thread example"};
    gsl::joining_thread t2{[](std::string) {}, text};
    gsl::joining_thread t3{[](const std::string&) {}, text};
    gsl::joining_thread t4{[](const std::string&) {}, std::ref(text)};
    gsl::joining_thread t5{[](std::string&&) {}, text};
    gsl::joining_thread t6{[](std::string&&) {}, std::move(text)};
    std::string other_text{"second gsl::joining_thread example"};
    gsl::joining_thread t7{[](std::string) {}, std::move(other_text)};
    gsl::joining_thread t8{std::move(t1)};
    gsl::joining_thread t9 = std::move(t5);
#ifdef CONFIRM_COMPILATION_ERRORS
    {
        gsl::joining_thread t10{};    // should not compile - no default c-tor
        gsl::joining_thread t11 = t6; // should not compile - no copy assigment
        gsl::joining_thread t12(t7);  // should not compile - no copy c-tor
    }
#endif
}

TEST_CASE("joining_thread get_id")
{
    std::thread::id currentThreadId{};
    std::thread::id joiningThreadId{};
    {
        gsl::joining_thread thread{
            [&currentThreadId]() { currentThreadId = std::this_thread::get_id(); }};
        joiningThreadId = thread.get_id();
    }
    REQUIRE(currentThreadId == joiningThreadId);
}

TEST_CASE("joining_thread hardware_concurrency")
{
    auto stdThreadHardwareConcurrency = std::thread::hardware_concurrency();
    auto joiningThreadHardwareConcurrency = gsl::joining_thread::hardware_concurrency();
    REQUIRE(joiningThreadHardwareConcurrency == stdThreadHardwareConcurrency);
}

TEST_CASE("joining_thread swap")
{
    {
        gsl::joining_thread t1{[]() {}};
        gsl::joining_thread t2{[]() {}};
        auto t1BeforeSwapId = t1.get_id();
        auto t2BeforeSwapId = t2.get_id();
        std::swap(t1, t2);
        REQUIRE(t1.get_id() == t2BeforeSwapId);
        REQUIRE(t2.get_id() == t1BeforeSwapId);
    }
    {
        gsl::joining_thread t1{[]() {}};
        auto t1BeforeSwapId = t1.get_id();
        std::swap(t1, gsl::joining_thread([]() {}));
        REQUIRE(t1.get_id() != t1BeforeSwapId);
    }
    {
        gsl::joining_thread t1{[]() {}};
        auto t1BeforeSwapId = t1.get_id();
        std::swap(gsl::joining_thread([]() {}), t1);
        REQUIRE(t1.get_id() != t1BeforeSwapId);
    }
}

TEST_CASE("check if joining_thread is running functor in seperate thread")
{
    struct Mock
    {
        bool called{false};
        std::thread::id calledFrom{};
    };
    Mock mock{};
    std::thread::id joiningThreadId{};
    {
        gsl::joining_thread t{[&mock]() {
            mock.called = true;
            mock.calledFrom = std::this_thread::get_id();
        }};
        joiningThreadId = t.get_id();
    }
    REQUIRE(mock.called);
    REQUIRE(mock.calledFrom == joiningThreadId);
    REQUIRE(joiningThreadId != std::this_thread::get_id());
}
