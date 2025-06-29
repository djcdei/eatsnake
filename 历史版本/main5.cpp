#include "myinclude.h"

static pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int StartGameFlag = 0;             // 进入游戏开始界面标志
int StopGameFlag = 0;              // 暂停游戏标志,1表示暂停,0表示开始
int SetGameFlag = 0;               // 重新开始游戏标志位
int OverGameFlag = 0;              // 进入游戏结算界面标志
unsigned int color_buf[800 * 480]; // 保存屏幕像素点信息，用于字体显示

Lcd lcd; // lcd对象定义为全局变量，方便调用lcd相关的成员函数

// 获取当前背景颜色参数一：\
    字体显示的起始x坐标\
    参数二：字体显示的起始y坐标\
    参数三：字体显示大小
void getBackgroundColor(int x_pos, int y_pos, int font_size)
{
    // 保存好背景色
    int x, y;
    unsigned int *p = color_buf;
    for (y = y_pos; y < y_pos + font_size; y++)
    {
        for (x = x_pos; x < 550; x++)
        {
            *p = lcd.lcd_read_point(x, y);
            p++;
        }
    }
}
// 恢复背景色
void showBackgroundColor(int x_pos, int y_pos, int font_size)
{
    int x, y;
    unsigned int *p = color_buf;
    for (y = y_pos; y < y_pos + font_size; y++)
    {
        for (x = x_pos; x < 550; x++)
        {
            lcd.lcd_draw_point(x, y, *p);
            p++;
        }
    }
}

/*精灵类*/
class Sprite
{
public:
    Sprite() {}
    Sprite(int _x, int _y, int _color = 0x00ff00) : x(_x), y(_y), color(_color) {}

    virtual ~Sprite() { std::cout << "Sprite析构" << std::endl; }

    virtual void draw()
    {
        try // 限制游戏区域
        {
            if (x < 50 || y < 50 || x >= 515 || y >= 415)
            {
                /*抛出碰壁错误*/
                throw "碰壁！！游戏结束";
            }
        }
        catch (const char *e)
        {
            /*处理游戏结束错误*/
            std::cout << "错误消息: " << e << std::endl;
            std::cout << "进入游戏结算界面" << std::endl;
            lcd.showBmp("over1.bmp", 0, 0);     // 唤起游戏结算界面
            OverGameFlag = 1;                   // 进入游戏结算界面标志置1
            pthread_cond_wait(&cond1, &mutex1); // 利用条件变量cond1阻塞
            if (OverGameFlag == 0)
                return;

            // 这里可以放你的错误处理代码
        }
        //  std::cout << "绘制精灵" << std::endl;
        //   lcd.showBmp("snake.bmp", x, y); // 绘制蛇身节点
        lcd.lcd_fill(x, y, 35, 35, color);
    }

    void MoveBy(int dx, int dy) // 移动偏移量
    {
        x += dx;
        y += dy;
    }

    // 碰撞检测，检测当前对象跟另一个对象的坐标是否相等
    bool Collision(const Sprite &other)
    {
        // std::cout << "检测碰撞:" << x << " " << y << std::endl;
        return (this->x == other.x && this->y == other.y);
    }
    // 封装一个成员函数，用来获取当前蛇身节点的坐标，在清空蛇移动后最后一个节点的后一个节点很有用
    int getX() const { return x; }
    int getY() const { return y; }
    
    int SetNode(int _x, int _y)
    {
        x = _x;
        y = _y;
    }

protected:
    int x;
    int y;
    int color; // 节点颜色
};

/*食物*/
class Food : public Sprite
{
public:
    Food() : Sprite(0, 0)
    {
        // 初始化映射表
        scoreTocolor[10] = 0x0000ff;
        scoreTocolor[20] = 0x00ff00;
        scoreTocolor[30] = 0xff0000;
        // ...
        Change(); // 初始化游戏开始时食物的坐标位置
        score = 10;
    }

