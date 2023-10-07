#ifndef __TS_H_
#define __TS_H_

class TouchScreen
{
public:
    TouchScreen(); 
    ~TouchScreen();

    int ts_open(const char *pathname);
    int ts_close();
    int ts_read(int &x, int &y, int &sta);

private:
    int fd_ts;
};

#endif // TOUCHSCREEN_H
