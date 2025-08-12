@echo off
setlocal
rem Use the VS "x64 Native Tools Command Prompt" so env is set.
set OUT=build
if not exist %OUT% mkdir %OUT%
pushd %OUT%

cl ..\win32_basic.c /nologo /Zi /W4 /WX- /std:c17 /EHsc /FC ^
  /wd5105 ^
  /Fe:win32_basic.exe ^
  user32.lib gdi32.lib

popd
