@echo off
rem Compile le banc de test PC (Visual Studio Build Tools 2022)
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
cd /d "%~dp0"
if not exist out mkdir out
cl /nologo /O2 /EHsc /W3 /utf-8 /D_CRT_SECURE_NO_WARNINGS /DHOST_TEST /I stubs /I .. /Fo:out\ /Fe:out\hosttest.exe ^
   hosttest.cpp ..\gifanim.cpp ..\canvas.cpp ..\font5x7.cpp ..\localtime.cpp ..\config.cpp ..\testpattern.cpp ..\configedit.cpp ..\netutil.cpp ^
   ..\clockstyle.cpp ..\clockrender.cpp ..\weatherdata.cpp ..\weathericons.cpp ..\gifmarks.cpp ..\messages.cpp ..\schedule.cpp ..\hapayload.cpp ..\lang.cpp stubs\host_extra.cpp
