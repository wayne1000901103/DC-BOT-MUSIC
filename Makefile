# 检测操作系统和环境
ifeq ($(REPLIT),1)
    # Replit 环境
    CC = gcc
    CFLAGS = -Wall -Wextra -O2 -fPIC
    LDFLAGS = -shared
    TARGET = libenv_config.so
    RM = rm -f
    MKDIR = mkdir -p
    INSTALL = install -m 644
    INSTALL_DIR = mkdir -p
    DESTDIR = /home/runner
else ifeq ($(OS),Windows_NT)
    # Windows 系统
    CC = gcc
    CFLAGS = -Wall -Wextra -O2
    LDFLAGS = -shared
    TARGET = env_config.dll
    RM = del /F /Q
    MKDIR = mkdir
    INSTALL = copy
    INSTALL_DIR = mkdir
else
    # Linux/Unix 系统
    CC = gcc
    CFLAGS = -Wall -Wextra -O2 -fPIC
    LDFLAGS = -shared
    TARGET = libenv_config.so
    RM = rm -f
    MKDIR = mkdir -p
    INSTALL = install -m 644
    INSTALL_DIR = install -d
endif

# 源文件和目标文件
SRCS = env_config.c
OBJS = $(SRCS:.c=.o)

# 默认目标
all: $(TARGET)

# 编译规则
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理规则
clean:
	$(RM) $(OBJS) $(TARGET)

# 安装规则
ifeq ($(REPLIT),1)
install: $(TARGET)
	$(INSTALL_DIR) $(DESTDIR)/lib
	$(INSTALL) -m 755 $(TARGET) $(DESTDIR)/lib
	$(INSTALL_DIR) $(DESTDIR)/include
	$(INSTALL) -m 644 env_config.h $(DESTDIR)/include
else ifeq ($(OS),Windows_NT)
install: $(TARGET)
	$(INSTALL_DIR) $(DESTDIR)\lib
	$(INSTALL) $(TARGET) $(DESTDIR)\lib
	$(INSTALL_DIR) $(DESTDIR)\include
	$(INSTALL) env_config.h $(DESTDIR)\include
else
install: $(TARGET)
	$(INSTALL_DIR) $(DESTDIR)/usr/local/lib
	$(INSTALL) -m 755 $(TARGET) $(DESTDIR)/usr/local/lib
	$(INSTALL_DIR) $(DESTDIR)/usr/local/include
	$(INSTALL) -m 644 env_config.h $(DESTDIR)/usr/local/include
endif

# 卸载规则
ifeq ($(REPLIT),1)
uninstall:
	$(RM) $(DESTDIR)/lib/$(TARGET)
	$(RM) $(DESTDIR)/include/env_config.h
else ifeq ($(OS),Windows_NT)
uninstall:
	$(RM) $(DESTDIR)\lib\$(TARGET)
	$(RM) $(DESTDIR)\include\env_config.h
else
uninstall:
	$(RM) $(DESTDIR)/usr/local/lib/$(TARGET)
	$(RM) $(DESTDIR)/usr/local/include/env_config.h
endif

.PHONY: all clean install uninstall 