import {
  AbletonLinkAudio,
  AbletonLinkAudioSink,
  AbletonLinkAudioSource,
} from '../index.ts';

describe('AbletonLinkAudio', () => {
  let link: any;

  beforeAll(() => {
    link = new AbletonLinkAudio(120.0, 'test-peer');
  });

  afterAll(() => {
    if (link && typeof link.close === 'function') {
      link.close();
    }
  });

  beforeEach(() => {
    if (link) {
      link.enable(false);
      link.enableLinkAudio(false);
      link.setTempo(120.0);
    }
  });

  test('should create instance with initial tempo and peer name', () => {
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

  test('should enable and disable LinkAudio', () => {
    expect(link.isLinkAudioEnabled()).toBe(false);
    link.enableLinkAudio(true);
    expect(link.isLinkAudioEnabled()).toBe(true);
    link.enableLinkAudio(false);
    expect(link.isLinkAudioEnabled()).toBe(false);
  });

  test('should manage channels and callbacks', () => {
    link.setChannelsChangedCallback(() => {});
    const channels = link.channels();
    expect(Array.isArray(channels)).toBe(true);
  });

  test('should capture and commit session state', () => {
    const state = link.captureAppSessionState();
    expect(state.tempo()).toBe(120.0);
    state.setTempo(123.0, Date.now() / 1000);
    link.commitAppSessionState(state);
  });

  test('should call on link thread', (done) => {
    link.callOnLinkThread(() => {
      done();
    });
  });

  test('should create sink and retain buffer safely', () => {
    const sink = new AbletonLinkAudioSink(link, 'test-channel', 1024);
    expect(sink.name()).toBe('test-channel');
    sink.requestMaxNumSamples(2048);
    expect(sink.maxNumSamples()).toBeGreaterThanOrEqual(1024);
    const handle = sink.retainBuffer();
    if (handle) {
      expect(handle.isValid()).toBe(true);
    }
  });

  test('should create source with dummy channel id', () => {
    const source = new AbletonLinkAudioSource(link, '0x0000000000000000', () => {});
    expect(source).toBeDefined();
    source.close();
  });

  // linkAudioUtils tests live in test/linkaudio-utils.test.ts
});
