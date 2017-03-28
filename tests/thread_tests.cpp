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

#include <UnitTest++/UnitTest++.h>
#include <mutex>
#include <random>
#include <string>

#include <gsl/gsl_assert>
#include <gsl/gsl_thread>

namespace
{

inline
int random_number(int from, int to)
{
	static std::mutex mtx;
	static std::mt19937 mt{std::random_device{}()};
	std::lock_guard<std::mutex> lock(mtx);
	return std::uniform_int_distribution<int>{from, to}(mt);
}

SUITE(thread_tests)
{
	class scope_test_fixture
	{
	public:
		scope_test_fixture(): v1(fill( 0, 49)), v2(fill(50, 99)), v3(fill( 0, 99)) {}

		std::mutex mtx;
		const std::vector<int> v1;
		const std::vector<int> v2;
		const std::vector<int> v3;

	private:
		std::vector<int> fill(int from, int to)
		{
			Ensures(from < to);
			std::vector<int> v(std::size_t(to - from));
			std::iota(std::begin(v), std::end(v), from);
			return v;
		}
	};

	TEST(same_scope)
	{
		scope_test_fixture fixture;

		std::vector<int> v;

		gsl::raii_thread t1{[&]{
			for(auto i: fixture.v1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(random_number(0, 1)));
				std::lock_guard<std::mutex> lock{fixture.mtx};
				v.push_back(i);
			}
		}};

		for(auto i: fixture.v2)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(random_number(0, 1)));
			std::lock_guard<std::mutex> lock{fixture.mtx};
			v.push_back(i);
		}

		CHECK(v != fixture.v3);
	}

	TEST(different_scope)
	{
		scope_test_fixture fixture;

		std::vector<int> v;

		{
			gsl::raii_thread t1{[&]{
				for(auto i: fixture.v1)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(random_number(0, 1)));
					std::lock_guard<std::mutex> lock{fixture.mtx};
					v.push_back(i);
				}
			}};
		}

		for(auto i: fixture.v2)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(random_number(0, 1)));
			std::lock_guard<std::mutex> lock{fixture.mtx};
			v.push_back(i);
		}

		CHECK(v == fixture.v3);
	}
}

int main(int, const char* []) { return UnitTest::RunAllTests(); }
