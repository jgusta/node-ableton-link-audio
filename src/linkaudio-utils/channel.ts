export async function waitForChannel<T>(
  link: { channels: () => T[] },
  predicate: (channel: T) => boolean,
  options: { timeoutMs?: number; pollMs?: number } = {}
): Promise<T> {
  const { timeoutMs = 5000, pollMs = 50 } = options;
  const start = Date.now();
  while (Date.now() - start < timeoutMs) {
    const channels = link.channels();
    const match = channels.find(predicate);
    if (match) {
      return match;
    }
    await new Promise((resolve) => setTimeout(resolve, pollMs));
  }
  throw new Error('Timed out waiting for channel');
}
