cd /d %~dp0

cd l6
cl ana.c /O2
cl ..\ccn.c /O2 /DNUM=6 ..\sqlite3.lib
cl cn.c /O2 /MT
rc cwn.rc
cl cwn.c cwn.res /O2 /link gdi32.lib user32.lib
cl mn.c /O2 /link gdi32.lib user32.lib
rc pms.rc
cl pms.cpp pms.res /O2 /link comctl32.lib comdlg32.lib gdi32.lib shell32.lib user32.lib
cl pn.c /O2
cl wn.c /O2

cd ..\ml
cl ana.c /O2
cl ..\ccn.c /O2 /DNUM=5 ..\sqlite3.lib
cl cn.c /O2
rc cwn.rc
cl cwn.c cwn.res /O2 /link gdi32.lib user32.lib
cl mn.c /O2 /link gdi32.lib user32.lib
cl pn.c /O2
cl wn.c /O2

cd ..\n3
cl ana.c /O2
cl cn.c /O2
cl wn.c /O2

cd ..\n4
cl ana.c /O2
cl cn.c /O2 /MT
cl wn.c /O2
