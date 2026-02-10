#include "abletonlink.h"
#include "abletonlink_audio.h"
#include <chrono>

namespace {
struct SessionStateHandle {
    SessionStateHandle() : state(abl_link_create_session_state()) {}
    ~SessionStateHandle() { abl_link_destroy_session_state(state); }
    SessionStateHandle(const SessionStateHandle&) = delete;
    SessionStateHandle& operator=(const SessionStateHandle&) = delete;
    SessionStateHandle(SessionStateHandle&&) = delete;
    SessionStateHandle& operator=(SessionStateHandle&&) = delete;

    abl_link_session_state state;
};
} // namespace

Napi::FunctionReference AbletonLinkWrapper::constructor;

Napi::Object AbletonLinkWrapper::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "AbletonLink", {
        InstanceMethod("enable", &AbletonLinkWrapper::Enable),
        InstanceMethod("isEnabled", &AbletonLinkWrapper::IsEnabled),
        InstanceMethod("getTempo", &AbletonLinkWrapper::GetTempo),
        InstanceMethod("setTempo", &AbletonLinkWrapper::SetTempo),
        InstanceMethod("getBeat", &AbletonLinkWrapper::GetBeat),
        InstanceMethod("getPhase", &AbletonLinkWrapper::GetPhase),
        InstanceMethod("getNumPeers", &AbletonLinkWrapper::GetNumPeers),
        InstanceMethod("setIsPlaying", &AbletonLinkWrapper::SetIsPlaying),
        InstanceMethod("isPlaying", &AbletonLinkWrapper::IsPlaying),
        InstanceMethod("enableStartStopSync", &AbletonLinkWrapper::EnableStartStopSync),
        InstanceMethod("isStartStopSyncEnabled", &AbletonLinkWrapper::IsStartStopSyncEnabled),
        InstanceMethod("forceBeatAtTime", &AbletonLinkWrapper::ForceBeatAtTime),
        InstanceMethod("getTimeForBeat", &AbletonLinkWrapper::GetTimeForBeat),
        InstanceMethod("close", &AbletonLinkWrapper::Close),
        InstanceMethod("requestBeatAtTime", &AbletonLinkWrapper::RequestBeatAtTime),
        InstanceMethod("requestBeatAtStartPlayingTime", &AbletonLinkWrapper::RequestBeatAtStartPlayingTime),
        InstanceMethod("setIsPlayingAndRequestBeatAtTime", &AbletonLinkWrapper::SetIsPlayingAndRequestBeatAtTime),
        InstanceMethod("timeForIsPlaying", &AbletonLinkWrapper::TimeForIsPlaying),
        InstanceMethod("setNumPeersCallback", &AbletonLinkWrapper::SetNumPeersCallback),
        InstanceMethod("setTempoCallback", &AbletonLinkWrapper::SetTempoCallback),
        InstanceMethod("setStartStopCallback", &AbletonLinkWrapper::SetStartStopCallback),
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("AbletonLink", func);
    return exports;
}

AbletonLinkWrapper::AbletonLinkWrapper(const Napi::CallbackInfo& info) 
    : Napi::ObjectWrap<AbletonLinkWrapper>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Initial tempo (number) expected").ThrowAsJavaScriptException();
        return;
    }

    double initialTempo = info[0].As<Napi::Number>().DoubleValue();
    link_ = abl_link_create(initialTempo);
}

AbletonLinkWrapper::~AbletonLinkWrapper() {
    CloseInternal();
}

Napi::Value AbletonLinkWrapper::Enable(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsBoolean()) {
        Napi::TypeError::New(env, "Boolean expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    bool enable = info[0].As<Napi::Boolean>().Value();
    abl_link_enable(link_, enable);
    
    return env.Undefined();
}

Napi::Value AbletonLinkWrapper::IsEnabled(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, abl_link_is_enabled(link_));
}

Napi::Value AbletonLinkWrapper::GetTempo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    double tempo = abl_link_tempo(sessionState.state);
    
    return Napi::Number::New(env, tempo);
}

