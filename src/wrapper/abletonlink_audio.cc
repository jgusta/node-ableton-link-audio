#include "abletonlink_audio.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace {
std::string NodeIdToHexString(const ableton::link::NodeId& id) {
    std::ostringstream stream;
    stream << id;
    return stream.str();
}

bool ParseNodeIdString(const std::string& input, ableton::link::NodeId& out) {
    std::string hex = input;
    if (hex.rfind("0x", 0) == 0 || hex.rfind("0X", 0) == 0) {
        hex = hex.substr(2);
    }
    if (hex.size() != 16) {
        return false;
    }

    ableton::link::NodeIdArray bytes{};
    for (size_t i = 0; i < bytes.size(); ++i) {
        const auto hi = hex[2 * i];
        const auto lo = hex[2 * i + 1];
        if (!std::isxdigit(static_cast<unsigned char>(hi)) ||
            !std::isxdigit(static_cast<unsigned char>(lo))) {
            return false;
        }
        const auto byte = std::stoul(hex.substr(2 * i, 2), nullptr, 16);
        bytes[i] = static_cast<std::uint8_t>(byte);
    }

    out = ableton::link::NodeId(bytes);
    return true;
}

Napi::Value OptionalToValue(Napi::Env env, const std::optional<double>& value) {
    if (value.has_value()) {
        return Napi::Number::New(env, *value);
    }
    return env.Null();
}
} // namespace

Napi::FunctionReference AbletonLinkAudioSessionStateWrapper::constructor;
Napi::FunctionReference AbletonLinkAudioWrapper::constructor;
Napi::FunctionReference AbletonLinkAudioSinkWrapper::constructor;
Napi::FunctionReference AbletonLinkAudioSinkBufferHandleWrapper::constructor;
Napi::FunctionReference AbletonLinkAudioBufferInfoWrapper::constructor;
Napi::FunctionReference AbletonLinkAudioSourceWrapper::constructor;

Napi::Object AbletonLinkAudioSessionStateWrapper::Init(Napi::Env env,
                                                       Napi::Object exports) {
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "AbletonLinkAudioSessionState", {
        InstanceMethod("tempo", &AbletonLinkAudioSessionStateWrapper::Tempo),
        InstanceMethod("setTempo", &AbletonLinkAudioSessionStateWrapper::SetTempo),
        InstanceMethod("beatAtTime", &AbletonLinkAudioSessionStateWrapper::BeatAtTime),
        InstanceMethod("phaseAtTime", &AbletonLinkAudioSessionStateWrapper::PhaseAtTime),
        InstanceMethod("timeAtBeat", &AbletonLinkAudioSessionStateWrapper::TimeAtBeat),
        InstanceMethod("requestBeatAtTime",
                       &AbletonLinkAudioSessionStateWrapper::RequestBeatAtTime),
        InstanceMethod("forceBeatAtTime",
                       &AbletonLinkAudioSessionStateWrapper::ForceBeatAtTime),
        InstanceMethod("setIsPlaying",
                       &AbletonLinkAudioSessionStateWrapper::SetIsPlaying),
        InstanceMethod("isPlaying", &AbletonLinkAudioSessionStateWrapper::IsPlaying),
        InstanceMethod("timeForIsPlaying",
                       &AbletonLinkAudioSessionStateWrapper::TimeForIsPlaying),
        InstanceMethod("requestBeatAtStartPlayingTime",
                       &AbletonLinkAudioSessionStateWrapper::RequestBeatAtStartPlayingTime),
        InstanceMethod(
            "setIsPlayingAndRequestBeatAtTime",
            &AbletonLinkAudioSessionStateWrapper::SetIsPlayingAndRequestBeatAtTime),
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("AbletonLinkAudioSessionState", func);
    return exports;
}

Napi::Object AbletonLinkAudioSessionStateWrapper::New(
    Napi::Env env, const ableton::LinkAudio::SessionState& state) {
    Napi::EscapableHandleScope scope(env);
    auto obj = constructor.New({});
    auto* wrapper = Napi::ObjectWrap<AbletonLinkAudioSessionStateWrapper>::Unwrap(obj);
    *wrapper->state_ = state;
    return scope.Escape(napi_value(obj)).ToObject();
}

AbletonLinkAudioSessionStateWrapper::AbletonLinkAudioSessionStateWrapper(
    const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<AbletonLinkAudioSessionStateWrapper>(info) {
    state_ = std::make_unique<ableton::LinkAudio::SessionState>(
        ableton::link::ApiState{}, false);
}

ableton::LinkAudio::SessionState& AbletonLinkAudioSessionStateWrapper::State() {
    return *state_;
}

Napi::Value AbletonLinkAudioSessionStateWrapper::Tempo(
    const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), state_->tempo());
}

void AbletonLinkAudioSessionStateWrapper::SetTempo(const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "Tempo (number) and time (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    const auto bpm = info[0].As<Napi::Number>().DoubleValue();
    const auto time =
        std::chrono::microseconds(static_cast<long long>(
            info[1].As<Napi::Number>().DoubleValue() * 1000000.0));
    state_->setTempo(bpm, time);
}

Napi::Value AbletonLinkAudioSessionStateWrapper::BeatAtTime(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "Time (number) and quantum (number) expected")
            .ThrowAsJavaScriptException();
        return info.Env().Null();
    }
    const auto time =
        std::chrono::microseconds(static_cast<long long>(
            info[0].As<Napi::Number>().DoubleValue() * 1000000.0));
    const auto quantum = info[1].As<Napi::Number>().DoubleValue();
    return Napi::Number::New(info.Env(), state_->beatAtTime(time, quantum));
}

