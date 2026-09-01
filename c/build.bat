@echo off
setlocal
cd /d "%~dp0"

where gcc >nul 2>&1
if %ERRORLEVEL%==0 (
  gcc -std=c11 -Wall -Wextra -g -I. -o whilec.exe util.c syntax.c lexer.c parser.c virtual_stack.c emit_wasm.c whilec.c
  if errorlevel 1 exit /b 1
  gcc -std=c11 -Wall -Wextra -g -I. -o whilei.exe util.c syntax.c lexer.c parser.c interpret.c virtual_stack.c whilei.c
  if errorlevel 1 exit /b 1
  gcc -std=c11 -Wall -Wextra -g -I. -o test_whilelang.exe util.c syntax.c lexer.c parser.c interpret.c virtual_stack.c emit_wasm.c test_whilelang.c
  if errorlevel 1 exit /b 1
  echo built whilec.exe whilei.exe test_whilelang.exe
  exit /b 0
)

where cl >nul 2>&1
if %ERRORLEVEL%==0 (
  cl /nologo /W3 /utf-8 /D_CRT_SECURE_NO_WARNINGS /I. /Fe:whilec.exe util.c syntax.c lexer.c parser.c virtual_stack.c emit_wasm.c whilec.c
  if errorlevel 1 exit /b 1
  cl /nologo /W3 /utf-8 /D_CRT_SECURE_NO_WARNINGS /I. /Fe:whilei.exe util.c syntax.c lexer.c parser.c interpret.c virtual_stack.c whilei.c
  if errorlevel 1 exit /b 1
  cl /nologo /W3 /utf-8 /D_CRT_SECURE_NO_WARNINGS /I. /Fe:test_whilelang.exe util.c syntax.c lexer.c parser.c interpret.c virtual_stack.c emit_wasm.c test_whilelang.c
  if errorlevel 1 exit /b 1
  del /Q *.obj 2>NUL
  echo built whilec.exe whilei.exe test_whilelang.exe
  exit /b 0
)

echo gcc か cl が見つかりません。
echo MinGW / w64devkit を入れるか、Visual Studio の Developer Command Prompt を使ってください。
exit /b 1
