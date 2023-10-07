default:
	arm-linux-gcc  test.c lcd.c  -o test.out  -L./ -lfont  -lm
	sshpass -p "djc1234" scp /mnt/hgfs/Win_Linuxshare/class/C++/贪吃蛇/font_test/*.out root@192.168.0.167:/djc/snake/ 
clean:
	rm -r *.out