import { AbletonLink } from '../index.ts';

describe('AbletonLink JS helpers', () => {
  let link: any;

  beforeEach(() => {
    link = new AbletonLink(120.0);
  });

  afterEach(() => {
    if (link && typeof link.close === 'function') {
      link.close();
    }
  });

  test('onTempoChange registers and can unsubscribe', (done) => {
    const unsub = link.onTempoChange((tempo: number) => {
      try {
        expect(typeof tempo).toBe('number');
        unsub();
        done();
      } catch (err) {
        done(err as Error);
      }
    });

    link.setTempo(123.0);
  });

  test('onStart/onStop respond to playing changes', (done) => {
    let started = false;
    const unsubStart = link.onStart(() => {
      started = true;
    });
    const unsubStop = link.onStop(() => {
      try {
        expect(started).toBe(true);
        unsubStart();
        unsubStop();
        done();
      } catch (err) {
        done(err as Error);
      }
    });

    link.setIsPlaying(true);
    setTimeout(() => {
      link.setIsPlaying(false);
    }, 25);
  });

  test('onStartStop supports multiple handlers and unsubscribe', (done) => {
    let calls = 0;
    const unsubA = link.onStartStop(() => {
      calls += 1;
    });
    const unsubB = link.onStartStop(() => {
      calls += 1;
    });

    link.setIsPlaying(true);
    setTimeout(() => {
      unsubA();
      unsubB();
      try {
        expect(calls).toBeGreaterThanOrEqual(2);
        done();
      } catch (err) {
        done(err as Error);
      }
    }, 25);
  });

  test('waitForSync resolves when enabled', async () => {
    link.enable(true);
    const result = await link.waitForSync({ timeoutMs: 1000, requirePeers: false });
    expect(result.enabled).toBe(true);
  });

  test('waitForSync respects start/stop requirement', async () => {
    link.enable(true);
    link.enableStartStopSync(true);
    const result = await link.waitForSync({
      timeoutMs: 1000,
      requirePeers: false,
      requireStartStopSync: true,
    });
    expect(result.startStopOk).toBe(true);
  });
});
