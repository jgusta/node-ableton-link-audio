#ifndef ABLETONLINK_H
#define ABLETONLINK_H

#include <napi.h>
#include <abl_link.h>
#include <chrono>
#include <mutex>

class AbletonLinkWrapper : public Napi::ObjectWrap<AbletonLinkWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    AbletonLinkWrapper(const Napi::CallbackInfo& info);
    ~AbletonLinkWrapper();

private:
    static Napi::FunctionReference constructor;
    
    // Link instance
    abl_link link_{};
    
    // Methods
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
    void Close(const Napi::CallbackInfo& info);
    
    // Quantized launch methods
    void RequestBeatAtTime(const Napi::CallbackInfo& info);
    void RequestBeatAtStartPlayingTime(const Napi::CallbackInfo& info);
    void SetIsPlayingAndRequestBeatAtTime(const Napi::CallbackInfo& info);
    
    // Transport timing
    Napi::Value TimeForIsPlaying(const Napi::CallbackInfo& info);
    
    // Callback methods
    void SetNumPeersCallback(const Napi::CallbackInfo& info);
    void SetTempoCallback(const Napi::CallbackInfo& info);
    void SetStartStopCallback(const Napi::CallbackInfo& info);
    
    // Utility functions
    int64_t getCurrentTimeMicros() const;
    
    // Callback handling
    void handleNumPeersCallback(std::size_t numPeers);
    void handleTempoCallback(double tempo);
    void handleStartStopCallback(bool isPlaying);
    static void NumPeersCallback(uint64_t numPeers, void* context);
    static void TempoCallback(double tempo, void* context);
    static void StartStopCallback(bool isPlaying, void* context);
    static void NoopNumPeersCallback(uint64_t numPeers, void* context);
    static void NoopTempoCallback(double tempo, void* context);
    static void NoopStartStopCallback(bool isPlaying, void* context);
    
    // Thread-safe callback references
    std::mutex callbackMutex_;
    Napi::ThreadSafeFunction numPeersCallback_;
    Napi::ThreadSafeFunction tempoCallback_;
    Napi::ThreadSafeFunction startStopCallback_;

    void CloseInternal();
};

#endif // ABLETONLINK_H
