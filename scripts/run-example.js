import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const name = process.argv[2];
if (!name) {
  console.error('Usage: pnpm example <name>');
  console.error('Example: pnpm example auto-tempo');
  process.exit(1);
}

const file = path.join(__dirname, '..', 'dist', 'examples', `${name}.js`);
if (!fs.existsSync(file)) {
  console.error(`Example not found: ${file}`);
  console.error('Did you run `pnpm build`?');
  process.exit(1);
}

await import(path.toNamespacedPath(file));
