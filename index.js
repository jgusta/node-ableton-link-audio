const bindings = require('bindings');
const addon = bindings('abletonlink');

// Export the native AbletonLink class
module.exports.AbletonLink = addon.AbletonLink;
module.exports.AbletonLinkAudio = addon.AbletonLinkAudio;
module.exports.AbletonLinkAudioSessionState = addon.AbletonLinkAudioSessionState;
module.exports.AbletonLinkAudioSink = addon.AbletonLinkAudioSink;
module.exports.AbletonLinkAudioSinkBufferHandle = addon.AbletonLinkAudioSinkBufferHandle;
module.exports.AbletonLinkAudioSource = addon.AbletonLinkAudioSource;
module.exports.AbletonLinkAudioBufferInfo = addon.AbletonLinkAudioBufferInfo;

// Convenience export for ES6 imports
module.exports.default = addon.AbletonLink;
