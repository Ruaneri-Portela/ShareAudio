# Copy x64/Release/*.exe and x64/Release/*.dll to destination
Copy-Item -Path ".\x64\Release\*.exe" -Destination "Y:/" -Force
Copy-Item -Path ".\x64\Release\*.dll" -Destination "Y:/" -Force
Copy-Item -Path ".\build\*.jar" -Destination "Y:/" -Force
Copy-Item -Path ".\build\libShareAudioJava.dll" -Destination "Y:/" -Force

# Copy build/*.exe to /y/ShareAudio_GCC.exe
New-Item -ItemType Directory -Force -Path "Y:/GCC/"
Copy-Item -Path ".\build\*.exe" -Destination "Y:/GCC/" -Force
Copy-Item -Path ".\build\*.dll" -Destination "Y:/GCC/" -Force

# Copy libportaudio.dll to destination
$paDLL = Get-Command libportaudio.dll | ForEach-Object { $_.Source }
Copy-Item -Path $paDLL -Destination "Y:/GCC/" -Force

Write-Host "Deployed to Y:/"
