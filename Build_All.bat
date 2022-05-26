@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

CALL "./Clean_All.bat"

REM Build is where we will generate project files and compile the program into.
IF NOT EXIST Build_Win32/ (
    MKDIR Build_Win32
)
IF NOT EXIST Build_Win64/ (
    MKDIR Build_Win64
)
IF NOT EXIST Build_Linux/ (
    MKDIR Build_Linux32
)
IF NOT EXIST Build_Linux/ (
    MKDIR Build_Linux64
)

REM First we check if we have cmake installed at path.
WHERE cmake
IF %ERRORLEVEL% NEQ 0 (
    REM If there is no cmake at path, we will search for cmake installed alongside visual studio.
    GOTO CMAKE_FROM_VS_INSTALL
) ELSE (
    GOTO CMAKE_LITERAL
)

:CMAKE_FROM_VS_INSTALL
for /f "usebackq tokens=*" %%i in (`call "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set InstallDir=%%i
)
SET cmake="%InstallDir%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
GOTO RUN_CMAKE

:CMAKE_LITERAL
REM We found cmake, so we can just run the command as is.
SET cmake=cmake

REM fallthrough intentionally to running cmake.
:RUN_CMAKE


IF "%~1"=="-debug" (
    echo Building - debug
    %cmake% -G "Visual Studio 17 2022" -A Win32 -B "Build_Win32" -DIs64Bit:BOOL=OFF
    %cmake% -G "Visual Studio 17 2022" -A x64 -B "Build_Win64" -DIs64Bit:BOOL=ON
    %cmake% --build "Build_Win32" --config=Debug
    %cmake% --build "Build_Win64" --config=Debug
    
    wsl cmake -G "Ninja" -B "Build_Linux32" -DCMAKE_BUILD_TYPE=Debug -DIs64Bit:BOOL=OFF
    wsl cmake -G "Ninja" -B "Build_Linux64" -DCMAKE_BUILD_TYPE=Debug -DIs64Bit:BOOL=ON
    wsl cmake --build "Build_Linux32"
    wsl cmake --build "Build_Linux64"
    
    Powershell -executionpolicy remotesigned -File ./buildArmaMod.ps1 "-debug"
) ELSE (
    echo Building - release
    %cmake% -G "Visual Studio 17 2022" -A Win32 -B "Build_Win32" -DIs64Bit:BOOL=OFF
    %cmake% -G "Visual Studio 17 2022" -A x64 -B "Build_Win64" -DIs64Bit:BOOL=ON
    %cmake% --build "Build_Win32" --config=Release
    %cmake% --build "Build_Win64" --config=Release
    
    wsl cmake -G "Ninja" -B "Build_Linux32" -DCMAKE_BUILD_TYPE=Release -DIs64Bit:BOOL=OFF
    wsl cmake -G "Ninja" -B "Build_Linux64" -DCMAKE_BUILD_TYPE=Release -DIs64Bit:BOOL=ON
    wsl cmake --build "Build_Linux32"
    wsl cmake --build "Build_Linux64"
    
    Powershell -executionpolicy remotesigned -File ./buildArmaMod.ps1
)