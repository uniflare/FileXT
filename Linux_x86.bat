@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

REM Build is where we will generate project files and compile the program into.
IF NOT EXIST Build_Linux32/ (
    MKDIR Build_Linux32
)
IF "%~1"=="-debug" (
    echo Building - debug
    wsl cmake -G "Ninja" -B "Build_Linux32" -DCMAKE_BUILD_TYPE=Debug -DIs64Bit:BOOL=OFF
    wsl cmake --build "Build_Linux32"
) ELSE (
    echo Building - release
    wsl cmake -G "Ninja" -B "Build_Linux32" -DCMAKE_BUILD_TYPE=Release -DIs64Bit:BOOL=OFF
    wsl cmake --build "Build_Linux32"
)