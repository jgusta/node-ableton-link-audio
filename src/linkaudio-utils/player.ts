import path from 'path';
// import { createRequire } from 'module';
import { readWavFileSync,type WavFileData } from './wav.ts';
import { LinkTimeScheduler } from './scheduler.ts';
import bindings from 'bindings'
// const require = createRequire(import.meta.url);
// const bindings = require('bindings') as (name: string) => any;
const addon = bindings('abletonlink') as any;

export interface WavSinkPlayer {
  sink: any;
  wav: WavFileData;
  scheduler: LinkTimeScheduler;
  start: () => void;
  stop: () => void;
}

export interface WavPlayerOptions {
  quantum?: number;
  framesPerBuffer?: number;
  schedulerCoarseMs?: number;
  maxBuffersPerTick?: number;
  targetLeadSec?: number;
  lowWaterSec?: number;
  refillCheckPeriodSec?: number;
  channelName?: string;
  syncMode?: 'free' | 'quantized' | 'resample';
  referenceTempo?: number;
  tempo?: number;
  loopLengthBeats?: number | null;
  loopQuantize?: number;
  adaptiveLead?: boolean;
  adaptiveLeadMinSec?: number;
  adaptiveLeadMaxSec?: number;
  adaptiveLeadStepSec?: number;
  adaptiveFailWindowMs?: number;
}

export const DEFAULT_WAV_OPTIONS: Required<WavPlayerOptions> = {
  quantum: 4.0,
  framesPerBuffer: 2048,
  schedulerCoarseMs: 2,
  maxBuffersPerTick: 8,
  targetLeadSec: 0.02,
  lowWaterSec: 0.01,
  refillCheckPeriodSec: 0.005,
  channelName: 'wav',
  syncMode: 'free',
  referenceTempo: 120,
  tempo: 120,
  loopLengthBeats: null,
  loopQuantize: 4,
  adaptiveLead: false,
  adaptiveLeadMinSec: 0.01,
  adaptiveLeadMaxSec: 0.08,
  adaptiveLeadStepSec: 0.005,
  adaptiveFailWindowMs: 1000,
};

