
cd D:\work\HelloX\BootTest\

del hcngui.bin
process -i hcngui.dll -o hcngui.bin
append -s hcngui.bin -a ASC16 -b 20000
append -s hcngui.bin -a HZK16 -b 30000

copy D:\work\HelloX\BootTest\hcngui.bin  D:\work\HelloX\BootTest\import\pthouse\
del hcnimge.bin
del master.bin
process -i master.dll -o master.bin
append -s realinit.bin -a miniker.bin -b 2000 -o image_1.bin
append -s image_1.bin -a master.bin -b 12000 -o image_2.bin
ren image_2.bin hcnimge.bin
del image_1.bin

make_usb_boot.exe  -make_vhd
