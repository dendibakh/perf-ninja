@echo See https://github.com/google/benchmark

@rem Check out the library.
git clone https://github.com/google/benchmark.git

@rem Benchmark requires Google Test as a dependency. Add the source tree as a subdirectory.
git clone https://github.com/google/googletest.git benchmark/googletest

@rem Go to the library root directory
cd benchmark
@rem Make a build directory to place the build output.
cmake -E make_directory "build"

@rem Find installed Visual Studio versions using vswhere
setlocal
set VSWHERE_PATH="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

@rem Get the installation version
for /f "tokens=* USEBACKQ" %%i in (`%VSWHERE_PATH% -latest -prerelease -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion`) do set INSTALLATION_VERSION=%%i

@rem Extract the major version number
for /f "tokens=1 delims=." %%i in ("%INSTALLATION_VERSION%") do set VS_MAJOR_VER=%%i

@rem Set the appropriate generator based on the major version number
if "%VS_MAJOR_VER%"=="17" (
    set GENERATOR="Visual Studio 17 2022"
) else (
    if "%VS_MAJOR_VER%"=="16" (
        set GENERATOR="Visual Studio 16 2019"
    ) else (
        echo Unsupported Visual Studio version!
        exit /b 1
    )
)
endlocal

echo Using generator %GENERATOR%

@rem Generate build system files with cmake.
cmake -E chdir "build" cmake -G %GENERATOR% -DCMAKE_BUILD_TYPE=Release ../
@rem Build the library.
cmake --build "build" --config Release --parallel 4

@rem Generate build system files with cmake.
cmake -E chdir "build" cmake -G %GENERATOR% -DCMAKE_BUILD_TYPE=Debug ../
@rem Build the library.
cmake --build "build" --config Debug --parallel 4
