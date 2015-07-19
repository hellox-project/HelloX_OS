del master.bin
del master.dll
copy ..\release\master.dll
process -i master.dll -o master.bin
vfmaker