export function createWavSinkPlayer(
  link: any,
  wavPath: string,
  options: WavPlayerOptions = {}
): WavSinkPlayer {
  const opts = { ...DEFAULT_WAV_OPTIONS, ...options };
  const wav = readWavFileSync(wavPath);
  const sink = new addon.AbletonLinkAudioSink(
    link,
    opts.channelName,
    opts.framesPerBuffer * wav.numChannels
  );

  const bufferSeconds = opts.framesPerBuffer / wav.sampleRate;
  const scheduler = new LinkTimeScheduler(link, {
    coarseMs: opts.schedulerCoarseMs ?? 2,
  });

  let targetLeadSec = opts.targetLeadSec ?? 0.02;
  let lowWaterSec = opts.lowWaterSec ?? 0.01;

  const syncMode = opts.syncMode ?? 'free';
  const referenceTempo = opts.referenceTempo ?? 120;
  const loopLengthBeats = opts.loopLengthBeats ?? null;
  const loopQuantize = opts.loopQuantize ?? opts.quantum ?? 4;

  let frameIndex = 0;
  let phase = 0;
  let nextSendTime = 0;
  let periodic: { cancel: () => void } | null = null;
  let lastFailTimeMs = 0;

  let tempoCache = opts.tempo ?? referenceTempo;
  let unsubscribeTempo: (() => void) | null = null;
  if (typeof link?.onTempoChange === 'function') {
    unsubscribeTempo = link.onTempoChange((t: number) => {
      if (Number.isFinite(t) && t > 0) tempoCache = t;
    });
  }

  function getTempo(sessionState: any) {
    if (sessionState && typeof sessionState.tempo === 'function') {
      const t = sessionState.tempo();
      if (Number.isFinite(t) && t > 0) return t;
    }
    if (typeof link?.getTempo === 'function') {
      const t = link.getTempo();
      if (Number.isFinite(t) && t > 0) return t;
    }
    return tempoCache;
  }

  function lerpSampleI16Interleaved(
    samplesI16: Int16Array,
    frame: number,
    numChannels: number,
    ch: number
  ) {
    const totalFrames = Math.floor(samplesI16.length / numChannels);
    const f0 = Math.floor(frame) % totalFrames;
    const f1 = (f0 + 1) % totalFrames;
    const frac = frame - Math.floor(frame);
    const s0 = samplesI16[f0 * numChannels + ch];
    const s1 = samplesI16[f1 * numChannels + ch];
    return (s0 + (s1 - s0) * frac) | 0;
  }

  function sendOneBuffer() {
    const handle = sink.retainBuffer();
    if (!handle || !handle.isValid()) return false;

    const sessionState = link.captureAppSessionState();
    const beatsAtBufferBegin = sessionState.beatAtTime(nextSendTime, opts.quantum);

    const out = handle.samples();
    if (!out) return false;

    const maxFrames = Math.floor(handle.maxNumSamples() / wav.numChannels);
    const totalFrames = Math.floor(wav.samples.length / wav.numChannels);
    const framesToWrite = Math.min(opts.framesPerBuffer, totalFrames, maxFrames);

    const outI16 = new Int16Array(out.buffer, out.byteOffset, out.length / 2);
    let didWrap = false;

    if (syncMode === 'resample') {
      const tempo = getTempo(sessionState);
      const rate = tempo / referenceTempo;
      for (let i = 0; i < framesToWrite; i += 1) {
        const srcFrame = phase;
        for (let ch = 0; ch < wav.numChannels; ch += 1) {
          outI16[i * wav.numChannels + ch] = lerpSampleI16Interleaved(
            wav.samples,
            srcFrame,
            wav.numChannels,
            ch
          );
        }
        phase += rate;
        if (phase >= totalFrames) {
          phase %= totalFrames;
          didWrap = true;
        }
      }
      nextSendTime += bufferSeconds;
    } else {
      for (let i = 0; i < framesToWrite; i += 1) {
        const srcFrame = (frameIndex + i) % totalFrames;
        for (let ch = 0; ch < wav.numChannels; ch += 1) {
          outI16[i * wav.numChannels + ch] =
            wav.samples[srcFrame * wav.numChannels + ch];
        }
      }

      frameIndex = (frameIndex + framesToWrite) % totalFrames;
      if (frameIndex === 0) didWrap = true;

      nextSendTime += bufferSeconds;

      if (didWrap && syncMode === 'quantized' && loopLengthBeats) {
        const b = sessionState.beatAtTime(nextSendTime, loopQuantize);
        const nextQ = Math.ceil(b / loopQuantize) * loopQuantize;
        if (typeof sessionState.timeAtBeat === 'function') {
          nextSendTime = sessionState.timeAtBeat(nextQ, loopQuantize);
        }
      }
    }

    const ok = handle.commit(
      sessionState,
      beatsAtBufferBegin,
      opts.quantum,
      framesToWrite,
      wav.numChannels,
      wav.sampleRate
    );

    return ok;
  }

  function adjustAdaptiveLead(usedRecovery: boolean) {
    if (!opts.adaptiveLead) return;
    const now = Date.now();
    if (usedRecovery) {
      lastFailTimeMs = now;
      targetLeadSec = Math.min(
        opts.adaptiveLeadMaxSec ?? targetLeadSec,
        targetLeadSec + (opts.adaptiveLeadStepSec ?? 0.005)
      );
      lowWaterSec = Math.max(
        opts.adaptiveLeadMinSec ?? 0.005,
        targetLeadSec / 2
      );
      return;
    }

    if (now - lastFailTimeMs > (opts.adaptiveFailWindowMs ?? 1000)) {
      targetLeadSec = Math.max(
        opts.adaptiveLeadMinSec ?? targetLeadSec,
        targetLeadSec - (opts.adaptiveLeadStepSec ?? 0.005)
      );
      lowWaterSec = Math.max(
        opts.adaptiveLeadMinSec ?? 0.005,
        targetLeadSec / 2
      );
    }
  }

  function refill() {
    const now = link.getClockTime();
    if (nextSendTime - now > lowWaterSec) return;

    let sent = 0;
    const maxToSend = opts.maxBuffersPerTick ?? 8;

    while (sent < maxToSend) {
      const n = link.getClockTime();
      if (nextSendTime - n >= targetLeadSec) break;
      if (!sendOneBuffer()) {
        adjustAdaptiveLead(true);
        break;
      }
      sent += 1;
    }

    if (sent === 0) {
      scheduler.scheduleIn(0.001, refill);
    } else {
      adjustAdaptiveLead(false);
    }
  }

  function start() {
    scheduler.start();
    const now = link.getClockTime();
    const buffersAhead = Math.ceil(targetLeadSec / bufferSeconds);
    nextSendTime = now + buffersAhead * bufferSeconds;
    refill();
    const checkPeriodSec =
      opts.refillCheckPeriodSec ?? Math.min(0.005, bufferSeconds);
    periodic = scheduler.scheduleEvery(checkPeriodSec, refill, {
      startAt: now + checkPeriodSec,
    });
    return {
      stop: () => {
        stop();
      },
    };
  }

  function stop() {
    periodic?.cancel();
    periodic = null;
    scheduler.stop();
    if (unsubscribeTempo) {
      unsubscribeTempo();
      unsubscribeTempo = null;
    }
  }

  return { sink, wav, scheduler, start, stop };
}

export async function playWav(
  link: any,
  wavPath: string,
  options: WavPlayerOptions = {}
): Promise<WavSinkPlayer> {
  const absolutePath = path.isAbsolute(wavPath)
    ? wavPath
    : path.join(process.cwd(), wavPath);
  const player = createWavSinkPlayer(link, absolutePath, options);
  player.start();
  return player;
}
