copy ..\JerryEngine\Release\JerryEngine.exe
ren JerryEngine.exe jerryvm.exe
del import\pthouse\jerryvm.exe
copy jerryvm.exe import\pthouse\
del jerryvm.exe
make_usb_boot -vhd

