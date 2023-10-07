#ifndef _MYINCLUDE_H
#define _MINCLUDE_H

// c语言常用的头文件
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <linux/input.h> //输入子系统模型有关的头文件
#include <sys/mman.h>    //内存映射有关的头文件
#include <sys/wait.h>
#include <signal.h> //跟signal有关的头文件
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>

// c++相关的头文件
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>    //矢量库
#include <algorithm> //算法库
#include <cstring>
#include <string>
#include <thread>
#include <map> //键值对库
#include <set>
#include <list>  //链表库
#include <queue> //队列库

#include "lcd.h"
#include "ts.h"
#include "font.h"
#include "key.h"
#include "mplayer.h"
#include "cJSON.h" //cJSON库

#endif