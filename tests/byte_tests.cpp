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
#include <gsl/gsl_byte>

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace gsl;

namespace
{

SUITE(byte_tests)
{
    TEST(construction)
    {
        {
            byte b = static_cast<byte>(4);
            CHECK(static_cast<unsigned char>(b) == 4);
        }

        {
            byte b = byte(12);
            CHECK(static_cast<unsigned char>(b) == 12);
        }
        
        {
            byte b = to_byte<12>();
            CHECK(static_cast<unsigned char>(b) == 12);
        }
        {
            unsigned char uc = 12;
            byte b = to_byte(uc);
            CHECK(static_cast<unsigned char>(b) == 12);
        }

        // waiting for C++17 enum class direct initializer support
        //{
        //    byte b { 14 };
        //    CHECK(static_cast<unsigned char>(b) == 14);
        //}
    }

    TEST(bitwise_operations)
    {
        byte b = to_byte<0xFF>();

        byte a = to_byte<0x00>();
        CHECK((b | a) == to_byte<0xFF>());
        CHECK(a == to_byte<0x00>());

        a |= b;
        CHECK(a == to_byte<0xFF>());

        a = to_byte<0x01>();
        CHECK((b & a) == to_byte<0x01>());

        a &= b;
        CHECK(a == to_byte<0x01>());

        CHECK((b ^ a) == to_byte<0xFE>());
        
        CHECK(a == to_byte<0x01>());
        a ^= b;
        CHECK(a == to_byte<0xFE>());

        a = to_byte<0x01>();
        CHECK(~a == to_byte<0xFE>());

        a = to_byte<0xFF>();
        CHECK((a << 4) == to_byte<0xF0>());
        CHECK((a >> 4) == to_byte<0x0F>());

        a <<= 4;
        CHECK(a == to_byte<0xF0>());
        a >>= 4;
        CHECK(a == to_byte<0x0F>());
    }
}

}

int main(int, const char* []) { return UnitTest::RunAllTests(); }