void AbletonLinkWrapper::SetTempo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Tempo (number) expected").ThrowAsJavaScriptException();
        return;
    }

    double tempo = info[0].As<Napi::Number>().DoubleValue();
    
    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    abl_link_set_tempo(sessionState.state, tempo, getCurrentTimeMicros());
    abl_link_commit_app_session_state(link_, sessionState.state);
}

Napi::Value AbletonLinkWrapper::GetBeat(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    double beat = abl_link_beat_at_time(sessionState.state, getCurrentTimeMicros(), 1.0);
    
    return Napi::Number::New(env, beat);
}

Napi::Value AbletonLinkWrapper::GetPhase(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Quantum (number) expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    double quantum = info[0].As<Napi::Number>().DoubleValue();
    
    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    double phase =
        abl_link_phase_at_time(sessionState.state, getCurrentTimeMicros(), quantum);
    
    return Napi::Number::New(env, phase);
}

Napi::Value AbletonLinkWrapper::GetNumPeers(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, static_cast<double>(abl_link_num_peers(link_)));
}

void AbletonLinkWrapper::SetIsPlaying(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsBoolean()) {
        Napi::TypeError::New(env, "Playing state (boolean) expected").ThrowAsJavaScriptException();
        return;
    }

    bool isPlaying = info[0].As<Napi::Boolean>().Value();
    
    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    abl_link_set_is_playing(
        sessionState.state, isPlaying, static_cast<uint64_t>(getCurrentTimeMicros()));
    abl_link_commit_app_session_state(link_, sessionState.state);
}

Napi::Value AbletonLinkWrapper::IsPlaying(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    bool isPlaying = abl_link_is_playing(sessionState.state);
    
    return Napi::Boolean::New(env, isPlaying);
}

void AbletonLinkWrapper::EnableStartStopSync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsBoolean()) {
        Napi::TypeError::New(env, "Boolean expected").ThrowAsJavaScriptException();
        return;
    }

    bool enable = info[0].As<Napi::Boolean>().Value();
    abl_link_enable_start_stop_sync(link_, enable);
}

Napi::Value AbletonLinkWrapper::IsStartStopSyncEnabled(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, abl_link_is_start_stop_sync_enabled(link_));
}

void AbletonLinkWrapper::ForceBeatAtTime(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 3 || !info[0].IsNumber() || !info[1].IsNumber() || !info[2].IsNumber()) {
        Napi::TypeError::New(env, "Beat (number), time (number), and quantum (number) expected").ThrowAsJavaScriptException();
        return;
    }

    double beat = info[0].As<Napi::Number>().DoubleValue();
    double timeInSeconds = info[1].As<Napi::Number>().DoubleValue();
    double quantum = info[2].As<Napi::Number>().DoubleValue();
    
    auto timeMicros = static_cast<uint64_t>(timeInSeconds * 1000000.0);

    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    abl_link_force_beat_at_time(sessionState.state, beat, timeMicros, quantum);
    abl_link_commit_app_session_state(link_, sessionState.state);
}

Napi::Value AbletonLinkWrapper::GetTimeForBeat(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Beat (number) and quantum (number) expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    double beat = info[0].As<Napi::Number>().DoubleValue();
    double quantum = info[1].As<Napi::Number>().DoubleValue();
    
    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    auto time = abl_link_time_at_beat(sessionState.state, beat, quantum);

    double timeInSeconds = static_cast<double>(time) / 1000000.0;
    return Napi::Number::New(env, timeInSeconds);
}

void AbletonLinkWrapper::Close(const Napi::CallbackInfo& info) {
    CloseInternal();
}