    void draw() // 绘制食物
    {
        /*随机绘制不同颜色的食物
        代表不同等级的积分（有加分和减分）
        创建一个map容器，存放不同随机数和积分的键值对*/
        // 使用积分作为键，从容器中查找颜色，并显示出来
        if (scoreTocolor.find(score) != scoreTocolor.end())
        {
            unsigned int foodcolor = scoreTocolor[score];
            lcd.lcd_fill(x, y, 35, 35, foodcolor);
        }
        else
        {
            // 处理找不到颜色对应积分的情况，可以设置默认积分
        }
    }
    // 改变食物位置
    void Change()
    {
        // 生成 x 坐标，确保是 35 的整数倍，并且在 (50, 500) 之间
        x = 50 + (rand() % 14) * 35;
        // 生成 y 坐标，确保是 35 的整数倍，并且在 (50, 400) 之间
        y = 50 + (rand() % 11) * 35;
        // 随机选择积分
        //  生成随机积分，其中1，2，3分别代表10，20，30分
        int randNum = rand() % 3 + 1;
        switch (randNum)
        {
        case 1:
            score = 10;
            break;
        case 2:
            score = 20;
            break;
        case 3:
            score = 30;
            break;
        default:
            score = 10; // 默认值
            break;
        }
    }

    // 获取食物的积分
    int getFoodScore()
    {
        return this->score;
    }

private:
    std::map<int, unsigned int> scoreTocolor; // 存放积分跟颜色的映射表，根据积分选择不同的颜色
    int score;
};

// 定义一个蛇对象，继承于Sprite
class Snake : public Sprite
{
public:
    Snake(int x = 0, int y = 0) : Sprite(x, y) // 使用默认参数，当没有参数传进来的时候就是默认构造函数
    {
        // 初始化3节蛇
        nodes.push_back(Sprite(120, 50, 0xff0000));
        nodes.push_back(Sprite(85, 50, 0x00ff00));
        nodes.push_back(Sprite(50, 50, 0x0000ff));
        this->dir = 0;      // 刚进入游戏蛇是停止状态
        this->tempdir = 52; // 点击开始运动按钮，蛇默认向右移动
        this->integral = 0;
    }
    ~Snake()
    {
        std::cout << "Snake析构,清空Sprite容器" << std::endl;
        nodes.clear();
    }
    // 蛇身增加
    void AddBody()
    {
        /*添加一个容器，存储若干个int类型颜色值，再通过随机数选出来，添加进蛇身容器，
        还可以根据不同颜色类型，添加不同等级的得分，（因为每次添加蛇身说明）*/
        nodes.push_back(Sprite(0, 0));
    }

    // 蛇身减少
    void DelBody()
    {
        nodes.pop_back();
    }

    // 蛇的复位
    void Reset()
    { // 先判断节点还有没有
        if (!nodes.empty())
        {
            nodes.clear();
            nodes.push_back(Sprite(120, 50, 0xff0000));
            nodes.push_back(Sprite(85, 50, 0x00ff00));
            nodes.push_back(Sprite(50, 50, 0x0000ff));
            dir = 0;
            tempdir = 52;
        }
        else
        {
            std::cout << "容器为空，无法复位" << std::endl;
        }
    }

    // 获取头节点x坐标
    int getFirstNodeX()
    {
        return nodes.front().getX();
    }
    // 获取头节点y坐标
    int getFirstNodeY()
    {
        return nodes.front().getY();
    }

    // 获取最后个节点x坐标
    int getLastNodeX()
    {
        return nodes.back().getX();
    }

    // 获取最后一个节点的y坐标
    int getLastNodeY()
    {
        return nodes.back().getY();
    }

