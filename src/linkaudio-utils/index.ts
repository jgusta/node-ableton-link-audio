import { parseWav, readWavFile, readWavFileSync, WavFileData } from './wav.ts';
import { sleep, sleepUntilLinkTime, LinkTimeScheduler } from './scheduler.ts';
import { waitForChannel } from './channel.ts';
import { createSourceIterator } from './source.ts';
import {
  DEFAULT_WAV_OPTIONS,
  createWavSinkPlayer,
  playWav,
  WavPlayerOptions,
  WavSinkPlayer,
} from './player.ts';

function callOnLinkThreadAsync(link: { callOnLinkThread: (cb: () => void) => void }) {
  return new Promise<void>((resolve) => {
    link.callOnLinkThread(() => resolve());
  });
}

export {
  parseWav,
  readWavFile,
  readWavFileSync,
  sleep,
  sleepUntilLinkTime,
  LinkTimeScheduler,
  waitForChannel,
  createSourceIterator,
  DEFAULT_WAV_OPTIONS,
  createWavSinkPlayer,
  playWav,
  callOnLinkThreadAsync,
  WavFileData,
  WavPlayerOptions,
  WavSinkPlayer,
};
