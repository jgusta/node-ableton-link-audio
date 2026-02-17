/* eslint-disable no-console */
import { fileURLToPath } from 'url';
import path from 'path';
import { AbletonLinkAudio, linkAudioUtils } from '../index.ts';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const WAV_PATH = path.join(__dirname, 'test-125bpm.wav');
const PEER_NAME = 'node-link-audio-sink';
const CHANNEL_NAME = 'nodewave';
const BPM = 120.0;
const QUANTUM = 4.0;
const FRAMES_PER_BUFFER = 2048;

function main() {
  const link = new AbletonLinkAudio(BPM, PEER_NAME);
  link.enable(true);
  link.enableLinkAudio(true);

  const player = linkAudioUtils.createWavSinkPlayer(link, WAV_PATH, {
    channelName: CHANNEL_NAME,
    quantum: QUANTUM,
    framesPerBuffer: FRAMES_PER_BUFFER,
  });
  console.log(
    `Loaded WAV: ${player.wav.sampleRate} Hz, ${player.wav.numChannels} channels`
  );
  console.log('Waiting for receivers. Audio is sent only when sources exist.');
  player.start();

  process.on('SIGINT', () => {
    player.stop();
    link.close();
    process.exit(0);
  });
}

main();
