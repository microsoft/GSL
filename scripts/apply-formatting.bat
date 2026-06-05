@echo off
setlocal enabledelayedexpansion

"%VCINSTALLDIR%Tools\Llvm\bin\clang-format" -version
if %errorlevel% neq 0 (
    echo [ERROR] clang-format not found, script should be called from a visual studio developer command prompt.
    exit /b %errorlevel%
)

for %%f in (include\gsl\* tests\*.h tests\*.cpp) do (
    echo formatting %%f
    "%VCINSTALLDIR%Tools\Llvm\bin\clang-format" -i --assume-filename x.cpp "%%f"
)
