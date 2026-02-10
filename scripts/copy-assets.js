import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const assets = [
  {
    src: path.join(__dirname, '..', 'examples', 'test-125bpm.wav'),
    dest: path.join(__dirname, '..', 'dist', 'examples', 'test-125bpm.wav'),
  },
];

for (const asset of assets) {
  if (!fs.existsSync(asset.src)) {
    console.error(`Asset missing: ${asset.src}`);
    process.exit(1);
  }
  fs.mkdirSync(path.dirname(asset.dest), { recursive: true });
  fs.copyFileSync(asset.src, asset.dest);
}
