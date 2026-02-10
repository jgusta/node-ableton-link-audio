import { parseWav, readWavFile, readWavFileSync, type WavFileData } from './wav.ts'
import { sleep, sleepUntilLinkTime, LinkTimeScheduler } from './scheduler.ts'
import { waitForChannel } from './channel.ts'
import { createSourceIterator } from './source.ts'
import {
  DEFAULT_WAV_OPTIONS,
  createWavSinkPlayer,
  playWav,
  type WavPlayerOptions,
  type WavSinkPlayer,
} from './player.ts'

function callOnLinkThreadAsync(link: { callOnLinkThread: (cb: () => void) => void }) {
  return new Promise<void>((resolve) => {
    link.callOnLinkThread(() => resolve())
  })
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
  type WavFileData,
  type WavPlayerOptions,
  type WavSinkPlayer,
}
