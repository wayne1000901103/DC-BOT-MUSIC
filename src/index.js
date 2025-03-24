require('dotenv').config();
const { Client, GatewayIntentBits } = require('discord.js');
const { setupMusicCommands } = require('./commands/music');
const { setupAudioProcessor } = require('./services/audioProcessor');

const client = new Client({
    intents: [
        GatewayIntentBits.Guilds,
        GatewayIntentBits.GuildVoiceStates,
        GatewayIntentBits.GuildMessages,
        GatewayIntentBits.MessageContent
    ]
});

// 初始化音频处理器
setupAudioProcessor();

// 设置音乐命令
setupMusicCommands(client);

// 登录到 Discord
client.login(process.env.DISCORD_TOKEN);

// 错误处理
client.on('error', error => {
    console.error('Discord client error:', error);
});

process.on('unhandledRejection', error => {
    console.error('Unhandled promise rejection:', error);
}); 