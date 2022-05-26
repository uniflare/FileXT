@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

REM Build is where we will generate project files and compile the program into.
IF NOT EXIST Build_Linux64/ (
    MKDIR Build_Linux64
)
IF "%~1"=="-debug" (
    echo Building - debug
    wsl cmake -G "Ninja" -B "Build_Linux64" -DCMAKE_BUILD_TYPE=Debug -DIs64Bit:BOOL=ON
    wsl cmake --build "Build_Linux64"
) ELSE (
    echo Building - release
    wsl cmake -G "Ninja" -B "Build_Linux64" -DCMAKE_BUILD_TYPE=Release -DIs64Bit:BOOL=ON
    wsl cmake --build "Build_Linux64"
)