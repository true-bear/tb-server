@echo off
chcp 65001 > nul
cd /d "%~dp0"
echo [INFO] Working directory: %cd%

set PROTOC=..\..\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe
set PROTO_FILE=message.proto
set OUT_CPP=..\server\logic\generated
set OUT_CS=..\dummy

if not exist %OUT_CPP% (
    mkdir %OUT_CPP%
)
if not exist %OUT_CS% (
    mkdir %OUT_CS%
)

echo [INFO] Executing protoc for C++
"%PROTOC%" --proto_path=. --cpp_out=%OUT_CPP% %PROTO_FILE%
echo [INFO] C++ exit code: %errorlevel%

echo [INFO] Executing protoc for C#
"%PROTOC%" --proto_path=. --csharp_out=%OUT_CS% %PROTO_FILE%
echo [INFO] C# exit code: %errorlevel%

echo.
echo [DONE] Press any key to exit
pause > nul
