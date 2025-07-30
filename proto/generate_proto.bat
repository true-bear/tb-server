@echo off

echo 1. setting variables...

set PROTOC=..\proto-build\install\bin\protoc.exe
set PROTO_SRC=message.proto
set PROTO_INCLUDE=..\proto-build\install\include
set CPP_OUT=..\server\logic\generated
set CSHARP_OUT=..\dummy

echo PROTOC: %PROTOC%
echo PROTO_SRC: %PROTO_SRC%
echo PROTO_INCLUDE: %PROTO_INCLUDE%
echo CPP_OUT: %CPP_OUT%
echo CSHARP_OUT: %CSHARP_OUT%

if not exist "%PROTOC%" (
    echo [ERROR] protoc.exe not found at %PROTOC%
    pause
    exit /b
)

if not exist "%PROTO_SRC%" (
    echo [ERROR] message.proto not found in %CD%
    pause
    exit /b
)

if not exist "%CPP_OUT%" mkdir "%CPP_OUT%"
if not exist "%CSHARP_OUT%" mkdir "%CSHARP_OUT%"

echo 2. generating C++...
"%PROTOC%" --proto_path="%PROTO_INCLUDE%" --proto_path="." --cpp_out="%CPP_OUT%" "%PROTO_SRC%"

echo 3. generating C#...
"%PROTOC%" --proto_path="%PROTO_INCLUDE%" --proto_path="." --csharp_out="%CSHARP_OUT%" "%PROTO_SRC%"

echo 4. done.
pause
