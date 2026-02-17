import { createRequire } from 'module';
import * as linkAudioUtils from './src/linkaudio-utils/index.ts';

const require = createRequire(import.meta.url);
const bindings = require('bindings') as (name: string) => any;
const addon = bindings('abletonlink') as any;

export const AbletonLink = addon.AbletonLink;
export const AbletonLinkAudio = addon.AbletonLinkAudio;
export const AbletonLinkAudioSessionState = addon.AbletonLinkAudioSessionState;
export const AbletonLinkAudioSink = addon.AbletonLinkAudioSink;
export const AbletonLinkAudioSinkBufferHandle = addon.AbletonLinkAudioSinkBufferHandle;
export const AbletonLinkAudioSource = addon.AbletonLinkAudioSource;
export const AbletonLinkAudioBufferInfo = addon.AbletonLinkAudioBufferInfo;
export { linkAudioUtils };

export interface LinkState {
  tempo: number;
  beat: number;
  phase: number;
  peers: number;
  playing: boolean;
}

export default addon.AbletonLink;

function addLinkHelpers(LinkClass: any) {
  if (!LinkClass || !LinkClass.prototype) return;

  LinkClass.prototype.onTempoChange = function onTempoChange(cb: (tempo: number) => void) {
    if (!this.__tempoHandlers) {
      this.__tempoHandlers = new Set();
      this.setTempoCallback((tempo: number) => {
        for (const handler of this.__tempoHandlers) {
          handler(tempo);
        }
      });
    }
    this.__tempoHandlers.add(cb);
    return () => this.__tempoHandlers.delete(cb);
  };

  LinkClass.prototype.onPeersChange = function onPeersChange(
    cb: (numPeers: number) => void
  ) {
    if (!this.__peersHandlers) {
      this.__peersHandlers = new Set();
      this.setNumPeersCallback((numPeers: number) => {
        for (const handler of this.__peersHandlers) {
          handler(numPeers);
        }
      });
    }
    this.__peersHandlers.add(cb);
    return () => this.__peersHandlers.delete(cb);
  };

  LinkClass.prototype.onStartStop = function onStartStop(
    cb: (isPlaying: boolean) => void
  ) {
    if (!this.__startStopHandlers) {
      this.__startStopHandlers = new Set();
      this.setStartStopCallback((isPlaying: boolean) => {
        for (const handler of this.__startStopHandlers) {
          handler(isPlaying);
        }
      });
    }
    this.__startStopHandlers.add(cb);
    return () => this.__startStopHandlers.delete(cb);
  };

  LinkClass.prototype.onStart = function onStart(cb: (isPlaying: true) => void) {
    return this.onStartStop((isPlaying: boolean) => {
      if (isPlaying) cb(true);
    });
  };

  LinkClass.prototype.onStop = function onStop(cb: (isPlaying: false) => void) {
    return this.onStartStop((isPlaying: boolean) => {
      if (!isPlaying) cb(false);
    });
  };

  LinkClass.prototype.waitForSync = async function waitForSync(options: {
    timeoutMs?: number;
    pollMs?: number;
    requirePeers?: boolean;
    requireStartStopSync?: boolean;
  } = {}) {
    const {
      timeoutMs = 5000,
      pollMs = 50,
      requirePeers = true,
      requireStartStopSync = false,
    } = options;
    const start = Date.now();
    while (Date.now() - start < timeoutMs) {
      const enabled = this.isEnabled();
      const peers = this.getNumPeers ? this.getNumPeers() : 0;
      const startStopOk =
        !requireStartStopSync || this.isStartStopSyncEnabled();
      const peersOk = !requirePeers || peers > 0;
      if (enabled && peersOk && startStopOk) {
        return { enabled, peers, startStopOk };
      }
      await new Promise((resolve) => setTimeout(resolve, pollMs));
    }
    throw new Error('Timed out waiting for Link sync');
  };
}

addLinkHelpers(addon.AbletonLink);
addLinkHelpers(addon.AbletonLinkAudio);
