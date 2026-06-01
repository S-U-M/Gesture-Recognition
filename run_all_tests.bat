@echo off
set EXE=C:\Users\T\source\repos\Gesture-Recognition\x64\Debug\Project Test.exe
set IMGDIR=C:\Users\T\source\repos\Gesture-Recognition

echo Running OK tests...
"%EXE%" "%IMGDIR%\ok1.jpg"
"%EXE%" "%IMGDIR%\ok2.jpg"

echo Running Thumbs Up tests...
"%EXE%" "%IMGDIR%\thumbsup1.jpg"
"%EXE%" "%IMGDIR%\thumbsup2.jpg"

echo Running Pointing tests...
"%EXE%" "%IMGDIR%\pointing1.jpg"
"%EXE%" "%IMGDIR%\pointing2.jpg"

echo Running Peace tests...
"%EXE%" "%IMGDIR%\peace1.jpg"
"%EXE%" "%IMGDIR%\peace2.jpg"

echo Running Stop tests...
"%EXE%" "%IMGDIR%\stop1.jpg"
"%EXE%" "%IMGDIR%\stop2.jpg"

echo.
echo All tests finished.
pause