#include "myinclude.h"

MusicPlayer::MusicPlayer()
    : m_processID(0), m_fifoFD(-1), m_musicPath("") {}

MusicPlayer::~MusicPlayer()
{
    if (m_fifoFD != -1)
    {
        std::cout << "close fifo" << std::endl;
        close(m_fifoFD);
    }
    if (m_processID != 0)
    {
        std::cout << "kill process" << std::endl;
        kill(m_processID, SIGKILL);
    }
}

void MusicPlayer::initMPlayer()
{
    if (access(MPLAYER_PATH, F_OK) == -1)
    {
        mkfifo(MPLAYER_PATH, 0777);
    }

    m_fifoFD = open(MPLAYER_PATH, O_RDWR);
    if (m_fifoFD == -1)
    {
        perror("open fifo error");
        exit(EXIT_FAILURE);
    }
}

void MusicPlayer::sendCommand(const char *cmd)
{
    if (write(m_fifoFD, cmd, strlen(cmd) + 1) == -1)
    {
        perror("write fifo error");
        exit(EXIT_FAILURE);
    }
    else
        std::cout << "send command: " << cmd << std::endl;
}

void MusicPlayer::switchMusic(const std::string &musicFile)
{
    playMusic(musicFile);
}

void MusicPlayer::playMusicThread(std::string command)
{
    std::cout << "音乐线程已开启" << std::endl;
    int result = system(command.c_str());
    if (result != 0)
    {
        perror("system");
        exit(EXIT_FAILURE); // Error exit
    }
}

void MusicPlayer::playMusic(const std::string &musicFile)
{
    initMPlayer();
    // 等待音乐播放线程结束
    if (m_musicThread.joinable())
    {
        m_musicThread.join();
    }
    // 如果正在播放音乐，先停止
    if (m_playing)
    {
        stopMusic();
    }

    m_musicPath = musicFile;
    std::ostringstream command_stream;
    // 切割字符串"/djc/snake/bgm/bgm1.mp3",切割最后一个/与倒数第二个/之间的字符，区分背景音乐和其他音效，音效只播放一次
    std::string s = m_musicPath;
    std::size_t lastSlash = s.find_last_of("/");
    std::size_t secondLastSlash = s.find_last_of("/", lastSlash - 1);

    std::string filename = s.substr(secondLastSlash + 1, lastSlash - secondLastSlash - 1);
    std::cout << "filename:" << filename << std::endl;
    // 判断filename是否与"audio"相等

    if (filename == "audio")
        command_stream
            << "killall -9 mplayer; "
            << "mplayer -slave -msglevel all=-1 -input file=/djc/snake/fifo -speed 1.5 " << m_musicPath << " &";
    else if (filename == "bgm")
        command_stream
            << "killall -9 mplayer; "
            << "mplayer -slave -msglevel all=-1 -input file=/djc/snake/fifo -loop 0 " << m_musicPath << " &";

    // 创建一个新线程来播放音乐
    m_musicThread = std::thread(&MusicPlayer::playMusicThread, this, command_stream.str());
    m_playing = true; // 标记为正在播放状态
}

void MusicPlayer::stopMusic()
{
    if (m_playing)
    {
        // 发送一个命令到mplayer的管道，来停止播放音乐
        system("killall -9 mplayer");
        m_playing = false; // 标记为停止播放状态
        std::cout << "音乐已停止" << std::endl;
    } // 等待音乐播放线程结束
    if (m_musicThread.joinable())
    {
        m_musicThread.join();
        std::cout << "播放音乐线程以结束" << std::endl;
    }
}
