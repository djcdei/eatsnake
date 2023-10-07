#include "myinclude.h"

using namespace std;

static font *g_font; // 字体文件描述符

// 构造函数
Lcd::Lcd() : g_pfd(nullptr), g_fd_lcd(-1) {}

// 析构函数
Lcd::~Lcd()
{
    std::cout << "lcd析构函数" << std::endl;
    if (g_fd_lcd > 0)
    {
        close(g_fd_lcd);
        std::cout << "lcd关闭成功" << std::endl;
    }

    if (g_pfd)
    {
        munmap(g_pfd, 800 * 480 * 4);
        std::cout << "munmap成功" << std::endl;
    }
}
// 打开LCD屏幕
void Lcd::lcd_open(const string &LcdName)
{
    this->g_fd_lcd = open(LcdName.c_str(), O_RDWR);
    if (this->g_fd_lcd < 0)
    {
        cout << "打开lcd失败" << endl;
        return;
    }
    // 将设备文件/dev/fb0映射到内存
    this->g_pfd = (unsigned int *)mmap(NULL,                   // 映射区的起始地址由系统自动分配
                                       800 * 480 * 4,          // 映射内存的大小，往往填写文件的大小
                                       PROT_READ | PROT_WRITE, // 映射区的保护形式，当前是可读可写
                                       MAP_SHARED,             // 共享，把映射内存的数据同步到文件
                                       g_fd_lcd,               // 映射的文件描述符
                                       0);
    if (g_pfd == MAP_FAILED)
    {
        printf("mmap fail\n");
        return;
    }
    printf("打开lcd成功\n");
    return; // 文件的偏移量，0就不需要进行的偏移
}

// 描点操作
void Lcd::lcd_draw_point(int x, int y, unsigned int color)
{
    *(g_pfd + y * 800 + x) = color;
}

// 填充
void Lcd::lcd_fill(int x, int y, int width, int height, unsigned color)
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

// 清屏操作
void Lcd::lcd_clear(unsigned int color)
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

// 关闭lcd
int Lcd::lcd_close(void)
{
    if (g_fd_lcd > 0)
        close(g_fd_lcd);

    // 解除内存映射
    if (g_pfd)
        munmap(g_pfd, 800 * 480 * 4);
    return 0;
}

// bmp图片的显示
void Lcd::showBmp(const std::string &fileName, int _x, int _y)
{
    // std::cout << "Bmpname:" << fileName << std::endl;
    std::ifstream file;
    file.open(fileName, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "打开文件失败" << std::endl;
        return;
    }
    // 读取BMP文件头
    char header[54];
    file.read(header, 54);

    // 解析BMP文件头
    int width = *(int *)&header[18];
    int height = *(int *)&header[22];
    int dataSize = *(int *)&header[34];
    // std::cout << "width:" << width << std::endl;
    //  std::cout << "height:" << height << std::endl;
    //  std::cout << "dataSite:" << dataSize << std::endl;

    // 计算每行像素数据的字节数
    int rowSize = (width * 3 + 3) & ~3; // 对齐到4字节边界

    // 分配内存来存储像素数据
    std::vector<char> pixelData(dataSize);
    file.read(pixelData.data(), dataSize);

    // 处理图像数据，这里可以根据需要进行显示或处理
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixelOffset = (height - y) * rowSize + x * 3; // 从数组最后往前打印，避免图片打反了
            int blue = pixelData[pixelOffset];
            int green = pixelData[pixelOffset + 1];
            int red = pixelData[pixelOffset + 2];
            int color = (red << 16) | (green << 8) | blue;
            lcd_draw_point(x + _x, y + _y, color);
        }
    }

    // 关闭文件
    file.close();
    // 打印宽高
    // cout << dec << "width: " << width << " height: " << height << endl;
}

// 封装在任意位置显示任意大小的bmp图片
/*
    w是图片实际的宽
    h是图片实际的高
    x是图片左上角显示的位置坐标
    y是图片左上角显示的位置坐标
*/

