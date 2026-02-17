export function sleep(ms: number, signal?: AbortSignal): Promise<void> {
  if (ms <= 0) return Promise.resolve();
  return new Promise((resolve, reject) => {
    const t = setTimeout(resolve, ms);
    const onAbort = () => {
      clearTimeout(t);
      reject(Object.assign(new Error('aborted'), { name: 'AbortError' }));
    };
    if (signal?.aborted) return onAbort();
    signal?.addEventListener('abort', onAbort, { once: true });
  });
}

export async function sleepUntilLinkTime(
  link: { getClockTime: () => number },
  targetTimeSec: number,
  signal?: AbortSignal,
  { coarseMs = 2 }: { coarseMs?: number } = {}
): Promise<void> {
  for (;;) {
    const now = link.getClockTime();
    const dtMs = (targetTimeSec - now) * 1000;
    if (dtMs <= 0) return;

    if (dtMs > coarseMs) {
      await sleep(dtMs - coarseMs, signal);
    } else {
      await Promise.resolve();
    }
  }
}

export class LinkTimeScheduler {
  private link: { getClockTime: () => number };
  private coarseMs: number;
  private _queue: { time: number; fn: (time: number) => void; cancelled: boolean }[] = [];
  private _running = false;
  private _abort: AbortController | null = null;
  private _wake: (() => void) | null = null;

  constructor(link: { getClockTime: () => number }, { coarseMs = 2 }: { coarseMs?: number } = {}) {
    this.link = link;
    this.coarseMs = coarseMs;
  }

  now(): number {
    return this.link.getClockTime();
  }

  private _insert(ev: { time: number; fn: (time: number) => void; cancelled: boolean }) {
    const q = this._queue;
    let i = q.findIndex((e) => e.time > ev.time);
    if (i === -1) i = q.length;
    q.splice(i, 0, ev);
  }

  scheduleAt(timeSec: number, fn: (time: number) => void) {
    const ev = { time: timeSec, fn, cancelled: false };
    const wasEarliest = this._queue.length === 0 || timeSec < this._queue[0].time;
    this._insert(ev);
    if (wasEarliest && this._wake) {
      this._wake();
      this._wake = null;
    }
    return {
      cancel: () => {
        ev.cancelled = true;
        if (this._queue[0] === ev && this._wake) {
          this._wake();
          this._wake = null;
        }
      },
    };
  }

  scheduleIn(delaySec: number, fn: (time: number) => void) {
    return this.scheduleAt(this.now() + delaySec, fn);
  }

  scheduleEvery(
    periodSec: number,
    fn: (time: number) => void,
    { startAt = null }: { startAt?: number | null } = {}
  ) {
    let next = startAt ?? (this.now() + periodSec);
    let handle: { cancel: () => void } | null = null;

    const tick = () => {
      fn(next);
      next += periodSec;
      handle = this.scheduleAt(next, tick);
    };

    handle = this.scheduleAt(next, tick);
    return { cancel: () => handle?.cancel() };
  }

  start() {
    if (this._running) return;
    this._running = true;
    this._abort = new AbortController();
    void this._loop(this._abort.signal).catch((e) => {
      if (e?.name !== 'AbortError') throw e;
    });
  }

  stop() {
    if (!this._running) return;
    this._running = false;
    this._abort?.abort();
    this._abort = null;
    this._queue.length = 0;
    this._wake?.();
    this._wake = null;
  }

  private async _waitForWork(signal: AbortSignal) {
    if (this._queue.length) return;
    await new Promise<void>((resolve, reject) => {
      this._wake = resolve;
      const onAbort = () =>
        reject(Object.assign(new Error('aborted'), { name: 'AbortError' }));
      if (signal?.aborted) return onAbort();
      signal?.addEventListener('abort', onAbort, { once: true });
    });
  }

  private async _loop(signal: AbortSignal) {
    while (!signal.aborted) {
      await this._waitForWork(signal);
      while (this._queue.length && this._queue[0].cancelled) this._queue.shift();
      if (!this._queue.length) continue;

      const nextTime = this._queue[0].time;
      const now = this.now();
      if (now < nextTime) {
        await Promise.race([
          sleepUntilLinkTime(this.link, nextTime, signal, { coarseMs: this.coarseMs }),
          new Promise<void>((resolve, reject) => {
            this._wake = resolve;
            const onAbort = () =>
              reject(Object.assign(new Error('aborted'), { name: 'AbortError' }));
            if (signal?.aborted) return onAbort();
            signal?.addEventListener('abort', onAbort, { once: true });
          }),
        ]);
        this._wake = null;
        continue;
      }

      const due: { time: number; fn: (time: number) => void; cancelled: boolean }[] = [];
      const fireNow = this.now();
      while (this._queue.length && this._queue[0].time <= fireNow) {
        const ev = this._queue.shift();
        if (ev && !ev.cancelled) due.push(ev);
      }

      for (const ev of due) {
        try {
          ev.fn(ev.time);
        } catch {
          // swallow errors in scheduler callback
        }
      }
    }
  }
}
