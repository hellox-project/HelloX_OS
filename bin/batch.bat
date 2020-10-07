cd ..\kernel\bin
copy ..\release\master.dll
del realinit.bin
del miniker.bin
del bootsect
nasm -f bin ..\arch\x86\miniker.asm -o miniker.bin
nasm -f bin ..\arch\x86\realinit.asm -o realinit.bin
nasm -f bin ..\arch\x86\bootsect.asm -o bootsect
del hcnimge.bin
del master.bin
process -i master.dll -o master.bin
append -s realinit.bin -a miniker.bin -b 2000 -o image_1.bin
append -s image_1.bin -a master.bin -b 12000 -o image_2.bin
ren image_2.bin hcnimge.bin
del image_1.bin
cd ..
cd ..\bin
copy ..\kernel\bin\hcnimge.bin
copy ..\kernel\bin\bootsect
cd ..\gui\guimaker
copy ..\release\hcngui.dll
del hcngui.bin
process -i hcngui.dll -o hcngui.bin
append -s hcngui.bin -a ASC16 -b 20000
append -s hcngui.bin -a HZK16 -b 30000
cd ..
cd ..\bin
copy ..\gui\guimaker\hcngui.bin .\import\pthouse
del usragent.dll
del usragent.bin
copy ..\kernel\release\usragent.dll
process -i usragent.dll -o usragent.bin
copy usragent.bin .\import\
copy ..\kernel\release\examapp.exe
copy examapp.exe .\import\
copy ..\app\cpuid\Release\cpuid.exe
copy cpuid.exe .\import\

