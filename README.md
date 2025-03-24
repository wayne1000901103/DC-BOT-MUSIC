# Discord 音乐机器人

一个功能强大的 Discord 音乐机器人，支持音乐播放、音效处理和后台管理。

## 功能特点

- 支持多种音频格式播放
- 实时音效处理（失真、合唱、镶边、相位、哇音）
- 智能后台任务管理
- 系统资源监控和自动调整
- 24/7 持续运行支持
- 跨平台支持（Replit、Windows、Linux）
- 环境变量配置管理
- 缓存和日志管理
- 代理支持
- 安全加密

## 安装

### Replit 安装（推荐）

1. 在 Replit 中导入项目：
   - 点击 "Create Repl"
   - 选择 "Import from GitHub"
   - 输入项目地址：`https://github.com/yourusername/discord-music-bot`

2. 配置环境：
   - Replit 会自动安装所需依赖
   - 在 `.env` 文件中配置 Discord Token 和其他设置
   - 点击 "Run" 按钮启动机器人

3. 24/7 运行设置：
   - 升级到 Replit Pro 计划
   - 启用 "Always On" 功能
   - 配置 UptimeRobot 监控
   - 设置自动重启策略

4. 监控和维护：
   - 使用 Replit 的日志查看器监控运行状态
   - 定期检查资源使用情况
   - 及时更新依赖包

### Windows 安装

1. 系统要求：
   - Windows 10 或更高版本
   - Visual Studio 2019 或更高版本（用于编译）
   - Git（用于克隆仓库）

