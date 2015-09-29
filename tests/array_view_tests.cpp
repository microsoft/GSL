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
using namespace gsl;

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
		auto sub = av.section({15, 0, 0}, gsl::index<3>{2, 2, 2});
		auto subsub = sub.section({1, 0, 0}, gsl::index<3>{1, 1, 1});
	}

	TEST(array_view_section)
	{
		std::vector<int> data(5 * 10);
		std::iota(begin(data), end(data), 0);
		const array_view<int, 5, 10> av = as_array_view(data).as_array_view(dim<5>(), dim<10>());

		strided_array_view<int, 2> av_section_1 = av.section({ 1, 2 }, { 3, 4 });
		CHECK((av_section_1[{0, 0}] == 12));
		CHECK((av_section_1[{0, 1}] == 13));
		CHECK((av_section_1[{1, 0}] == 22));
		CHECK((av_section_1[{2, 3}] == 35));

		strided_array_view<int, 2> av_section_2 = av_section_1.section({ 1, 2 }, { 2,2 });
		CHECK((av_section_2[{0, 0}] == 24));
		CHECK((av_section_2[{0, 1}] == 25));
		CHECK((av_section_2[{1, 0}] == 34));
	}
 
	TEST(strided_array_view_constructors)
	{
		// Check stride constructor
		{
			int arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
			const int carr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

			strided_array_view<int, 1> sav1{ arr, {{9}, {1}} }; // T -> T
			CHECK(sav1.bounds().index_bounds() == index<1>{ 9 });
			CHECK(sav1.bounds().stride() == 1);
			CHECK(sav1[0] == 1 && sav1[8] == 9);


			strided_array_view<const int, 1> sav2{ carr, {{ 4 }, { 2 }} }; // const T -> const T
			CHECK(sav2.bounds().index_bounds() == index<1>{ 4 });
			CHECK(sav2.bounds().strides() == index<1>{2});
			CHECK(sav2[0] == 1 && sav2[3] == 7);

			strided_array_view<int, 2> sav3{ arr, {{ 2, 2 },{ 6, 2 }} }; // T -> const T
			CHECK((sav3.bounds().index_bounds() == index<2>{ 2, 2 }));
			CHECK((sav3.bounds().strides() == index<2>{ 6, 2 }));
			CHECK((sav3[{0, 0}] == 1 && sav3[{0, 1}] == 3 && sav3[{1, 0}] == 7));
		}

		// Check array_view constructor
		{
			int arr[] = { 1, 2 };

			// From non-cv-qualified source
			{
				const array_view<int> src{ arr };

				strided_array_view<int, 1> sav{ src, {2, 1} };
				CHECK(sav.bounds().index_bounds() == index<1>{ 2 });
				CHECK(sav.bounds().strides() == index<1>{ 1 });
				CHECK(sav[1] == 2);

#if _MSC_VER > 1800
				strided_array_view<const int, 1> sav_c{ {src}, {2, 1} };
#else                
				strided_array_view<const int, 1> sav_c{ array_view<const int>{src}, strided_bounds<1>{2, 1} };
#endif                
				CHECK(sav_c.bounds().index_bounds() == index<1>{ 2 });
				CHECK(sav_c.bounds().strides() == index<1>{ 1 });
				CHECK(sav_c[1] == 2);

#if _MSC_VER > 1800
				strided_array_view<volatile int, 1> sav_v{ {src}, {2, 1} };
#else                
				strided_array_view<volatile int, 1> sav_v{ array_view<volatile int>{src}, strided_bounds<1>{2, 1} };
#endif                
				CHECK(sav_v.bounds().index_bounds() == index<1>{ 2 });
				CHECK(sav_v.bounds().strides() == index<1>{ 1 });
				CHECK(sav_v[1] == 2);

#if _MSC_VER > 1800
				strided_array_view<const volatile int, 1> sav_cv{ {src}, {2, 1} };
#else                
				strided_array_view<const volatile int, 1> sav_cv{ array_view<const volatile int>{src}, strided_bounds<1>{2, 1} };
#endif                
				CHECK(sav_cv.bounds().index_bounds() == index<1>{ 2 });
				CHECK(sav_cv.bounds().strides() == index<1>{ 1 });
				CHECK(sav_cv[1] == 2);
			}

			// From const-qualified source
			{
				const array_view<const int> src{ arr };

				strided_array_view<const int, 1> sav_c{ src, {2, 1} };
				CHECK(sav_c.bounds().index_bounds() == index<1>{ 2 });
				CHECK(sav_c.bounds().strides() == index<1>{ 1 });
				CHECK(sav_c[1] == 2);

#if _MSC_VER > 1800
				strided_array_view<const volatile int, 1> sav_cv{ {src}, {2, 1} };
#else
				strided_array_view<const volatile int, 1> sav_cv{ array_view<const volatile int>{src}, strided_bounds<1>{2, 1} };
#endif                
				
				CHECK(sav_cv.bounds().index_bounds() == index<1>{ 2 });
				CHECK(sav_cv.bounds().strides() == index<1>{ 1 });
				CHECK(sav_cv[1] == 2);
			}

			// From volatile-qualified source
			{
				const array_view<volatile int> src{ arr };

				strided_array_view<volatile int, 1> sav_v{ src, {2, 1} };
				CHECK(sav_v.bounds().index_bounds() == index<1>{ 2 });
				CHECK(sav_v.bounds().strides() == index<1>{ 1 });
				CHECK(sav_v[1] == 2);

#if _MSC_VER > 1800
				strided_array_view<const volatile int, 1> sav_cv{ {src}, {2, 1} };
#else
				strided_array_view<const volatile int, 1> sav_cv{ array_view<const volatile int>{src}, strided_bounds<1>{2, 1} };
#endif                
				CHECK(sav_cv.bounds().index_bounds() == index<1>{ 2 });
				CHECK(sav_cv.bounds().strides() == index<1>{ 1 });
				CHECK(sav_cv[1] == 2);
			}

			// From cv-qualified source
			{
				const array_view<const volatile int> src{ arr };

				strided_array_view<const volatile int, 1> sav_cv{ src, {2, 1} };
				CHECK(sav_cv.bounds().index_bounds() == index<1>{ 2 });
				CHECK(sav_cv.bounds().strides() == index<1>{ 1 });
				CHECK(sav_cv[1] == 2);
			}
		}

		// Check const-casting constructor
		{
			int arr[2] = { 4, 5 };

			const array_view<int, 2> av(arr, 2);
			array_view<const int, 2> av2{ av };
			CHECK(av2[1] == 5);

			static_assert(std::is_convertible<const array_view<int, 2>, array_view<const int, 2>>::value, "ctor is not implicit!");
		
			const strided_array_view<int, 1> src{ arr, {2, 1} };
			strided_array_view<const int, 1> sav{ src };
			CHECK(sav.bounds().index_bounds() == index<1>{ 2 });
			CHECK(sav.bounds().stride() == 1);
			CHECK(sav[1] == 5);
			
			static_assert(std::is_convertible<const strided_array_view<int, 1>, strided_array_view<const int, 1>>::value, "ctor is not implicit!");
		}

		// Check copy constructor
		{
			int arr1[2] = { 3, 4 };
			const strided_array_view<int, 1> src1{ arr1, {2, 1} };
			strided_array_view<int, 1> sav1{ src1 };
 
			CHECK(sav1.bounds().index_bounds() == index<1>{ 2 });
			CHECK(sav1.bounds().stride() == 1);
			CHECK(sav1[0] == 3);

			int arr2[6] = { 1, 2, 3, 4, 5, 6 };
			const strided_array_view<const int, 2> src2{ arr2, {{ 3, 2 }, { 2, 1 }} };
			strided_array_view<const int, 2> sav2{ src2 };
			CHECK((sav2.bounds().index_bounds() == index<2>{ 3, 2 }));
			CHECK((sav2.bounds().strides() == index<2>{ 2, 1 }));
			CHECK((sav2[{0, 0}] == 1 && sav2[{2, 0}] == 5));
		}

		// Check const-casting assignment operator
		{
			int arr1[2] = { 1, 2 };
			int arr2[6] = { 3, 4, 5, 6, 7, 8 };

			const strided_array_view<int, 1> src{ arr1, {{2}, {1}} };
			strided_array_view<const int, 1> sav{ arr2, {{3}, {2}} };
			strided_array_view<const int, 1>& sav_ref = (sav = src);
			CHECK(sav.bounds().index_bounds() == index<1>{ 2 });
			CHECK(sav.bounds().strides() == index<1>{ 1 });
			CHECK(sav[0] == 1);
			CHECK(&sav_ref == &sav);
		}
		
		// Check copy assignment operator
		{
			int arr1[2] = { 3, 4 };
			int arr1b[1] = { 0 };
			const strided_array_view<int, 1> src1{ arr1, {2, 1} };
			strided_array_view<int, 1> sav1{ arr1b, {1, 1} };
			strided_array_view<int, 1>& sav1_ref = (sav1 = src1);
			CHECK(sav1.bounds().index_bounds() == index<1>{ 2 });
			CHECK(sav1.bounds().strides() == index<1>{ 1 });
			CHECK(sav1[0] == 3);
			CHECK(&sav1_ref == &sav1);

			const int arr2[6] = { 1, 2, 3, 4, 5, 6 };
			const int arr2b[1] = { 0 };
			const strided_array_view<const int, 2> src2{ arr2, {{ 3, 2 },{ 2, 1 }} };
			strided_array_view<const int, 2> sav2{ arr2b, {{ 1, 1 },{ 1, 1 }} };
			strided_array_view<const int, 2>& sav2_ref = (sav2 = src2);
			CHECK((sav2.bounds().index_bounds() == index<2>{ 3, 2 }));
			CHECK((sav2.bounds().strides() == index<2>{ 2, 1 }));
			CHECK((sav2[{0, 0}] == 1 && sav2[{2, 0}] == 5));
			CHECK(&sav2_ref == &sav2);
		}
	}

	TEST(strided_array_view_slice)
	{
		std::vector<int> data(5 * 10);
		std::iota(begin(data), end(data), 0);
		const array_view<int, 5, 10> src = as_array_view(data).as_array_view(dim<5>(), dim<10>());

		const strided_array_view<int, 2> sav{ src, {{5, 10}, {10, 1}} };
#ifdef CONFIRM_COMPILATION_ERRORS
		const strided_array_view<const int, 2> csav{ {src},{ { 5, 10 },{ 10, 1 } } };
#endif
		const strided_array_view<const int, 2> csav{ array_view<const int, 5, 10>{ src }, { { 5, 10 },{ 10, 1 } } };

		strided_array_view<int, 1> sav_sl = sav[2];
		CHECK(sav_sl[0] == 20);
		CHECK(sav_sl[9] == 29);

		strided_array_view<const int, 1> csav_sl = sav[3];
		CHECK(csav_sl[0] == 30);
		CHECK(csav_sl[9] == 39);

		CHECK(sav[4][0] == 40);
		CHECK(sav[4][9] == 49);
	}

	TEST(strided_array_view_column_major)
	{
		// strided_array_view may be used to accomodate more peculiar
		// use cases, such as column-major multidimensional array
		// (aka. "FORTRAN" layout).

		int cm_array[3 * 5] = {
			1, 4, 7, 10, 13,
			2, 5, 8, 11, 14,
			3, 6, 9, 12, 15
		};
		strided_array_view<int, 2> cm_sav{ cm_array, {{ 5, 3 },{ 1, 5 }} };

		// Accessing elements
		CHECK((cm_sav[{0, 0}] == 1));
		CHECK((cm_sav[{0, 1}] == 2));
		CHECK((cm_sav[{1, 0}] == 4));
		CHECK((cm_sav[{4, 2}] == 15));

		// Slice
		strided_array_view<int, 1> cm_sl = cm_sav[3];

		CHECK(cm_sl[0] == 10);
		CHECK(cm_sl[1] == 11);
		CHECK(cm_sl[2] == 12);

		// Section 
		strided_array_view<int, 2> cm_sec = cm_sav.section( { 2, 1 }, { 3, 2 });

		CHECK((cm_sec.bounds().index_bounds() == index<2>{3, 2}));
		CHECK((cm_sec[{0, 0}] == 8));
		CHECK((cm_sec[{0, 1}] == 9));
		CHECK((cm_sec[{1, 0}] == 11));
		CHECK((cm_sec[{2, 1}] == 15));
	}

	TEST(strided_array_view_bounds)
	{
		int arr[] = { 0, 1, 2, 3 };
		array_view<int> av(arr);

		{
			// incorrect sections
			
			CHECK_THROW(av.section(0, 0)[0], fail_fast);
			CHECK_THROW(av.section(1, 0)[0], fail_fast);
			CHECK_THROW(av.section(1, 1)[1], fail_fast);
			
			CHECK_THROW(av.section(2, 5), fail_fast);
			CHECK_THROW(av.section(5, 2), fail_fast);
			CHECK_THROW(av.section(5, 0), fail_fast);
			CHECK_THROW(av.section(0, 5), fail_fast);
			CHECK_THROW(av.section(5, 5), fail_fast);
		}

		{
			// zero stride
			strided_array_view<int, 1> sav{ av, {{4}, {}} };
			CHECK(sav[0] == 0);
			CHECK(sav[3] == 0);
			CHECK_THROW(sav[4], fail_fast);
		}

		{
			// zero extent
			strided_array_view<int, 1> sav{ av,{ {},{1} } };
			CHECK_THROW(sav[0], fail_fast);
		}

		{
			// zero extent and stride
			strided_array_view<int, 1> sav{ av,{ {},{} } };
			CHECK_THROW(sav[0], fail_fast);
		}

		{
			// strided array ctor with matching strided bounds 
			strided_array_view<int, 1> sav{ arr,{ 4, 1 } };
			CHECK(sav.bounds().index_bounds() == index<1>{ 4 });
			CHECK(sav[3] == 3);
			CHECK_THROW(sav[4], fail_fast);
		}

		{
			// strided array ctor with smaller strided bounds 
			strided_array_view<int, 1> sav{ arr,{ 2, 1 } };
			CHECK(sav.bounds().index_bounds() == index<1>{ 2 });
			CHECK(sav[1] == 1);
			CHECK_THROW(sav[2], fail_fast);
		}

		{
			// strided array ctor with fitting irregular bounds 
			strided_array_view<int, 1> sav{ arr,{ 2, 3 } };
			CHECK(sav.bounds().index_bounds() == index<1>{ 2 });
			CHECK(sav[0] == 0);
			CHECK(sav[1] == 3);
			CHECK_THROW(sav[2], fail_fast);
		}

		{
			// bounds cross data boundaries - from static arrays
			CHECK_THROW((strided_array_view<int, 1> { arr, { 3, 2 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { arr, { 3, 3 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { arr, { 4, 5 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { arr, { 5, 1 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { arr, { 5, 5 } }), fail_fast);
		}

		{
			// bounds cross data boundaries - from array view
			CHECK_THROW((strided_array_view<int, 1> { av, { 3, 2 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { av, { 3, 3 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { av, { 4, 5 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { av, { 5, 1 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { av, { 5, 5 } }), fail_fast);
		}

		{
			// bounds cross data boundaries - from dynamic arrays
			CHECK_THROW((strided_array_view<int, 1> { av.data(), 4, { 3, 2 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { av.data(), 4, { 3, 3 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { av.data(), 4, { 4, 5 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { av.data(), 4, { 5, 1 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { av.data(), 4, { 5, 5 } }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1> { av.data(), 2, { 2, 2 } }), fail_fast);
		}

#ifdef CONFIRM_COMPILATION_ERRORS
		{
			strided_array_view<int, 1> sav0{ av.data(), { 3, 2 } };
			strided_array_view<int, 1> sav1{ arr, { 1 } };
			strided_array_view<int, 1> sav2{ arr, { 1,1,1 } };
			strided_array_view<int, 1> sav3{ av, { 1 } };
			strided_array_view<int, 1> sav4{ av, { 1,1,1 } };
			strided_array_view<int, 2> sav5{ av.as_array_view(dim<2>(), dim<2>()), { 1 } };
			strided_array_view<int, 2> sav6{ av.as_array_view(dim<2>(), dim<2>()), { 1,1,1 } };
			strided_array_view<int, 2> sav7{ av.as_array_view(dim<2>(), dim<2>()), { { 1,1 },{ 1,1 },{ 1,1 } } };
		}
#endif
		
		{
			// stride initializer list size should match the rank of the array
			CHECK_THROW((index<1>{ 0,1 }), fail_fast);
			CHECK_THROW((strided_array_view<int, 1>{ arr, {1, {1,1}} }), fail_fast);
#ifdef _MSC_VER
			CHECK_THROW((strided_array_view<int, 1>{ arr, {{1,1 }, {1,1}} }), fail_fast);
#endif
			CHECK_THROW((strided_array_view<int, 1>{ av, {1, {1,1}} }), fail_fast);
#ifdef _MSC_VER
			CHECK_THROW((strided_array_view<int, 1>{ av, {{1,1 }, {1,1}} }), fail_fast);
#endif			
			CHECK_THROW((strided_array_view<int, 2>{ av.as_array_view(dim<2>(), dim<2>()), {{1}, {1}} }), fail_fast);
			CHECK_THROW((strided_array_view<int, 2>{ av.as_array_view(dim<2>(), dim<2>()), {{1}, {1,1,1}} }), fail_fast);
#ifdef _MSC_VER
			CHECK_THROW((strided_array_view<int, 2>{ av.as_array_view(dim<2>(), dim<2>()), {{1,1,1}, {1}} }), fail_fast);
#endif
		}

	}

	TEST(strided_array_view_type_conversion)
	{
		int arr[] = { 0, 1, 2, 3 };
		array_view<int> av(arr);

		{
			strided_array_view<int, 1> sav{ av.data(), av.size(), { av.size() / 2, 2 } };
#ifdef CONFIRM_COMPILATION_ERRORS
			strided_array_view<long, 1> lsav1 = sav.as_strided_array_view<long, 1>();
#endif
		}
		{
			strided_array_view<int, 1> sav{ av, { av.size() / 2, 2 } };
#ifdef CONFIRM_COMPILATION_ERRORS
			strided_array_view<long, 1> lsav1 = sav.as_strided_array_view<long, 1>();
#endif
		}

		array_view<const byte, dynamic_range> bytes = av.as_bytes();

		// retype strided array with regular strides - from raw data
		{
			strided_bounds<2> bounds{ { 2, bytes.size() / 4 }, { bytes.size() / 2, 1 } };
			strided_array_view<const byte, 2> sav2{ bytes.data(), bytes.size(), bounds };
			strided_array_view<const int, 2> sav3 = sav2.as_strided_array_view<const int>();
			CHECK(sav3[0][0] == 0);
			CHECK(sav3[1][0] == 2);
			CHECK_THROW(sav3[1][1], fail_fast);
			CHECK_THROW(sav3[0][1], fail_fast);
		}

		// retype strided array with regular strides - from array_view
		{
			strided_bounds<2> bounds{ { 2, bytes.size() / 4 }, { bytes.size() / 2, 1 } };
			array_view<const byte, 2, dynamic_range> bytes2 = bytes.as_array_view(dim<2>(), dim<>(bytes.size() / 2));
			strided_array_view<const byte, 2> sav2{ bytes2, bounds };
			strided_array_view<int, 2> sav3 = sav2.as_strided_array_view<int>();
			CHECK(sav3[0][0] == 0);
			CHECK(sav3[1][0] == 2);
			CHECK_THROW(sav3[1][1], fail_fast);
			CHECK_THROW(sav3[0][1], fail_fast);
		}

		// retype strided array with not enough elements - last dimension of the array is too small
		{
			strided_bounds<2> bounds{ { 4,2 },{ 4, 1 } };
			array_view<const byte, 2, dynamic_range> bytes2 = bytes.as_array_view(dim<2>(), dim<>(bytes.size() / 2));
			strided_array_view<const byte, 2> sav2{ bytes2, bounds };
			CHECK_THROW(sav2.as_strided_array_view<int>(), fail_fast);
		}

		// retype strided array with not enough elements - strides are too small
		{
			strided_bounds<2> bounds{ { 4,2 },{ 2, 1 } };
			array_view<const byte, 2, dynamic_range> bytes2 = bytes.as_array_view(dim<2>(), dim<>(bytes.size() / 2));
			strided_array_view<const byte, 2> sav2{ bytes2, bounds };
			CHECK_THROW(sav2.as_strided_array_view<int>(), fail_fast);
		}

		// retype strided array with not enough elements - last dimension does not divide by the new typesize
		{
			strided_bounds<2> bounds{ { 2,6 },{ 4, 1 } };
			array_view<const byte, 2, dynamic_range> bytes2 = bytes.as_array_view(dim<2>(), dim<>(bytes.size() / 2));
			strided_array_view<const byte, 2> sav2{ bytes2, bounds };
			CHECK_THROW(sav2.as_strided_array_view<int>(), fail_fast);
		}

		// retype strided array with not enough elements - strides does not divide by the new typesize
		{
			strided_bounds<2> bounds{ { 2, 1 },{ 6, 1 } };
			array_view<const byte, 2, dynamic_range> bytes2 = bytes.as_array_view(dim<2>(), dim<>(bytes.size() / 2));
			strided_array_view<const byte, 2> sav2{ bytes2, bounds };
			CHECK_THROW(sav2.as_strided_array_view<int>(), fail_fast);
		}

		// retype strided array with irregular strides - from raw data
		{
			strided_bounds<1> bounds{ bytes.size() / 2, 2 };
			strided_array_view<const byte, 1> sav2{ bytes.data(), bytes.size(), bounds };
			CHECK_THROW(sav2.as_strided_array_view<int>(), fail_fast);
		}

		// retype strided array with irregular strides - from array_view
		{
			strided_bounds<1> bounds{ bytes.size() / 2, 2 };
			strided_array_view<const byte, 1> sav2{ bytes, bounds };
			CHECK_THROW(sav2.as_strided_array_view<int>(), fail_fast);
		}
	}

	TEST(empty_arrays)
	{
#ifdef CONFIRM_COMPILATION_ERRORS
		{
			array_view<int, 1> empty;
			strided_array_view<int, 2> empty2;
			strided_array_view<int, 1> empty3{ nullptr,{ 0, 1 } };
		}
#endif

		{
			array_view<int, 0> empty_av(nullptr);

			CHECK(empty_av.bounds().index_bounds() == index<1>{ 0 });
			CHECK_THROW(empty_av[0], fail_fast);
			CHECK_THROW(empty_av.begin()[0], fail_fast);
			CHECK_THROW(empty_av.cbegin()[0], fail_fast);		
			for (auto& v : empty_av)
			{
				CHECK(false);
			}
		}

		{
			array_view<int> empty_av = {};
			CHECK(empty_av.bounds().index_bounds() == index<1>{ 0 });
			CHECK_THROW(empty_av[0], fail_fast);
			CHECK_THROW(empty_av.begin()[0], fail_fast);
			CHECK_THROW(empty_av.cbegin()[0], fail_fast); 
			for (auto& v : empty_av)
			{
				CHECK(false);
			}
		}

		{
			array_view<int, 0> empty_av(nullptr);
			strided_array_view<int, 1> empty_sav{ empty_av, { 0, 1 } };

			CHECK(empty_sav.bounds().index_bounds() == index<1>{ 0 });
			CHECK_THROW(empty_sav[0], fail_fast);
			CHECK_THROW(empty_sav.begin()[0], fail_fast);
			CHECK_THROW(empty_sav.cbegin()[0], fail_fast);

			for (auto& v : empty_sav)
			{
				CHECK(false);
			}
		}

		{
			strided_array_view<int, 1> empty_sav{ nullptr, 0, { 0, 1 } };

			CHECK(empty_sav.bounds().index_bounds() == index<1>{ 0 });
			CHECK_THROW(empty_sav[0], fail_fast);
			CHECK_THROW(empty_sav.begin()[0], fail_fast);
			CHECK_THROW(empty_sav.cbegin()[0], fail_fast);

			for (auto& v : empty_sav)
			{
				CHECK(false);
			}
		}
	}

	TEST(index_constructor)
	{
		auto arr = new int[8];
		for (int i = 0; i < 4; ++i)
		{
			arr[2 * i] = 4 + i;
			arr[2 * i + 1] = i;
		}

		array_view<int, dynamic_range> av(arr, 8);

		size_t a[1] = { 0 };
		index<1> i = a;

		CHECK(av[i] == 4);

		auto av2 = av.as_array_view(dim<4>(), dim<>(2));
		size_t a2[2] = { 0, 1 };
		index<2> i2 = a2;

		CHECK(av2[i2] == 0);
		CHECK(av2[0][i] == 4);

		delete[] arr;
	}

	TEST(index_operations)
	{
		size_t a[3] = { 0, 1, 2 };
		size_t b[3] = { 3, 4, 5 };
		index<3> i = a;
		index<3> j = b;

		CHECK(i[0] == 0);
		CHECK(i[1] == 1);
		CHECK(i[2] == 2);

		{
			index<3> k = i + j;

			CHECK(i[0] == 0);
			CHECK(i[1] == 1);
			CHECK(i[2] == 2);
			CHECK(k[0] == 3);
			CHECK(k[1] == 5);
			CHECK(k[2] == 7);
		}

		{
			index<3> k = i * 3;

			CHECK(i[0] == 0);
			CHECK(i[1] == 1);
			CHECK(i[2] == 2);
			CHECK(k[0] == 0);
			CHECK(k[1] == 3);
			CHECK(k[2] == 6);
		}

		{
			index<2> k = index<2>::shift_left(i);

			CHECK(i[0] == 0);
			CHECK(i[1] == 1);
			CHECK(i[2] == 2);
			CHECK(k[0] == 1);
			CHECK(k[1] == 2);
		}

	}

	void iterate_second_column(array_view<int, dynamic_range, dynamic_range> av)
	{
		auto length = av.size() / 2;

		// view to the second column 
		auto section = av.section({ 0,1 }, { length,1 });

		CHECK(section.size() == length);
		for (unsigned int i = 0; i < section.size(); ++i)
		{
			CHECK(section[i][0] == av[i][1]);
		}

		for (unsigned int i = 0; i < section.size(); ++i)
		{
			auto idx = index<2>{ i,0 }; // avoid braces inside the CHECK macro
			CHECK(section[idx] == av[i][1]);
		}

		CHECK(section.bounds().index_bounds()[0] == length);
		CHECK(section.bounds().index_bounds()[1] == 1);
		for (unsigned int i = 0; i < section.bounds().index_bounds()[0]; ++i)
		{
			for (unsigned int j = 0; j < section.bounds().index_bounds()[1]; ++j)
			{
				auto idx = index<2>{ i,j }; // avoid braces inside the CHECK macro
				CHECK(section[idx] == av[i][1]);
			}
		}

		size_t idx = 0;
		for (auto num : section)
		{
			CHECK(num == av[idx][1]);
			idx++;
		}
	}

	TEST(array_view_section_iteration)
	{
		int arr[4][2] = { { 4,0 },{ 5,1 },{ 6,2 },{ 7,3 } };

		// static bounds
		{
			array_view<int, 4, 2> av = arr;
			iterate_second_column(av);
		}
		// first bound is dynamic
		{
			array_view<int, dynamic_range, 2> av  = arr;
			iterate_second_column(av);
		}
		// second bound is dynamic
		{
			array_view<int, 4, dynamic_range> av = arr;
			iterate_second_column(av);
		}
		// both bounds are dynamic
		{
			array_view<int, dynamic_range, dynamic_range> av(arr, 4);
			iterate_second_column(av);
		}
	}

	TEST(dynamic_array_view_section_iteration)
	{
		unsigned int height = 4, width = 2;
		unsigned int size = height * width;

		auto arr = new int[size];
		for (int unsigned i = 0; i < size; ++i)
		{
			arr[i] = i;
		}

		auto av = as_array_view(arr, size);

		// first bound is dynamic
		{
			array_view<int, dynamic_range, 2> av2 = av.as_array_view(dim<>(height), dim<>(width));
			iterate_second_column(av2);
		}
		// second bound is dynamic
		{
			array_view<int, 4, dynamic_range> av2 = av.as_array_view(dim<>(height), dim<>(width));
			iterate_second_column(av2);
		}
		// both bounds are dynamic
		{
			array_view<int, dynamic_range, dynamic_range> av2 = av.as_array_view(dim<>(height), dim<>(width));
			iterate_second_column(av2);
		}

		delete[] arr;
	}

	void iterate_every_other_element(array_view<int, dynamic_range> av)
	{
		// pick every other element

		auto length = av.size() / 2;
#if _MSC_VER > 1800
		auto bounds = strided_bounds<1>({ length }, { 2 });
#else
		auto bounds = strided_bounds<1>(index<1>{ length }, index<1>{ 2 });
#endif        
		strided_array_view<int, 1> strided(&av.data()[1], av.size() - 1, bounds);

		CHECK(strided.size() == length);
		CHECK(strided.bounds().index_bounds()[0] == length);
		for (unsigned int i = 0; i < strided.size(); ++i)
		{
			CHECK(strided[i] == av[2 * i + 1]);
		}

		int idx = 0;
		for (auto num : strided)
		{
			CHECK(num == av[2 * idx + 1]);
			idx++;
		}
	}

	TEST(strided_array_view_section_iteration)
	{
		int arr[8] = {4,0,5,1,6,2,7,3};

		// static bounds
		{
			array_view<int, 8> av(arr, 8);
			iterate_every_other_element(av);
		}

		// dynamic bounds
		{
			array_view<int, dynamic_range> av(arr, 8);
			iterate_every_other_element(av);
		}
	}

	TEST(dynamic_strided_array_view_section_iteration)
	{
		auto arr = new int[8];
		for (int i = 0; i < 4; ++i)
		{
			arr[2 * i] = 4 + i;
			arr[2 * i + 1] = i;
		}
		
		auto av = as_array_view(arr, 8);
		iterate_every_other_element(av);

		delete[] arr;
	}

	void iterate_second_slice(array_view<int, dynamic_range, dynamic_range, dynamic_range> av)
	{
		int expected[6] = { 2,3,10,11,18,19 };
		auto section = av.section({ 0,1,0 }, { 3,1,2 });

		for (unsigned int i = 0; i < section.extent<0>(); ++i)
		{
			for (unsigned int j = 0; j < section.extent<1>(); ++j)
				for (unsigned int k = 0; k < section.extent<2>(); ++k)
				{
					auto idx = index<3>{ i,j,k }; // avoid braces in the CHECK macro
					CHECK(section[idx] == expected[2 * i + 2 * j + k]);                   
				}
		}

		for (unsigned int i = 0; i < section.extent<0>(); ++i)
		{
			for (unsigned int j = 0; j < section.extent<1>(); ++j)
				for (unsigned int k = 0; k < section.extent<2>(); ++k)
					CHECK(section[i][j][k] == expected[2 * i + 2 * j + k]);
		}

		int i = 0;
		for (auto num : section)
		{
			CHECK(num == expected[i]);
			i++;
		}
	}

	TEST(strided_array_view_section_iteration_3d)
	{
		int arr[3][4][2];
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 4; ++j)
				for (unsigned int k = 0; k < 2; ++k)
					arr[i][j][k] = 8 * i + 2 * j + k;
		}

		{
			array_view<int, 3, 4, 2> av = arr;
			iterate_second_slice(av);
		}
	}

	TEST(dynamic_strided_array_view_section_iteration_3d)
	{
		unsigned int height = 12, width = 2;
		unsigned int size = height * width;

		auto arr = new int[size];
		for (int unsigned i = 0; i < size; ++i)
		{
			arr[i] = i;
		}

		{
			auto av = as_array_view(arr, 24).as_array_view(dim<3>(),dim<4>(),dim<2>());
			iterate_second_slice(av);
		}

		{
			auto av = as_array_view(arr, 24).as_array_view(dim<>(3), dim<4>(), dim<2>());
			iterate_second_slice(av);
		}

		{
			auto av = as_array_view(arr, 24).as_array_view(dim<3>(), dim<>(4), dim<2>());
			iterate_second_slice(av);
		}

		{
			auto av = as_array_view(arr, 24).as_array_view(dim<3>(), dim<4>(), dim<>(2));
			iterate_second_slice(av);
		}
		delete[] arr;
	}

	TEST(strided_array_view_conversion)
	{
		// get an array_view of 'c' values from the list of X's

		struct X { int a; int b; int c; };

		X arr[4] = { { 0,1,2 },{ 3,4,5 },{ 6,7,8 },{ 9,10,11 } };

		auto s = sizeof(int) / sizeof(byte);
		auto d2 = 3 * s;
		auto d1 = sizeof(int) * 12 / d2;

		// convert to 4x12 array of bytes
		auto av = as_array_view(arr, 4).as_bytes().as_array_view(dim<>(d1), dim<>(d2));

		CHECK(av.bounds().index_bounds()[0] == 4);
		CHECK(av.bounds().index_bounds()[1] == 12);

		// get the last 4 columns
		auto section = av.section({ 0, 2 * s }, { 4, s }); // { { arr[0].c[0], arr[0].c[1], arr[0].c[2], arr[0].c[3] } , { arr[1].c[0], ... } , ... }

		// convert to array 4x1 array of integers
		auto cs = section.as_strided_array_view<int>(); // { { arr[0].c }, {arr[1].c } , ... } 

		CHECK(cs.bounds().index_bounds()[0] == 4);
		CHECK(cs.bounds().index_bounds()[1] == 1);

		// transpose to 1x4 array 
		strided_bounds<2> reverse_bounds{ 
			{ cs.bounds().index_bounds()[1] , cs.bounds().index_bounds()[0] },
			{ cs.bounds().strides()[1], cs.bounds().strides()[0] }
		};

		strided_array_view<int, 2> transposed{ cs.data(), cs.bounds().total_size(), reverse_bounds };
		
		// slice to get a one-dimensional array of c's
		strided_array_view<int, 1> result = transposed[0];

		CHECK(result.bounds().index_bounds()[0] == 4);
		CHECK_THROW(result.bounds().index_bounds()[1], fail_fast);

		int i = 0;
		for (auto& num : result)
		{
			CHECK(num == arr[i].c);
			i++;
		}

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
            CHECK(av.sub(2,3).length() == 3);
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
            CHECK_THROW(av.sub(0,6).length(), fail_fast);
            CHECK_THROW(av.sub(1,5).length(), fail_fast);
		}

		{
			array_view<int, 5> av = arr;
			CHECK((av.sub<5,0>().bounds() == static_bounds<size_t, 0>()));
            CHECK((av.sub<5, 0>().length() == 0));
            CHECK(av.sub(5,0).length() == 0);
            CHECK_THROW(av.sub(6,0).length(), fail_fast);
		}

		{
			array_view<int, dynamic_range> av;
			CHECK((av.sub<0,0>().bounds() == static_bounds<size_t, 0>()));
			CHECK((av.sub<0,0>().length() == 0));
			CHECK(av.sub(0,0).length() == 0);
            CHECK_THROW((av.sub<1,0>().length()), fail_fast);
		}

        {
            array_view<int> av;
            CHECK(av.sub(0).length() == 0);
            CHECK_THROW(av.sub(1).length(), fail_fast);
        }

        {
            array_view<int> av = arr;
            CHECK(av.sub(0).length() == 5);
            CHECK(av.sub(1).length() == 4);
            CHECK(av.sub(4).length() == 1);
            CHECK(av.sub(5).length() == 0);
            CHECK_THROW(av.sub(6).length(), fail_fast);
            auto av2 = av.sub(1);
            for (int i = 0; i < 4; ++i)
                CHECK(av2[i] == i+2);
        }
        
        {
            array_view<int,5> av = arr;
            CHECK(av.sub(0).length() == 5);
            CHECK(av.sub(1).length() == 4);
            CHECK(av.sub(4).length() == 1);
            CHECK(av.sub(5).length() == 0);
            CHECK_THROW(av.sub(6).length(), fail_fast);
            auto av2 = av.sub(1);
            for (int i = 0; i < 4; ++i)
                CHECK(av2[i] == i+2);
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
		{
			int arr[10][2];
			auto av1 = as_array_view(arr);
			array_view<const int, dynamic_range, 2> av2 = av1;

			CHECK(av1 == av2);

			array_view<array_view_options<int, char>, 20> av3 = av1.as_array_view(dim<>(20));
			CHECK(av3 == av2 && av3 == av1);
		}

		{
			auto av1 = nullptr;
			auto av2 = nullptr;
			CHECK(av1 == av2);
			CHECK(!(av1 != av2));
			CHECK(!(av1 < av2));
			CHECK(av1 <= av2);
			CHECK(!(av1 > av2));
			CHECK(av1 >= av2);
			CHECK(av2 == av1);
			CHECK(!(av2 != av1));
			CHECK(!(av2 < av1));
			CHECK(av2 <= av1);
			CHECK(!(av2 > av1));
			CHECK(av2 >= av1);
		}

		{
			int arr[] = { 2, 1 }; // bigger

			array_view<int> av1 = nullptr;
			array_view<int> av2 = arr;

			CHECK(av1 != av2);
			CHECK(av2 != av1);
			CHECK(!(av1 == av2));
			CHECK(!(av2 == av1));
			CHECK(av1 < av2);
			CHECK(!(av2 < av1));
			CHECK(av1 <= av2);
			CHECK(!(av2 <= av1));
			CHECK(av2 > av1);
			CHECK(!(av1 > av2));
			CHECK(av2 >= av1);
			CHECK(!(av1 >= av2));
		}

		{
			int arr1[] = { 1, 2 };
			int arr2[] = { 1, 2 };
			array_view<int> av1 = arr1;
			array_view<int> av2 = arr2;

			CHECK(av1 == av2);
			CHECK(!(av1 != av2));
			CHECK(!(av1 < av2));
			CHECK(av1 <= av2);
			CHECK(!(av1 > av2));
			CHECK(av1 >= av2);
			CHECK(av2 == av1);
			CHECK(!(av2 != av1));
			CHECK(!(av2 < av1));
			CHECK(av2 <= av1);
			CHECK(!(av2 > av1));
			CHECK(av2 >= av1);
		}

		{
			int arr[] = { 1, 2, 3 };

			array_view<int> av1 = { &arr[0], 2 }; // shorter
			array_view<int> av2 = arr;            // longer

			CHECK(av1 != av2);
			CHECK(av2 != av1);
			CHECK(!(av1 == av2));
			CHECK(!(av2 == av1));
			CHECK(av1 < av2);
			CHECK(!(av2 < av1));
			CHECK(av1 <= av2);
			CHECK(!(av2 <= av1));
			CHECK(av2 > av1);
			CHECK(!(av1 > av2));
			CHECK(av2 >= av1);
			CHECK(!(av1 >= av2));
		}

		{
			int arr1[] = { 1, 2 }; // smaller
			int arr2[] = { 2, 1 }; // bigger

			array_view<int> av1 = arr1;
			array_view<int> av2 = arr2;

			CHECK(av1 != av2);
			CHECK(av2 != av1);
			CHECK(!(av1 == av2));
			CHECK(!(av2 == av1));
			CHECK(av1 < av2);
			CHECK(!(av2 < av1));
			CHECK(av1 <= av2);
			CHECK(!(av2 <= av1));
			CHECK(av2 > av1);
			CHECK(!(av1 > av2));
			CHECK(av2 >= av1);
			CHECK(!(av1 >= av2));
		}
	}
}

int main(int, const char *[])
{
	return UnitTest::RunAllTests();
}
