import path from 'path';
import { AbletonLinkAudio, linkAudioUtils } from '../index.ts';

describe('linkAudioUtils', () => {
  let link: any;

  beforeAll(() => {
    link = new AbletonLinkAudio(120.0, 'test-peer-utils');
  });

  afterAll(() => {
    if (link && typeof link.close === 'function') {
      link.close();
    }
  });

  beforeEach(() => {
    link.enable(false);
    link.enableLinkAudio(false);
    link.setTempo(120.0);
  });

  test('createWavSinkPlayer wires scheduler and wav metadata', () => {
    link.enable(true);
    link.enableLinkAudio(true);
    const wavPath = path.join(process.cwd(), 'examples', 'test-125bpm.wav');
    const player = linkAudioUtils.createWavSinkPlayer(link, wavPath, {
      channelName: 'test-wav',
      framesPerBuffer: 256,
      targetLeadSec: 0.01,
      lowWaterSec: 0.005,
      syncMode: 'free',
    });
    expect(player).toBeDefined();
    expect(player.sink).toBeDefined();
    expect(player.wav).toBeDefined();
    expect(typeof player.wav.sampleRate).toBe('number');
    expect(typeof player.wav.numChannels).toBe('number');
    expect(player.scheduler).toBeDefined();
    expect(typeof player.scheduler.start).toBe('function');
    player.start();
    player.stop();
  });

  test('LinkTimeScheduler schedules callbacks', () => {
    const scheduler = new linkAudioUtils.LinkTimeScheduler(link, { coarseMs: 1 });
    scheduler.start();
    let fired = false;
    const handle = scheduler.scheduleIn(0.001, () => {
      fired = true;
    });
    handle.cancel();
    scheduler.scheduleIn(0.001, () => {
      fired = true;
    });
    scheduler.stop();
    expect(typeof fired).toBe('boolean');
  });

  test('createWavSinkPlayer supports resample mode', () => {
    link.enable(true);
    link.enableLinkAudio(true);
    const wavPath = path.join(process.cwd(), 'examples', 'test-125bpm.wav');
    const player = linkAudioUtils.createWavSinkPlayer(link, wavPath, {
      channelName: 'test-wav-resample',
      framesPerBuffer: 256,
      syncMode: 'resample',
      referenceTempo: 125,
      tempo: 125,
      adaptiveLead: true,
    });
    player.start();
    player.stop();
  });

  test('createWavSinkPlayer supports quantized mode', () => {
    link.enable(true);
    link.enableLinkAudio(true);
    const wavPath = path.join(process.cwd(), 'examples', 'test-125bpm.wav');
    const player = linkAudioUtils.createWavSinkPlayer(link, wavPath, {
      channelName: 'test-wav-quantized',
      framesPerBuffer: 256,
      syncMode: 'quantized',
      loopLengthBeats: 4,
      loopQuantize: 4,
    });
    player.start();
    player.stop();
  });

  test('readWavFileSync returns wav metadata', () => {
    const wav = linkAudioUtils.readWavFileSync(
      path.join(process.cwd(), 'examples', 'test-125bpm.wav')
    );
    expect(wav).toBeDefined();
    expect(typeof wav.sampleRate).toBe('number');
    expect(typeof wav.numChannels).toBe('number');
    expect(wav.samples).toBeInstanceOf(Int16Array);
  });

  test('readWavFile returns wav metadata async', async () => {
    const wav = await linkAudioUtils.readWavFile(
      path.join(process.cwd(), 'examples', 'test-125bpm.wav')
    );
    expect(wav).toBeDefined();
    expect(typeof wav.sampleRate).toBe('number');
    expect(typeof wav.numChannels).toBe('number');
    expect(wav.samples).toBeInstanceOf(Int16Array);
  });

  test('waitForChannel times out when no channel exists', async () => {
    link.enable(true);
    link.enableLinkAudio(true);
    await expect(
      linkAudioUtils.waitForChannel(link, (ch: { name: string }) => ch.name === 'nope', {
        timeoutMs: 50,
        pollMs: 10,
      })
    ).rejects.toThrow('Timed out');
  });

  test('createSourceIterator can close immediately', async () => {
    link.enable(true);
    link.enableLinkAudio(true);
    const iterator = linkAudioUtils.createSourceIterator(
      link,
      '0x0000000000000000'
    );
    const result = await iterator.return();
    expect(result.done).toBe(true);
  });
});