2. 安装依赖：
   - 安装 [MSYS2](https://www.msys2.org/)
   - 打开 MSYS2 终端，运行：
   ```bash
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-make
   pacman -S mingw-w64-x86_64-libcurl
   pacman -S mingw-w64-x86_64-json-c
   pacman -S mingw-w64-x86_64-fftw
   pacman -S mingw-w64-x86_64-libwebsockets
   ```

3. 克隆仓库：
   ```bash
   git clone https://github.com/yourusername/discord-music-bot.git
   cd discord-music-bot
   ```

4. 编译：
   ```bash
   make
   ```

5. 安装：
   ```bash
   make install
   ```

### Linux 安装

1. 克隆仓库：
```bash
git clone https://github.com/yourusername/discord-music-bot.git
cd discord-music-bot
```

2. 编译：
```bash
make
```

3. 安装：
```bash
sudo make install
```

## 配置

### Replit 配置

1. 配置文件位置：
   - 主配置文件：`/home/runner/.config/discord-music-bot/.env`
   - 缓存目录：`/home/runner/.cache/discord-music-bot`
   - 日志目录：`/home/runner/.local/share/discord-music-bot/logs`

2. 环境变量设置：
   - 在 Replit 的 Secrets 中设置敏感信息
   - 使用 `.env` 文件设置其他配置
   - 确保所有路径使用绝对路径

3. 资源限制：
   - 监控 Replit 的资源使用情况
   - 适当调整缓存大小和日志级别
   - 定期清理临时文件

### Windows 配置

1. 配置文件位置：
   - 主配置文件：`%APPDATA%\DiscordMusicBot\.env`
   - 缓存目录：`%LOCALAPPDATA%\DiscordMusicBot\cache`
   - 日志目录：`%LOCALAPPDATA%\DiscordMusicBot\logs`

2. 环境变量设置：
   - 打开系统环境变量设置
   - 添加 `DISCORD_MUSIC_BOT_HOME` 变量
   - 将 MSYS2 的 bin 目录添加到 PATH

3. 防火墙设置：
   - 允许 Discord 音乐机器人的网络访问
   - 配置端口转发（如果需要）

### 环境变量

创建 `.env` 文件并配置以下参数：

```env
# Discord 配置
DISCORD_TOKEN=your_discord_token
TARGET_CHANNEL_ID=your_channel_id

# 音频处理配置
AUDIO_SAMPLE_RATE=44100
AUDIO_CHANNELS=2
AUDIO_BUFFER_SIZE=4096
AUDIO_QUEUE_SIZE=100

# 后台管理配置
MAX_THREADS=4
MAX_CONCURRENT_TASKS=10
CPU_THRESHOLD=80.0
MEMORY_THRESHOLD=80.0

# 音频效果默认参数
DEFAULT_VOLUME=1.0
DEFAULT_DISTORTION=0.0
DEFAULT_CHORUS_RATE=1.0
DEFAULT_CHORUS_DEPTH=0.5
DEFAULT_CHORUS_MIX=0.5
DEFAULT_FLANGER_RATE=1.0
DEFAULT_FLANGER_DEPTH=0.5
DEFAULT_FLANGER_MIX=0.5
DEFAULT_PHASER_RATE=1.0
DEFAULT_PHASER_DEPTH=0.5
DEFAULT_PHASER_MIX=0.5
DEFAULT_WAH_FREQ=1.0
DEFAULT_WAH_Q=1.0
DEFAULT_WAH_MIX=0.5

# 系统资源限制
MAX_MEMORY_USAGE=512MB
MAX_CPU_USAGE=80.0%
MAX_DISK_USAGE=1GB

# 缓存配置
CACHE_DIR=~/.cache/discord-music-bot
MAX_CACHE_SIZE=100MB
CACHE_EXPIRY=24h

# 日志配置
LOG_LEVEL=INFO
LOG_FILE=~/.local/share/discord-music-bot/logs/bot.log
MAX_LOG_SIZE=10MB
MAX_LOG_FILES=5

# 网络配置
PROXY_ENABLED=false
PROXY_URL=
PROXY_USERNAME=
PROXY_PASSWORD=
TIMEOUT=30

# 安全配置
ENCRYPTION_KEY=your_encryption_key
API_RATE_LIMIT=100
MAX_CONNECTIONS=50
```

### 24/7 运行配置

1. Replit 24/7 运行：
   - 升级到 Replit Pro 计划
   - 启用 "Always On" 功能
   - 配置 UptimeRobot 监控
   - 设置自动重启策略
   - 定期检查运行状态

2. Windows 服务配置：
   ```powershell
   # 创建服务
   New-Service -Name "DiscordMusicBot" -BinaryPathName "C:\Program Files\DiscordMusicBot\discord_music_bot.exe" -StartupType Automatic
   
   # 启动服务
   Start-Service -Name "DiscordMusicBot"
   
   # 停止服务
   Stop-Service -Name "DiscordMusicBot"
   
   # 删除服务
   Remove-Service -Name "DiscordMusicBot"
   ```

3. 使用 systemd（Linux）：
```bash
sudo nano /etc/systemd/system/discord-music-bot.service
```

添加以下内容：
```ini
[Unit]
Description=Discord Music Bot
After=network.target

[Service]
Type=simple
User=your_username
WorkingDirectory=/path/to/discord-music-bot
ExecStart=/path/to/discord-music-bot
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

## 使用方法

### Replit 使用说明

1. 启动方式：
   - 点击 Replit 的 "Run" 按钮
   - 使用 Replit 的 Shell 运行 `./discord_music_bot`
   - 通过 UptimeRobot 监控运行状态

2. 常见问题解决：
   - 如果出现内存不足，调整缓存大小
   - 如果出现连接问题，检查网络设置
   - 如果出现权限问题，检查文件权限

3. 日志查看：
   - 使用 Replit 的日志查看器
   - 查看 `logs` 目录下的日志文件
   - 设置适当的日志级别

### Windows 使用说明

1. 启动方式：
   - 命令行启动：打开命令提示符，运行 `discord_music_bot.exe`
   - 服务启动：通过服务管理器启动 "DiscordMusicBot" 服务
   - 快捷方式：双击桌面快捷方式

2. 常见问题解决：
   - 如果出现 DLL 缺失错误，确保 MSYS2 的 bin 目录已添加到 PATH
   - 如果出现权限问题，以管理员身份运行
   - 如果出现端口占用，检查防火墙设置

3. 日志查看：
   - 打开事件查看器
   - 导航到 "Windows 日志" > "应用程序"
   - 查找 "DiscordMusicBot" 相关日志

1. 启动机器人：
```bash
./discord_music_bot
```

2. 常用命令：
```
!play <url> - 播放音乐
!stop - 停止播放
!pause - 暂停播放
!resume - 恢复播放
!skip - 跳过当前歌曲
!queue - 显示播放队列
!volume <0-100> - 调整音量
!effects - 显示可用音效
!effect <name> <value> - 设置音效参数
```

## 开发

### 目录结构

```
discord-music-bot/
├── src/
│   ├── main.c
│   ├── discord_client.c
│   ├── audio_processor.c
│   ├── background_manager.c
│   └── env_config.c
├── include/
│   ├── discord_client.h
│   ├── audio_processor.h
│   ├── background_manager.h
│   └── env_config.h
├── tests/
├── docs/
├── .env
├── Makefile
├── .replit
├── replit.nix
└── README.md
```

### 编译选项

- `make` - 编译项目
- `make clean` - 清理编译文件
- `make install` - 安装
- `make uninstall` - 卸载

## 贡献

1. Fork 项目
2. 创建特性分支
3. 提交更改
4. 推送到分支
5. 创建 Pull Request

## 许可证

MIT License

## 联系方式

- 问题反馈：GitHub Issues
- 邮件：your.email@example.com
- Discord：your_discord_username 