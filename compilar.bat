@echo off
setlocal

set PATH=C:\Users\Usuario\Desktop\Proyectos\MSYS2\ucrt64\bin;C:\Users\Usuario\Desktop\Proyectos\MSYS2\usr\bin;%PATH%

echo Compilando blobwars...
make.exe -f Makefile.windows
if %ERRORLEVEL% NEQ 0 goto error

echo Regenerando blobwars.pak...
if exist blobwars.pak del blobwars.pak
make.exe -f Makefile.windows buildpak
if %ERRORLEVEL% NEQ 0 goto error

echo.
echo Compilacion exitosa!
goto fin

:error
echo.
echo Error en la compilacion.

:fin
pause
