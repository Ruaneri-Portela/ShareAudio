# Copy x64/Release/*.exe and x64/Release/*.dll to destination
Copy-Item -Path ".\x64\Release\*.exe" -Destination "Y:/" -Force
Copy-Item -Path ".\x64\Release\*.dll" -Destination "Y:/" -Force

# Copy libportaudio.dll to destination
$paDLL = Get-Command libportaudio.dll | ForEach-Object { $_.Source }
Copy-Item -Path $paDLL -Destination "Y:/" -Force

# Copy build/*.exe to /y/ShareAudio_GCC.exe
Copy-Item -Path ".\build\*.exe" -Destination "Y:/ShareAudio_GCC.exe" -Force

Write-Host "Deployed to Y:/"
