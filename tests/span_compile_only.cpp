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

#include <gsl/span>
#include <gsl/string_span>
#include <gsl/gsl_algorithm>
using namespace std;
using namespace gsl;

// issue #1 - mulitple calls to std::terminate 
// was showing in all tests below, now resolved

// issue #2
// the tests below have following problems:
// - need to track references that are changed by the compiler from values
//      - this should hoist loads and stores of span data ptr out of loops
// - need to simplify conditions and perform SROA before running vectorizer
//      - shows when using iterators
// - need to inlnine functions that return objects in /EHsc mode
//      - begin() and end() do not inline
// - need to prove unnecessary range checks away
//      - there should be no need to call std::terminate in all them, except for the first
// 
// those tests should vectorize in the end

// range check should always show up here
void test_span_unoptimizable_rangecheck(span<int> s, int len)
{
    for (ptrdiff_t i = 0; i < len; i++)
    {
        s[i] += 1;
    }
}

// should vectorize
// vectorizes after passing a trivially copyable bit
// gcc, clang vectorizes 
void test_span_for(span<int> s) {
    for (ptrdiff_t i = 0; i < s.size(); i++)
    {
        s[i] += 1;
    }
}

// should vectorize
// gcc, clang vectorizes
void test_span_iter(span<int> s) {
    for (auto it = s.begin(); it != s.end(); ++it)
    {
        (*it) += 1;
    }
}

// should vectorize
// gcc, clang vectorizes
void test_span_rangeiter(span<int> s) {
    for (auto& e : s)
    {
        e += 1;
    }
}


// issue #3 - postponing until others are done, can be just a side effect of the rest

// issue #4
typedef unsigned  char UINT8;
typedef unsigned  short UINT16;
struct IDE_DRIVE_SAVED_STATE
{
    UINT8 Lba48Bit[6];
};

struct IDE_DRIVE_STATE
{
    IDE_DRIVE_SAVED_STATE Saved{};

    gsl::span<UINT16> CommandBuffer{};
};


void static_size_array_span(IDE_DRIVE_STATE& Drive)
{
    span<UINT8> sp{ Drive.Saved.Lba48Bit }; // optimize null check away

    for (int i = 0; i< 6; i++)
    {
        sp[i] = 1;
    }
}

// issue #5 - can we cut down the number of branches there?
// 4 branches in test_convert_span_Joe
//
// Theoreticaly, for converting to byte-size type, we should be 
// able to prove the size of the new buffer being always good.
//
// Practically, the constructor call below cannot assume that 
// data and size are from a valid span, so we can do it only 
// inside span's implementation - for example, having a constructor 
// from byte span, but that has problems as well:
//
// How to to convert span<T> to span<byte> in a way that is 
// portable and standards compliant?
// - needs aligment checking
// - how do you convert from byte to T? 
//      option 1: placement new - gets compilecated for non-pods
//      option 2: reinterpret_cast - standard forbids accessing 
//                the data  =after it
//
// doing those conversions is possible, but doing them 
// efficiently requires a non-portable implementation

template <class NewClass, class CurrentSpan>
gsl::span<NewClass> convert_span(CurrentSpan s)
{
    Expects((s.size_bytes() % sizeof(NewClass)) == 0);

    NewClass* ptr = reinterpret_cast<NewClass*>(s.data());
    return { ptr, s.size_bytes() / static_cast<int>(sizeof(NewClass)) };
}

template <UINT8, class CurrentSpan>
gsl::span<UINT8> convert_span(CurrentSpan s)
{
    UINT8* ptr = reinterpret_cast<UINT8*>(s.data());
    return { ptr, s.size_bytes() };
}


UINT8 test_convert_span_Joe(IDE_DRIVE_STATE& Drive)
{
    span<UINT8> curBuffer = convert_span<UINT8, gsl::span<UINT16>>(Drive.CommandBuffer);
 
    return curBuffer[0];
}


// issue #6
// 0 branches in all compilers
int mysubspan1(std::ptrdiff_t i)
{
    int x[] = { 0,1,2,3,4,5 };
    span<int, 6> s = { x };

    auto subspan = s.last(3);

    return subspan.size();
}