void AbletonLinkWrapper::CloseInternal() {
    if (link_.impl == nullptr) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (numPeersCallback_) {
            numPeersCallback_.Abort();
            numPeersCallback_.Release();
        }
        if (tempoCallback_) {
            tempoCallback_.Abort();
            tempoCallback_.Release();
        }
        if (startStopCallback_) {
            startStopCallback_.Abort();
            startStopCallback_.Release();
        }
    }

    abl_link_set_num_peers_callback(link_, &AbletonLinkWrapper::NoopNumPeersCallback, nullptr);
    abl_link_set_tempo_callback(link_, &AbletonLinkWrapper::NoopTempoCallback, nullptr);
    abl_link_set_start_stop_callback(link_, &AbletonLinkWrapper::NoopStartStopCallback, nullptr);
    abl_link_enable(link_, false);
    abl_link_destroy(link_);
    link_.impl = nullptr;
}

int64_t AbletonLinkWrapper::getCurrentTimeMicros() const {
    return abl_link_clock_micros(link_);
}

// Quantized launch methods
void AbletonLinkWrapper::RequestBeatAtTime(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 3 || !info[0].IsNumber() || !info[1].IsNumber() || !info[2].IsNumber()) {
        Napi::TypeError::New(env, "Beat (number), time (number), and quantum (number) expected").ThrowAsJavaScriptException();
        return;
    }

    double beat = info[0].As<Napi::Number>().DoubleValue();
    double timeInSeconds = info[1].As<Napi::Number>().DoubleValue();
    double quantum = info[2].As<Napi::Number>().DoubleValue();
    
    auto timeMicros = static_cast<int64_t>(timeInSeconds * 1000000.0);

    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    abl_link_request_beat_at_time(sessionState.state, beat, timeMicros, quantum);
    abl_link_commit_app_session_state(link_, sessionState.state);
}

void AbletonLinkWrapper::RequestBeatAtStartPlayingTime(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Beat (number) and quantum (number) expected").ThrowAsJavaScriptException();
        return;
    }

    double beat = info[0].As<Napi::Number>().DoubleValue();
    double quantum = info[1].As<Napi::Number>().DoubleValue();
    
    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    abl_link_request_beat_at_start_playing_time(sessionState.state, beat, quantum);
    abl_link_commit_app_session_state(link_, sessionState.state);
}

void AbletonLinkWrapper::SetIsPlayingAndRequestBeatAtTime(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 4 || !info[0].IsBoolean() || !info[1].IsNumber() || !info[2].IsNumber() || !info[3].IsNumber()) {
        Napi::TypeError::New(env, "Playing state (boolean), time (number), beat (number), and quantum (number) expected").ThrowAsJavaScriptException();
        return;
    }

    bool isPlaying = info[0].As<Napi::Boolean>().Value();
    double timeInSeconds = info[1].As<Napi::Number>().DoubleValue();
    double beat = info[2].As<Napi::Number>().DoubleValue();
    double quantum = info[3].As<Napi::Number>().DoubleValue();
    
    auto timeMicros = static_cast<uint64_t>(timeInSeconds * 1000000.0);

    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    abl_link_set_is_playing_and_request_beat_at_time(
        sessionState.state, isPlaying, timeMicros, beat, quantum);
    abl_link_commit_app_session_state(link_, sessionState.state);
}

// Transport timing
Napi::Value AbletonLinkWrapper::TimeForIsPlaying(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    SessionStateHandle sessionState;
    abl_link_capture_app_session_state(link_, sessionState.state);
    auto time = abl_link_time_for_is_playing(sessionState.state);
    
    double timeInSeconds = static_cast<double>(time) / 1000000.0;
    return Napi::Number::New(env, timeInSeconds);
}

// Callback methods
void AbletonLinkWrapper::SetNumPeersCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Function expected").ThrowAsJavaScriptException();
        return;
    }
    
    std::lock_guard<std::mutex> lock(callbackMutex_);
    
    // Release any existing callback
    if (numPeersCallback_) {
        numPeersCallback_.Release();
    }
    
    // Create a thread-safe function
    numPeersCallback_ = Napi::ThreadSafeFunction::New(
        env,
        info[0].As<Napi::Function>(),
        "NumPeersCallback",
        0,
        1
    );
    numPeersCallback_.Unref(env);
    
    // Set the callback on the Link instance
    abl_link_set_num_peers_callback(link_, &AbletonLinkWrapper::NumPeersCallback, this);
}

