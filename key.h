#ifndef __KEY_H_
#define __KEY_H_
#include <string>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

class KeyboardInput
{
public:
    KeyboardInput();
    ~KeyboardInput();
    std::string readInput(); // 获取键盘信息成员函数

private:
    struct termios oldt_, newt_;
    int oldf_;
};

#endif // KEYBOARD_INPUT_H