// 2 branches in msvc, 2 clang, ,2 gcc
int mysubspan2(std::ptrdiff_t i)
{
    int x[] = { 0,1,2,3,4,5 };
    span<int, 6> s = { x };

    auto subspan = s.last(i);

    return subspan.size();
}

// 0
int mysubspan3(std::ptrdiff_t i)
{
    int x[] = { 0,1,2,3,4,5 };
    span<int, 6> s = { x };

    auto subspan = s.subspan(3);

    return subspan.size();
}

// 4,4,4
span<int> mysubspan4(int* p, std::ptrdiff_t size, std::ptrdiff_t i)
{
    if (p != nullptr)
    {
        span<int> s = { p, size };
        return s.last(i);
    }

    return { };
}
// 4,5,4
span<int> mysubspan5(int* p, std::ptrdiff_t size, std::ptrdiff_t i)
{
    if (p != nullptr)
    {
        span<int> s = { p, size };
        return s.subspan(i);
    }

    return { };
}

// 3
span<int> mysubspan6(int* p, std::ptrdiff_t size, std::ptrdiff_t i)
{
    if (p != nullptr)
    {
        span<int> s = { p, size };
        return s.subspan(3);
    }

    return { };
}

// 4
span<int> mysubspan7(int* p, std::ptrdiff_t size, std::ptrdiff_t i)
{
    if (p != nullptr)
    {
        span<int> s = { p, size };
        return s.subspan(i, 3);
    }

    return { };
}
// 4,6,4
span<int> mysubspan8(int* p, std::ptrdiff_t size, std::ptrdiff_t i)
{
    if (p != nullptr)
    {
        span<int> s = { p, size };
        return s.subspan(i, size - i);
    }

    return { };
}

// 9
span<int> mysubspan9(int* p, std::ptrdiff_t size, std::ptrdiff_t i)
{
    if (p != nullptr)
    {
        span<int> s = { p, size };
        return s.subspan(3, i);
    }

    return { };
}

// issue #7
// those iterations should be optimized into oblivion
void bar(span<uint8_t>);
void test_string_for0()
{
    uint8_t bytes[10];
    span<uint8_t> byteSpan = { bytes };

    cstring_span<> cdModelType = ensure_z("iVtrau lDC");
    for (int i = 0; i < 10; ++i)
    {
        byteSpan[i] = cdModelType[i];
    }

    bar(byteSpan);
}

void test_string_for1()
{
    uint8_t bytes[10];
    span<uint8_t> byteSpan = { bytes };

    cstring_span<> cdModelType = { "iVtrau lDC" };
    for (int i = 0; i < 10; ++i)
    {
        byteSpan[i] = cdModelType[i];
    }
    bar(byteSpan);
}

void test_string_for2()
{
    uint8_t bytes[10];
    span<uint8_t> byteSpan = { bytes };

    const char tmp[] = { 'i', 'V', 't', 'r', 'a', 'u', ' ', 'l','D', 'C' };
    cstring_span<> cdModelType = { tmp };
    for (int i = 0; i < 10; ++i)
    {
        byteSpan[i] = cdModelType[i];
    }

    bar(byteSpan);
}


// sanity check if terminate actually terminates 
// Example: if compiling with no exceptions and /D_HAS_EXCEPTION=0, it does not!
void doterminate()
{
    std::terminate();
}

// issue #7 copy -  std::copy should be memmove

void copy_span(gsl::span<int> x, gsl::span<int> y)
{
   std::copy(x.begin(), x.end(), y.begin());
}

void gsl_copy_span(gsl::span<int> x, gsl::span<int> y)
{
   gsl::copy(x, y);
}

void test_string_std_copy()
{
    uint8_t bytes[10];
    span<uint8_t> byteSpan = { bytes };

    cstring_span<> cdModelType = ensure_z("iVtrau lDC");
    std::copy(cdModelType.begin(), cdModelType.end(), byteSpan.begin());
   
    bar(byteSpan);
}

void test_string_gsl_copy()
{
    uint8_t bytes[10];
    span<uint8_t> byteSpan = { bytes };

    cstring_span<> cdModelType = ensure_z("iVtrau lDC");
    gsl::copy(span<const char>{cdModelType} , byteSpan);
   
    bar(byteSpan);
}