/**
 * Ableton Link - Real-time music synchronization
 */
export declare class AbletonLink {
  /**
   * Creates a new Ableton Link instance
   * @param bpm Initial tempo in beats per minute
   */
  constructor(bpm: number);

  /**
   * Enable or disable Link synchronization
   * @param enabled Whether to enable Link
   */
  enable(enabled: boolean): void;

  /**
   * Check if Link is currently enabled
   * @returns Whether Link is enabled
   */
  isEnabled(): boolean;

  /**
   * Get the current tempo
   * @returns Current tempo in beats per minute
   */
  getTempo(): number;

  /**
   * Set a new tempo
   * @param bpm New tempo in beats per minute
   */
  setTempo(bpm: number): void;

  /**
   * Get the current beat position
   * @returns Current beat position
   */
  getBeat(): number;

  /**
   * Get the current phase for a given quantum
   * @param quantum The quantum (beat subdivision) to use
   * @returns Current phase within the quantum (0.0 to quantum)
   */
  getPhase(quantum: number): number;

  /**
   * Get the number of connected Link peers
   * @returns Number of connected peers
   */
  getNumPeers(): number;

  /**
   * Set the playing state
   * @param playing Whether transport should be playing
   */
  setIsPlaying(playing: boolean): void;

  /**
   * Check if transport is currently playing
   * @returns Whether transport is playing
   */
  isPlaying(): boolean;

  /**
   * Force a specific beat at a specific time
   * @param beat The beat position to force
   * @param time The time in seconds
   * @param quantum The quantum to use for alignment
   */
  forceBeatAtTime(beat: number, time: number, quantum: number): void;

  /**
   * Get the time for a specific beat
   * @param beat The beat position
   * @param quantum The quantum to use
   * @returns Time in seconds
   */
  getTimeForBeat(beat: number, quantum: number): number;

  /**
   * Close the Link instance and release resources
   */
  close(): void;

  /**
   * Enable or disable start/stop synchronization
   * @param enabled Whether to enable start/stop sync
   */
  enableStartStopSync(enabled: boolean): void;

  /**
   * Check if start/stop synchronization is enabled
   * @returns Whether start/stop sync is enabled
   */
  isStartStopSyncEnabled(): boolean;

  /**
   * Set a callback to be notified when the number of peers changes
   * @param callback Function called with the new peer count
   */
  setNumPeersCallback(callback: (numPeers: number) => void): void;

  /**
   * Set a callback to be notified when the tempo changes
   * @param callback Function called with the new tempo in BPM
   */
  setTempoCallback(callback: (tempo: number) => void): void;

  /**
   * Set a callback to be notified when the play/stop state changes
   * @param callback Function called with the new playing state
   */
  setStartStopCallback(callback: (isPlaying: boolean) => void): void;

  /**
   * Request a specific beat to occur at a specific time with quantization
   * @param beat The desired beat position
   * @param time The time in seconds when the beat should occur
   * @param quantum The quantum for quantization
   */
  requestBeatAtTime(beat: number, time: number, quantum: number): void;

  /**
   * Request a specific beat to occur when transport starts playing
   * @param beat The desired beat position when playback starts
   * @param quantum The quantum for quantization
   */
  requestBeatAtStartPlayingTime(beat: number, quantum: number): void;

  /**
   * Set playing state and request a specific beat at a specific time
   * @param isPlaying Whether to start or stop playback
   * @param time The time in seconds for the state change
   * @param beat The desired beat position
   * @param quantum The quantum for quantization
   */
  setIsPlayingAndRequestBeatAtTime(
    isPlaying: boolean,
    time: number,
    beat: number,
    quantum: number
  ): void;

  /**
   * Get the time when the transport will start or stop
   * @returns Time in seconds when transport state will change
   */
  timeForIsPlaying(): number;

  onTempoChange(callback: (tempo: number) => void): () => void;
  onPeersChange(callback: (numPeers: number) => void): () => void;
  onStartStop(callback: (isPlaying: boolean) => void): () => void;
  onStart(callback: (isPlaying: true) => void): () => void;
  onStop(callback: (isPlaying: false) => void): () => void;
  waitForSync(options?: {
    timeoutMs?: number;
    pollMs?: number;
    requirePeers?: boolean;
    requireStartStopSync?: boolean;
  }): Promise<{ enabled: boolean; peers: number; startStopOk: boolean }>;
}

