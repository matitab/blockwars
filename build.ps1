param(
    [string]$MsysRoot,
    [switch]$Rebuild,
    [switch]$Repack,
    [switch]$UI,
    [switch]$VerifyUI,
    [switch]$CheckOnly
)

$ErrorActionPreference = 'Stop'
$previousPath = $env:PATH
Push-Location $PSScriptRoot
try {
    $candidates = @()
    if ($MsysRoot) {
        $candidates = @($MsysRoot)
    } else {
        if ($env:BLOCKWARS_MSYS2) { $candidates += $env:BLOCKWARS_MSYS2 }
        $candidates += Join-Path (Split-Path $PSScriptRoot -Parent) 'MSYS2'
        $candidates += 'C:\msys64'
    }
    $toolchain = $null
    foreach ($root in $candidates) {
        $bin = Join-Path $root 'ucrt64\bin'
        if ((Test-Path (Join-Path $bin 'g++.exe')) -and
            (Test-Path (Join-Path $bin 'mingw32-make.exe')) -and
            (Test-Path (Join-Path $root 'ucrt64\include\SDL2\SDL.h'))) {
            $toolchain = (Resolve-Path $root).Path
            break
        }
    }
    if (!$toolchain) {
        throw 'No se encontro MSYS2 UCRT64 con g++, mingw32-make y SDL2. Usar -MsysRoot o BLOCKWARS_MSYS2.'
    }
    $bin = Join-Path $toolchain 'ucrt64\bin'
    $env:PATH = "$bin;$(Join-Path $toolchain 'usr\bin');$previousPath"
    $compiler = Join-Path $bin 'g++.exe'
    $make = Join-Path $bin 'mingw32-make.exe'
    foreach ($library in @('SDL2', 'SDL2_image', 'SDL2_mixer', 'SDL2_ttf', 'SDL2_net', 'z')) {
        $libRoot = Join-Path $toolchain 'ucrt64\lib'
        if (!(Test-Path (Join-Path $libRoot "lib$library.dll.a")) -and
            !(Test-Path (Join-Path $libRoot "lib$library.a"))) {
            throw "Falta la biblioteca $library en $libRoot."
        }
    }
    Write-Host "Toolchain: $toolchain"
    & $compiler --version
    if ($LASTEXITCODE -ne 0) { throw 'No se pudo ejecutar el compilador.' }
    if (!$CheckOnly) {
        # Native make avoids MSYS shell path conversion for Windows tools.
        $compilerArgument = 'CXX="' + $compiler.Replace('\', '/') + '"'
        $arguments = @('-f', 'Makefile.windows', $compilerArgument, 'blobwars.exe')
        if ($Rebuild) { $arguments += '-B' }
        & $make @arguments
        if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion del juego.' }

        if ($UI -or $VerifyUI) {
            & $make -f Makefile.windows $compilerArgument ui-preview.exe
            if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion de UI.' }
            if ($VerifyUI) {
                $uiOutput = Join-Path $PSScriptRoot '.build\ui'
                New-Item -ItemType Directory -Path $uiOutput -Force | Out-Null
                $layoutTest = Join-Path $uiOutput 'uiLayoutTest.exe'
                & $compiler -Wall -Wextra tests/uiLayoutTest.cpp -o $layoutTest
                if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion de pruebas de layout.' }
                & $layoutTest
                if ($LASTEXITCODE -ne 0) { throw 'Fallaron las pruebas de layout.' }
                $configTest = Join-Path $uiOutput 'uiConfigTest.exe'
                & $compiler -Wall -Wextra ('-I' + (Join-Path $toolchain 'ucrt64\include')) tests/uiConfigTest.cpp -o $configTest
                if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion de pruebas de configuracion UI.' }
                & $configTest
                if ($LASTEXITCODE -ne 0) { throw 'Fallaron las pruebas de configuracion UI.' }
                $optionsTest = Join-Path $uiOutput 'uiOptionsTest.exe'
                & $compiler -Wall -Wextra ('-I' + (Join-Path $toolchain 'ucrt64\include')) ('-L' + (Join-Path $toolchain 'ucrt64\lib')) tests/uiOptionsTest.cpp uiOptionsView.o uiRenderer.o -lSDL2_image -lSDL2_ttf -lSDL2 -o $optionsTest
                if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion de pruebas de Options.' }
                & $optionsTest $uiOutput
                if ($LASTEXITCODE -ne 0) { throw 'Fallaron las pruebas de Options.' }
                & $make -f Makefile.windows $compilerArgument .build/ui/gameOverTest.exe
                if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion de pruebas de Game Over.' }
                & (Join-Path $uiOutput 'gameOverTest.exe') $uiOutput
                if ($LASTEXITCODE -ne 0) { throw 'Fallaron las pruebas de Game Over.' }
                & (Join-Path $PSScriptRoot 'ui-preview.exe') --verify $uiOutput
                if ($LASTEXITCODE -ne 0) { throw 'Fallo la verificacion de texto y renderizado UI.' }
            }
        }

        & $make -f Makefile.windows $compilerArgument pak.exe
        if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion del empaquetador.' }

        $cache = Join-Path $PSScriptRoot '.build'
        New-Item -ItemType Directory -Path $cache -Force | Out-Null
        $inventory = foreach ($directory in @('data', 'gfx', 'sound', 'music')) {
            if (!(Test-Path $directory -PathType Container)) { throw "Falta la carpeta $directory." }
            Get-ChildItem -LiteralPath $directory -File -Recurse | ForEach-Object {
                '{0}|{1}|{2}' -f $_.FullName.Substring($PSScriptRoot.Length), $_.Length, $_.LastWriteTimeUtc.Ticks
            }
        }
        $inventory += 'pak.exe|' + (Get-Item 'pak.exe').LastWriteTimeUtc.Ticks
        $signature = ($inventory | Sort-Object) -join "`n"
        $manifest = Join-Path $cache 'resources.txt'
        $previous = if (Test-Path $manifest) { [IO.File]::ReadAllText($manifest) } else { '' }
        if ($Repack -or !(Test-Path 'blobwars.pak') -or $signature -cne $previous) {
            $staging = Join-Path $cache 'blobwars.pak'
            Write-Host 'Actualizando recursos...'
            & (Join-Path $PSScriptRoot 'pak.exe') data gfx sound music $staging > (Join-Path $cache 'pak.log')
            if ($LASTEXITCODE -ne 0) { throw 'Fallo el empaquetado; el pak anterior se conserva.' }
            if (!(Test-Path $staging) -or (Get-Item $staging).Length -lt 8) {
                throw 'El empaquetador no produjo un archivo valido.'
            }
            $destination = Join-Path $PSScriptRoot 'blobwars.pak'
            if (Test-Path $destination) {
                [IO.File]::Replace($staging, $destination, (Join-Path $cache 'blobwars.previous.pak'))
            } else {
                [IO.File]::Move($staging, $destination)
            }
            [IO.File]::WriteAllText($manifest, $signature)
        } else {
            Write-Host 'Recursos sin cambios; se conserva blobwars.pak.'
        }
        Write-Host 'Compilacion completa: blobwars.exe'
    }
} catch {
    [Console]::Error.WriteLine($_.Exception.Message)
    exit 1
} finally {
    $env:PATH = $previousPath
    Pop-Location
}