Napi::Value AbletonLinkAudioSessionStateWrapper::PhaseAtTime(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "Time (number) and quantum (number) expected")
            .ThrowAsJavaScriptException();
        return info.Env().Null();
    }
    const auto time =
        std::chrono::microseconds(static_cast<long long>(
            info[0].As<Napi::Number>().DoubleValue() * 1000000.0));
    const auto quantum = info[1].As<Napi::Number>().DoubleValue();
    return Napi::Number::New(info.Env(), state_->phaseAtTime(time, quantum));
}

Napi::Value AbletonLinkAudioSessionStateWrapper::TimeAtBeat(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "Beat (number) and quantum (number) expected")
            .ThrowAsJavaScriptException();
        return info.Env().Null();
    }
    const auto beat = info[0].As<Napi::Number>().DoubleValue();
    const auto quantum = info[1].As<Napi::Number>().DoubleValue();
    const auto time = state_->timeAtBeat(beat, quantum);
    return Napi::Number::New(info.Env(), time.count() / 1000000.0);
}

void AbletonLinkAudioSessionStateWrapper::RequestBeatAtTime(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 3 || !info[0].IsNumber() || !info[1].IsNumber() ||
        !info[2].IsNumber()) {
        Napi::TypeError::New(
            info.Env(), "Beat (number), time (number), and quantum (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    const auto beat = info[0].As<Napi::Number>().DoubleValue();
    const auto time =
        std::chrono::microseconds(static_cast<long long>(
            info[1].As<Napi::Number>().DoubleValue() * 1000000.0));
    const auto quantum = info[2].As<Napi::Number>().DoubleValue();
    state_->requestBeatAtTime(beat, time, quantum);
}

void AbletonLinkAudioSessionStateWrapper::ForceBeatAtTime(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 3 || !info[0].IsNumber() || !info[1].IsNumber() ||
        !info[2].IsNumber()) {
        Napi::TypeError::New(
            info.Env(), "Beat (number), time (number), and quantum (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    const auto beat = info[0].As<Napi::Number>().DoubleValue();
    const auto time =
        std::chrono::microseconds(static_cast<long long>(
            info[1].As<Napi::Number>().DoubleValue() * 1000000.0));
    const auto quantum = info[2].As<Napi::Number>().DoubleValue();
    state_->forceBeatAtTime(beat, time, quantum);
}

void AbletonLinkAudioSessionStateWrapper::SetIsPlaying(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[0].IsBoolean() || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "IsPlaying (boolean) and time (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    const auto isPlaying = info[0].As<Napi::Boolean>().Value();
    const auto time =
        std::chrono::microseconds(static_cast<long long>(
            info[1].As<Napi::Number>().DoubleValue() * 1000000.0));
    state_->setIsPlaying(isPlaying, time);
}

Napi::Value AbletonLinkAudioSessionStateWrapper::IsPlaying(
    const Napi::CallbackInfo& info) {
    return Napi::Boolean::New(info.Env(), state_->isPlaying());
}

Napi::Value AbletonLinkAudioSessionStateWrapper::TimeForIsPlaying(
    const Napi::CallbackInfo& info) {
    const auto time = state_->timeForIsPlaying();
    return Napi::Number::New(info.Env(), time.count() / 1000000.0);
}

void AbletonLinkAudioSessionStateWrapper::RequestBeatAtStartPlayingTime(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "Beat (number) and quantum (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    const auto beat = info[0].As<Napi::Number>().DoubleValue();
    const auto quantum = info[1].As<Napi::Number>().DoubleValue();
    state_->requestBeatAtStartPlayingTime(beat, quantum);
}

void AbletonLinkAudioSessionStateWrapper::SetIsPlayingAndRequestBeatAtTime(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 4 || !info[0].IsBoolean() || !info[1].IsNumber() ||
        !info[2].IsNumber() || !info[3].IsNumber()) {
        Napi::TypeError::New(info.Env(),
                             "Playing (boolean), time (number), beat (number), and "
                             "quantum (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    const auto isPlaying = info[0].As<Napi::Boolean>().Value();
    const auto time =
        std::chrono::microseconds(static_cast<long long>(
            info[1].As<Napi::Number>().DoubleValue() * 1000000.0));
    const auto beat = info[2].As<Napi::Number>().DoubleValue();
    const auto quantum = info[3].As<Napi::Number>().DoubleValue();
    state_->setIsPlayingAndRequestBeatAtTime(isPlaying, time, beat, quantum);
}

Napi::Object AbletonLinkAudioWrapper::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "AbletonLinkAudio", {
        InstanceMethod("enable", &AbletonLinkAudioWrapper::Enable),
        InstanceMethod("isEnabled", &AbletonLinkAudioWrapper::IsEnabled),
        InstanceMethod("getTempo", &AbletonLinkAudioWrapper::GetTempo),
        InstanceMethod("setTempo", &AbletonLinkAudioWrapper::SetTempo),
        InstanceMethod("getBeat", &AbletonLinkAudioWrapper::GetBeat),
        InstanceMethod("getPhase", &AbletonLinkAudioWrapper::GetPhase),
        InstanceMethod("getNumPeers", &AbletonLinkAudioWrapper::GetNumPeers),
        InstanceMethod("setIsPlaying", &AbletonLinkAudioWrapper::SetIsPlaying),
        InstanceMethod("isPlaying", &AbletonLinkAudioWrapper::IsPlaying),
        InstanceMethod("enableStartStopSync",
                       &AbletonLinkAudioWrapper::EnableStartStopSync),
        InstanceMethod("isStartStopSyncEnabled",
                       &AbletonLinkAudioWrapper::IsStartStopSyncEnabled),
        InstanceMethod("forceBeatAtTime", &AbletonLinkAudioWrapper::ForceBeatAtTime),
        InstanceMethod("getTimeForBeat", &AbletonLinkAudioWrapper::GetTimeForBeat),
        InstanceMethod("requestBeatAtTime", &AbletonLinkAudioWrapper::RequestBeatAtTime),
        InstanceMethod("requestBeatAtStartPlayingTime",
                       &AbletonLinkAudioWrapper::RequestBeatAtStartPlayingTime),
        InstanceMethod("setIsPlayingAndRequestBeatAtTime",
                       &AbletonLinkAudioWrapper::SetIsPlayingAndRequestBeatAtTime),
        InstanceMethod("timeForIsPlaying", &AbletonLinkAudioWrapper::TimeForIsPlaying),
        InstanceMethod("getClockTime", &AbletonLinkAudioWrapper::GetClockTime),
        InstanceMethod("isLinkAudioEnabled",
                       &AbletonLinkAudioWrapper::IsLinkAudioEnabled),
        InstanceMethod("enableLinkAudio", &AbletonLinkAudioWrapper::EnableLinkAudio),
        InstanceMethod("setPeerName", &AbletonLinkAudioWrapper::SetPeerName),
        InstanceMethod("setChannelsChangedCallback",
                       &AbletonLinkAudioWrapper::SetChannelsChangedCallback),
        InstanceMethod("channels", &AbletonLinkAudioWrapper::Channels),
        InstanceMethod("callOnLinkThread", &AbletonLinkAudioWrapper::CallOnLinkThread),
        InstanceMethod("captureAppSessionState",
                       &AbletonLinkAudioWrapper::CaptureAppSessionState),
        InstanceMethod("commitAppSessionState",
                       &AbletonLinkAudioWrapper::CommitAppSessionState),
        InstanceMethod("captureAudioSessionState",
                       &AbletonLinkAudioWrapper::CaptureAudioSessionState),
        InstanceMethod("commitAudioSessionState",
                       &AbletonLinkAudioWrapper::CommitAudioSessionState),
        InstanceMethod("setNumPeersCallback",
                       &AbletonLinkAudioWrapper::SetNumPeersCallback),
        InstanceMethod("setTempoCallback",
                       &AbletonLinkAudioWrapper::SetTempoCallback),
        InstanceMethod("setStartStopCallback",
                       &AbletonLinkAudioWrapper::SetStartStopCallback),
        InstanceMethod("close", &AbletonLinkAudioWrapper::Close),
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("AbletonLinkAudio", func);
    return exports;
}

AbletonLinkAudioWrapper::AbletonLinkAudioWrapper(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<AbletonLinkAudioWrapper>(info) {
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsString()) {
        Napi::TypeError::New(info.Env(),
                             "Initial tempo (number) and peer name (string) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    const auto bpm = info[0].As<Napi::Number>().DoubleValue();
    const auto name = info[1].As<Napi::String>().Utf8Value();
    link_ = std::make_unique<ableton::LinkAudio>(bpm, name);
}

AbletonLinkAudioWrapper::~AbletonLinkAudioWrapper() {
    CloseInternal();
}

ableton::LinkAudio& AbletonLinkAudioWrapper::LinkAudio() {
    return *link_;
}

Napi::Value AbletonLinkAudioWrapper::Enable(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsBoolean()) {
        Napi::TypeError::New(info.Env(), "Boolean expected").ThrowAsJavaScriptException();
        return info.Env().Null();
    }
    link_->enable(info[0].As<Napi::Boolean>().Value());
    return info.Env().Undefined();
}

Napi::Value AbletonLinkAudioWrapper::IsEnabled(const Napi::CallbackInfo& info) {
    return Napi::Boolean::New(info.Env(), link_->isEnabled());
}

Napi::Value AbletonLinkAudioWrapper::GetTempo(const Napi::CallbackInfo& info) {
    auto state = link_->captureAppSessionState();
    return Napi::Number::New(info.Env(), state.tempo());
}

void AbletonLinkAudioWrapper::SetTempo(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(info.Env(), "Tempo (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    auto state = link_->captureAppSessionState();
    state.setTempo(info[0].As<Napi::Number>().DoubleValue(), getCurrentTime());
    link_->commitAppSessionState(state);
}

Napi::Value AbletonLinkAudioWrapper::GetBeat(const Napi::CallbackInfo& info) {
    auto state = link_->captureAppSessionState();
    return Napi::Number::New(info.Env(), state.beatAtTime(getCurrentTime(), 1.0));
}

Napi::Value AbletonLinkAudioWrapper::GetPhase(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(info.Env(), "Quantum (number) expected")
            .ThrowAsJavaScriptException();
        return info.Env().Null();
    }
    const auto quantum = info[0].As<Napi::Number>().DoubleValue();
    auto state = link_->captureAppSessionState();
    return Napi::Number::New(info.Env(), state.phaseAtTime(getCurrentTime(), quantum));
}

Napi::Value AbletonLinkAudioWrapper::GetNumPeers(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), static_cast<double>(link_->numPeers()));
}

void AbletonLinkAudioWrapper::SetIsPlaying(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsBoolean()) {
        Napi::TypeError::New(info.Env(), "Playing state (boolean) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    auto state = link_->captureAppSessionState();
    state.setIsPlaying(info[0].As<Napi::Boolean>().Value(), getCurrentTime());
    link_->commitAppSessionState(state);
}

Napi::Value AbletonLinkAudioWrapper::IsPlaying(const Napi::CallbackInfo& info) {
    auto state = link_->captureAppSessionState();
    return Napi::Boolean::New(info.Env(), state.isPlaying());
}

void AbletonLinkAudioWrapper::EnableStartStopSync(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsBoolean()) {
        Napi::TypeError::New(info.Env(), "Boolean expected").ThrowAsJavaScriptException();
        return;
    }
    link_->enableStartStopSync(info[0].As<Napi::Boolean>().Value());
}

Napi::Value AbletonLinkAudioWrapper::IsStartStopSyncEnabled(
    const Napi::CallbackInfo& info) {
    return Napi::Boolean::New(info.Env(), link_->isStartStopSyncEnabled());
}

void AbletonLinkAudioWrapper::ForceBeatAtTime(const Napi::CallbackInfo& info) {
    if (info.Length() < 3 || !info[0].IsNumber() || !info[1].IsNumber() ||
        !info[2].IsNumber()) {
        Napi::TypeError::New(info.Env(),
                             "Beat (number), time (number), and quantum (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    const auto beat = info[0].As<Napi::Number>().DoubleValue();
    const auto time =
        std::chrono::microseconds(static_cast<long long>(
            info[1].As<Napi::Number>().DoubleValue() * 1000000.0));
    const auto quantum = info[2].As<Napi::Number>().DoubleValue();
    auto state = link_->captureAppSessionState();
    state.forceBeatAtTime(beat, time, quantum);
    link_->commitAppSessionState(state);
}

Napi::Value AbletonLinkAudioWrapper::GetTimeForBeat(const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "Beat (number) and quantum (number) expected")
            .ThrowAsJavaScriptException();
        return info.Env().Null();
    }
    const auto beat = info[0].As<Napi::Number>().DoubleValue();
    const auto quantum = info[1].As<Napi::Number>().DoubleValue();
    auto state = link_->captureAppSessionState();
    const auto time = state.timeAtBeat(beat, quantum);
    return Napi::Number::New(info.Env(), time.count() / 1000000.0);
}

void AbletonLinkAudioWrapper::RequestBeatAtTime(const Napi::CallbackInfo& info) {
    if (info.Length() < 3 || !info[0].IsNumber() || !info[1].IsNumber() ||
        !info[2].IsNumber()) {
        Napi::TypeError::New(
            info.Env(), "Beat (number), time (number), and quantum (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    const auto beat = info[0].As<Napi::Number>().DoubleValue();
    const auto time =
        std::chrono::microseconds(static_cast<long long>(
            info[1].As<Napi::Number>().DoubleValue() * 1000000.0));
    const auto quantum = info[2].As<Napi::Number>().DoubleValue();
    auto state = link_->captureAppSessionState();
    state.requestBeatAtTime(beat, time, quantum);
    link_->commitAppSessionState(state);
}

void AbletonLinkAudioWrapper::RequestBeatAtStartPlayingTime(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "Beat (number) and quantum (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    const auto beat = info[0].As<Napi::Number>().DoubleValue();
    const auto quantum = info[1].As<Napi::Number>().DoubleValue();
    auto state = link_->captureAppSessionState();
    state.requestBeatAtStartPlayingTime(beat, quantum);
    link_->commitAppSessionState(state);
}

void AbletonLinkAudioWrapper::SetIsPlayingAndRequestBeatAtTime(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 4 || !info[0].IsBoolean() || !info[1].IsNumber() ||
        !info[2].IsNumber() || !info[3].IsNumber()) {
        Napi::TypeError::New(info.Env(),
                             "Playing state (boolean), time (number), beat (number), and "
                             "quantum (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    const auto isPlaying = info[0].As<Napi::Boolean>().Value();
    const auto time =
        std::chrono::microseconds(static_cast<long long>(
            info[1].As<Napi::Number>().DoubleValue() * 1000000.0));
    const auto beat = info[2].As<Napi::Number>().DoubleValue();
    const auto quantum = info[3].As<Napi::Number>().DoubleValue();
    auto state = link_->captureAppSessionState();
    state.setIsPlayingAndRequestBeatAtTime(isPlaying, time, beat, quantum);
    link_->commitAppSessionState(state);
}

Napi::Value AbletonLinkAudioWrapper::TimeForIsPlaying(const Napi::CallbackInfo& info) {
    auto state = link_->captureAppSessionState();
    const auto time = state.timeForIsPlaying();
    return Napi::Number::New(info.Env(), time.count() / 1000000.0);
}

Napi::Value AbletonLinkAudioWrapper::GetClockTime(const Napi::CallbackInfo& info) {
    const auto micros = link_->clock().micros().count();
    return Napi::Number::New(info.Env(), micros / 1000000.0);
}

Napi::Value AbletonLinkAudioWrapper::IsLinkAudioEnabled(
    const Napi::CallbackInfo& info) {
    return Napi::Boolean::New(info.Env(), link_->isLinkAudioEnabled());
}

void AbletonLinkAudioWrapper::EnableLinkAudio(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsBoolean()) {
        Napi::TypeError::New(info.Env(), "Boolean expected").ThrowAsJavaScriptException();
        return;
    }
    link_->enableLinkAudio(info[0].As<Napi::Boolean>().Value());
}

void AbletonLinkAudioWrapper::SetPeerName(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(info.Env(), "Name (string) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    link_->setPeerName(info[0].As<Napi::String>().Utf8Value());
}

void AbletonLinkAudioWrapper::SetChannelsChangedCallback(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(info.Env(), "Function expected")
            .ThrowAsJavaScriptException();
        return;
    }

    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (channelsChangedCallback_) {
        channelsChangedCallback_.Abort();
        channelsChangedCallback_.Release();
    }

    channelsChangedCallback_ = Napi::ThreadSafeFunction::New(
        info.Env(), info[0].As<Napi::Function>(), "ChannelsChangedCallback", 0, 1);
    channelsChangedCallback_.Unref(info.Env());

    link_->setChannelsChangedCallback([this]() { handleChannelsChangedCallback(); });
}

Napi::Value AbletonLinkAudioWrapper::Channels(const Napi::CallbackInfo& info) {
    const auto channels = link_->channels();
    auto result = Napi::Array::New(info.Env(), channels.size());
    for (size_t i = 0; i < channels.size(); ++i) {
        const auto& channel = channels[i];
        auto obj = Napi::Object::New(info.Env());
        obj.Set("id", NodeIdToHexString(channel.id));
        obj.Set("name", channel.name);
        obj.Set("peerId", NodeIdToHexString(channel.peerId));
        obj.Set("peerName", channel.peerName);
        result.Set(i, obj);
    }
    return result;
}

void AbletonLinkAudioWrapper::CallOnLinkThread(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(info.Env(), "Function expected")
            .ThrowAsJavaScriptException();
        return;
    }

    auto tsfn = std::make_shared<Napi::ThreadSafeFunction>(
        Napi::ThreadSafeFunction::New(
            info.Env(), info[0].As<Napi::Function>(), "LinkThreadCall", 0, 1));
    tsfn->Unref(info.Env());

    link_->callOnLinkThread([tsfn]() {
        tsfn->BlockingCall([](Napi::Env env, Napi::Function callback) {
            callback.Call({});
        });
        tsfn->Release();
    });
}

Napi::Value AbletonLinkAudioWrapper::CaptureAppSessionState(
    const Napi::CallbackInfo& info) {
    return AbletonLinkAudioSessionStateWrapper::New(
        info.Env(), link_->captureAppSessionState());
}

void AbletonLinkAudioWrapper::CommitAppSessionState(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(info.Env(), "SessionState object expected")
            .ThrowAsJavaScriptException();
        return;
    }
    auto* wrapper = Napi::ObjectWrap<AbletonLinkAudioSessionStateWrapper>::Unwrap(
        info[0].As<Napi::Object>());
    link_->commitAppSessionState(wrapper->State());
}

Napi::Value AbletonLinkAudioWrapper::CaptureAudioSessionState(
    const Napi::CallbackInfo& info) {
    return AbletonLinkAudioSessionStateWrapper::New(
        info.Env(), link_->captureAudioSessionState());
}

void AbletonLinkAudioWrapper::CommitAudioSessionState(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(info.Env(), "SessionState object expected")
            .ThrowAsJavaScriptException();
        return;
    }
    auto* wrapper = Napi::ObjectWrap<AbletonLinkAudioSessionStateWrapper>::Unwrap(
        info[0].As<Napi::Object>());
    link_->commitAudioSessionState(wrapper->State());
}

void AbletonLinkAudioWrapper::SetNumPeersCallback(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(info.Env(), "Function expected")
            .ThrowAsJavaScriptException();
        return;
    }

    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (numPeersCallback_) {
        numPeersCallback_.Abort();
        numPeersCallback_.Release();
    }

    numPeersCallback_ = Napi::ThreadSafeFunction::New(
        info.Env(), info[0].As<Napi::Function>(), "NumPeersCallback", 0, 1);
    numPeersCallback_.Unref(info.Env());

    link_->setNumPeersCallback([this](std::size_t numPeers) {
        handleNumPeersCallback(numPeers);
    });
}

