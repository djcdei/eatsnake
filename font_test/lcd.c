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

char color_buf[800 * 480 * 4] = {0};

unsigned int *g_fb = NULL;

int g_fd_lcd;

font *g_font = NULL;

// 描点操作
void lcd_draw_point(int x, int y, unsigned int color)
{
    *(g_fb + y * 800 + x) = color;
}

// 获点操作
unsigned int lcd_read_point(int x, int y)
{
    return *(g_fb + y * 800 + x);
}

// 清屏操作
void lcd_clear(unsigned int color)
{
    int x, y;

    for (y = 479; y >= 0; y--)
    {
        for (x = 0; x < 800; x++)
        {
            // 对每个像素点进行赋值
            lcd_draw_point(x, y, color);
        }
    }
}

// 填充函数
void lcd_fill(int x, int y, int width, int height, unsigned int color)
{
    int x_s = x;
    int y_s = y;
    int x_e = x + width;
    int y_e = y + height;

    if (x_e > LCD_WIDTH)
        x_e = LCD_WIDTH;

    if (y_e > LCD_HEIGHT)
        y_e = LCD_HEIGHT;

    for (y = y_s; y < y_e; y++)
    {
        for (x = x_s; x < x_e; x++)
        {
            // 对每个像素点进行赋值
            lcd_draw_point(x, y, color);
        }
    }
}

// 位图显示
int lcd_draw_bmp(const char *pathname, int x, int y)
{

    // 打开4.bmp
    int fd_bmp = open(pathname, O_RDONLY);

    if (fd_bmp < 0)
    {
        printf("open %s fail\n", pathname);
        return -1;
    }

    // 位图文件头
    BITMAPFILEHEADER file_head;
    read(fd_bmp, &file_head, sizeof(file_head));
    printf("%s图片大小:%d bytes\n", pathname, file_head.bfSize);

    // 位图信息段，获取位图的高度、宽度、颜色深度
    BITMAPINFOHEADER info_head;
    read(fd_bmp, &info_head, sizeof(info_head));
    printf("%s图片尺寸:宽%ld 高%ld\n", pathname, info_head.biWidth, info_head.biHeight);
    printf("%s图片颜色深度:%d\n", pathname, info_head.biBitCount);

    int bmp_rgb_size = info_head.biWidth * info_head.biHeight * info_head.biBitCount / 8;

    // 定义一个变长数组
    char bmp_buf[bmp_rgb_size];

    // 读取该bmp的所有RGB的数据
    read(fd_bmp, bmp_buf, bmp_rgb_size);

    // 关闭bmp文件
    close(fd_bmp);

    // 开始显示图片
    int x_s = x;
    int y_s = y;
    int x_e = x_s + info_head.biWidth;
    int y_e = y_s + info_head.biHeight;

    int x_pos, y_pos;

    unsigned int color;
    int i = 0;

    for (y_pos = y_e - 1; y_pos >= y_s; y_pos--)
    {
        for (x_pos = x_s; x_pos < x_e; x_pos++, i += 3)
        {

            color = (bmp_buf[i + 2] << 16) | (bmp_buf[i + 1] << 8) | bmp_buf[i];
            lcd_draw_point(x_pos, y_pos, color);
        }
    }

    return 0;
}

int lcd_draw_pokemon(const char *pathname, int x, int y)
{

    int fd_bmp = open(pathname, O_RDONLY);

    if (fd_bmp < 0)
    {
        printf("open %s fail\n", pathname);
        return -1;
    }

    // 位图文件头
    BITMAPFILEHEADER file_head;
    read(fd_bmp, &file_head, sizeof(file_head));
    printf("%s图片大小:%d bytes\n", pathname, file_head.bfSize);

    // 位图信息段，获取位图的高度、宽度、颜色深度
    BITMAPINFOHEADER info_head;
    read(fd_bmp, &info_head, sizeof(info_head));
    printf("%s图片尺寸:宽%ld 高%ld\n", pathname, info_head.biWidth, info_head.biHeight);
    printf("%s图片颜色深度:%d\n", pathname, info_head.biBitCount);

    int bmp_rgb_size = info_head.biWidth * info_head.biHeight * info_head.biBitCount / 8;

    // 定义一个变长数组
    char bmp_buf[bmp_rgb_size];

    // 读取该bmp的所有RGB的数据
    read(fd_bmp, bmp_buf, bmp_rgb_size);

    // 关闭bmp文件
    close(fd_bmp);

    // 开始显示图片
    int x_s = x;
    int y_s = y;
    int x_e = x_s + info_head.biWidth;
    int y_e = y_s + info_head.biHeight;

    int x_pos, y_pos;

    unsigned int color;
    int i = 0;

    for (y_pos = y_e - 1; y_pos >= y_s; y_pos--)
    {
        for (x_pos = x_s; x_pos < x_e; x_pos++, i += 3)
        {

            color = (bmp_buf[i + 2] << 16) | (bmp_buf[i + 1] << 8) | bmp_buf[i];

            if (color >= 0xF10000)
                continue;

            lcd_draw_point(x_pos, y_pos, color);
        }
    }

    return 0;
}

