#include "myinclude.h"

TouchScreen::TouchScreen() : fd_ts(-1) {}

TouchScreen::~TouchScreen()
{
    ts_close();
}

int TouchScreen::ts_open(const char *pathname)
{
    if (fd_ts < 0)
    {
        fd_ts = open(pathname, O_RDONLY);
        if (fd_ts < 0)
        {
            perror("open");
            std::cerr << "Failed to open touch screen" << std::endl;
            return -1;
        }
        std::cout << "Opened touch screen" << std::endl;
    }
    return 0;
}

int TouchScreen::ts_close()
{
    if (fd_ts >= 0)
    {
        close(fd_ts);
        fd_ts = -1;
        std::cout << "触摸屏关闭成功" << std::endl;
    }
    return 0;
}

/*读取触摸屏状态赋值给x,y*/
int TouchScreen::ts_read(int &x, int &y, int &sta)
{
    if (fd_ts < 0)
        return -1;

    struct input_event ie; // 定义触摸结构体
    while (1)
    {
        read(fd_ts, &ie, sizeof(ie)); // 读取触摸坐标，赋值给ie结构体
        // 绝对值坐标事件
        if (ie.type == EV_ABS)
        {
            if (ie.code == ABS_X)
            {
                x = ie.value; // 获取x坐标
            }
            if (ie.code == ABS_Y)
            {
                y = ie.value; // 获取y坐标
            }
        }

        if (ie.type == EV_KEY && ie.code == BTN_TOUCH)
        {
            sta = ie.value; // 获取按压事件，（按下为1，松开为0）
            break;
        }
    }
    return 0;
}
