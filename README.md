# @jgusta/abletonlinkaudio

This is a fork of `@ktamas77/abletonlink`: https://github.com/ktamas77/ableton-link

Node.ts native bindings for Ableton Link - Real-time music synchronization

## Overview

This package provides a TypeScript/Node.ts wrapper around the native Ableton Link C++ SDK to expose tempo sync functionality in JavaScript.

## Features

- **Tempo Synchronization**: Get/set tempo (BPM) across connected peers
- **Beat & Phase Information**: Query beat position and phase for quantized synchronization
- **Transport Control**: Start/stop playback state synchronization
- **Peer Discovery**: Automatic discovery of Link-enabled applications on the network
- **Quantum Support**: Beat subdivision for perfect loop synchronization
- **TypeScript Support**: Full type definitions for a great developer experience

## Installation

```bash
npm install @jgusta/abletonlinkaudio
```

Note: This package includes native bindings and will be compiled during installation.

## Usage

```typescript
import { AbletonLink } from '@jgusta/abletonlinkaudio';

// Create a new Link instance with initial tempo
const link = new AbletonLink(120.0);

// Enable Link
link.enable(true);

// Enable start/stop sync to synchronize transport state
link.enableStartStopSync(true);

// Get current tempo
console.log('Current tempo:', link.getTempo());

// Set new tempo
link.setTempo(128.0);

// Get number of connected peers
console.log('Connected peers:', link.getNumPeers());

// Query beat information
const beat = link.getBeat();
const phase = link.getPhase(4.0); // 4-beat quantum

// Transport control
link.setIsPlaying(true);
console.log('Is playing:', link.isPlaying());

// Set up callbacks for real-time notifications
link.setNumPeersCallback((numPeers) => {
  console.log(`Peers changed: ${numPeers}`);
});

link.setTempoCallback((tempo) => {
  console.log(`Tempo changed: ${tempo} BPM`);
});

link.setStartStopCallback((isPlaying) => {
  console.log(`Playing state: ${isPlaying}`);
});
```

### LinkAudio (audio sharing)

```typescript
import { AbletonLinkAudio, linkAudioUtils } from '@jgusta/abletonlinkaudio';

const linkAudio = new AbletonLinkAudio(120.0, 'my-peer');
linkAudio.enable(true);
linkAudio.enableLinkAudio(true);

// Channels exposed by peers
const channels = linkAudio.channels();
console.log(channels);
```

### LinkAudio WAV playback (buffered)

`linkAudioUtils` includes a Link-time scheduler and a WAV sink player that
maintains a small lead-time buffer to absorb JS event-loop jitter.

```typescript
const player = linkAudioUtils.createWavSinkPlayer(linkAudio, './loop.wav', {
  channelName: 'main',
  framesPerBuffer: 512,
  maxBuffersPerTick: 8,
  quantum: 4,
  targetLeadSec: 0.02,
  lowWaterSec: 0.01,
});

player.start();
// later: player.stop();
```

Key options:
- `framesPerBuffer`: smaller = lower latency, higher underrun risk
- `targetLeadSec`: amount of audio time to keep queued ahead
- `lowWaterSec`: refill trigger threshold
- `refillCheckPeriodSec`: how often the refill check runs (Link time)
- `schedulerCoarseMs`: coarse sleep margin for the scheduler

## Examples

The `examples/` directory contains several scripts demonstrating different features:

### basic.ts

A simple example showing core Link functionality including tempo sync, beat/phase tracking, and transport control. Great starting point for understanding the basics.

### callbacks.ts

Demonstrates the callback system for real-time notifications when peers connect/disconnect, tempo changes, or transport starts/stops. Shows how to build reactive applications.

### quantized-launch.ts

Advanced example showing quantized beat alignment and synchronized starts. Essential for DAW-like applications that need sample-accurate synchronization with other Link peers.

### auto-tempo.ts

Shows how to automatically adopt the tempo from an existing Link session. Useful for applications that want to immediately sync with whatever session is already running.

### monitor-playing.ts

Real-time monitor for the isPlaying state. Shows current play/stop status, tempo, beat position, and detects state changes. Useful for debugging transport synchronization issues.

### diagnose-playing.ts

Diagnostic tool that checks all prerequisites for isPlaying functionality including start/stop sync, network connectivity, and peer discovery. Provides troubleshooting hints if synchronization isn't working.

### diagnose-playing-sync.ts

Advanced diagnostic that analyzes the synchronization sequence when joining a session, helping identify timing issues with play state synchronization.

### sync-with-playing.ts

Shows how to properly wait for initial synchronization when joining a Link session that may already be playing.

### join-playing-session.ts

Best practices example demonstrating the recommended initialization sequence for reliable synchronization with existing Link sessions.

### test-initial-sync.ts

Tests various timing scenarios for enableStartStopSync to understand synchronization behavior.

### force-sync-workaround.ts

Demonstrates workarounds to force synchronization when joining a session that's already playing.

### audio-sink-play-wav.ts

Plays a WAV file into a LinkAudio sink using the buffered scheduler utilities.

### typescript-example.ts

Demonstrates TypeScript usage with full type safety and autocompletion support.

Build and run any example with:

```bash
pnpm build
node dist/examples/basic.ts
```

## API Reference

### `new AbletonLink(bpm: number)`

Creates a new Ableton Link instance with the specified initial tempo.

### `enable(enabled: boolean): void`

Enable or disable Link synchronization.

### `isEnabled(): boolean`

Check if Link is currently enabled.

### `getTempo(): number`

Get the current tempo in BPM.

### `setTempo(bpm: number): void`

Set a new tempo in BPM.

### `getNumPeers(): number`

Get the number of connected Link peers.

### `getBeat(): number`

Get the current beat position.

### `getPhase(quantum: number): number`

Get the current phase for the given quantum (beat subdivision).

### `isPlaying(): boolean`

Check if transport is playing.

**Important:** Start/stop sync must be enabled with `enableStartStopSync(true)` for this method to work correctly. Without start/stop sync, Link only synchronizes tempo and beat position, not play/stop state.

**Note on synchronization:** When joining a Link session that's already playing, there may be a brief delay before the play state synchronizes. The initial state will be "stopped" until the Link protocol completes synchronization. Use the start/stop callback to be notified when the state updates. See the `join-playing-session.ts` example for best practices.

### `setIsPlaying(playing: boolean): void`

Start or stop transport playback.

### `enableStartStopSync(enabled: boolean): void`

Enable or disable start/stop synchronization with other Link peers. When enabled, play/stop state changes will be synchronized across all connected applications.

### `isStartStopSyncEnabled(): boolean`

Check if start/stop synchronization is enabled.

### `forceBeatAtTime(beat: number, time: number, quantum: number): void`

Forces a beat at a specific time with the given quantum.

## Acknowledgements

This package is a fork of `@ktamas77/abletonlink` (https://github.com/ktamas77/ableton-link). Thanks to @ktamas77 for the original project and ongoing work.
