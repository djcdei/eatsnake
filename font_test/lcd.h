#ifndef __LCD_H__
#define __LCD_H__

#define LCD_WIDTH 800
#define LCD_HEIGHT 480
#define RGB(r, g, b) ((r << 16) | (g << 8) | b)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

typedef short WORD;
typedef int DWORD;
typedef long LONG;

typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;      // 位图文件的类型，必须为BM(1-2字节）
    DWORD bfSize;     // 位图文件的大小，以字节为单位（3-6字节，低位在前）【*】
    WORD bfReserved1; // 位图文件保留字，必须为0(7-8字节）
    WORD bfReserved2; // 位图文件保留字，必须为0(9-10字节）
    DWORD bfOffBits;  // 位图数据的起始位置，以相对于位图（11-14字节，低位在前）
                      // 文件头的偏移量表示，以字节为单位
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;    // 本结构所占用字节数（15-18字节）
    LONG biWidth;    // 位图的宽度，以像素为单位（19-22字节）【*】
    LONG biHeight;   // 位图的高度，以像素为单位（23-26字节）【*】
    WORD biPlanes;   // 目标设备的级别，必须为1(27-28字节）
    WORD biBitCount; // 每个像素所需的位数，必须是1（双色），（29-30字节）【*】
    // 4(16色），8(256色）16(高彩色)或24（真彩色）之一
    DWORD biCompression; // 位图压缩类型，必须是0（不压缩），（31-34字节）
    // 1(BI_RLE8压缩类型）或2(BI_RLE4压缩类型）之一
    DWORD biSizeImage;    // 位图的大小(其中包含了为了补齐列数是4的倍数而添加的空字节)，以字节为单位（35-38字节）
    LONG biXPelsPerMeter; // 位图水平分辨率，每米像素数（39-42字节）
    LONG biYPelsPerMeter; // 位图垂直分辨率，每米像素数（43-46字节)
    DWORD biClrUsed;      // 位图实际使用的颜色表中的颜色数（47-50字节）
    DWORD biClrImportant; // 位图显示过程中重要的颜色数（51-54字节）
} __attribute__((packed)) BITMAPINFOHEADER;

extern int lcd_open(const char *pathname);
extern int lcd_close(void);

extern int lcd_draw_pokemon(const char *pathname, int x, int y);
extern int lcd_draw_bmp(const char *pathname, int x, int y);
extern void lcd_fill(int x, int y, int width, int height, unsigned int color);
extern void lcd_clear(unsigned int color);
extern void lcd_draw_point(int x, int y, unsigned int color);
extern unsigned int lcd_read_point(int x, int y);
extern int lcd_font_select(const char *pathname);
extern int lcd_draw_string(const char *str,
                           int x,
                           int y,
                           unsigned int font_color,
                           unsigned int font_size);

extern void getColors(int value, int *a, int *b, int *c, int *d);

extern int g_fd_lcd;
extern unsigned int *g_fb;

int utf8_strlen(const char *str, unsigned int *cn_total, unsigned int *en_total);

#endif