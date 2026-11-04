// Copyright (c) 2026 PiPedal contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <cmath>
#include <cstdint>
#include <limits>
#include <algorithm>

/** RT accumulation only while armed; arm/cancel are serialized against rendering
 * by the host's existing callback lock. Complete release/acquire freezes results.
 * @see docs/specs/tone3000-gig-rig/tone3000-gig-rig.md [DES-STAGE-BRIDGE] [7.6]
 */
class Tone3000StageMeter {
public:
    struct Result {double rms=0,peak=0,seconds=0;uint64_t samples=0;int channels=0;bool finite=false,clipped=false,silent=true,valid=false;};
    bool arm(double sampleRate,double seconds) noexcept {
        if(!std::isfinite(sampleRate)||sampleRate<8000||sampleRate>384000||!std::isfinite(seconds)||seconds<1||seconds>10)return false;
        rate=sampleRate;target=(uint64_t)std::ceil(sampleRate*seconds);frames=0;channels=0;sum=peak=0;finite=true;
        state.store(1,std::memory_order_release);return true;
    }
    void cancel() noexcept {state.store(0,std::memory_order_release);}
    bool complete()const noexcept {return state.load(std::memory_order_acquire)==2;}
    void add(const float* const* data,int count,uint64_t length)noexcept {
        if(state.load(std::memory_order_acquire)!=1||count<=0||!data)return;
        const auto wanted=std::min(length,target-frames);if(!wanted)return;
        if(channels&&channels!=count)finite=false;channels=count;
        for(int c=0;c<count;++c) {
            if(!data[c]){finite=false;continue;}
            for(uint64_t i=0;i<wanted;++i){const double value=data[c][i];if(!std::isfinite(value)){finite=false;continue;}sum+=value*value;peak=std::max(peak,std::abs(value));}
        }
        frames+=wanted;if(frames>=target)state.store(2,std::memory_order_release);
    }
    Result result()const noexcept {
        if(!complete())return {};
        Result output;output.rms=frames&&channels?std::sqrt(sum/(frames*(double)channels)):0;
        output.peak=peak;output.samples=frames;output.channels=channels;output.seconds=frames/rate;
        output.finite=finite&&std::isfinite(output.rms)&&std::isfinite(peak);
        output.clipped=peak>=1;output.silent=output.rms<3.16e-3;
        output.valid=output.finite&&!output.clipped&&!output.silent&&frames>=target;
        return output;
    }
private:
    std::atomic<int> state{0};
    uint64_t frames=0,target=0;int channels=0;double sum=0,peak=0,rate=0;bool finite=true;
};