void AbletonLinkAudioWrapper::SetTempoCallback(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(info.Env(), "Function expected")
            .ThrowAsJavaScriptException();
        return;
    }

    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (tempoCallback_) {
        tempoCallback_.Abort();
        tempoCallback_.Release();
    }

    tempoCallback_ = Napi::ThreadSafeFunction::New(
        info.Env(), info[0].As<Napi::Function>(), "TempoCallback", 0, 1);
    tempoCallback_.Unref(info.Env());

    link_->setTempoCallback([this](double tempo) { handleTempoCallback(tempo); });
}

void AbletonLinkAudioWrapper::SetStartStopCallback(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(info.Env(), "Function expected")
            .ThrowAsJavaScriptException();
        return;
    }

    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (startStopCallback_) {
        startStopCallback_.Abort();
        startStopCallback_.Release();
    }

    startStopCallback_ = Napi::ThreadSafeFunction::New(
        info.Env(), info[0].As<Napi::Function>(), "StartStopCallback", 0, 1);
    startStopCallback_.Unref(info.Env());

    link_->setStartStopCallback([this](bool isPlaying) {
        handleStartStopCallback(isPlaying);
    });
}

void AbletonLinkAudioWrapper::Close(const Napi::CallbackInfo& info) {
    CloseInternal();
}

