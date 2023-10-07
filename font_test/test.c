#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "font.h"
#include "lcd.h"

unsigned int color_buf[800 * 480];
// 总体来说，这个程序的作用是在LCD屏幕上显示一个不断变化的字符串，\
同时保留部分屏幕的背景色，以便在更新文本时恢复背景，以实现滚动文本的效果。

// 显示文字内容
void showfont(const char *buf, int x_pos, int y_pos, unsigned int font_color, unsigned int font_size)
{ // 选择字体
	lcd_font_select("/usr/share/fonts/simkai.ttf");

	// 保存好背景色
	int x, y;
	unsigned int *p = color_buf;
	for (y = y_pos; y < y_pos + font_size; y++)
	{
		for (x = x_pos; x < LCD_WIDTH; x++)
		{
			*p = lcd_read_point(x, y);
			p++;
		}
	}

	//  恢复背景色
	p = color_buf;
	// 更新背景颜色
	for (y = y_pos; y < y_pos + font_size; y++)
	{
		for (x = x_pos; x < LCD_WIDTH; x++)
		{
			lcd_draw_point(x, y, *p);
			p++;
		}
	}

	// getColor为abgr颜色格式填写
	lcd_draw_string(buf, x_pos, y_pos, font_color, font_size); // 绘制文字
															   // sleep(2);
}











int main(int argc, char **argv)
{
	char buf[64] = {0};

	unsigned int font_size = 32;

	unsigned int cnt = 0;

	// 打开lcd设备
	lcd_open("/dev/fb0");
	lcd_fill(0, 0, 800, 480, 0x00ff00);
	sprintf(buf, "段竞成");
	int x = 0, y = 0;
	for (int i = 0; i < 200; i++)
	{
		// 屏幕清屏
		//lcd_fill(x, 100, 100, 100, 0x00ff00);
		lcd_clear(0xffffff);
		// showfont(buf, x, 50, 0xff000000, font_size);
		lcd_font_select("/usr/share/fonts/simkai.ttf");
		lcd_draw_string(buf, x, 100, 0x0000ff00, font_size);
		x += 1;
		// usleep(100 * 300);
	}


	lcd_close();

	return 0;
}