    // 获取积分
    int getSnakeScore()
    {
        return this->integral;
    }
    // 设置积分
    void setSnakeScore(int _score)
    {
        this->integral += _score;
    }
    void draw() // 把蛇所有的节点绘制出来
    {
        int i = 0;
        // 先判断容器是否为空
        if (!nodes.empty())
        {
            this->lastNode = nodes.at(nodes.size() - 1);

            for (auto &node : nodes)
            {
                node.draw();
                std::cout << "当前绘制第" << i++ << "个节点" << std::endl;
                // sleep(2);
            }

            usleep(1000 * 300);

            // 在这里加延时，刷出最新的蛇后延时一段时间再覆盖尾巴的后一个节点，解决了一个小bug，如果不延时的话，移动刷出的节点马上就被覆盖了，特别是刚开始只有一个节点的时候
            if ((dir == 71 || dir == 67 || dir == 49 || dir == 52) && StopGameFlag == 1) // 限制在运动的时候才清空最后一个节点
            {
                // std::cout << "ClearLastNode" << std::endl;
                lcd.lcd_fill(lastNode.getX(), lastNode.getY(), 35, 35, 0xffffff);
            }
        }
        else
        {
            //  std::cout << "vector<Sprite> nodes 容器为空" << std::endl;
        }
    }
    // 蛇的身体移动
    void BodyMove()
    {
        if ((dir == 71 || dir == 67 || dir == 49 || dir == 52)) // 只有蛇在动的时候节点才需要往前移动
        {
            for (int i = nodes.size() - 1; i > 0; i--)
            {
                // 最后一节等于前一节，所有节点往前移动
                int x = nodes.at(i - 1).getX();
                int y = nodes.at(i - 1).getY();
                nodes.at(i).SetNode(x, y);
            }
        }

        // 改变方向
        switch (dir)
        {
        case 71: // 上
            nodes[0].MoveBy(0, -35);

            break;
        case 67: // 下
            nodes[0].MoveBy(0, 35);

            break;
        case 49: // 左
            nodes[0].MoveBy(-35, 0);

            break;
        case 52: // 右
            nodes[0].MoveBy(35, 0);
            break;
        default:
            break;
        }
    }

    bool Collision(const Food &other) // 注意传的是Food参数
    {
        // 检查蛇头是否跟食物碰撞
        return (nodes[0].Collision(other)); // 一环扣一环属实666
    }

    // 清空最后一个节点
    void ClearLastNode()
    {
    }

public:
    int dir;     // 蛇的方向
    int tempdir; // 临时方向，在蛇暂停的时候保存当前方向

private:
    std::vector<Sprite> nodes; // 蛇的所有节点
    Sprite lastNode;
    int integral; // 保存积分
};

// 设置游戏场景
class GameScene
{
public:
    GameScene()
    {
    }
    ~GameScene()
    {
        std::cout << "GameScene析构" << std::endl;
    }

    // 游戏运行
    void run()
    {
        food.draw();      // 绘制食物
        snake.draw();     // 绘制蛇身
        SnakeEatFood();   // 吃食物，判断完吃食物后马上所有节点要前移，不吃完食物添加的那一节节点默认的坐标是0,0会刷到外面去出现Bug
        snake.BodyMove(); // 移动蛇
        // 获取蛇头的x,y坐标，写进buf缓冲区
        sprintf(buf, "X:%d Y:%d 积分:%d", snake.getFirstNodeX(), snake.getFirstNodeY(), snake.getSnakeScore());
        std::cout << "buf:" << buf << std::endl;
        showBackgroundColor(50, 0, 32);
        lcd.lcd_draw_string(buf, 50, 0, 0x0000ff00, 32);
    }

    Snake &getSnake() // 获取当前snake成员对象
    {
        return snake;
    }

    // 判断碰撞
    void SnakeEatFood()
    {
        if (snake.Collision(food))
        {
            std::cout << "吃到食物" << std::endl;
            snake.setSnakeScore(food.getFoodScore()); // 这里要先把当前食物的积分加上去，不能放在后面，加完之后食物被刷新了积分也会刷新·
                                                      // std::cout << "吃到积分为:" << food.getFoodScore() << std::endl;
            snake.AddBody();
            food.Change();
        }
    }

private:
    Snake snake;  // 把蛇放到这个场景
    Food food;    // 把食物放进这个场景
    char buf[64]; // 文字缓冲区
};

// 开启一个线程，获取键盘信息(按照这个模式，再添加一个线程获取lcd屏幕点击信息，实现触屏、按键双重控制)
class KeyPressed
{
public:
    KeyPressed(Snake &snake) : snake_(snake), shouldExit(false)
    {
        std::thread t(&KeyPressed::run, this);
        t.detach();
    }
    ~KeyPressed()
    {
        std::cout << "键盘监听线程析构了" << std::endl;
        shouldExit = true;
    }