/**
 * LinkAudio identifier (hex string, e.g. "0x0123...")
 */
export type LinkAudioId = string;

/**
 * LinkAudio channel description
 */
export interface LinkAudioChannel {
  id: LinkAudioId;
  name: string;
  peerId: LinkAudioId;
  peerName: string;
}

/**
 * LinkAudio buffer info for received audio
 */
export declare class AbletonLinkAudioBufferInfo {
  numChannels(): number;
  numFrames(): number;
  sampleRate(): number;
  count(): number;
  sessionBeatTime(): number;
  tempo(): number;
  sessionId(): LinkAudioId;
  beginBeats(sessionState: AbletonLinkAudioSessionState, quantum: number): number | null;
  endBeats(sessionState: AbletonLinkAudioSessionState, quantum: number): number | null;
}

/**
 * LinkAudio sink buffer handle for writing samples
 */
export declare class AbletonLinkAudioSinkBufferHandle {
  isValid(): boolean;
  samples(): Buffer | null;
  maxNumSamples(): number;
  commit(
    sessionState: AbletonLinkAudioSessionState,
    beatsAtBufferBegin: number,
    quantum: number,
    numFrames: number,
    numChannels: number,
    sampleRate: number
  ): boolean;
}

/**
 * LinkAudio sink for sending audio
 */
export declare class AbletonLinkAudioSink {
  constructor(link: AbletonLinkAudio, name: string, maxNumSamples: number);
  name(): string;
  setName(name: string): void;
  requestMaxNumSamples(numSamples: number): void;
  maxNumSamples(): number;
  retainBuffer(): AbletonLinkAudioSinkBufferHandle | null;
}

/**
 * LinkAudio source for receiving audio
 */
export declare class AbletonLinkAudioSource {
  constructor(
    link: AbletonLinkAudio,
    channelId: LinkAudioId,
    callback: (buffer: { samples: Buffer; info: AbletonLinkAudioBufferInfo }) => void
  );
  id(): LinkAudioId | null;
  close(): void;
}

/**
 * LinkAudio session state
 */
export declare class AbletonLinkAudioSessionState {
  tempo(): number;
  setTempo(bpm: number, time: number): void;
  beatAtTime(time: number, quantum: number): number;
  phaseAtTime(time: number, quantum: number): number;
  timeAtBeat(beat: number, quantum: number): number;
  requestBeatAtTime(beat: number, time: number, quantum: number): void;
  forceBeatAtTime(beat: number, time: number, quantum: number): void;
  setIsPlaying(isPlaying: boolean, time: number): void;
  isPlaying(): boolean;
  timeForIsPlaying(): number;
  requestBeatAtStartPlayingTime(beat: number, quantum: number): void;
  setIsPlayingAndRequestBeatAtTime(
    isPlaying: boolean,
    time: number,
    beat: number,
    quantum: number
  ): void;
}

/**
 * Ableton LinkAudio wrapper with audio sharing
 */
export declare class AbletonLinkAudio {
  constructor(bpm: number, name: string);

  enable(enabled: boolean): void;
  isEnabled(): boolean;
  getTempo(): number;
  setTempo(bpm: number): void;
  getBeat(): number;
  getPhase(quantum: number): number;
  getNumPeers(): number;
  setIsPlaying(playing: boolean): void;
  isPlaying(): boolean;
  forceBeatAtTime(beat: number, time: number, quantum: number): void;
  getTimeForBeat(beat: number, quantum: number): number;
  enableStartStopSync(enabled: boolean): void;
  isStartStopSyncEnabled(): boolean;
  requestBeatAtTime(beat: number, time: number, quantum: number): void;
  requestBeatAtStartPlayingTime(beat: number, quantum: number): void;
  setIsPlayingAndRequestBeatAtTime(
    isPlaying: boolean,
    time: number,
    beat: number,
    quantum: number
  ): void;
  timeForIsPlaying(): number;
  getClockTime(): number;

