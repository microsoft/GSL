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

#pragma once

#ifndef GSL_FINAL_ACTION_H
#define GSL_FINAL_ACTION_H

#ifdef _MSC_VER
// VS 2013 workarounds
#if _MSC_VER <= 1800
// noexcept is not understood 
#define noexcept /* nothing */
#endif // _MSC_VER <= 1800
#endif // _MSC_VER

namespace gsl
{

//
// GSL.util: Utilities
//////////////////////

// This component is a `gsl` utility that allows you to ensure that an act
// gets performed at the end of a scope
template <class F>
class final_action
{
public:
    explicit final_action(F f) noexcept : f_(std::move(f)), invoke_(true) {}

    final_action(final_action&& other) noexcept : f_(std::move(other.f_)), invoke_(true) { other.invoke_ = false; }

    ~final_action() noexcept { if (invoke_) f_(); }

private:
	final_action(const final_action&) = delete;
    final_action& operator=(const final_action&) = delete;

private:
    F f_;
    bool invoke_;
};

//
// Function name: finally
//
// This is convenience function to generate a final_action
//
template <class F>
final_action<F> finally(const F& f) noexcept { return final_action<F>(f); }

template <class F>
final_action<F> finally(F&& f) noexcept {	return final_action<F>(std::forward<F>(f)); }

} // close namespace gsl

#endif // GSL_FINAL_ACTION_H
