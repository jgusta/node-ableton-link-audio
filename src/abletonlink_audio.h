#ifndef ABLETONLINK_AUDIO_H
#define ABLETONLINK_AUDIO_H

#include <napi.h>
#include <ableton/LinkAudio.hpp>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

class AbletonLinkAudioSessionStateWrapper
    : public Napi::ObjectWrap<AbletonLinkAudioSessionStateWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::Object New(Napi::Env env, const ableton::LinkAudio::SessionState& state);

    AbletonLinkAudioSessionStateWrapper(const Napi::CallbackInfo& info);

    ableton::LinkAudio::SessionState& State();

private:
    static Napi::FunctionReference constructor;

    Napi::Value Tempo(const Napi::CallbackInfo& info);
    void SetTempo(const Napi::CallbackInfo& info);
    Napi::Value BeatAtTime(const Napi::CallbackInfo& info);
    Napi::Value PhaseAtTime(const Napi::CallbackInfo& info);
    Napi::Value TimeAtBeat(const Napi::CallbackInfo& info);
    void RequestBeatAtTime(const Napi::CallbackInfo& info);
    void ForceBeatAtTime(const Napi::CallbackInfo& info);
    void SetIsPlaying(const Napi::CallbackInfo& info);
    Napi::Value IsPlaying(const Napi::CallbackInfo& info);
    Napi::Value TimeForIsPlaying(const Napi::CallbackInfo& info);
    void RequestBeatAtStartPlayingTime(const Napi::CallbackInfo& info);
    void SetIsPlayingAndRequestBeatAtTime(const Napi::CallbackInfo& info);

    std::unique_ptr<ableton::LinkAudio::SessionState> state_;
};

class AbletonLinkAudioWrapper : public Napi::ObjectWrap<AbletonLinkAudioWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    AbletonLinkAudioWrapper(const Napi::CallbackInfo& info);
    ~AbletonLinkAudioWrapper();

    ableton::LinkAudio& LinkAudio();

private:
    static Napi::FunctionReference constructor;

    std::unique_ptr<ableton::LinkAudio> link_;

    Napi::Value Enable(const Napi::CallbackInfo& info);
    Napi::Value IsEnabled(const Napi::CallbackInfo& info);
    Napi::Value GetTempo(const Napi::CallbackInfo& info);
    void SetTempo(const Napi::CallbackInfo& info);
    Napi::Value GetBeat(const Napi::CallbackInfo& info);
    Napi::Value GetPhase(const Napi::CallbackInfo& info);
    Napi::Value GetNumPeers(const Napi::CallbackInfo& info);
    void SetIsPlaying(const Napi::CallbackInfo& info);
    Napi::Value IsPlaying(const Napi::CallbackInfo& info);
    void EnableStartStopSync(const Napi::CallbackInfo& info);
    Napi::Value IsStartStopSyncEnabled(const Napi::CallbackInfo& info);
    void ForceBeatAtTime(const Napi::CallbackInfo& info);
    Napi::Value GetTimeForBeat(const Napi::CallbackInfo& info);
    void RequestBeatAtTime(const Napi::CallbackInfo& info);
    void RequestBeatAtStartPlayingTime(const Napi::CallbackInfo& info);
    void SetIsPlayingAndRequestBeatAtTime(const Napi::CallbackInfo& info);
    Napi::Value TimeForIsPlaying(const Napi::CallbackInfo& info);

    Napi::Value IsLinkAudioEnabled(const Napi::CallbackInfo& info);
    void EnableLinkAudio(const Napi::CallbackInfo& info);
    void SetPeerName(const Napi::CallbackInfo& info);
    void SetChannelsChangedCallback(const Napi::CallbackInfo& info);
    Napi::Value Channels(const Napi::CallbackInfo& info);
    void CallOnLinkThread(const Napi::CallbackInfo& info);

    Napi::Value CaptureAppSessionState(const Napi::CallbackInfo& info);
    void CommitAppSessionState(const Napi::CallbackInfo& info);
    Napi::Value CaptureAudioSessionState(const Napi::CallbackInfo& info);
    void CommitAudioSessionState(const Napi::CallbackInfo& info);

    void SetNumPeersCallback(const Napi::CallbackInfo& info);
    void SetTempoCallback(const Napi::CallbackInfo& info);
    void SetStartStopCallback(const Napi::CallbackInfo& info);
    void Close(const Napi::CallbackInfo& info);

    std::chrono::microseconds getCurrentTime() const;
    void handleNumPeersCallback(std::size_t numPeers);
    void handleTempoCallback(double tempo);
    void handleStartStopCallback(bool isPlaying);
    void handleChannelsChangedCallback();

    void CloseInternal();

    std::mutex callbackMutex_;
    Napi::ThreadSafeFunction numPeersCallback_;
    Napi::ThreadSafeFunction tempoCallback_;
    Napi::ThreadSafeFunction startStopCallback_;
    Napi::ThreadSafeFunction channelsChangedCallback_;
};

