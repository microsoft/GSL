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
#include <array_view.h>
#include <numeric>
#include <array>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <functional>
#include <algorithm>


using namespace std;
using namespace Guide;

namespace 
{
	void use(int&) {}
	struct BaseClass {};
	struct DerivedClass : BaseClass {};
}

SUITE(array_view_tests)
{
	TEST(basics)
	{
		auto ptr = as_array_view(new int[10], 10);
		fill(ptr.begin(), ptr.end(), 99);
		for (int num : ptr)
		{
			CHECK(num == 99);
		}

		delete[] ptr.data();


		static_bounds<size_t, 4, dynamic_range, 2> bounds{ 3 };
	
#ifdef CONFIRM_COMPILATION_ERRORS
		array_view<int, 4, dynamic_range, 2> av(nullptr, bounds);
		av.extent();
		av.extent<2>();
		av[8][4][3];
#endif
	}

	TEST (array_view_convertible)
	{
#ifdef CONFIRM_COMPILATION_ERRORS
		array_view<int, 7, 4, 2> av1(nullptr, b1);
#endif
	
		auto f = [&]() { array_view<int, 7, 4, 2> av1(nullptr); };
		CHECK_THROW(f(), fail_fast); 
		
		array_view<int, 7, dynamic_range, 2> av1(nullptr);

#ifdef CONFIRM_COMPILATION_ERRORS
		static_bounds<size_t, 7, dynamic_range, 2> b12(b11);
		b12 = b11;
		b11 = b12;
	
		array_view<int, dynamic_range> av1 = nullptr;
		array_view<int, 7, dynamic_range, 2> av2(av1);
		array_view<int, 7, 4, 2> av2(av1);
#endif

		array_view<DerivedClass> avd;
#ifdef CONFIRM_COMPILATION_ERRORS
		array_view<BaseClass> avb = avd;
#endif
		array_view<const DerivedClass> avcd = avd;
	}

	TEST(boundary_checks)
	{
		int arr[10][2];
		auto av = as_array_view(arr);

		fill(begin(av), end(av), 0);

		av[2][0] = 1;
		av[1][1] = 3;

		// out of bounds
		CHECK_THROW(av[1][3] = 3, fail_fast);
		CHECK_THROW((av[{1, 3}] = 3), fail_fast);

		CHECK_THROW(av[10][2], fail_fast);
		CHECK_THROW((av[{10,2}]), fail_fast);
	}

	void overloaded_func(array_view<const int, dynamic_range, 3, 5> exp, int expected_value) {
		for (auto val : exp)
		{
			CHECK(val == expected_value);
		}
	}

	void overloaded_func(array_view<const char, dynamic_range, 3, 5> exp, char expected_value) {
		for (auto val : exp)
		{
			CHECK(val == expected_value);
		}
	}

	void fixed_func(array_view<int, 3, 3, 5> exp, int expected_value) {
		for (auto val : exp)
		{
			CHECK(val == expected_value);
		}
	}

	TEST(array_view_parameter_test)
	{
		auto data = new int[4][3][5];

		auto av = as_array_view(data, 4);

		CHECK(av.size() == 60);

		fill(av.begin(), av.end(), 34);
					
		int count = 0;
		for_each(av.rbegin(), av.rend(), [&](int val) { count += val; });
		CHECK(count == 34 * 60);
		overloaded_func(av, 34);

		overloaded_func(av.as_array_view(dim<>(4), dim<>(3), dim<>(5)), 34);

		//fixed_func(av, 34);
		delete[] data;
	}


	TEST(md_access)
	{
		unsigned int width = 5, height = 20;

		unsigned int imgSize = width * height;
		auto image_ptr = new int[imgSize][3];

		// size check will be done
		auto image_view = as_array_view(image_ptr, imgSize).as_array_view(dim<>(height), dim<>(width), dim<3>());

		iota(image_view.begin(), image_view.end(), 1);

		int expected = 0;
		for (unsigned int i = 0; i < height; i++)
		{
			for (unsigned int j = 0; j < width; j++)
			{
				CHECK(expected + 1 == image_view[i][j][0]);
				CHECK(expected + 2 == image_view[i][j][1]);
				CHECK(expected + 3 == image_view[i][j][2]);
				
				auto val = image_view[{i, j, 0}];
				CHECK(expected + 1 == val);
				val = image_view[{i, j, 1}];
				CHECK(expected + 2 == val);
				val = image_view[{i, j, 2}];
				CHECK(expected + 3 == val);

				expected += 3;
			}
		}
	}

	TEST(array_view_factory_test)
	{
		{
			int * arr = new int[150];

			auto av = as_array_view(arr, dim<10>(), dim<>(3), dim<5>());

			fill(av.begin(), av.end(), 24);
			overloaded_func(av, 24);

			delete[] arr;


			array<int, 15> stdarr{ 0 };
			auto av2 = as_array_view(stdarr);
			overloaded_func(av2.as_array_view(dim<>(1), dim<3>(), dim<5>()), 0);


			string str = "ttttttttttttttt"; // size = 15
			auto t = str.data();
			auto av3 = as_array_view(str);
			overloaded_func(av3.as_array_view(dim<>(1), dim<3>(), dim<5>()), 't');
		}

		{
			int a[3][4][5];
			auto av = as_array_view(a);
			const int (*b)[4][5];
			b = a;
			auto bv = as_array_view(b, 3);

			CHECK(av == bv);

			const std::array<double, 3> arr = {0.0, 0.0, 0.0};
			auto cv = as_array_view(arr);

			vector<float> vec(3);
			auto dv = as_array_view(vec);
			
#ifdef CONFIRM_COMPILATION_ERRORS
			auto dv2 = as_array_view(std::move(vec));
#endif
		}
	}

	TEST (array_view_reshape_test)
	{
		int a[3][4][5];
		auto av = as_array_view(a);
		auto av2 = av.as_array_view(dim<60>());
		auto av3 = av2.as_array_view(dim<3>(), dim<4>(), dim<5>());
		auto av4 = av3.as_array_view(dim<4>(), dim<>(3), dim<5>());
		auto av5 = av4.as_array_view(dim<3>(), dim<4>(), dim<5>());
		auto av6 = av5.as_array_view(dim<12>(), dim<>(5));
		
		fill(av6.begin(), av6.end(), 1);
		
		auto av7 = av6.as_bytes();
		
		auto av8 = av7.as_array_view<int>();

		CHECK(av8.size() == av6.size());
		for (size_t i = 0; i < av8.size(); i++)
		{
			CHECK(av8[i] == 1);
		}

#ifdef CONFIRM_COMPILATION_ERRORS
		struct Foo {char c[11];};
		auto av9 = av7.as_array_view<Foo>();
#endif
	}


	TEST (array_view_section_test)
	{
		int a[30][4][5];
		
		auto av = as_array_view(a);
		auto sub = av.section({15, 0, 0}, Guide::index<3>{2, 2, 2});
		auto subsub = sub.section({1, 0, 0}, Guide::index<3>{1, 1, 1});
	}


	TEST(constructors)
	{
		array_view<int, dynamic_range> av(nullptr);
		CHECK(av.length() == 0);

		array_view<int, dynamic_range> av2;
		CHECK(av2.length() == 0);

		array_view<int, dynamic_range> av3(nullptr, 0);
		CHECK(av3.length() == 0);

		// Constructing from a nullptr + length is specifically disallowed
		auto f = [&]() {array_view<int, dynamic_range> av4(nullptr, 2);};
		CHECK_THROW(f(), fail_fast);

		int arr1[2][3];
		array_view<int, 2, dynamic_range> av5(arr1);

		array<int, 15> arr2;
		array_view<int, 15> av6(arr2);

		vector<int> vec1(19);
		array_view<int> av7(vec1);
		CHECK(av7.length() == 19);


		array_view<int> av8;
		CHECK(av8.length() == 0);
		array_view<int> av9(arr2);
		CHECK(av9.length() == 15);


#ifdef CONFIRM_COMPILATION_ERRORS
		array_view<int, 4> av10;
		DerivedClass *p = nullptr;
		array_view<BaseClass> av11(p, 0);
#endif


	}

	TEST(copyandassignment)
	{
		array_view<int, dynamic_range> av1;

		int arr[] = {3, 4, 5};
		av1 = arr;
		array_view<array_view_options<const int, unsigned char>, dynamic_range> av2;
		av2 = av1;
	}

	TEST(array_view_first)
	{
		int arr[5] = { 1, 2, 3, 4, 5 };

		{
			array_view<int, 5> av = arr;
			CHECK((av.first<2>().bounds() == static_bounds<size_t, 2>()));
			CHECK(av.first<2>().length() == 2);
			CHECK(av.first(2).length() == 2);
		}

		{
			array_view<int, 5> av = arr;
			CHECK((av.first<0>().bounds() == static_bounds<size_t, 0>()));
			CHECK(av.first<0>().length() == 0);
			CHECK(av.first(0).length() == 0);
		}

		{
			array_view<int, 5> av = arr;
			CHECK((av.first<5>().bounds() == static_bounds<size_t, 5>()));
			CHECK(av.first<5>().length() == 5);
			CHECK(av.first(5).length() == 5);
		}

		{
			array_view<int, 5> av = arr;
#ifdef CONFIRM_COMPILATION_ERRORS
			CHECK(av.first<6>().bounds() == static_bounds<size_t, 6>());
			CHECK(av.first<6>().length() == 6);
#endif
			CHECK_THROW(av.first(6).length(), fail_fast);
		}

		{
			array_view<int, dynamic_range> av;
			CHECK((av.first<0>().bounds() == static_bounds<size_t, 0>()));
			CHECK(av.first<0>().length() == 0);
			CHECK(av.first(0).length() == 0);
		}
	}

	TEST(array_view_last)
	{
		int arr[5] = { 1, 2, 3, 4, 5 };

		{
			array_view<int, 5> av = arr;
			CHECK((av.last<2>().bounds() == static_bounds<size_t, 2>()));
			CHECK(av.last<2>().length() == 2);
			CHECK(av.last(2).length() == 2);
		}

		{
			array_view<int, 5> av = arr;
			CHECK((av.last<0>().bounds() == static_bounds<size_t, 0>()));
			CHECK(av.last<0>().length() == 0);
			CHECK(av.last(0).length() == 0);
		}

		{
			array_view<int, 5> av = arr;
			CHECK((av.last<5>().bounds() == static_bounds<size_t, 5>()));
			CHECK(av.last<5>().length() == 5);
			CHECK(av.last(5).length() == 5);
		}

		
		{
			array_view<int, 5> av = arr;
#ifdef CONFIRM_COMPILATION_ERRORS
			CHECK((av.last<6>().bounds() == static_bounds<size_t, 6>()));
			CHECK(av.last<6>().length() == 6);
#endif
			CHECK_THROW(av.last(6).length(), fail_fast);
		}

		{
			array_view<int, dynamic_range> av;
			CHECK((av.last<0>().bounds() == static_bounds<size_t, 0>()));
			CHECK(av.last<0>().length() == 0);
			CHECK(av.last(0).length() == 0);
		}
	}

	TEST(custmized_array_view_size)
	{
		double (*arr)[3][4] = new double[100][3][4];
		array_view<array_view_options<double, char>, dynamic_range, 3, 4> av1(arr, (char)10);

		struct EffectiveStructure
		{
			double* v1;
			char v2;
		};
		CHECK(sizeof(av1) == sizeof(EffectiveStructure));

		CHECK_THROW(av1[10][3][4], fail_fast);

		array_view<const double, dynamic_range, 6, 4> av2 = av1.as_array_view(dim<>(5), dim<6>(), dim<4>());
		
	}

	TEST(array_view_sub)
	{
		int arr[5] = { 1, 2, 3, 4, 5 };

		{
			array_view<int, 5> av = arr;
			CHECK((av.sub<2,2>().bounds() == static_bounds<size_t, 2>()));
			CHECK((av.sub<2,2>().length() == 2));
			CHECK(av.sub(2,2).length() == 2);
		}


		{
			array_view<int, 5> av = arr;
			CHECK((av.sub<0,0>().bounds() == static_bounds<size_t, 0>()));
			CHECK((av.sub<0,0>().length() == 0));
			CHECK(av.sub(0,0).length() == 0);
		}

		{
			array_view<int, 5> av = arr;
			CHECK((av.sub<0,5>().bounds() == static_bounds<size_t, 5>()));
			CHECK((av.sub<0,5>().length() == 5));
			CHECK(av.sub(0,5).length() == 5);
		}

		{
			array_view<int, 5> av = arr;
#ifdef CONFIRM_COMPILATION_ERRORS
			CHECK((av.sub<5,0>().bounds() == static_bounds<size_t, 0>()));
			CHECK((av.sub<5,0>().length() == 0));
#endif
			CHECK_THROW(av.sub(5,0).length(), fail_fast);
		}

		{
			array_view<int, dynamic_range> av;
			CHECK((av.sub<0,0>().bounds() == static_bounds<size_t, 0>()));
			CHECK((av.sub<0,0>().length() == 0));
			CHECK(av.sub(0,0).length() == 0);
		}
	}

	void AssertNullEmptyProperties(array_view<int, dynamic_range>& av)
	{
		CHECK(av.length() == 0);
		CHECK(av.data() == nullptr);
		CHECK(!av);
	}

	template <class T, class U>
	void AssertContentsMatch(T a1, U a2)
	{
		CHECK(a1.length() == a2.length());
		for (size_t i = 0; i < a1.length(); ++i)
			CHECK(a1[i] == a2[i]);
	}

	TEST(TestNullConstruction)
	{
		array_view<int, dynamic_range> av;
		AssertNullEmptyProperties(av);

		array_view<int, dynamic_range> av2(nullptr);
		AssertNullEmptyProperties(av2);
	}

	TEST(ArrayConstruction)
	{
		int a[] = { 1, 2, 3, 4 };

		array_view<int, dynamic_range> av = { &a[1], 3 };
		CHECK(av.length() == 3);

		array_view<int, dynamic_range> av3 = { a, 2 };
		CHECK(av3.length() == 2);

		array_view<int, dynamic_range> av2 = a;
		CHECK(av2.length() == 4);
	}

	TEST(NonConstConstConversions)
	{
		int a[] = { 1, 2, 3, 4 };

#ifdef CONFIRM_COMPILATION_ERRORS
		array_view<const int, dynamic_range> cav = a;
		array_view<int, dynamic_range> av = cav;
#else
		array_view<int, dynamic_range> av = a;
		array_view<const int, dynamic_range> cav = av;             
#endif
		AssertContentsMatch(av, cav);
	}

	TEST(FixedSizeConversions)
	{            
		int arr[] = { 1, 2, 3, 4 };
	
		// converting to an array_view from an equal size array is ok
		array_view<int, 4> av4 = arr;
		CHECK(av4.length() == 4);

		// converting to dynamic_range a_v is always ok
		{
			array_view<int, dynamic_range> av = av4;
		}
		{
			array_view<int, dynamic_range> av = arr;
		}

		// initialization or assignment to static array_view that REDUCES size is NOT ok
#ifdef CONFIRM_COMPILATION_ERRORS
		{
			array_view<int, 2> av2 = arr;
		}
		{
			array_view<int, 2> av2 = av4;
		}
#endif

		{
			array_view<int, dynamic_range> av = arr;
			array_view<int, 2> av2 = av;
		}

#ifdef CONFIRM_COMPILATION_ERRORS
		{
			array_view<int, dynamic_range> av = arr;
			array_view<int, 2, 1> av2 = av.as_array_view(dim<2>(), dim<2>());
		}
#endif

		{
			array_view<int, dynamic_range> av = arr;
			auto f = [&]() {array_view<int, 2, 1> av2 = av.as_array_view(dim<>(2), dim<>(2));};
			CHECK_THROW(f(), fail_fast);
		}

		// but doing so explicitly is ok
			
		// you can convert statically
		{
			array_view<int, 2> av2 = {arr, 2};
		}
		{
			array_view<int, 1> av2 = av4.first<1>();
		}
			
		// ...or dynamically
		{
			// NB: implicit conversion to array_view<int,2> from array_view<int,dynamic_range>
			array_view<int, 1> av2 = av4.first(1); 
		}

		// initialization or assignment to static array_view that requires size INCREASE is not ok.
		int arr2[2]  = { 1, 2 };

#ifdef CONFIRM_COMPILATION_ERRORS
		{
			array_view<int, 4> av4 = arr2;
		}
		{
			array_view<int, 2> av2 = arr2;
			array_view<int, 4> av4 = av2;
		}
#endif
		{
			auto f = [&]() {array_view<int, 4> av4 = {arr2, 2};};
			CHECK_THROW(f(), fail_fast);
		}

		// this should fail - we are trying to assign a small dynamic a_v to a fixed_size larger one
		array_view<int, dynamic_range> av = arr2;
		auto f = [&](){ array_view<int, 4> av2 = av; };
		CHECK_THROW(f(), fail_fast);
	}

	TEST(AsWriteableBytes)        
	{
		int a[] = { 1, 2, 3, 4 };

		{
#ifdef CONFIRM_COMPILATION_ERRORS
			// you should not be able to get writeable bytes for const objects
			array_view<const int, dynamic_range> av = a;
			auto wav = av.as_writeable_bytes();
#endif
		}

		{
			array_view<int, dynamic_range> av;
			auto wav = av.as_writeable_bytes();
			CHECK(wav.length() == av.length());
			CHECK(wav.length() == 0);
			CHECK(wav.bytes() == 0);
		}

		{
			array_view<int, dynamic_range> av = a;
			auto wav = av.as_writeable_bytes();
			CHECK(wav.data() == (byte*)&a[0]);
			CHECK(wav.length() == sizeof(a));
		}

	}


	TEST(ArrayViewComparison)
	{
		int arr[10][2];
		auto av1 = as_array_view(arr);
		array_view<const int, dynamic_range, 2> av2 = av1;

		CHECK(av1 == av2);

		array_view<array_view_options<int, char>, 20> av3 = av1.as_array_view(dim<>(20));
		CHECK(av3 == av2 && av3 == av1);

	}
}

int main(int, const char *[])
{
	return UnitTest::RunAllTests();
}
