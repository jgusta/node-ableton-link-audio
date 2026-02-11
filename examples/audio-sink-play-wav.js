/* eslint-disable no-console */
const fs = require('fs');
const path = require('path');
const { AbletonLinkAudio, AbletonLinkAudioSink } = require('..');

const WAV_PATH = path.join(__dirname, 'test.wav');
const PEER_NAME = 'node-link-audio-sink';
const CHANNEL_NAME = 'test-wav';
const BPM = 120.0;
const QUANTUM = 4.0;
const FRAMES_PER_BUFFER = 2048;
const SCHEDULER_INTERVAL_MS = 5;
const MAX_BUFFERS_PER_TICK = 4;

function readWavFile(filePath) {
  const buffer = fs.readFileSync(filePath);
  if (buffer.toString('ascii', 0, 4) !== 'RIFF' || buffer.toString('ascii', 8, 12) !== 'WAVE') {
    throw new Error('Invalid WAV file');
  }

  let offset = 12;
  let fmt;
  let dataOffset = -1;
  let dataSize = 0;

  while (offset + 8 <= buffer.length) {
    const chunkId = buffer.toString('ascii', offset, offset + 4);
    const chunkSize = buffer.readUInt32LE(offset + 4);
    const chunkStart = offset + 8;
    if (chunkId === 'fmt ') {
      fmt = {
        audioFormat: buffer.readUInt16LE(chunkStart),
        numChannels: buffer.readUInt16LE(chunkStart + 2),
        sampleRate: buffer.readUInt32LE(chunkStart + 4),
        bitsPerSample: buffer.readUInt16LE(chunkStart + 14),
      };
    } else if (chunkId === 'data') {
      dataOffset = chunkStart;
      dataSize = chunkSize;
      break;
    }
    offset = chunkStart + chunkSize + (chunkSize % 2);
  }

  if (!fmt || dataOffset < 0) {
    throw new Error('WAV missing fmt or data chunk');
  }
  if (fmt.audioFormat !== 1 || fmt.bitsPerSample !== 16) {
    throw new Error('Only PCM 16-bit WAV is supported');
  }

  const samples = new Int16Array(
    buffer.buffer,
    buffer.byteOffset + dataOffset,
    dataSize / 2
  );

  return {
    samples,
    numChannels: fmt.numChannels,
    sampleRate: fmt.sampleRate,
  };
}

function main() {
  const wav = readWavFile(WAV_PATH);
  console.log(`Loaded WAV: ${wav.sampleRate} Hz, ${wav.numChannels} channels`);

  const link = new AbletonLinkAudio(BPM, PEER_NAME);
  link.enable(true);
  link.enableLinkAudio(true);

  const sink = new AbletonLinkAudioSink(
    link,
    CHANNEL_NAME,
    FRAMES_PER_BUFFER * wav.numChannels
  );

  let frameIndex = 0;
  const totalFrames = Math.floor(wav.samples.length / wav.numChannels);
  const bufferSeconds = FRAMES_PER_BUFFER / wav.sampleRate;
  let nextSendTime = link.getClockTime() + bufferSeconds;

  console.log('Waiting for receivers. Audio is sent only when sources exist.');

  const timer = setInterval(() => {
    let buffersSent = 0;
    while (buffersSent < MAX_BUFFERS_PER_TICK) {
      const now = link.getClockTime();
      if (now < nextSendTime) {
        break;
      }

      const handle = sink.retainBuffer();
      if (!handle || !handle.isValid()) {
        break;
      }

      const sessionState = link.captureAppSessionState();
      const beatsAtBufferBegin = sessionState.beatAtTime(nextSendTime, QUANTUM);

      const out = handle.samples();
      if (!out) {
        break;
      }

      const maxFrames = Math.floor(handle.maxNumSamples() / wav.numChannels);
      const framesToWrite = Math.min(FRAMES_PER_BUFFER, totalFrames, maxFrames);
      const outI16 = new Int16Array(
        out.buffer,
        out.byteOffset,
        out.length / 2
      );
      for (let i = 0; i < framesToWrite; i += 1) {
        const srcFrame = (frameIndex + i) % totalFrames;
        for (let ch = 0; ch < wav.numChannels; ch += 1) {
          outI16[i * wav.numChannels + ch] =
            wav.samples[srcFrame * wav.numChannels + ch];
        }
      }
      frameIndex = (frameIndex + framesToWrite) % totalFrames;

      const ok = handle.commit(
        sessionState,
        beatsAtBufferBegin,
        QUANTUM,
        framesToWrite,
        wav.numChannels,
        wav.sampleRate
      );
      if (!ok) {
        console.warn('Failed to commit audio buffer');
        break;
      }

      nextSendTime += bufferSeconds;
      buffersSent += 1;
    }
  }, SCHEDULER_INTERVAL_MS);

  process.on('SIGINT', () => {
    clearInterval(timer);
    link.close();
    process.exit(0);
  });
}

main();