class AbletonLinkAudioSinkBufferHandleWrapper
    : public Napi::ObjectWrap<AbletonLinkAudioSinkBufferHandleWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::Object New(Napi::Env env,
                            std::unique_ptr<ableton::LinkAudioSink::BufferHandle> handle);

    AbletonLinkAudioSinkBufferHandleWrapper(const Napi::CallbackInfo& info);
    ~AbletonLinkAudioSinkBufferHandleWrapper();

private:
    static Napi::FunctionReference constructor;

    Napi::Value IsValid(const Napi::CallbackInfo& info);
    Napi::Value Samples(const Napi::CallbackInfo& info);
    Napi::Value MaxNumSamples(const Napi::CallbackInfo& info);
    Napi::Value Commit(const Napi::CallbackInfo& info);

    std::unique_ptr<ableton::LinkAudioSink::BufferHandle> handle_;
    Napi::Reference<Napi::Buffer<int16_t>> samples_;
};

class AbletonLinkAudioSinkWrapper : public Napi::ObjectWrap<AbletonLinkAudioSinkWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    AbletonLinkAudioSinkWrapper(const Napi::CallbackInfo& info);
    ~AbletonLinkAudioSinkWrapper();

private:
    static Napi::FunctionReference constructor;

    Napi::Value Name(const Napi::CallbackInfo& info);
    void SetName(const Napi::CallbackInfo& info);
    void RequestMaxNumSamples(const Napi::CallbackInfo& info);
    Napi::Value MaxNumSamples(const Napi::CallbackInfo& info);
    Napi::Value RetainBuffer(const Napi::CallbackInfo& info);

    std::shared_ptr<ableton::LinkAudioSink> sink_;
    Napi::ObjectReference linkRef_;
};

class AbletonLinkAudioBufferInfoWrapper
    : public Napi::ObjectWrap<AbletonLinkAudioBufferInfoWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::Object New(
        Napi::Env env, const ableton::LinkAudioSource::BufferHandle::Info& info);

    AbletonLinkAudioBufferInfoWrapper(const Napi::CallbackInfo& info);

private:
    static Napi::FunctionReference constructor;

    Napi::Value NumChannels(const Napi::CallbackInfo& info);
    Napi::Value NumFrames(const Napi::CallbackInfo& info);
    Napi::Value SampleRate(const Napi::CallbackInfo& info);
    Napi::Value Count(const Napi::CallbackInfo& info);
    Napi::Value SessionBeatTime(const Napi::CallbackInfo& info);
    Napi::Value Tempo(const Napi::CallbackInfo& info);
    Napi::Value SessionId(const Napi::CallbackInfo& info);
    Napi::Value BeginBeats(const Napi::CallbackInfo& info);
    Napi::Value EndBeats(const Napi::CallbackInfo& info);

    ableton::LinkAudioSource::BufferHandle::Info info_{};
};

class AbletonLinkAudioSourceWrapper : public Napi::ObjectWrap<AbletonLinkAudioSourceWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    AbletonLinkAudioSourceWrapper(const Napi::CallbackInfo& info);
    ~AbletonLinkAudioSourceWrapper();

private:
    static Napi::FunctionReference constructor;

    Napi::Value Id(const Napi::CallbackInfo& info);
    void Close(const Napi::CallbackInfo& info);
    void CloseInternal();

    void handleBuffer(
        const ableton::LinkAudioSource::BufferHandle& handle);

    static void BufferCallback(
        const ableton::LinkAudioSource::BufferHandle& handle,
        void* context);

    std::shared_ptr<ableton::LinkAudioSource> source_;
    Napi::ObjectReference linkRef_;
    Napi::ThreadSafeFunction bufferCallback_;
};

Napi::Object InitAbletonLinkAudio(Napi::Env env, Napi::Object exports);

#endif // ABLETONLINK_AUDIO_H
