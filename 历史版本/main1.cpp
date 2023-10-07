#include"myinclude.h"
/*精灵类*/

Lcd lcd;
class Sprite
{
public:
    Sprite() {}
    Sprite(int _x, int _y, int _color = 0x00ff00) : x(_x), y(_y), color(_color) {}

    virtual ~Sprite() {}

    virtual void draw()
    {
        // 设置坐标和颜色，使用lcd_fill填充
        //  std::cout << "颜色 " << this->color << std::endl;
        // std::cout << "坐标 " << x << "," << y << std::endl;
        try // 限制游戏区域
        {
            if (x < 50 || y < 50 || x > 550 || y > 450)
            {
                /*抛出碰壁错误*/
                throw "碰壁！！游戏结束";
            }
        }
        catch (const char *e)
        {
            /*处理游戏结束错误*/
            std::cout << "错误消息: " << e << std::endl;
            x = 50;
            y = 50;
            // 这里可以放你的错误处理代码
        }
        // lcd.lcd_fill(x, y, 20, 20, color);
        lcd.showBmp("snake.bmp", x, y);
    }

    void MoveBy(int dx, int dy)
    {
        x += dx;
        y += dy;
    }

    // 碰撞检测，检测当前对象跟另一个对象的坐标是否相等
    bool Collision(const Sprite &other)
    {
        //    std::cout << "检测碰撞:" << x << " " << y << std::endl;
        return (this->x == other.x && this->y == other.y);
    }
    int getX() const { return x; }
    int getY() const { return y; }

protected:
    int x;
    int y;
    int color;
};

/*食物*/
class Food : public Sprite
{
public:
    Food() : Sprite(0, 0)
    {
        Change();
    }
    void draw() // 绘制食物
    {
        // lcd.lcd_fill(x, y, 20, 20, 0xff0000);
        lcd.showBmp("food1.bmp", x, y);
    }
    // 改变食物位置
    void Change()
    {
        // 生成 x 坐标，确保是 35 的整数倍，并且在 (50, 550) 之间
        x = 50 + (rand() % 14) * 35; // 14 是 (550-50)/35 的结果
        // 生成 y 坐标，确保是 35 的整数倍，并且在 (50, 450) 之间
        y = 50 + (rand() % 13) * 35; // 13 是 (450-50)/35 的结果
    }
};

// 定义一个蛇对象，继承于Sprite
class Snake : public Sprite
{
public:
    Snake(int x = 0, int y = 0) : Sprite(x, y) // 使用默认参数，当没有参数传进来的时候就是默认构造函数
    {
        // 初始化3节蛇
        //  nodes.push_back(Sprite(100, 0));
        //  nodes.push_back(Sprite(80, 0));
        //  nodes.push_back(Sprite(60, 0));
        nodes.push_back(Sprite(120, 50));
        nodes.push_back(Sprite(85, 50));
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
        for (auto &node : nodes)
        {
            node.draw();
        }
    }
    // 蛇的身体移动
    void BodyMove()
    {
        // int lastX = nodes.at(nodes.size() - 1).getX();
        // int lastY = nodes.at(nodes.size() - 1).getY();
        Sprite lastNode = nodes.at(nodes.size() - 1);
        //  std::cout << "lastX:" << lastX << "lastY:" << lastY << std::endl;
        // std::cout << "移动" << std::endl;
        for (int i = nodes.size() - 1; i > 0; i--)
        {
            // 最后一节等于前一节
            nodes.at(i) = nodes.at(i - 1);
            // std::cout << "X:" << nodes.at(i).getX() << "Y" << nodes.at(i).getY() << std::endl;
        }
        if (dir == 71 || dir == 67 || dir == 49 || dir == 52) // 限制在运动的时候才清空最后一个节点
            lcd.lcd_fill(lastNode.getX(), lastNode.getY(), 35, 35, 0xffffff);
        // 改变方向
        switch (dir)
        {
        case 71:
            nodes[0].MoveBy(0, -35);
            // std::cout << "上" << std::endl;
            break;
        case 67: // 下
            nodes[0].MoveBy(0, 35);
            //  std::cout << "下" << std::endl;
            break;
        case 49: // 左
            nodes[0].MoveBy(-35, 0);
            //  std::cout << "左" << std::endl;
            break;
        case 52: // 右
            nodes[0].MoveBy(35, 0);
            // std::cout << "右" << std::endl;
            break;
        case 65:
            std::cout << "退出" << std::endl;
            exit(0);
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
};

// 设置游戏场景
class GameScene
{
public:
    GameScene()
    {
    }

    void run()
    {
        // std::cout << "游戏开始" << std::endl;
        food.draw();
        snake.draw();
        SnakeEatFood();
        // 移动蛇，改变蛇的坐标
        // std::cout << "dirffff" << snake.dir << std::endl;
        snake.BodyMove();
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
        std::cout << "KeyPressed exit" << std::endl;
        shouldExit = true;
    }
    void run()
    {
        KeyboardInput keyboard;
        while (!shouldExit)
        {
            //  std::cout << "开始监听" << std::endl;
            std::string userInput = keyboard.readInput(); // 没按下对应条件的按键就阻塞在这里
            switch (userInput.at(0) - '0')
            {
            case 71:
                if (snake_.dir != 67)
                    snake_.dir = 71;
                break;
            case 67:
                if (snake_.dir != 71)
                    snake_.dir = 67;
                break;
            case 49:
                if (snake_.dir != 52)
                    snake_.dir = 49;
                break;
            case 52:
                if (snake_.dir != 49)
                    snake_.dir = 52;
                break;
            case 65:
                snake_.dir = 65;
            default:
                break;
            }
            //   snake_.dir = userInput.at(0) - '0';           // 获取字符串的第一个字符，当然在readInput函数里面改一下返回值类型为字符类型也行（按需求修改，这玩意之前用来输入法显示的）
            //   std::cout << "监听dir" << snake_.dir << std::endl;
            if (snake_.dir == 65)
            {
                std::cout << "退出循环" << std::endl;
                shouldExit = true;
                break;
            }
            usleep(1000 * 100);
        }
        if (shouldExit)
        {
            std::cout << "结束游戏" << std::endl;
        }
    }

private:
    Snake &snake_; // 这里不再继承Snake类，直接把需要移动的Snake对象引用传进来，再去更改这个对象的方向变量dir
    bool shouldExit;
};

int main()
{
    // 设置随机数种子
    srand(time(nullptr));
    // 创建Lcd对象
    lcd.lcd_open("/dev/fb0");
    lcd.lcd_clear(0xffffff);
    GameScene scene;
    KeyPressed key(scene.getSnake());
    while (1)
    {
        scene.run();
        usleep(1000 * 300);
    }

    return 0;
}