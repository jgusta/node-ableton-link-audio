import { createRequire } from 'module';

const require = createRequire(import.meta.url);
const bindings = require('bindings') as (name: string) => any;
const addon = bindings('abletonlink') as any;

export interface SourceIteratorPayload {
  samples: Buffer;
  info: any;
}

export function createSourceIterator(link: any, channelId: string) {
  const queue: SourceIteratorPayload[] = [];
  let resolver: ((value: IteratorResult<SourceIteratorPayload>) => void) | null = null;
  let closed = false;

  const source = new addon.AbletonLinkAudioSource(
    link,
    channelId,
    (payload: SourceIteratorPayload) => {
      if (resolver) {
        const resolve = resolver;
        resolver = null;
        resolve({ value: payload, done: false });
      } else {
        queue.push(payload);
      }
    }
  );

  return {
    [Symbol.asyncIterator]() {
      return this;
    },
    next(): Promise<IteratorResult<SourceIteratorPayload>> {
      if (closed) {
        return Promise.resolve({ value: undefined, done: true });
      }
      if (queue.length > 0) {
        return Promise.resolve({ value: queue.shift()!, done: false });
      }
      return new Promise((resolve) => {
        resolver = resolve;
      });
    },
    return(): Promise<IteratorResult<SourceIteratorPayload>> {
      closed = true;
      source.close();
      return Promise.resolve({ done: true, value: undefined });
    },
  };
}
