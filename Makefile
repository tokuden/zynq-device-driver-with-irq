# Linux kernelのソースコードのパスを指定する。コンパイルにはソースコードが必要なので。
# for Ubuntu x64
# KERNEL_SRC=/lib/modules/${shell uname -r}/build

# for ARM(Zynq)
KERNEL_SRC=/home/tokuden/linux-digilent/

CFILES = hello.c fpga_reg_util.c

# 作成したいモジュール名
obj-m := driver.o

# <作成したいモジュール名>-objsに、モジュールを構成するオブジェクトの一覧を列挙する
driver-objs := hello.o fpga_reg_util.o

# Compile-time flags

all:
	make -C $(KERNEL_SRC) ARCH=arm M=$(PWD) V=1 modules
clean:
	make -C $(KERNEL_SRC) M=$(PWD) V=1 clean
