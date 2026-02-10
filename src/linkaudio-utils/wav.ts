import fs from 'fs';

export interface WavFileData {
  samples: Int16Array;
  numChannels: number;
  sampleRate: number;
}

export function parseWav(buffer: Buffer): WavFileData {
  if (
    buffer.toString('ascii', 0, 4) !== 'RIFF' ||
    buffer.toString('ascii', 8, 12) !== 'WAVE'
  ) {
    throw new Error('Invalid WAV file');
  }

  let offset = 12;
  let fmt:
    | {
        audioFormat: number;
        numChannels: number;
        sampleRate: number;
        bitsPerSample: number;
      }
    | undefined;
  let dataOffset = -1;
  let dataSize = 0;

  while (offset + 8 <= buffer.length) {
    const chunkId = buffer.toString('ascii', offset, offset + 4);
    const chunkSize = buffer.readUInt32LE(offset + 4);
    const chunkStart = offset + 8;
    if (chunkId === 'fmt ') {
      fmt = {
        audioFormat: buffer.readUInt16LE(chunkStart),
        numChannels: buffer.readUInt16LE(chunkStart + 2),
        sampleRate: buffer.readUInt32LE(chunkStart + 4),
        bitsPerSample: buffer.readUInt16LE(chunkStart + 14),
      };
    } else if (chunkId === 'data') {
      dataOffset = chunkStart;
      dataSize = chunkSize;
      break;
    }
    offset = chunkStart + chunkSize + (chunkSize % 2);
  }

  if (!fmt || dataOffset < 0) {
    throw new Error('WAV missing fmt or data chunk');
  }
  if (fmt.audioFormat !== 1 || fmt.bitsPerSample !== 16) {
    throw new Error('Only PCM 16-bit WAV is supported');
  }

  const samples = new Int16Array(
    buffer.buffer,
    buffer.byteOffset + dataOffset,
    dataSize / 2
  );

  return {
    samples,
    numChannels: fmt.numChannels,
    sampleRate: fmt.sampleRate,
  };
}

export async function readWavFile(filePath: string): Promise<WavFileData> {
  const buffer = await fs.promises.readFile(filePath);
  return parseWav(buffer);
}

export function readWavFileSync(filePath: string): WavFileData {
  const buffer = fs.readFileSync(filePath);
  return parseWav(buffer);
}
