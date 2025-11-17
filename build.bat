@echo off

mkdir build 2>nul
cd build

echo 正在配置项目...
cmake.exe .. -G "Visual Studio 17 2022" -T v143 -A WIN32 -DCMAKE_POLICY_VERSION_MINIMUM=3.5

if %ERRORLEVEL% NEQ 0 (
    echo CMake配置失败
    exit /b %ERRORLEVEL%
)

echo 正在构建项目...
cmake.exe --build . --config Release --verbose

if %ERRORLEVEL% NEQ 0 (
    echo 构建失败
    exit /b %ERRORLEVEL%
)

echo 构建完成!