import { AbletonLink } from '../index.ts';

describe('AbletonLink', () => {
  let link: any;

  beforeEach(() => {
    link = new AbletonLink(120.0);
  });

  afterEach(() => {
    if (link && typeof link.close === 'function') {
      link.close();
    }
  });

  test('should create instance with initial tempo', () => {
    expect(link).toBeDefined();
    expect(link.getTempo()).toBe(120.0);
  });

  test('should enable and disable Link', () => {
    expect(link.isEnabled()).toBe(false);
    link.enable(true);
    expect(link.isEnabled()).toBe(true);
    link.enable(false);
    expect(link.isEnabled()).toBe(false);
  });

  test('should set and get tempo', () => {
    link.setTempo(135.0);
    expect(link.getTempo()).toBe(135.0);
  });

  test('should get beat and phase', () => {
    const beat = link.getBeat();
    const phase = link.getPhase(4.0);
    expect(typeof beat).toBe('number');
    expect(typeof phase).toBe('number');
  });

  test('should get number of peers', () => {
    const peers = link.getNumPeers();
    expect(typeof peers).toBe('number');
  });

  test('should control playing state', () => {
    link.enableStartStopSync(true);
    link.setIsPlaying(true);
    expect(link.isPlaying()).toBe(true);
    link.setIsPlaying(false);
    expect(link.isPlaying()).toBe(false);
  });

  test('should enable start/stop sync', () => {
    link.enableStartStopSync(true);
    expect(link.isStartStopSyncEnabled()).toBe(true);
  });

  test('should set callbacks', () => {
    link.setNumPeersCallback(() => {});
    link.setTempoCallback(() => {});
    link.setStartStopCallback(() => {});
  });

  test('should handle quantized methods', () => {
    const quantum = 4.0;
    const beat = link.getBeat();
    const phase = link.getPhase(quantum);
    expect(typeof beat).toBe('number');
    expect(typeof phase).toBe('number');
  });

  test('should force beat at time', () => {
    const time = typeof link.getClockTime === 'function' ? link.getClockTime() : Date.now() / 1000;
    link.forceBeatAtTime(4.0, time, 4.0);
  });
});
