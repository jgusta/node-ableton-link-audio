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

  test('should create Link instance with initial tempo', () => {
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
    link.setTempo(130.0);
    expect(link.getTempo()).toBe(130.0);
  });

  test('should get beat position', () => {
    const beat = link.getBeat();
    expect(typeof beat).toBe('number');
  });

  test('should get phase for quantum', () => {
    const phase = link.getPhase(4.0);
    expect(typeof phase).toBe('number');
  });

  test('should get number of peers', () => {
    const peers = link.getNumPeers();
    expect(typeof peers).toBe('number');
  });

  test('should set and get playing state', () => {
    link.enableStartStopSync(true);
    link.setIsPlaying(true);
    expect(link.isPlaying()).toBe(true);
    link.setIsPlaying(false);
    expect(link.isPlaying()).toBe(false);
  });

  test('should get time for beat', () => {
    const time = link.getTimeForBeat(4.0, 4.0);
    expect(typeof time).toBe('number');
  });

  test('should force beat at time', () => {
    const time = typeof link.getClockTime === 'function' ? link.getClockTime() : Date.now() / 1000;
    link.forceBeatAtTime(4.0, time, 4.0);
  });

  test('should handle invalid constructor arguments', () => {
    expect(() => new (AbletonLink as any)('invalid')).toThrow();
  });

  test('should handle invalid method arguments', () => {
    expect(() => link.setTempo('invalid')).toThrow();
  });
});
