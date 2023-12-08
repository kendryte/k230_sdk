/**
 * Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __COMM_MSG_H__
#define __COMM_MSG_H__

#include <cstddef>
#include <cstdint>

#define SEND_SYNC_MODULE_ID   1
#define SEND_ASYNC_MODULE_ID  2
#define SEND_ONLY_MODULE_ID   3

// UserMessage : TLV
struct UserMessage {
    int type;
    int len = 0;
    char data[];
};

//
enum class UserMsgType : int {
    UserMsgBase = 0,
    // client->server
    CLIENT_READY,
    KEY_PRESSED,
    PLAYBACK,
    EXIT,
    // server->client
    SERVER_MSG_BASE = 100,
    PLAYBACK_ACK,
    CURRENT_MODE_TYPE,
    //
    UserMsgMax = 1000
};

struct UserEventData {
    enum class EventType : int {
        INVALID,
        PIR_WAKEUP,
        KEY_WAKEUP,
        STAY_ALARM,
        BOTTOM
    } type {EventType::INVALID};
    int64_t timestamp_ms;  // gettimeofday_ms
    uint8_t *jpeg{nullptr};
    size_t jpeg_size = 0;
};

struct AVEncFrameData {
    uint8_t *data{nullptr};
    size_t size{0};
    int64_t timestamp_ms{0};  // gettimeofday_ms
    enum class Type : int { INVALID, H264, H265, PCMU, BOTTOM} type{Type::INVALID};
    bool keyframe{false};
    uint32_t sequence{0};
    uint32_t flags{0};
};

#endif /* __COMM_MSG_H__ */
