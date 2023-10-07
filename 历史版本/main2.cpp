#include"myinclude.h"

static pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int GameSet = 0;      // 重新开始游戏标志位
int GameoverFlag = 0; // 进入游戏结算界面标志

Lcd lcd; // lcd对象定义为全局变量，方便调用lcd相关的成员函数

/*精灵类*/
class Sprite
{
public:
    Sprite() {}
    Sprite(int _x, int _y, int _color = 0x00ff00) : x(_x), y(_y), color(_color) {}

    virtual ~Sprite() {}

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
            // 唤起游戏结算界面
            lcd.showBmp("over1.bmp", 0, 0);
            // 进入游戏结算界面标志置1
            GameoverFlag = 1;
            // 加锁
            pthread_cond_wait(&cond1, &mutex1);
            // 解锁
            if (GameoverFlag == 0)
                return;
            else if (GameSet == 1) // 重新开始游戏
            {
                GameSet = 0;
                GameoverFlag = 0;
                x = 50;
                y = 50;
            }
            // 线程1

            // 这里可以放你的错误处理代码
        }
        std::cout << "绘制精灵" << std::endl;
        lcd.showBmp("snake.bmp", x, y); // 绘制蛇身节点
                                        // sleep(2);
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
        this->dir = 52; // 默认向右移动
    }
    // 蛇身增加
    void AddBody()
    {
        nodes.push_back(Sprite());
    }

    void draw() // 把蛇所有的节点绘制出来
    {
        lastNode = nodes.at(nodes.size() - 1);
        for (auto &node : nodes)
        {
            node.draw();
        }
        usleep(1000 * 300);                                     // 在这里加延时，刷出最新的蛇后延时一段时间再覆盖尾巴的后一个节点，解决了一个小bug，如果不延时的话，移动刷出的节点马上就被覆盖了，特别是刚开始只有一个节点的时候
        if ((dir == 71 || dir == 67 || dir == 49 || dir == 52)) // 限制在运动的时候才清空最后一个节点
        {
            std::cout << "lastNode.getX() = " << lastNode.getX() << std::endl;
            std::cout << "lastNode.getY() = " << lastNode.getY() << std::endl;
            lcd.lcd_fill(lastNode.getX(), lastNode.getY(), 35, 35, 0xffffff);
        }
    }
    // 蛇的身体移动
    void BodyMove()
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

    bool Collision(const Food &other) // 注意传的是Food参数
    {
        // 检查蛇头是否跟食物碰撞
        return (nodes[0].Collision(other)); // 一环扣一环属实666
    }

public:
    int dir; // 蛇的方向
private:
    std::vector<Sprite> nodes; // 蛇的所有节点
    Sprite lastNode;
};

// 设置游戏场景
class GameScene
{
public:
    GameScene() {}
    ~GameScene() {}

    void run()
    {
        // std::cout << "游戏开始" << std::endl;
        food.draw();      // 绘制食物
        snake.draw();     // 绘制蛇身
        SnakeEatFood();   // 吃食物
        snake.BodyMove(); // 移动蛇
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
            std::cout << "开始监听" << std::endl;
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

// 开启一个线程，获取触摸屏触摸信息
class TatcheInput
{

public:
    TatcheInput(Snake &snake) : _snake(snake)
    {
        ts.ts_open("/dev/input/event0");
       std::thread t(&TatcheInput::run, this);
        t.detach();
    }
    ~TatcheInput()
    {
        std::cout << "触摸屏线程析构了" << std::endl;
        ts.ts_close();
    }
    void run()
    {
        while (true)
        {
            ts.ts_read(ts_x, ts_y, ts_sta);
            std::cout << "ts_x:" << ts_x << " ts_y:" << ts_y << " ts_sta:" << ts_sta << std::endl;
            std::cout << "松开" << std::endl;
            if (ts_sta == 0)
            {
                if (660 < ts_x && ts_x < 715 && 295 < ts_y && ts_y < 360) // 上
                {
                    if (_snake.dir != 67)
                        _snake.dir = 71;
                }
                else if (660 < ts_x && ts_x < 715 && 385 < ts_y && ts_y < 450) // 下
                {
                    if (_snake.dir != 71)
                        _snake.dir = 67;
                    std::cout << "下" << std::endl;
                }
                else if (590 < ts_x && ts_x < 640 && 385 < ts_y && ts_y < 450) // 左
                {
                    if (_snake.dir != 52)
                        _snake.dir = 49;
                    std::cout << "左" << std::endl;
                }
                else if ((730 < ts_x && ts_x < 780) && (385 < ts_y && ts_y < 450)) // 右
                {
                    if (_snake.dir != 49)
                        _snake.dir = 52;
                    std::cout << "右" << std::endl;
                }
                else if (0 < ts_x && ts_x < 100 && 0 < ts_y && ts_y < 100) // 退出
                {
                    std::cout << "触摸屏按下退出" << std::endl;
                    _snake.dir = 65;
                    break;
                }
                else if (415 < ts_x && ts_x < 565 && 195 < ts_y && ts_y < 235 && GameoverFlag == 1) // 退出游戏
                {
                    std::cout << "退出结算界面" << std::endl;
                    GameoverFlag = 0;
                    // 唤醒等待在cond1上的线程
                    pthread_cond_signal(&cond1);
                    _snake.dir = 65;
                    break;
                }
                else if (170 < ts_x && ts_x < 325 && 195 < ts_y && ts_y < 235 && GameoverFlag == 1) // 退出游戏
                {
                    std::cout << "继续游戏" << std::endl;
                    GameSet = 1;
                    pthread_cond_signal(&cond1);
                    sleep(1);
                    _snake.dir = 52;
                    break;
                }
            }
        }
        return;
    }
    // 创建一个滑动事件成员函数

    void slide()
    {
    }

private:
    Snake &_snake;
    int ts_x, ts_y, ts_sta;
    TouchScreen ts;
};




int main(int argc, char const *argv[])
{
    // 设置随机数种子
    srand(time(nullptr));
    // 创建Lcd对象
    lcd.lcd_open("/dev/fb0");
    lcd.lcd_clear(0xffffff);
    lcd.showBmp("menu.bmp", 0, 0);
    GameScene scene;                  // 游戏场景
    KeyPressed key(scene.getSnake()); // 键盘监听线程
    TatcheInput t(scene.getSnake());  // 主线程用来触摸屏监听
    while (1)
    {
        std::cout << "dir:" << scene.getSnake().dir << std::endl;
        // 判断是否要退出
        if (scene.getSnake().dir == 65)
        {
            std::cout << "主函数退出" << std::endl;
            break;
        }
        scene.run();
        // usleep(1000 * 500);
        //  sleep(2);
    }

    return 0;
}