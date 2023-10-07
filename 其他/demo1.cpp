/*用来测试showBmp函数有没有用的*/

#include <iostream>
#include <cstdio>
#include "lcd.h"
int main(int argc, char **argv)
{
    std::string bmp_file = argv[1];

    Lcd lcd;
    lcd.lcd_open("/dev/fb0");
    lcd.lcd_clear(0);
    lcd.showBmp(bmp_file, 0, 0);
    lcd.lcd_close();
    return 0;
}