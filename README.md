# zynq-device-driver-with-irq
Zynqで動作するLinuxのデバイスドライバでの、割込みの使用例です。  
  
### これは何ですか？ ###
Zynqで動作するLinuxのデバイスドライバのソースコードと、その生成物です。  
ZynqでのPL→PSの割込みの使用例が殆ど無かったので、GitHubで公開することにいたしました。  
　　
### License ###
GPL  
  
### 使用方法 ###
ソースコード自体はWeb上でもご覧になれます。もしローカルで保存する場合は、  
$git clone https://github.com/tokuden/zynq-device-driver-with-irq.git  
とするか、右にある"Download ZIP"ボタンを押してください。  
  
Makefileには、Linux kernelのソースを相対パスで指定するなど、環境に依存した部分があります。そこを各自編集してください。  
build.shも同様です。クロスコンパイラ(arm-xilinx-linux-gnueabi-など)を指定する必要があります。  
hello.cがデバイスドライバの主要な部分になります。こちらをぜひご参照ください。  
fpga_reg_util.cは、ioremap()などを利用した、AXIバス経由でのFPGA内のレジスタ読み書きなど、FPGAに依存した部分のコードになります。  
  