void AbletonLinkWrapper::SetTempoCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Function expected").ThrowAsJavaScriptException();
        return;
    }
    
    std::lock_guard<std::mutex> lock(callbackMutex_);
    
    // Release any existing callback
    if (tempoCallback_) {
        tempoCallback_.Release();
    }
    
    // Create a thread-safe function
    tempoCallback_ = Napi::ThreadSafeFunction::New(
        env,
        info[0].As<Napi::Function>(),
        "TempoCallback",
        0,
        1
    );
    tempoCallback_.Unref(env);
    
    // Set the callback on the Link instance
    abl_link_set_tempo_callback(link_, &AbletonLinkWrapper::TempoCallback, this);
}

void AbletonLinkWrapper::SetStartStopCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Function expected").ThrowAsJavaScriptException();
        return;
    }
    
    std::lock_guard<std::mutex> lock(callbackMutex_);
    
    // Release any existing callback
    if (startStopCallback_) {
        startStopCallback_.Release();
    }
    
    // Create a thread-safe function
    startStopCallback_ = Napi::ThreadSafeFunction::New(
        env,
        info[0].As<Napi::Function>(),
        "StartStopCallback",
        0,
        1
    );
    startStopCallback_.Unref(env);
    
    // Set the callback on the Link instance
    abl_link_set_start_stop_callback(
        link_, &AbletonLinkWrapper::StartStopCallback, this);
}

// Callback handlers
void AbletonLinkWrapper::handleNumPeersCallback(std::size_t numPeers) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (numPeersCallback_) {
        numPeersCallback_.BlockingCall([numPeers](Napi::Env env, Napi::Function callback) {
            callback.Call({Napi::Number::New(env, static_cast<double>(numPeers))});
        });
    }
}

void AbletonLinkWrapper::handleTempoCallback(double tempo) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (tempoCallback_) {
        tempoCallback_.BlockingCall([tempo](Napi::Env env, Napi::Function callback) {
            callback.Call({Napi::Number::New(env, tempo)});
        });
    }
}

void AbletonLinkWrapper::handleStartStopCallback(bool isPlaying) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (startStopCallback_) {
        startStopCallback_.BlockingCall([isPlaying](Napi::Env env, Napi::Function callback) {
            callback.Call({Napi::Boolean::New(env, isPlaying)});
        });
    }
}

void AbletonLinkWrapper::NumPeersCallback(uint64_t numPeers, void* context) {
    auto* wrapper = static_cast<AbletonLinkWrapper*>(context);
    if (wrapper) {
        wrapper->handleNumPeersCallback(static_cast<std::size_t>(numPeers));
    }
}

void AbletonLinkWrapper::TempoCallback(double tempo, void* context) {
    auto* wrapper = static_cast<AbletonLinkWrapper*>(context);
    if (wrapper) {
        wrapper->handleTempoCallback(tempo);
    }
}

void AbletonLinkWrapper::StartStopCallback(bool isPlaying, void* context) {
    auto* wrapper = static_cast<AbletonLinkWrapper*>(context);
    if (wrapper) {
        wrapper->handleStartStopCallback(isPlaying);
    }
}

void AbletonLinkWrapper::NoopNumPeersCallback(uint64_t numPeers, void* context) {
    (void)numPeers;
    (void)context;
}

void AbletonLinkWrapper::NoopTempoCallback(double tempo, void* context) {
    (void)tempo;
    (void)context;
}

void AbletonLinkWrapper::NoopStartStopCallback(bool isPlaying, void* context) {
    (void)isPlaying;
    (void)context;
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    AbletonLinkWrapper::Init(env, exports);
    InitAbletonLinkAudio(env, exports);
    return exports;
}

NODE_API_MODULE(abletonlink, Init)
