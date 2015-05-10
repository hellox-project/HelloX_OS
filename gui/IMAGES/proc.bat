del master.bin
del hcngui.bin
process -i master.dll -o master.bin
process -i hcngui.dll -o hcngui.bin
append -s master.bin -a hcngui.bin
append -s master.bin -a ASC16
append -s master.bin -a hzk16
vfmaker