int lcd_font_select(const char *pathname)
{
    if (pathname == NULL)
    {
        printf("字体路径参数为NULL\n");

        return -1;
    }

    if (access(pathname, F_OK) == -1)
    {
        printf("%s 不存在\n", pathname);

        return -1;
    }

    // 若以前加载过字体，卸载字体
    if (g_font)
    {
        fontUnload(g_font);
        g_font = NULL;
    }

    // 加载字体
    g_font = fontLoad((char *)pathname);

    if (g_font == NULL)
    {
        printf("加载 %s 失败\n", pathname);

        return -1;
    }

    return 0;
}

int is_utf8_chinese(char ch)
{
    return (ch & 0xE0) == 0xE0; // 判断是否是UTF-8中文字符的第一个字节
}

int is_utf8_english(char ch)
{
    return (ch & 0x80) == 0x00; // 判断是否是UTF-8英文字符的字节
}

int utf8_strlen(const char *str, unsigned int *cn_total, unsigned int *en_total)
{
    int len = 0;
    int i = 0;

    while (str[i] != '\0')
    {
        if (is_utf8_chinese(str[i]))
        {
            len += 3; // 中文字符占三个字节
            i += 3;
            (*cn_total)++;
        }
        else if (is_utf8_english(str[i]))
        {
            len++; // 英文字符占一个字节
            i++;
            (*en_total)++;
        }
    }

    return len;
}

void getColors(int value, int *a, int *b, int *c, int *d)
{
    *a = (value & 0xFF);
    *b = ((value >> 8) & 0xFF);
    *c = ((value >> 16) & 0xFF);
    *d = ((value >> 24) & 0xFF);
}

int lcd_draw_string(const char *str,
                    int x,                   // 画板起始位置x坐标
                    int y,                   // 画板起始位置y坐标
                    unsigned int font_color, // 字体颜色

                    unsigned int font_size) // 字体大小
{
    if (g_font == NULL)
    {
        printf("目前还没有加载字体，无法显示...\n");
        return -1;
    }

    unsigned int cn_total = 0, en_total = 0;

    int a, b, g, r;
    getColors(font_color, &a, &b, &g, &r);

    // 获取汉字数量、英文及符号等数量
    int len = utf8_strlen(str, &cn_total, &en_total);
    printf("[lcd_draw_string] len = %d\n", len);
    printf("[lcd_draw_string] cn_total = %d, en_total=%d\n", cn_total, en_total);

    // 字体大小的设置
    fontSetSize(g_font, font_size);

    unsigned int bitmap_draw_board_width = cn_total * font_size + en_total * font_size / 2;
    unsigned int bitmap_draw_board_height = font_size;

    printf("[lcd_draw_string] bitmap_draw_board_width=%d\n", bitmap_draw_board_width);
    printf("[lcd_draw_string] bitmap_draw_board_height=%d\n", bitmap_draw_board_height);

    // 创建一个画板（点阵图）
    // 画板宽、高，色深32位色即4字节
    bitmap *bm = createBitmap(bitmap_draw_board_width,
                              bitmap_draw_board_height,
                              4);

    // 设置画板底色
    int bm_x_s = x, bm_y_s = y;
    int bm_x_e = x + bitmap_draw_board_width;
    int bm_y_e = y + bitmap_draw_board_height;
    int bm_x, bm_y;
    unsigned int *p = (unsigned int *)bm->map;

    // 获取指定区域的颜色块

    for (bm_y = bm_y_s; bm_y < bm_y_e; bm_y++)
    {
        for (bm_x = bm_x_s; bm_x < bm_x_e; bm_x++)
        {
            *p = lcd_read_point(bm_x, bm_y);

            p++;
        }
    }

    // 将字体写到点阵图上
    // f:操作的字库
    // 0,0:字体在画板中的起始位置
    // str:显示的内容
    // getColor(0,100,100,100):字体颜色
    // 默认为0
    fontPrint(g_font, bm, 0, 0, (char *)str, getColor(a, b, g, r), 0);
    printf("a: %d, b: %d, g: %d, r: %d\n", a, b, g, r);

    // 把字体框输出到LCD屏幕上
    // g_fb:mmap后内存映射首地址
    // x,y:画板显示的起始位置
    // bm:画板
    show_font_to_lcd(g_fb, x, y, bm);

    // 销毁画板
    destroyBitmap(bm);

    return 0;
}