    void run()
    {
        KeyboardInput keyboard;
        while (!shouldExit)
        {
            // std::cout << "开始监听" << std::endl;
            std::string userInput = keyboard.readInput(); // 没按下对应条件的按键就阻塞在这里
            switch (userInput.at(0) - '0')
            {
            case 71: // 上'w'
                if (snake_.dir != 67)
                    snake_.dir = 71;
                break;
            case 67: // 下's'
                if (snake_.dir != 71)
                    snake_.dir = 67;
                break;
            case 49: // 左'a'
                if (snake_.dir != 52)
                    snake_.dir = 49;
                break;
            case 52: // 右'd'
                if (snake_.dir != 49)
                    snake_.dir = 52;
                break;
            case 65: // 退出'q'
                snake_.dir = 65;
            default:
                break;
            }
            if (snake_.dir == 65)
            {
                std::cout << "退出键盘监听循环" << std::endl;
                shouldExit = true;
                break;
            }
            usleep(1000 * 100);
        }
        return;
    }

private:
    Snake &snake_; // 这里不再继承Snake类，直接把需要移动的Snake对象引用传进来，再去更改这个对象的方向变量dir
    bool shouldExit;
};

// 主线程，获取触摸屏触摸信息
class TatcheInput
{
public:
    TatcheInput()
    {
        ts.ts_open("/dev/input/event0");
    }
    ~TatcheInput()
    {
        std::cout << "触摸屏线程析构了" << std::endl;
        ts.ts_close();
        // 等待线程结束
        if (thread.joinable())
        {
            thread.join();
        }
    }

    // 子线程执行的函数
    void StartFun()
    {
        while (!shouldExit)
        {
            // std::cout << "游戏线程启动" << std::endl;
            if (StartGameFlag == 1 && StopGameFlag == 1) // 开始游戏标志)
            {
                //  std::cout << "游戏线程开始" << std::endl;
                gamescene.run();
            }
            else if (StopGameFlag == 0)
            {
                //  std::cout << "游戏线程暂停" << std::endl;
                usleep(1000 * 500); // 休眠减少cpu负担
            }

            //  std::cout << "SatrtGameFlag:" << StartGameFlag << std::endl;
            //   std::cout << "StopGameFlag:" << StopGameFlag << std::endl;
            // std::cout << "SetGameFlag:" << SetGameFlag << std::endl;
            //   std::cout << "OverGameFlag:" << OverGameFlag << std::endl;
        }
        std::cout << "游戏线程退出" << std::endl;

        // gamescene.~GameScene();
        return;
    }

