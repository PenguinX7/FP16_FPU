设计中使用opcode区分运算，0=加法，1=乘法，2=除法，3=MAC，4=ex，5=lnx
更改FP16_FPU_test.c中的opcode值，然后运行run.bat会执行编译和运行两步生成新的txt文件
已有的test_add.txt等是我已经生成用于测试的文件
add、mcl、div覆盖为两个加数都是从[fbff,fb00],[80ff,00ff],[7b00,7bff]进行边界测试
ex和lnx为规格化合法的全覆盖