std::chrono::microseconds AbletonLinkAudioWrapper::getCurrentTime() const {
    return link_->clock().micros();
}

void AbletonLinkAudioWrapper::handleNumPeersCallback(std::size_t numPeers) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (numPeersCallback_) {
        numPeersCallback_.BlockingCall([numPeers](Napi::Env env, Napi::Function callback) {
            callback.Call({Napi::Number::New(env, static_cast<double>(numPeers))});
        });
    }
}

void AbletonLinkAudioWrapper::handleTempoCallback(double tempo) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (tempoCallback_) {
        tempoCallback_.BlockingCall([tempo](Napi::Env env, Napi::Function callback) {
            callback.Call({Napi::Number::New(env, tempo)});
        });
    }
}

void AbletonLinkAudioWrapper::handleStartStopCallback(bool isPlaying) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (startStopCallback_) {
        startStopCallback_.BlockingCall([isPlaying](Napi::Env env, Napi::Function callback) {
            callback.Call({Napi::Boolean::New(env, isPlaying)});
        });
    }
}

void AbletonLinkAudioWrapper::handleChannelsChangedCallback() {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (channelsChangedCallback_) {
        channelsChangedCallback_.BlockingCall(
            [](Napi::Env env, Napi::Function callback) { callback.Call({}); });
    }
}

