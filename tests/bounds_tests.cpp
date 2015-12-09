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
#include <span.h>
#include <vector>

using namespace std;
using namespace gsl;;

namespace 
{
    void use(std::ptrdiff_t&) {}
}

SUITE(bounds_test)
{
	TEST(basic_bounds)
	{
		for (auto point : static_bounds<dynamic_range, 3, 4 > { 2 })
		{
			for (decltype(point)::size_type j = 0;
			     j < static_cast<decltype(point)::size_type>(decltype(point)::rank);
			     j++)
			{
				use(j);
				use(point[j]);
			}
		}
	}
	
	TEST(bounds_basic)
	{
		static_bounds<3, 4, 5> b;
		auto a = b.slice();
		(void)a;           
		static_bounds<4, dynamic_range, 2> x{ 4 };
		x.slice().slice();
	}
	
	TEST (arrayview_iterator)
	{
		static_bounds<4, dynamic_range, 2> bounds{ 3 };
		
		auto itr = bounds.begin();
		(void)itr;	
#ifdef CONFIRM_COMPILATION_ERRORS
		span<int, 4, dynamic_range, 2> av(nullptr, bounds);
	
		auto itr2 = av.cbegin();
	
		for (auto& v : av) {
			v = 4;
		}
		fill(av.begin(), av.end(), 0);
#endif			
	}
	
	TEST (bounds_convertible)
	{
		static_bounds<7, 4, 2> b1;
		static_bounds<7, dynamic_range, 2> b2 = b1;
		(void)b2;	
#ifdef CONFIRM_COMPILATION_ERRORS
		static_bounds<7, dynamic_range, 1> b4 = b2; 
#endif
	
		static_bounds<dynamic_range, dynamic_range, dynamic_range> b3 = b1;
		static_bounds<7, 4, 2> b4 = b3; 
		(void)b4;

		static_bounds<dynamic_range> b11;
	
		static_bounds<dynamic_range> b5;
		static_bounds<34> b6;
		
		b5 = static_bounds<20>();
		CHECK_THROW(b6 = b5, fail_fast);
		b5 = static_bounds<34>();
		b6 = b5;

		CHECK(b5 == b6);
		CHECK(b5.size() == b6.size());
	}                                 
}

int main(int, const char *[])
{
	return UnitTest::RunAllTests();
}
