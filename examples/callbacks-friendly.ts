/* eslint-disable no-console */
import { AbletonLink } from '../index.ts';

const link = new AbletonLink(120.0);
link.enable(true);
link.enableStartStopSync(true);

const unsubTempo = link.onTempoChange((tempo: number) => {
  console.log('Tempo changed:', tempo);
});

const unsubPeers = link.onPeersChange((peers: number) => {
  console.log('Peers changed:', peers);
});

const unsubStart = link.onStart(() => {
  console.log('Started');
});

const unsubStop = link.onStop(() => {
  console.log('Stopped');
});

(async () => {
  await link.waitForSync({ timeoutMs: 5000, requirePeers: false });
  console.log('Link sync ready');

  link.setIsPlaying(true);
  setTimeout(() => link.setIsPlaying(false), 1000);
})();

process.on('SIGINT', () => {
  unsubTempo();
  unsubPeers();
  unsubStart();
  unsubStop();
  link.close();
  process.exit(0);
});
