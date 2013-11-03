rm -rf /sdcard/trace.*
cp /sdcard/pl ./pl
cp /sdcard/targets.txt ./targets.txt
chmod 777 ./pl
ps > /sdcard/trace.ALL
./pl