void AbletonLinkAudioWrapper::CloseInternal() {
    if (!link_) {
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
        if (channelsChangedCallback_) {
            channelsChangedCallback_.Abort();
            channelsChangedCallback_.Release();
        }
    }

    link_->setNumPeersCallback([](std::size_t) {});
    link_->setTempoCallback([](double) {});
    link_->setStartStopCallback([](bool) {});
    link_->setChannelsChangedCallback([]() {});
    link_->enableLinkAudio(false);
    link_->enable(false);
    link_.reset();
}

Napi::Object AbletonLinkAudioSinkBufferHandleWrapper::Init(
    Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    Napi::Function func =
        DefineClass(env, "AbletonLinkAudioSinkBufferHandle", {
            InstanceMethod("isValid",
                           &AbletonLinkAudioSinkBufferHandleWrapper::IsValid),
            InstanceMethod("samples",
                           &AbletonLinkAudioSinkBufferHandleWrapper::Samples),
            InstanceMethod("maxNumSamples",
                           &AbletonLinkAudioSinkBufferHandleWrapper::MaxNumSamples),
            InstanceMethod("commit", &AbletonLinkAudioSinkBufferHandleWrapper::Commit),
        });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("AbletonLinkAudioSinkBufferHandle", func);
    return exports;
}

