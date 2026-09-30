@echo off
setlocal

cd /d "%~dp0"

set PATH=C:\Users\Usuario\Desktop\Proyectos\MSYS2\ucrt64\bin;C:\Users\Usuario\Desktop\Proyectos\MSYS2\usr\bin;%PATH%

echo Limpiando compilacion anterior...
make.exe -f Makefile.windows clean
if %ERRORLEVEL% NEQ 0 (
    echo No se pudo usar "make clean", borrando archivos .o a mano...
    del /s /q *.o >nul 2>&1
)

echo Compilando blobwars...
make.exe -f Makefile.windows
if %ERRORLEVEL% NEQ 0 goto error

echo Regenerando blobwars.pak...
if exist blobwars.pak del blobwars.pak
make.exe -f Makefile.windows buildpak
if %ERRORLEVEL% NEQ 0 goto error

echo.
echo Compilacion limpia exitosa!
goto fin

:error
echo.
echo Error en la compilacion.

:fin
pause
