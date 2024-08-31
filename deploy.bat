call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
set APP="upd-windlg"
msbuild %APP%.sln /p:Configuration=Release /p:Platform=x64
move /Y x64_Release\%APP%.exe "."
rmdir /S /Q x64_Release
pause
