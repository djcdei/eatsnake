#include <iostream>
#include <cstdlib>
#include <ncurses.h>
#include <unistd.h>
#include <vector>
#include <ctime>

using namespace std;

bool gameover;
const int width = 20;
const int height = 10;
int x, y, fruitX, fruitY, score;
vector<int> tailX, tailY;
int nTail;
enum eDirection
{
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
};
eDirection dir;

void setup()
{
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);
    srand(time(NULL));
    gameover = false;
    dir = STOP;
    x = width / 2;
    y = height / 2;
    fruitX = rand() % width;
    fruitY = rand() % height;
    score = 0;
}

// 渲染游戏界面，包括墙、蛇、水果和得分
void draw()
{
    clear();
    for (int i = 0; i < width + 2; i++)
        mvprintw(0, i, "#");
    for (int i = 0; i < height + 2; i++)
    {
        for (int j = 0; j < width + 2; j++)
        {
            if (i == 0 || i == height + 1)
                mvprintw(i, j, "#");
            else if (j == 0 || j == width + 1)
                mvprintw(i, j, "#");
            else if (i == y && j == x)
                mvprintw(i, j, "O");
            else if (i == fruitY && j == fruitX)
                mvprintw(i, j, "F");
            else
            {
                bool printTail = false;
                for (int k = 0; k < nTail; k++)
                {
                    if (tailX[k] == j && tailY[k] == i)
                    {
                        mvprintw(i, j, "o");
                        printTail = true;
                    }
                }
                if (!printTail)
                    mvprintw(i, j, " ");
            }
        }
    }
    mvprintw(height + 4, 0, "Score: %d", score);
    refresh();
}

// 处理用户输入，根据按键设置蛇的移动方向
void input()
{
    keypad(stdscr, true);
    halfdelay(1);
    int ch = getch();
    switch (ch)
    {
    case KEY_LEFT:
        dir = LEFT;
        break;
    case KEY_RIGHT:
        dir = RIGHT;
        break;
    case KEY_UP:
        dir = UP;
        break;
    case KEY_DOWN:
        dir = DOWN;
        break;
    case 'q':
        gameover = true;
        break;
    }
}

// 处理游戏逻辑，包括蛇的移动、吃水果、碰撞检测等
void logic()
{
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x;
    tailY[0] = y;
    for (int i = 1; i < nTail; i++)
    {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }
    switch (dir)
    {
    case LEFT: // 左移
        x--;
        break;
    case RIGHT: // 右移
        x++;
        break;
    case UP: // 向上
        y--;
        break;
    case DOWN: // 向下
        y++;
        break;
    default:
        break;
    }
    if (x >= width)
        x = 0;
    else if (x < 0)
        x = width - 1;
    if (y >= height)
        y = 0;
    else if (y < 0)
        y = height - 1;
    for (int i = 0; i < nTail; i++)
    {
        if (tailX[i] == x && tailY[i] == y)
            gameover = true;
    }
    if (x == fruitX && y == fruitY)
    {
        score += 10;
        fruitX = rand() % width;
        fruitY = rand() % height;
        nTail++;
    }
}

int main()
{
    setup();
    while (!gameover)
    {
        draw();
        cout << "score1: " << score << endl;
        input();
        cout << "score2: " << score << endl;
        logic();
        cout << "score3: " << score << endl;
        usleep(100000);
        cout << "score4: " << score << endl;
    }
    endwin();
    return 0;
}
