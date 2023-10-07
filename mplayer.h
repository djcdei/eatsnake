#ifndef __MPLAYER_H_
#define __MPLAYER_H_

#define MPLAYER_PATH "/djc/snake/fifo"
#include <stdio.h>
class MusicPlayer
{
private:
    int m_processID; // 进程ID
    int m_fifoFD;    // FIFO文件描述符
    std::string m_musicPath;
    std::thread m_musicThread;
bool m_playing;

public:
    MusicPlayer();

    ~MusicPlayer();

    void initMPlayer(); // 初始化mplayer管道
    void sendCommand(const char *cmd);
    void playMusic(const std::string &musicFile);
   
    void switchMusic(const std::string &musicFile);
    void playMusicThread(std::string command);
    void stopMusicThread();
    void stopMusic();
};

#endif
