// Copyright (c) 2026 PiPedal contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "Tone3000StageBridge.hpp"
#include "Tone3000StageMeter.hpp"
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
class TONE3000Processor;
/** @see docs/specs/tone3000-gig-rig/tone3000-gig-rig.md [DES-STAGE-BRIDGE] [7.5] [7.6] */
class Tone3000StageSession final : private juce::Timer,
    private juce::AudioProcessorValueTreeState::Listener,
    public std::enable_shared_from_this<Tone3000StageSession> {
public:
    explicit Tone3000StageSession(TONE3000Processor&);
    ~Tone3000StageSession() override;
    int32_t submit(uint64_t,uint32_t,const char*,uint32_t);
    int32_t read(uint64_t,char*,uint32_t);
    void shutdown();
    void measure(const juce::AudioBuffer<float>&) noexcept;
private:
    void begin(uint64_t,uint32_t,const juce::String&);
    void timerCallback() override;
    void parameterChanged(const juce::String&,float) override;
    void cancelNative();
    bool applyBlockEdit(const juce::var&, bool);
    void restoreBlockEdit();
    void finish(const juce::String&);
    void publish(juce::DynamicObject::Ptr);
    juce::var runtime();
    bool liveStereo(const juce::var&) const;
    std::mutex mutex;
    TONE3000Processor* owner;
    std::vector<juce::String> parameterIds;
    std::shared_ptr<Tone3000StageSession> timerLifetime;
    uint64_t token=0, baselineParameters=0;
    uint32_t command=0;
    bool active=false;
    bool cancelRequested=false;
    double deadlineMs=0;
    int chainRevision=0;
    std::string result;
    std::atomic<uint64_t> parameterRevision{0};
    std::atomic<bool> ownCompletionWrites{false};
    Tone3000StageMeter meter;
    juce::String blockEditId, blockEditKind, blockEditParam;
    int blockEditBand=-1;
    juce::var blockEditBaseline;
    bool blockEditCommitted=false;
};
