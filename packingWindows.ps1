# Remove existing VERSION.h file
Remove-Item -Path ".\src\VERSION.h" -Force

# Get Git version
$GIT_VERSION = git describe --tags
Add-Content -Path ".\src\VERSION.h" -Value "#define VERSION `"$GIT_VERSION`""

# Clean and make
make clean
make

# Packing for MSVC
$version = git describe --tags
$builds = @("Win32", "ARM64", "x64")

foreach ($build in $builds) {
    if ($build -eq "Win32") {
        $msbild = "x86"
    } else {
        $msbild = $build
    }
    & "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ShareAudio.sln -p:Configuration=Release -p:Platform="$msbild"
    & "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ShareAudio.sln -p:Configuration=Debug -p:Platform="$msbild"

    New-Item -ItemType Directory -Path ".\build\$build" -Force
    Copy-Item -Path ".\$build\Release\*.exe" -Destination ".\build\$build" -Force
    Copy-Item -Path ".\$build\Release\*.dll" -Destination ".\build\$build" -Force
    Compress-Archive -Path ".\build\$build\*" -DestinationPath ".\build\ShareAudio_MSVC_$msbild($version)_win.zip" -Force
    Remove-Item -Path ".\build\$build" -Recurse -Force
}

# Packing for x64_GNU_GCC
New-Item -ItemType Directory -Path ".\build\x64_GNU_GCC" -Force
$paDLL = Get-Command libportaudio.dll | ForEach-Object { $_.Source }
Copy-Item -Path $paDLL -Destination ".\build\" -Force
Copy-Item -Path ".\build\*.dll" -Destination ".\build\x64_GNU_GCC" -Force
Copy-Item -Path ".\build\*.exe" -Destination ".\build\x64_GNU_GCC" -Force
Compress-Archive -Path ".\build\x64_GNU_GCC\*" -DestinationPath ".\build\ShareAudio_GNU-GCC_x64($version)_win.zip" -Force
Remove-Item -Path ".\build\x64_GNU_GCC" -Recurse -Force
