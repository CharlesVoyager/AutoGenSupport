@echo off
del *.sdf /f /s /q /a
rd ipch /s /q
for /f "delims=" %%i in ('dir ipch /s /b') do rd /s /q "%%i"
del /s *.cod;*.obj;*.bsc;*.pdb;*.sbr;*.exp;*.map;*.res;*.idb;*.pch;*.ncb;*.ilk;*.bak;*.aps;*.plg;*.opt;*.user;*.suo
echo "Finished."
pause