  isLinkAudioEnabled(): boolean;
  enableLinkAudio(enabled: boolean): void;
  setPeerName(name: string): void;
  setChannelsChangedCallback(callback: () => void): void;
  channels(): LinkAudioChannel[];
  callOnLinkThread(callback: () => void): void;

  captureAppSessionState(): AbletonLinkAudioSessionState;
  commitAppSessionState(sessionState: AbletonLinkAudioSessionState): void;
  captureAudioSessionState(): AbletonLinkAudioSessionState;
  commitAudioSessionState(sessionState: AbletonLinkAudioSessionState): void;

  setNumPeersCallback(callback: (numPeers: number) => void): void;
  setTempoCallback(callback: (tempo: number) => void): void;
  setStartStopCallback(callback: (isPlaying: boolean) => void): void;

  close(): void;

  onTempoChange(callback: (tempo: number) => void): () => void;
  onPeersChange(callback: (numPeers: number) => void): () => void;
  onStartStop(callback: (isPlaying: boolean) => void): () => void;
  onStart(callback: (isPlaying: true) => void): () => void;
  onStop(callback: (isPlaying: false) => void): () => void;
  waitForSync(options?: {
    timeoutMs?: number;
    pollMs?: number;
    requirePeers?: boolean;
    requireStartStopSync?: boolean;
  }): Promise<{ enabled: boolean; peers: number; startStopOk: boolean }>;
}

export interface WavFileData {
  samples: Int16Array;
  numChannels: number;
  sampleRate: number;
}

export interface WavSinkPlayer {
  sink: AbletonLinkAudioSink;
  wav: WavFileData;
  scheduler: LinkTimeScheduler;
  start(): void;
  stop(): void;
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

export interface LinkTimeScheduler {
  now(): number;
  scheduleAt(
    timeSec: number,
    fn: (timeSec: number) => void
  ): { cancel(): void };
  scheduleIn(
    delaySec: number,
    fn: (timeSec: number) => void
  ): { cancel(): void };
  scheduleEvery(
    periodSec: number,
    fn: (timeSec: number) => void,
    options?: { startAt?: number | null }
  ): { cancel(): void };
  start(): void;
  stop(): void;
}

export declare const linkAudioUtils: {
  parseWav(buffer: Buffer): WavFileData;
  readWavFile(filePath: string): Promise<WavFileData>;
  readWavFileSync(filePath: string): WavFileData;
  sleep(ms: number, signal?: AbortSignal): Promise<void>;
  sleepUntilLinkTime(
    link: AbletonLinkAudio,
    targetTimeSec: number,
    signal?: AbortSignal,
    options?: { coarseMs?: number }
  ): Promise<void>;
  callOnLinkThreadAsync(link: AbletonLinkAudio): Promise<void>;
  waitForChannel(
    link: AbletonLinkAudio,
    predicate: (channel: LinkAudioChannel) => boolean,
    options?: { timeoutMs?: number; pollMs?: number }
  ): Promise<LinkAudioChannel>;
  createSourceIterator(
    link: AbletonLinkAudio,
    channelId: LinkAudioId
  ): AsyncIterable<{ samples: Buffer; info: AbletonLinkAudioBufferInfo }>;
  LinkTimeScheduler: { new (
    link: AbletonLinkAudio,
    options?: { coarseMs?: number }
  ): LinkTimeScheduler };
  DEFAULT_WAV_OPTIONS: Required<WavPlayerOptions>;
  createWavSinkPlayer(
    link: AbletonLinkAudio,
    wavPath: string,
    options?: WavPlayerOptions
  ): WavSinkPlayer;
  playWav(
    link: AbletonLinkAudio,
    wavPath: string,
    options?: WavPlayerOptions
  ): Promise<WavSinkPlayer>;
};

/**
 * Link state information
 */
export interface LinkState {
  /** Current tempo in BPM */
  tempo: number;
  /** Current beat position */
  beat: number;
  /** Current phase for 4-beat quantum */
  phase: number;
  /** Number of connected peers */
  peers: number;
  /** Whether transport is playing */
  playing: boolean;
}

/**
 * Callback for peer count changes
 */
export type PeerCountCallback = (numPeers: number) => void;

/**
 * Callback for tempo changes
 */
export type TempoCallback = (tempo: number) => void;