Napi::Object AbletonLinkAudioSinkBufferHandleWrapper::New(
    Napi::Env env, std::unique_ptr<ableton::LinkAudioSink::BufferHandle> handle) {
    Napi::EscapableHandleScope scope(env);
    auto obj = constructor.New({});
    auto* wrapper = Napi::ObjectWrap<AbletonLinkAudioSinkBufferHandleWrapper>::Unwrap(obj);
    wrapper->handle_ = std::move(handle);
    if (wrapper->handle_ && static_cast<bool>(*wrapper->handle_)) {
        auto* samples = wrapper->handle_->samples;
        const auto maxSamples = wrapper->handle_->maxNumSamples;
        wrapper->samples_ = Napi::Persistent(
            Napi::Buffer<int16_t>::New(env, samples, maxSamples,
                                       [](Napi::Env, int16_t*) {}));
        wrapper->samples_.SuppressDestruct();
    }
    return scope.Escape(napi_value(obj)).ToObject();
}

AbletonLinkAudioSinkBufferHandleWrapper::AbletonLinkAudioSinkBufferHandleWrapper(
    const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<AbletonLinkAudioSinkBufferHandleWrapper>(info) {}

AbletonLinkAudioSinkBufferHandleWrapper::~AbletonLinkAudioSinkBufferHandleWrapper() {
    samples_.Reset();
    handle_.reset();
}

Napi::Value AbletonLinkAudioSinkBufferHandleWrapper::IsValid(
    const Napi::CallbackInfo& info) {
    return Napi::Boolean::New(info.Env(),
                              handle_ && static_cast<bool>(*handle_));
}

Napi::Value AbletonLinkAudioSinkBufferHandleWrapper::Samples(
    const Napi::CallbackInfo& info) {
    if (!samples_) {
        return info.Env().Null();
    }
    return samples_.Value();
}

Napi::Value AbletonLinkAudioSinkBufferHandleWrapper::MaxNumSamples(
    const Napi::CallbackInfo& info) {
    if (!handle_) {
        return Napi::Number::New(info.Env(), 0);
    }
    return Napi::Number::New(info.Env(),
                             static_cast<double>(handle_->maxNumSamples));
}

Napi::Value AbletonLinkAudioSinkBufferHandleWrapper::Commit(
    const Napi::CallbackInfo& info) {
    if (!handle_) {
        return Napi::Boolean::New(info.Env(), false);
    }
    if (info.Length() < 6 || !info[0].IsObject() || !info[1].IsNumber() ||
        !info[2].IsNumber() || !info[3].IsNumber() || !info[4].IsNumber() ||
        !info[5].IsNumber()) {
        Napi::TypeError::New(info.Env(),
                             "SessionState, beatsAtBufferBegin, quantum, numFrames, "
                             "numChannels, and sampleRate expected")
            .ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    auto* stateWrapper = Napi::ObjectWrap<AbletonLinkAudioSessionStateWrapper>::Unwrap(
        info[0].As<Napi::Object>());
    const auto beatsAtBufferBegin = info[1].As<Napi::Number>().DoubleValue();
    const auto quantum = info[2].As<Napi::Number>().DoubleValue();
    const auto numFrames =
        static_cast<size_t>(info[3].As<Napi::Number>().Uint32Value());
    const auto numChannels =
        static_cast<size_t>(info[4].As<Napi::Number>().Uint32Value());
    const auto sampleRate =
        static_cast<uint32_t>(info[5].As<Napi::Number>().Uint32Value());
    const auto result = handle_->commit(stateWrapper->State(),
                                        beatsAtBufferBegin,
                                        quantum,
                                        numFrames,
                                        numChannels,
                                        sampleRate);
    return Napi::Boolean::New(info.Env(), result);
}

Napi::Object AbletonLinkAudioSinkWrapper::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "AbletonLinkAudioSink", {
        InstanceMethod("name", &AbletonLinkAudioSinkWrapper::Name),
        InstanceMethod("setName", &AbletonLinkAudioSinkWrapper::SetName),
        InstanceMethod("requestMaxNumSamples",
                       &AbletonLinkAudioSinkWrapper::RequestMaxNumSamples),
        InstanceMethod("maxNumSamples", &AbletonLinkAudioSinkWrapper::MaxNumSamples),
        InstanceMethod("retainBuffer", &AbletonLinkAudioSinkWrapper::RetainBuffer),
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("AbletonLinkAudioSink", func);
    return exports;
}

AbletonLinkAudioSinkWrapper::AbletonLinkAudioSinkWrapper(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<AbletonLinkAudioSinkWrapper>(info) {
    if (info.Length() < 3 || !info[0].IsObject() || !info[1].IsString() ||
        !info[2].IsNumber()) {
        Napi::TypeError::New(info.Env(),
                             "LinkAudio instance, name (string), and maxNumSamples "
                             "(number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    auto* linkWrapper = Napi::ObjectWrap<AbletonLinkAudioWrapper>::Unwrap(
        info[0].As<Napi::Object>());
    const auto name = info[1].As<Napi::String>().Utf8Value();
    const auto maxNumSamples =
        static_cast<size_t>(info[2].As<Napi::Number>().Uint32Value());
    sink_ = std::make_shared<ableton::LinkAudioSink>(linkWrapper->LinkAudio(),
                                                     name,
                                                     maxNumSamples);
    linkRef_ = Napi::Persistent(info[0].As<Napi::Object>());
    linkRef_.SuppressDestruct();
}

AbletonLinkAudioSinkWrapper::~AbletonLinkAudioSinkWrapper() {
    linkRef_.Reset();
    sink_.reset();
}

Napi::Value AbletonLinkAudioSinkWrapper::Name(const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), sink_->name());
}

void AbletonLinkAudioSinkWrapper::SetName(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(info.Env(), "Name (string) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    sink_->setName(info[0].As<Napi::String>().Utf8Value());
}

void AbletonLinkAudioSinkWrapper::RequestMaxNumSamples(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(info.Env(), "numSamples (number) expected")
            .ThrowAsJavaScriptException();
        return;
    }
    sink_->requestMaxNumSamples(
        static_cast<size_t>(info[0].As<Napi::Number>().Uint32Value()));
}

Napi::Value AbletonLinkAudioSinkWrapper::MaxNumSamples(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), static_cast<double>(sink_->maxNumSamples()));
}

