@ECHO OFF

cl.exe /nologo /W0 iathooker.cpp /MT /link /DLL /OUT:iathooker.dll

cl.exe /nologo /Ox /MT /W0 /GS- /DNDEBUG /Tp iattarget.cpp /link /OUT:iattarget.exe /SUBSYSTEM:CONSOLE
del *.obj *.lib *.exp