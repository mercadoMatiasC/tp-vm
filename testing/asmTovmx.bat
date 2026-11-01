@echo off
start /wait vmt param.asm param.vmx -o v=2
move /y param.vmx "C:\MV"