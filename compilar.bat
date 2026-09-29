@echo off
setlocal

REM Configurar PATH para MSYS2
set PATH=C:\Users\Usuario\Desktop\Proyectos\MSYS2\ucrt64\bin;C:\Users\Usuario\Desktop\Proyectos\MSYS2\usr\bin;%PATH%

REM Compilar
echo Compilando blobwars...
C:\Users\Usuario\Desktop\Proyectos\MSYS2\usr\bin\make.exe -f Makefile.windows

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilacion exitosa!
    echo Ejecutable: blobwars.exe
) else (
    echo.
    echo Error en la compilacion.
)

pause