Napi::Value AbletonLinkAudioSinkWrapper::RetainBuffer(const Napi::CallbackInfo& info) {
    auto handle = std::make_unique<ableton::LinkAudioSink::BufferHandle>(*sink_);
    if (!static_cast<bool>(*handle)) {
        return info.Env().Null();
    }
    return AbletonLinkAudioSinkBufferHandleWrapper::New(info.Env(), std::move(handle));
}

Napi::Object AbletonLinkAudioBufferInfoWrapper::Init(Napi::Env env,
                                                     Napi::Object exports) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(env, "AbletonLinkAudioBufferInfo", {
        InstanceMethod("numChannels", &AbletonLinkAudioBufferInfoWrapper::NumChannels),
        InstanceMethod("numFrames", &AbletonLinkAudioBufferInfoWrapper::NumFrames),
        InstanceMethod("sampleRate", &AbletonLinkAudioBufferInfoWrapper::SampleRate),
        InstanceMethod("count", &AbletonLinkAudioBufferInfoWrapper::Count),
        InstanceMethod("sessionBeatTime",
                       &AbletonLinkAudioBufferInfoWrapper::SessionBeatTime),
        InstanceMethod("tempo", &AbletonLinkAudioBufferInfoWrapper::Tempo),
        InstanceMethod("sessionId", &AbletonLinkAudioBufferInfoWrapper::SessionId),
        InstanceMethod("beginBeats", &AbletonLinkAudioBufferInfoWrapper::BeginBeats),
        InstanceMethod("endBeats", &AbletonLinkAudioBufferInfoWrapper::EndBeats),
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("AbletonLinkAudioBufferInfo", func);
    return exports;
}

Napi::Object AbletonLinkAudioBufferInfoWrapper::New(
    Napi::Env env, const ableton::LinkAudioSource::BufferHandle::Info& info) {
    Napi::EscapableHandleScope scope(env);
    auto obj = constructor.New({});
    auto* wrapper = Napi::ObjectWrap<AbletonLinkAudioBufferInfoWrapper>::Unwrap(obj);
    wrapper->info_ = info;
    return scope.Escape(napi_value(obj)).ToObject();
}

AbletonLinkAudioBufferInfoWrapper::AbletonLinkAudioBufferInfoWrapper(
    const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<AbletonLinkAudioBufferInfoWrapper>(info) {}

Napi::Value AbletonLinkAudioBufferInfoWrapper::NumChannels(
    const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), static_cast<double>(info_.numChannels));
}

Napi::Value AbletonLinkAudioBufferInfoWrapper::NumFrames(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), static_cast<double>(info_.numFrames));
}

