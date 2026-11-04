// Copyright (c) 2026 PiPedal contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>

#define TONE3000_STAGE_URI "https://www.tone3000.com/plugin#pipedal-stage-v1"
#define TONE3000_STAGE_ABI_VERSION 1u
#define TONE3000_STAGE_MAX_REQUEST 256u
#define TONE3000_STAGE_MAX_RESPONSE 32768u

enum Tone3000StageCommand {
    TONE3000_STAGE_RUNTIME = 1,
    TONE3000_STAGE_AUTO_BALANCE = 2,
    TONE3000_STAGE_AUTO_ALIGN = 3,
    TONE3000_STAGE_CANCEL = 4,
    TONE3000_STAGE_MEASURE_OUTPUT = 5,
    TONE3000_STAGE_LINKED_PAN = 6,
    TONE3000_STAGE_EXCLUSIVE_SOLO = 7,
    TONE3000_STAGE_BLOCK_EDIT = 8
};
enum Tone3000StageError {
    TONE3000_STAGE_UNSUPPORTED = -1,
    TONE3000_STAGE_INVALID = -2,
    TONE3000_STAGE_BUSY = -3,
    TONE3000_STAGE_CLOSED = -4,
    TONE3000_STAGE_STALE = -5,
    TONE3000_STAGE_CAPACITY = -6
};

/** Non-RT-only LV2 extension. instance is the plugin's own LV2_Handle.
 * submit returns zero if queued; errors are negative. Token must be nonzero.
 * read returns zero while queued, positive JSON bytes INCLUDING terminal NUL,
 * or a negative error. Completed results remain readable until the next submit.
 * Cancel uses the same active token and no payload. No mutation is replayed.
 * @see docs/specs/tone3000-gig-rig/tone3000-gig-rig.md [DES-STAGE-BRIDGE] [7.5] [7.6]
 */
struct Tone3000StageBridge {
    uint32_t abiVersion;
    uint32_t maxRequestBytes;
    uint32_t maxResponseBytes;
    int32_t (*submit)(void* instance, uint64_t token, uint32_t command,
                      const char* request, uint32_t requestBytes);
    int32_t (*read)(void* instance, uint64_t token, char* response, uint32_t capacity);
};

#ifdef __cplusplus
/** The LV2 wrapper dynamic-casts only its own AudioProcessor to this interface.
 * @see docs/specs/tone3000-gig-rig/tone3000-gig-rig.md [DES-STAGE-BRIDGE] [7.5]
 */
class Tone3000StageProcessor {
public:
    virtual ~Tone3000StageProcessor() = default;
    virtual int32_t submitStageRequest(uint64_t, uint32_t, const char*, uint32_t) = 0;
    virtual int32_t readStageResult(uint64_t, char*, uint32_t) = 0;
};
#endif
