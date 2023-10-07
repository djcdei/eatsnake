// 创建一个播放背景音乐类
#include "myinclude.h"


MPlayerController ::MPlayerController() : mplayer(NULL) {}

MPlayerController ::~MPlayerController()
{
    if (mplayer)
    {
        std::cout << "mplayer close" << std::endl;
        stop();
    }
}

void MPlayerController ::play(const char *filepath)
{
    mplayer = popen(("mplayer -slave -quiet " + std::string(filepath)).c_str(), "w"); // 创建一个子进程，用来运行mplayer，可以往mplayer写入命令进行音乐的控制
    if (!mplayer)
    {
        // handle error
        std::cout << "mplayer open error" << std::endl;
    }
}

void MPlayerController::changeSong(const char *filepath)
{
    // 先关闭当前的mplayer进程
    if (mplayer)
    {
        std::cout << "mplayer close" << std::endl;
        fputs("quit\n", mplayer);
        fflush(mplayer);
        pclose(mplayer);
        mplayer = NULL;
    }
    // 再打开新的mplayer进程来播放新的歌曲
    mplayer = popen(("mplayer -slave -quiet " + std::string(filepath)).c_str(), "w");
    if (!mplayer)
    {
        // handle error
        std::cout << "mplayer open error" << std::endl;
    }
}

void MPlayerController ::pause()
{
    if (mplayer)
    {
        std::cout << "pause" << std::endl;
        fputs("pause\n", mplayer);
        fflush(mplayer);
    }
}

void MPlayerController ::stop()
{
    if (mplayer)
    {
        std::cout << "stop" << std::endl;
        fputs("quit\n", mplayer);
        fflush(mplayer);
        pclose(mplayer);
        mplayer = NULL;
    }
}

void MPlayerController::close()
{
    if (mplayer)
    {
        stop();
        pclose(mplayer);
        mplayer = NULL;
    }
}
