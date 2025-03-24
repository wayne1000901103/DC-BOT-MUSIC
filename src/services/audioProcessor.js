const { createAudioResource } = require('@discordjs/voice');
const play = require('play-dl');

class AudioProcessor {
    constructor() {
        this.queue = new Map();
    }

    async processAudio(url) {
        try {
            const stream = await play.stream(url);
            return createAudioResource(stream.stream, {
                inputType: stream.type
            });
        } catch (error) {
            console.error('音频处理错误:', error);
            throw error;
        }
    }

    addToQueue(guildId, url) {
        if (!this.queue.has(guildId)) {
            this.queue.set(guildId, []);
        }
        this.queue.get(guildId).push(url);
    }

    getQueue(guildId) {
        return this.queue.get(guildId) || [];
    }

    clearQueue(guildId) {
        this.queue.delete(guildId);
    }

    removeFromQueue(guildId, index) {
        const queue = this.queue.get(guildId);
        if (queue && index >= 0 && index < queue.length) {
            queue.splice(index, 1);
        }
    }
}

module.exports = new AudioProcessor(); 