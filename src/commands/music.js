const { createAudioPlayer, createAudioResource, joinVoiceChannel } = require('@discordjs/voice');
const play = require('play-dl');

const player = createAudioPlayer();

function setupMusicCommands(client) {
    client.on('messageCreate', async message => {
        if (message.author.bot) return;

        const prefix = '!';
        if (!message.content.startsWith(prefix)) return;

        const args = message.content.slice(prefix.length).trim().split(/ +/);
        const command = args.shift().toLowerCase();

        switch (command) {
            case 'play':
                await handlePlay(message, args.join(' '));
                break;
            case 'stop':
                handleStop(message);
                break;
            case 'pause':
                handlePause(message);
                break;
            case 'resume':
                handleResume(message);
                break;
            case 'skip':
                handleSkip(message);
                break;
            case 'queue':
                handleQueue(message);
                break;
            case 'volume':
                handleVolume(message, args[0]);
                break;
        }
    });
}

async function handlePlay(message, url) {
    try {
        const voiceChannel = message.member.voice.channel;
        if (!voiceChannel) {
            return message.reply('你需要先加入语音频道！');
        }

        const connection = joinVoiceChannel({
            channelId: voiceChannel.id,
            guildId: message.guild.id,
            adapterCreator: message.guild.voiceAdapterCreator,
        });

        const stream = await play.stream(url);
        const resource = createAudioResource(stream.stream, {
            inputType: stream.type
        });

        connection.subscribe(player);
        player.play(resource);

        message.reply(`正在播放: ${url}`);
    } catch (error) {
        console.error('播放错误:', error);
        message.reply('播放时发生错误！');
    }
}

function handleStop(message) {
    player.stop();
    message.reply('已停止播放');
}

function handlePause(message) {
    player.pause();
    message.reply('已暂停播放');
}

function handleResume(message) {
    player.unpause();
    message.reply('已恢复播放');
}

function handleSkip(message) {
    player.stop();
    message.reply('已跳过当前歌曲');
}

function handleQueue(message) {
    message.reply('队列功能开发中...');
}

function handleVolume(message, volume) {
    const vol = parseInt(volume);
    if (isNaN(vol) || vol < 0 || vol > 100) {
        return message.reply('请输入有效的音量值 (0-100)');
    }
    player.setVolume(vol / 100);
    message.reply(`音量已设置为 ${vol}%`);
}

module.exports = {
    setupMusicCommands
}; 