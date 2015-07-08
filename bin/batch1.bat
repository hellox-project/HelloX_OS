cd ..\kernel\bin
del hcnimge.bin
append -s realinit.bin -a miniker.bin -b 2000 -o image_1.bin
append -s image_1.bin -a master.bin -b 12000 -o image_2.bin
ren image_2.bin hcnimge.bin
del image_1.bin
cd ..
cd ..\bin
copy ..\kernel\bin\hcnimge.bin
cd ..\gui\guimaker
copy ..\release\hcngui.dll
del hcngui.bin
process -i hcngui.dll -o hcngui.bin
append -s hcngui.bin -a ASC16 -b 20000
append -s hcngui.bin -a HZK16 -b 30000
cd ..
cd ..\bin
copy ..\gui\guimaker\hcngui.bin .\import\pthouse
del vdisk.vhd
make_usb_boot -make_vhd