// 滚动文本显示
// 定义两个位图，用于双缓冲
bitmap *bitmap1 = NULL;
bitmap *bitmap2 = NULL;

// 标志以跟踪活动位图
int activeBitmap = 1; // 1 代表 bitmap1，2 代表 bitmap2

// 在非活动位图上绘制文本的函数
void drawTextOnInactiveBitmap(const char *str, int x, int y, unsigned int font_color, unsigned int font_size)
{
    int a, b, g, r;
    getColors(font_color, &a, &b, &g, &r);
    // 清除非活动位图（bitmap2）
    clearBitmap(bitmap2, 0);

    // 在非活动位图（bitmap2）上绘制文本
    fontSetSize(g_font, font_size);
    fontPrint(g_font, bitmap2, x, y, (char *)str, getColor(a, b, g, r), 0);
}

// 交换活动和非活动位图的函数
void swapBitmaps()
{
    if (activeBitmap == 1)
    {
        activeBitmap = 2;
    }
    else
    {
        activeBitmap = 1;
    }
}

// 滚动文本的主循环
void scrollText(const char *str, int x, int y, unsigned int font_color, unsigned int font_size)
{

    if (g_font == NULL)
    {
        printf("目前还没有加载字体，无法显示...\n");
        return -1;
    }

    unsigned int cn_total = 0, en_total = 0;

    int a, b, g, r;
    getColors(font_color, &a, &b, &g, &r);

    // 获取汉字数量、英文及符号等数量
    int len = utf8_strlen(str, &cn_total, &en_total);
    printf("[lcd_draw_string] len = %d\n", len);
    printf("[lcd_draw_string] cn_total = %d, en_total=%d\n", cn_total, en_total);

    // 字体大小的设置
    fontSetSize(g_font, font_size);

    unsigned int bitmap_draw_board_width = cn_total * font_size + en_total * font_size / 2;
    unsigned int bitmap_draw_board_height = font_size;

    printf("[lcd_draw_string] bitmap_draw_board_width=%d\n", bitmap_draw_board_width);
    printf("[lcd_draw_string] bitmap_draw_board_height=%d\n", bitmap_draw_board_height);

    // 使用屏幕尺寸初始化位图
    bitmap1 = createBitmap(bitmap_draw_board_width, bitmap_draw_board_height, 4);
    bitmap2 = createBitmap(bitmap_draw_board_width, bitmap_draw_board_height, 4);

    int scrollPosition = 0;
    int scrollSpeed = 2; // 根据需要调整滚动速度

    while (1)
    {
        // 在非活动位图上绘制文本
        drawTextOnInactiveBitmap(str, x - scrollPosition, y, font_color, font_size);

        // 交换位图
        swapBitmaps();

        // 在屏幕上显示活动位图
        show_bitmap_to_lcd(g_fb, activeBitmap == 1 ? bitmap1 : bitmap2);

        // 等待或延迟以控制滚动速度
        usleep(50000); // 根据需要调整延迟

        // 更新滚动位置
        scrollPosition += scrollSpeed;
        if (scrollPosition >= bitmap_draw_board_width)
        {
            scrollPosition = 0;
        }
    }
}

int lcd_open(const char *pathname)
{
    // 打开/dev/fb0
    int g_fd_lcd = open(pathname, O_RDWR);

    if (g_fd_lcd < 0)
    {
        printf("open %s fail\n", pathname);
        return -1;
    }

    // 将设备文件/dev/fb0映射到内存
    g_fb = mmap(NULL,                   // 映射区的起始地址由系统自动分配
                800 * 480 * 4,          // 映射内存的大小，往往填写文件的大小
                PROT_READ | PROT_WRITE, // 映射区的保护形式，当前是可读可写
                MAP_SHARED,             // 共享，把映射内存的数据同步到文件
                g_fd_lcd,               // 映射的文件描述符
                0);                     // 文件的偏移量，0就不需要进行的偏移

    if (g_fb == MAP_FAILED)
    {
        printf("mmap fail\n");
        return -1;
    }

    return 0;
}

int lcd_close(void)
{
    if (g_fd_lcd > 0)
        close(g_fd_lcd);

    // 解除内存映射
    if (g_fb)
    {
        munmap(g_fb, 800 * 480 * 4);
        g_fb = NULL;
    }

    // 若以前加载过字体，卸载字体
    if (g_font)
    {
        fontUnload(g_font);
        g_font = NULL;
    }

    return 0;
}
