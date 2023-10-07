#include "myinclude.h"

KeyboardInput::KeyboardInput()
{
    // 获取当前终端属性
    tcgetattr(STDIN_FILENO, &oldt_);

    // 复制终端属性到新的变量
    newt_ = oldt_;

    // 关闭回显，即不显示输入的字符
    newt_.c_lflag &= ~(ICANON | ECHO);

    // 设置终端属性
    tcsetattr(STDIN_FILENO, TCSANOW, &newt_);

    // 设置文件描述符为非阻塞模式
    oldf_ = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf_ | O_NONBLOCK);
}

KeyboardInput::~KeyboardInput()
{
    // 恢复终端属性
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt_);
}

std::string KeyboardInput::readInput()
{
    std::string str;
    int ch;

    while (1)
    {
        // 清空str
        str.clear();
        ch = getchar();

        if (ch != EOF)
        {
            // 判断上下左右键
            if (ch == 'q')
            {
                str = "q";
                std::cout << "str: " << str << std::endl;
                break;
            }
            else if (ch == 'w')
            {
                str = "w";
                std::cout << "str: " << str << std::endl;
                break;
            }
            else if (ch == 'a')
            {
                str = "a";
                std::cout << "str: " << str << std::endl;
                break;
            }
            else if (ch == 's')
            {
                str = "s";
                std::cout << "str: " << str << std::endl;
                break;
            }
            else if (ch == 'd')
            {
                str = "d";
                std::cout << "str: " << str << std::endl;
                break;
            }
        }
        usleep(1000);
    }
    return str;
}

// int main()
// {
//     KeyboardInput keyboard;

//     std::string userInput = keyboard.readInput();
//     std::cout << "用户输入: " << userInput << std::endl;
//     int dir=userInput[0]-'0';
//     std::cout<<dir<<std::endl;

//     return 0;
// }