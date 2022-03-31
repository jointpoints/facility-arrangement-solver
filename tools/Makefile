FASOLVER_VERSION = 0.1
FASOLVER_TESTBUILD = test 
#FASOLVER_TESTBUILD = 



ifeq ($(OS), Windows_NT)
CPLEX_PATH = D:\Programs\CPLEX
MSVC_PATH = D:\Programs\VisualStudio2022\VC\Tools\MSVC\14.31.31103
WINKITS_PATH = C:\Program Files (x86)\Windows Kits

BUILD_PATH = Facility Arrangement Solver ($(FASOLVER_TESTBUILD)build for Windows, v.$(FASOLVER_VERSION))

build : win_prepare win_build win_link win_setup win_clean
else
#	Setup for Linux
endif



BUILD_UNITS = PlanarMetric Logger
SOURCE_PATH = source
OBJ_PATH = __buildtmp__



# Build for Windows
win_prepare :
	powershell New-Item -Path $(OBJ_PATH) -ItemType Directory -Force
	powershell New-Item -Path \"..\$(BUILD_PATH)\" -ItemType Directory -Force

win_build : $(addprefix win_,$(addsuffix _generic,$(BUILD_UNITS))) win_main

win_link :
	cl $(OBJ_PATH)\main.obj $(addprefix $(OBJ_PATH)\,$(addsuffix .obj,$(BUILD_UNITS))) "$(CPLEX_PATH)\cplex\lib\x64_windows_msvc14\stat_mda\cplex2010.lib" "$(CPLEX_PATH)\cplex\lib\x64_windows_msvc14\stat_mda\ilocplex.lib" "$(CPLEX_PATH)\concert\lib\x64_windows_msvc14\stat_mda\concert.lib" -Fe"..\$(BUILD_PATH)\Facility Arrangement Solver.exe" -link -LIBPATH:"$(MSVC_PATH)\lib\x64" -LIBPATH:"$(WINKITS_PATH)\10\Lib\10.0.19041.0\um\x64" -LIBPATH:"$(WINKITS_PATH)\10\Lib\10.0.19041.0\ucrt\x64"

win_setup :
	powershell Copy-Item -Path \"$(CPLEX_PATH)\cplex\bin\x64_win64\cplex2010.dll\" -Destination \"..\$(BUILD_PATH)\"

win_clean :
	powershell Remove-Item -Path $(OBJ_PATH) -Force -Recurse

win_%_generic :
	cl -std:c++20 -EHsc -MD -c -I "$(CPLEX_PATH)\cplex\include" -I "$(CPLEX_PATH)\concert\include" -I "$(MSVC_PATH)\include" -I "$(WINKITS_PATH)\10\Include\10.0.19041.0\ucrt" -Fo"$(OBJ_PATH)\$*.obj" "..\$(SOURCE_PATH)\$*\$*.cpp"

win_main :
	cl -std:c++20 -EHsc -MD -c -I "$(CPLEX_PATH)\cplex\include" -I "$(CPLEX_PATH)\concert\include" -I "$(MSVC_PATH)\include" -I "$(WINKITS_PATH)\10\Include\10.0.19041.0\ucrt" -Fo"$(OBJ_PATH)\main.obj" "..\$(SOURCE_PATH)\main.cpp"

win_Logger_generic :
	cl -std:c++latest -EHsc -MD -c -I "$(CPLEX_PATH)\cplex\include" -I "$(CPLEX_PATH)\concert\include" -I "$(MSVC_PATH)\include" -I "$(WINKITS_PATH)\10\Include\10.0.19041.0\ucrt" -Fo"$(OBJ_PATH)\Logger.obj" "..\$(SOURCE_PATH)\Logger\Logger.cpp"
