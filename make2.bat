cl /MD /O2 -c -Fonixio.obj -I"%LUA_DEV%\include" nixio.c /D __WINNT__
cl /MD /O2 -c -Fofile.obj -I"%LUA_DEV%\include" file.c /D __WINNT__
link -dll -out:nixio.dll "%LUA_DEV%\lib\lua5.1.lib" nixio.obj file.obj
