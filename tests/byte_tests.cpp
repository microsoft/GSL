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
#include <gsl/gsl_byte.h>

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

        // waiting for C++17 enum class direct initializer support
        //{
        //    byte b { 14 };
        //    CHECK(static_cast<unsigned char>(b) == 14);
        //}
    }

    TEST(bitwise_operations)
    {
        byte b = byte(0xFF);

        byte a = byte(0x00);
        CHECK((b | a) == byte(0xFF));
        CHECK(a == byte(0x00));

        a |= b;
        CHECK(a == byte(0xFF));

        a = byte(0x01);
        CHECK((b & a) == byte(0x01));

        a &= b;
        CHECK(a == byte(0x01));

        CHECK((b ^ a) == byte(0xFE));
        
        CHECK(a == byte(0x01));
        a ^= b;
        CHECK(a == byte(0xFE));

        a = byte(0x01);
        CHECK(~a == byte(0xFE));

        a = byte(0xFF);
        CHECK((a << 4) == byte(0xF0));
        CHECK((a >> 4) == byte(0x0F));

        a <<= 4;
        CHECK(a == byte(0xF0));
        a >>= 4;
        CHECK(a == byte(0x0F));
    }
}

}

int main(int, const char* []) { return UnitTest::RunAllTests(); }
