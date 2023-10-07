#include"myinclude.h"

static pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int StartGameFlag = 0; // 进入游戏开始界面标志
int StopGameFlag = 0;  // 暂停游戏标志,1表示暂停,0表示开始
int SetGameFlag = 0;   // 重新开始游戏标志位
int OverGameFlag = 0;  // 进入游戏结算界面标志

Lcd lcd; // lcd对象定义为全局变量，方便调用lcd相关的成员函数

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
            std::cout << "x:" << x << "y:" << y << std::endl;
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
        lcd.showBmp("snake.bmp", x, y); // 绘制蛇身节点
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
        Change(); // 初始化游戏开始时食物的坐标位置
    }
    void draw() // 绘制食物
    {
        lcd.showBmp("food1.bmp", x, y); // 绘制食物图片
    }
    // 改变食物位置
    void Change()
    {
        // 生成 x 坐标，确保是 35 的整数倍，并且在 (50, 500) 之间
        x = 50 + (rand() % 14) * 35;
        // 生成 y 坐标，确保是 35 的整数倍，并且在 (50, 400) 之间
        y = 50 + (rand() % 11) * 35; //
    }
};

// 定义一个蛇对象，继承于Sprite
class Snake : public Sprite
{
public:
    Snake(int x = 0, int y = 0) : Sprite(x, y) // 使用默认参数，当没有参数传进来的时候就是默认构造函数
    {
        // 初始化3节蛇
        //  nodes.push_back(Sprite(120, 50));
        // nodes.push_back(Sprite(85, 50));
        nodes.push_back(Sprite(50, 50));
        this->dir = 0;      // 刚进入游戏蛇是停止状态
        this->tempdir = 52; // 点击开始运动按钮，蛇默认向右移动
    }
    ~Snake()
    {
        std::cout << "Snake析构,清空Sprite容器" << std::endl;
        nodes.clear();
    }
    // 蛇身增加
    void AddBody()
    {
        nodes.push_back(Sprite());
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
            nodes.push_back(Sprite(50, 50));
            // nodes.push_back(Sprite(85, 50));
            //   nodes.push_back(Sprite(120, 50));
            dir = 0;
            tempdir = 52;
        }
        else
        {
            std::cout << "容器为空，无法复位" << std::endl;
        }
    }

    void draw() // 把蛇所有的节点绘制出来
    {
        // 先判断容器是否为空
        if (!nodes.empty())
        {
            this->lastNode = nodes.at(nodes.size() - 1);
            for (auto &node : nodes)
            {
                node.draw();
            }

            usleep(1000 * 500);                                                          // 在这里加延时，刷出最新的蛇后延时一段时间再覆盖尾巴的后一个节点，解决了一个小bug，如果不延时的话，移动刷出的节点马上就被覆盖了，特别是刚开始只有一个节点的时候
            if ((dir == 71 || dir == 67 || dir == 49 || dir == 52) && StopGameFlag == 1) // 限制在运动的时候才清空最后一个节点
            {
                std::cout << "----------清尾--------------" << std::endl;
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
        std::cout << "-------------移动身体--------------" << std::endl;
        if (!nodes.empty())
        {
            for (int i = nodes.size() - 1; i > 0; i--)
            {
                // 最后一节等于前一节，所有节点往前移动
                nodes.at(i) = nodes.at(i - 1);
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
    int dir; // 蛇的方向
    int tempdir;

private:
    std::vector<Sprite> nodes; // 蛇的所有节点
    Sprite lastNode;
};

// 设置游戏场景
class GameScene
{
public:
    GameScene() {}
    ~GameScene()
    {
        std::cout << "GameScene析构" << std::endl;
    }

    void run()
    {

        food.draw();      // 绘制食物
        snake.draw();     // 绘制蛇身
        snake.BodyMove(); // 移动蛇

        SnakeEatFood(); // 吃食物
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
            snake.AddBody();
            food.Change();
        }
    }

private:
    // 把蛇放到这个场景
    Snake snake;
    Food food;
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
        //    thread = std::thread(&TatcheInput::StartFun, this);
        // std::thread t(&TatcheInput::run, this);
        // t.detach();
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
                // std::cout << "游戏线程开始" << std::endl;
                gamescene.run();
                //  lcd.lcd_fill(50, 50, 500, 400, 0xffffff);
            }
            else if (StopGameFlag == 0)
            {
                // std::cout << "游戏线程暂停" << std::endl;
                sleep(1); // 休眠减少cpu负担
            }
        }

        std::cout << "游戏线程退出" << std::endl;
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
                if ((660 < ts_x && ts_x < 715) && (295 < ts_y && ts_y < 360) && StartGameFlag == 1) // 上
                {
                    StopGameFlag = 1;
                    if (gamescene.getSnake().dir != 67)
                        gamescene.getSnake().dir = 71;
                    std::cout << "上" << std::endl;
                }
                else if ((660 < ts_x && ts_x < 715) && (385 < ts_y && ts_y < 450) && StartGameFlag == 1) // 下
                {
                    StopGameFlag = 1;
                    if (gamescene.getSnake().dir != 71)
                        gamescene.getSnake().dir = 67;
                    std::cout << "下" << std::endl;
                }
                else if ((590 < ts_x && ts_x < 640) && (385 < ts_y && ts_y < 450) && StartGameFlag == 1) // 左
                {
                    StopGameFlag = 1;
                    if (gamescene.getSnake().dir != 52)
                        gamescene.getSnake().dir = 49;
                    std::cout << "左" << std::endl;
                }
                else if ((730 < ts_x && ts_x < 780) && (385 < ts_y && ts_y < 450) && StartGameFlag == 1) // 右
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
                    StartGameFlag = 0;              // 开始游戏标志置一
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
                        std::cout << "开始----------------------------" << std::endl;

                        lcd.showBmp("gstop.bmp", 650, 0);
                        StopGameFlag = !StopGameFlag;
                        // gamescene.getSnake().dir = gamescene.getSnake().tempdir;
                        //   lcd.lcd_fill(50, 50, 500, 400, 0xffffff);
                        break;
                    case 1:
                        std::cout << "暂停----------------------------" << std::endl;
                        //  gamescene.getSnake().tempdir = gamescene.getSnake().dir; // 保留当前的运动状态
                        lcd.showBmp("gstart.bmp", 650, 0);
                        StopGameFlag = !StopGameFlag;
                        // lcd.lcd_fill(50, 50, 500, 400, 0xffffff);
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

    // KeyPressed key(scene.getSnake()); // 键盘监听线程

    TatcheInput t; // 主线程用来触摸屏监听
    t.run();
    lcd.lcd_clear(0xffffff);
    return 0;
}