Napi::Value AbletonLinkAudioBufferInfoWrapper::SampleRate(
    const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), static_cast<double>(info_.sampleRate));
}

Napi::Value AbletonLinkAudioBufferInfoWrapper::Count(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), static_cast<double>(info_.count));
}

Napi::Value AbletonLinkAudioBufferInfoWrapper::SessionBeatTime(
    const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), info_.sessionBeatTime);
}

Napi::Value AbletonLinkAudioBufferInfoWrapper::Tempo(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), info_.tempo);
}

Napi::Value AbletonLinkAudioBufferInfoWrapper::SessionId(
    const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), NodeIdToHexString(info_.sessionId));
}

Napi::Value AbletonLinkAudioBufferInfoWrapper::BeginBeats(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[0].IsObject() || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "SessionState and quantum expected")
            .ThrowAsJavaScriptException();
        return info.Env().Null();
    }
    auto* stateWrapper = Napi::ObjectWrap<AbletonLinkAudioSessionStateWrapper>::Unwrap(
        info[0].As<Napi::Object>());
    const auto quantum = info[1].As<Napi::Number>().DoubleValue();
    return OptionalToValue(
        info.Env(), info_.beginBeats(stateWrapper->State(), quantum));
}

Napi::Value AbletonLinkAudioBufferInfoWrapper::EndBeats(
    const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[0].IsObject() || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "SessionState and quantum expected")
            .ThrowAsJavaScriptException();
        return info.Env().Null();
    }
    auto* stateWrapper = Napi::ObjectWrap<AbletonLinkAudioSessionStateWrapper>::Unwrap(
        info[0].As<Napi::Object>());
    const auto quantum = info[1].As<Napi::Number>().DoubleValue();
    return OptionalToValue(
        info.Env(), info_.endBeats(stateWrapper->State(), quantum));
}

Napi::Object AbletonLinkAudioSourceWrapper::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(env, "AbletonLinkAudioSource", {
        InstanceMethod("id", &AbletonLinkAudioSourceWrapper::Id),
        InstanceMethod("close", &AbletonLinkAudioSourceWrapper::Close),
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("AbletonLinkAudioSource", func);
    return exports;
}

AbletonLinkAudioSourceWrapper::AbletonLinkAudioSourceWrapper(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<AbletonLinkAudioSourceWrapper>(info) {
    if (info.Length() < 3 || !info[0].IsObject() || !info[1].IsString() ||
        !info[2].IsFunction()) {
        Napi::TypeError::New(info.Env(),
                             "LinkAudio instance, channelId (string), and callback "
                             "expected")
            .ThrowAsJavaScriptException();
        return;
    }

    auto* linkWrapper = Napi::ObjectWrap<AbletonLinkAudioWrapper>::Unwrap(
        info[0].As<Napi::Object>());
    const auto idStr = info[1].As<Napi::String>().Utf8Value();
    ableton::ChannelId channelId{};
    if (!ParseNodeIdString(idStr, channelId)) {
        Napi::TypeError::New(info.Env(), "Invalid channelId string").ThrowAsJavaScriptException();
        return;
    }

    bufferCallback_ = Napi::ThreadSafeFunction::New(
        info.Env(), info[2].As<Napi::Function>(), "LinkAudioSourceCallback", 0, 1);
    bufferCallback_.Unref(info.Env());

    source_ = std::make_shared<ableton::LinkAudioSource>(
        linkWrapper->LinkAudio(),
        channelId,
        [this](auto handle) { handleBuffer(handle); });
    linkRef_ = Napi::Persistent(info[0].As<Napi::Object>());
    linkRef_.SuppressDestruct();
}

AbletonLinkAudioSourceWrapper::~AbletonLinkAudioSourceWrapper() {
    CloseInternal();
}

void AbletonLinkAudioSourceWrapper::Close(const Napi::CallbackInfo& info) {
    CloseInternal();
}

void AbletonLinkAudioSourceWrapper::CloseInternal() {
    if (bufferCallback_) {
        bufferCallback_.Abort();
        bufferCallback_.Release();
    }
    source_.reset();
    linkRef_.Reset();
}

Napi::Value AbletonLinkAudioSourceWrapper::Id(const Napi::CallbackInfo& info) {
    if (!source_) {
        return info.Env().Null();
    }
    return Napi::String::New(info.Env(), NodeIdToHexString(source_->id()));
}

void AbletonLinkAudioSourceWrapper::handleBuffer(
    const ableton::LinkAudioSource::BufferHandle& handle) {
    if (!bufferCallback_) {
        return;
    }

    bufferCallback_.BlockingCall(
        [handle](Napi::Env env, Napi::Function callback) {
            const auto numSamples = handle.info.numFrames * handle.info.numChannels;
            auto samples = Napi::Buffer<int16_t>::Copy(env, handle.samples, numSamples);
            auto info = AbletonLinkAudioBufferInfoWrapper::New(env, handle.info);
            auto payload = Napi::Object::New(env);
            payload.Set("samples", samples);
            payload.Set("info", info);
            callback.Call({payload});
        });
}

Napi::Object InitAbletonLinkAudio(Napi::Env env, Napi::Object exports) {
    AbletonLinkAudioSessionStateWrapper::Init(env, exports);
    AbletonLinkAudioWrapper::Init(env, exports);
    AbletonLinkAudioSinkBufferHandleWrapper::Init(env, exports);
    AbletonLinkAudioSinkWrapper::Init(env, exports);
    AbletonLinkAudioBufferInfoWrapper::Init(env, exports);
    AbletonLinkAudioSourceWrapper::Init(env, exports);
    return exports;
}
