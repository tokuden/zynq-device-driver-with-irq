ビルドする場合は、
$./build.sh
です。Linux kernelのsourceへのパスは、環境によって違うと思います。適宜変更してください。

ZedBoardのLinuxで、ドライバを起動するためには、以下のようなコマンドを打つ必要があります。
$mkdir sd
$mount /dev/mmcblk0p1 /sd
$insmod /sd/driver.ko