CC = g++
ARM_CC = arm-linux-c++

# 编译选项
CXXFLAGS = -std=c++11
LDFLAGS = -pthread
FONTFLAGS := -L./ -lfont -lm

SSH_PASS = sshpass
REMOTE_HOST = root@192.168.63.167
REMOTE_DIR = /djc/snake
SSHPASS = djc1234

# 目标文件设置
TARGET = a.out
ARM_TARGET = a-arm.out
JSON_TARGET = log.json
# 源文件列表
SOURCE = main.cpp lcd.cpp key.cpp ts.cpp cJSON.c mplayer.cpp

# 默认目标
all: $(ARM_TARGET)

# 本地编译
$(TARGET): $(SOURCE)
	$(CC) $^ -o $@ $(CXXFLAGS) $(LDFLAGS) $(FONTFLAGS)

# 交叉编译
$(ARM_TARGET): $(SOURCE)
	$(ARM_CC) $^ -o $@ $(CXXFLAGS) $(LDFLAGS) $(FONTFLAGS)

# 传输到远程主机
transfer: $(ARM_TARGET)
	$(SSH_PASS) -p $(SSHPASS) scp $^ $(REMOTE_HOST):$(REMOTE_DIR)

# 从远程主机下载文件
download: 
	$(SSH_PASS) -p $(SSHPASS) scp $(REMOTE_HOST):$(REMOTE_DIR)/$(JSON_TARGET) .

.PHONY: clean transfer

clean:
	rm -f $(TARGET) $(ARM_TARGET)
