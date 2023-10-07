#include <iostream>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "myinclude.h"
#define MAX 10
/*精灵类*/

// class Sprite
// {
// public:
//     Sprite() : Sprite(0, 0){};
//     Sprite(int _x, int _y, int _color = 0x0000ff)
//     {
//         this->x = _x;
//         this->y = _y;
//         this->color = _color;
//     }

// protected:
//     int x;
//     int y;
//     int color;
// };

// // 定义一个蛇对象，继承于Sprite
// class Snake : public Sprite
// {
// public:
//     Snake(int x = 0, int y = 0) : Sprite(x, y) // 使用默认参数，当没有参数传进来的时候就是默认构造函数
//     {

//     }

//     // 蛇的身体移动
//     void BodyMove(int _x)
//     {
//         std::cout << "移动" <<std::endl;
//         x = _x;
//     }

// private:
//     std::vector<Sprite> nodes; // 蛇的所有节点
// };

// struct A
// {
//     char t : 4;
//     char k : 4;
//     unsigned short i : 8;
//     unsigned long m;
// };

// int main()
// {
//     //printf("sizeof(A):%d\n", sizeof(A));
//     unsigned char A[MAX],i;
//     for(i=0;i<=MAX;i++)
//     {printf("i:%d\n",i);
//     sleep(2);
//         A[i]=i;
//     }
// }

void GetMemory(char **p, int num)
{
    *p = (char *)malloc(num);
}

void Foo(char str[100])
{
    std::cout << sizeof(str) << std::endl;
}
int main()
{
    // char *str = NULL;
    // GetMemory(&str, 100);
    // strcpy(str, "hellodfsf");
    // printf("str:%s\n", str);
    // printf(str);
    // // printf("sizeof(str):%d\n",str);

    char str[] = "raysharp.cn";
    char *p = str;
    int n = 10;
    std::cout << sizeof(str) << std::endl;
    std::cout << sizeof(p) << std::endl;
    std::cout << sizeof(n) << std::endl;

    char a[100] = {0};
    Foo(a);

    void *p2 = malloc(100);
    std::cout << sizeof(p2) << std::endl;


    return 0;
}