int Lcd::show_anybmp(int w, int h, int x, int y, char *bmpname)
{
    int bmpfd; // bmp图片的文件描述符
    int i, j;

    // 定义数组存放像素点的RGB
    char bmpbuf[w * h * 3];
    // 定义数组存放转换得到的ARGB
    int lcdbuf[w * h]; // int占4字节
    int tempbuf[w * h];
    // 打开你要显示的bmp图片   w*h
    bmpfd = open(bmpname, O_RDWR);
    if (bmpfd == -1)
    {
        perror("打开图片");
        return -1;
    }

    // 跳过前面没有用的54字节
    lseek(bmpfd, 54, SEEK_SET);

    // 判断bmp图片的宽所占的字节数能否被4整除
    if ((w * 3) % 4 != 0)
    {
        for (i = 0; i < h; i++)
        {
            read(bmpfd, &bmpbuf[i * w * 3], w * 3);
            lseek(bmpfd, 4 - (w * 3) % 4, SEEK_CUR); // 跳过填充的垃圾数据
        }
    }
    else
        // 从55字节读取bmp的像素点颜色值
        read(bmpfd, bmpbuf, w * h * 3); // bmpbuf[0] B  bmpbuf[1] G bmpbuf[2] R  一个像素点的RGB
                                        // bmpbuf[3]  bmpbuf[4]  bmpbuf[5]
    // 3字节的RGB-->4字节的ARGB   位运算+左移操作
    for (i = 0; i < w * h; i++)
        lcdbuf[i] = bmpbuf[3 * i] | bmpbuf[3 * i + 1] << 8 | bmpbuf[3 * i + 2] << 16 | 0x00 << 24;
    // 00[2][1][0]

    for (i = 0; i < w; i++)
        for (j = 0; j < h; j++)
            //*(lcdmem+(y+j)*800+x+i)=lcdbuf[j*w+i];  图片颠倒
            //  *(lcdmem + (y + j) * 800 + x + i) = lcdbuf[(h - 1 - j) * w + i];

            lcd_draw_point(i, j, lcdbuf[(h - 1 - j) * w + i]);
    // 关闭
    close(bmpfd);
    lcd_close();

    return 0;
}

// 获点操作
unsigned int Lcd::lcd_read_point(int x, int y)
{
    return *(g_pfd + y * 800 + x);
}

// 选择字体
int Lcd::lcd_font_select(const char *pathname)
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

void getColors(int value, int *a, int *b, int *c, int *d)
{
    *a = (value & 0xFF);
    *b = ((value >> 8) & 0xFF);
    *c = ((value >> 16) & 0xFF);
    *d = ((value >> 24) & 0xFF);
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

// 在lcd上显示要显示的文字
int Lcd::lcd_draw_string(const char *str, int x, int y, unsigned int font_color, unsigned int font_size)
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
   // printf("[lcd_draw_string] len = %d\n", len);
    //printf("[lcd_draw_string] cn_total = %d, en_total=%d\n", cn_total, en_total);

    // 字体大小的设置
    fontSetSize(g_font, font_size);

    unsigned int bitmap_draw_board_width = cn_total * font_size + en_total * font_size / 2;
    unsigned int bitmap_draw_board_height = font_size;

   // printf("[lcd_draw_string] bitmap_draw_board_width=%d\n", bitmap_draw_board_width);
   // printf("[lcd_draw_string] bitmap_draw_board_height=%d\n", bitmap_draw_board_height);

    // 创建一个画板（点阵图）
    // 画板宽、高，色深32位色即4字节
    // bitmap *bm = createBitmapWithInit(bitmap_draw_board_width,
    //                           bitmap_draw_board_height,
    //                           4,board_color);
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
    // 0,0:字体在画板中的起始位置,默认为0
    // str:显示的内容
    // getColor(0,100,100,100):字体颜色
    // 默认为0
    fontPrint(g_font, bm, 0, 0, (char *)str, font_color, 0);

    // 把字体框输出到LCD屏幕上
    // g_pfd:mmap后内存映射首地址
    // x,y:画板显示的起始位置
    // bm:画板
    show_font_to_lcd(g_pfd, x, y, bm);

    // 销毁画板
    destroyBitmap(bm);

    return 0;
}