    void run()
    {
        while (true)
        {
            ts.ts_read(ts_x, ts_y, ts_sta);

            // std::cout << "松开" << std::endl;
            if (ts_sta == 0)
            {
                std::cout << "ts_x:" << ts_x << " ts_y:" << ts_y << " ts_sta:" << ts_sta << std::endl;
                // 方向键区域，限制在游戏开始且游戏没有暂停按键才有效
                if ((660 < ts_x && ts_x < 715) && (295 < ts_y && ts_y < 360) && StartGameFlag == 1 && StopGameFlag == 1) // 上
                {
                    StopGameFlag = 1;
                    if (gamescene.getSnake().dir != 67)
                        gamescene.getSnake().dir = 71;
                    std::cout << "上" << std::endl;
                }
                else if ((660 < ts_x && ts_x < 715) && (385 < ts_y && ts_y < 450) && StartGameFlag == 1 && StopGameFlag == 1) // 下
                {
                    StopGameFlag = 1;
                    if (gamescene.getSnake().dir != 71)
                        gamescene.getSnake().dir = 67;
                    std::cout << "下" << std::endl;
                }
                else if ((590 < ts_x && ts_x < 640) && (385 < ts_y && ts_y < 450) && StartGameFlag == 1 && StopGameFlag == 1) // 左
                {
                    StopGameFlag = 1;
                    if (gamescene.getSnake().dir != 52)
                        gamescene.getSnake().dir = 49;
                    std::cout << "左" << std::endl;
                }
                else if ((730 < ts_x && ts_x < 780) && (385 < ts_y && ts_y < 450) && StartGameFlag == 1 && StopGameFlag == 1) // 右
                {
                    StopGameFlag = 1;
                    if (gamescene.getSnake().dir != 49)
                        gamescene.getSnake().dir = 52;
                    std::cout << "右" << std::endl;
                }
                else if (0 < ts_x && ts_x < 100 && 0 < ts_y && ts_y < 100 && OverGameFlag == 0) // 退出游戏界面
                {
                    std::cout << "触摸屏按下返回键" << std::endl;
                    StartGameFlag = 0;
                    this->shouldExit = true; // 游戏线程退出
                    // 等待线程完成
                    if (thread.joinable())
                    {
                        thread.join();
                    }
                    lcd.showBmp("start.bmp", 0, 0); // 返回主界面
                }

                else if (415 < ts_x && ts_x < 565 && 195 < ts_y && ts_y < 235 && OverGameFlag == 1) // 退出游戏
                {
                    std::cout << "结算界面点击退出游戏" << std::endl;
                    OverGameFlag = 0;
                    pthread_cond_signal(&cond1);
                    this->shouldExit = true; // 游戏线程退出
                    // 等待线程完成
                    if (thread.joinable())
                    {
                        thread.join();
                    }
                    lcd.showBmp("start.bmp", 0, 0); // 返回主界面
                    StartGameFlag = 0;              // 开始游戏标志置0
                }

                else if (170 < ts_x && ts_x < 325 && 195 < ts_y && ts_y < 235 && OverGameFlag == 1) // 继续游戏
                {
                    std::cout << "结算界面点击继续游戏" << std::endl;
                    gamescene.getSnake().Reset(); // 调用蛇类的复位函数，重新回到出生地
                    OverGameFlag = 0;             // 结束游戏标志位置0，继续游戏

                    pthread_cond_signal(&cond1); // 唤醒条件变量
                    sleep(1);
                    lcd.showBmp("game.bmp", 0, 0);
                }

                // 按下开始按键进入游戏界面
                else if (300 < ts_x && ts_x < 475 && 405 < ts_y && ts_y < 470 && StartGameFlag == 0)
                {
                    StartGameFlag = 1;
                    StopGameFlag = 0; // 复位暂停游戏标志位
                    lcd.showBmp("game.bmp", 0, 0);
                    lcd.showBmp("gstart.bmp", 650, 0);
                    getBackgroundColor(50, 0, 32);
                    // 创建一个线程，并运行GameScene的成员函数run()
                    this->shouldExit = false;
                    GameScene scene; // 创建一个游戏场景
                    this->gamescene = scene;
                    thread = std::thread(&TatcheInput::StartFun, this);
                }

                else if (650 < ts_x && ts_x < 750 && 0 < ts_y && ts_y < 100 && StartGameFlag == 1)
                {
                    std::cout << "暂停/开始" << std::endl;
                    switch (StopGameFlag)
                    {
                    case 0: // 满足条件，蛇从静止状态变为运动状态，给dir标志位赋值默认往右
                        /* code */
                        std::cout << "开始" << std::endl;

                        lcd.showBmp("gstop.bmp", 650, 0);
                        StopGameFlag = !StopGameFlag;
                        gamescene.getSnake().dir = gamescene.getSnake().tempdir;
                        //   lcd.lcd_fill(gamescene.getSnake().getLastNodeX(), gamescene.getSnake().getLastNodeY(), 35, 35, 0xffffff);
                        break;
                    case 1:
                        std::cout << "暂停" << std::endl;
                        gamescene.getSnake().tempdir = gamescene.getSnake().dir; // 保留当前的运动状态
                        lcd.showBmp("gstart.bmp", 650, 0);
                        StopGameFlag = !StopGameFlag;
                        gamescene.getSnake().dir = 0;
                        //  lcd.lcd_fill(gamescene.getSnake().getLastNodeX(), gamescene.getSnake().getLastNodeY(), 35, 35, 0xffffff);
                        break;
                    default:
                        break;
                    }
                }
                else if (0 < ts_x && ts_x < 80 && 400 < ts_y && ts_y < 480 && StartGameFlag == 0)
                {
                    break;
                }
            }
        }
        return;
    }

private:
    TouchScreen ts;      // 触摸屏对象
    GameScene gamescene; // 游戏场景对象
    std::thread thread;
    int ts_x, ts_y, ts_sta;
    bool shouldExit;
};

int main(int argc, char const *argv[])
{

    // 设置随机数种子
    srand(time(nullptr));
    // 创建Lcd对象
    lcd.lcd_open("/dev/fb0");
    lcd.lcd_clear(0xffffff);
    lcd.showBmp("start.bmp", 0, 0);
    lcd.lcd_font_select("/usr/share/fonts/simkai.ttf"); // 字体选择

    // KeyPressed key(scene.getSnake()); // 键盘监听线程

    TatcheInput t; // 主线程用来触摸屏监听
    t.run();
    lcd.lcd_clear(0xffffff);
    return 0;
}