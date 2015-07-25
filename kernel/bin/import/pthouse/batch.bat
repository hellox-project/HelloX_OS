del hcnimge.bin
del master.bin
process -i master.dll -o master.bin
append -s realinit.bin -a miniker.bin -b 2000 -o image_1.bin
append -s image_1.bin -a master.bin -b 12000 -o image_2.bin
ren image_2.bin hcnimge.bin
del image_1.bin
copy hcnimge.bin c:\
dumpf32.exe
copy bootsect.dos c:\

mkdir C:\PTHOUSE
copy asc16 C:\PTHOUSE
copy hzk16 C:\PTHOUSE
copy network.exe C:\PTHOUSE
copy MODCFG.INI C:\PTHOUSE
copy hcngui.bin C:\PTHOUSE
copy master.dll C:\PTHOUSE
copy batch.bat C:\PTHOUSE
copy process.exe C:\PTHOUSE
copy append.exe C:\PTHOUSE

mkdir C:\HCGUIAPP
copy *.hcx C:\HCGUIAPP
