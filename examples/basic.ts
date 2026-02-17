/* eslint-disable no-console */
import { AbletonLink } from '../index.ts';

const link = new AbletonLink(120.0);

console.log('Ableton Link initialized');
console.log('Initial tempo:', link.getTempo());
console.log('Initial beat:', link.getBeat());
console.log('Initial phase:', link.getPhase(4.0));
console.log('Initial number of peers:', link.getNumPeers());

link.enable(true);
link.enableStartStopSync(true);

console.log('Link enabled:', link.isEnabled());
console.log('Start/stop sync enabled:', link.isStartStopSyncEnabled());

setInterval(() => {
  console.log(
    `Tempo: ${link.getTempo()} BPM, Beat: ${link.getBeat()}, Phase: ${link.getPhase(4.0)}, Peers: ${link.getNumPeers()}, Playing: ${link.isPlaying()}`
  );
}, 1000);

process.on('SIGINT', () => {
  link.close();
  process.exit(0);
});
