#ifndef __COMM_RPC_MSG_H__
#define __COMM_RPC_MSG_H__

#include <cstddef>
#include <cstdint>
#include <string>

struct ServerInfo {
    // TODO
    bool has_bidirection_speech = false;
    std::string speech_stream_name;
    int speech_service_port = 8554;
};

struct UserEventData {
    enum class EventType : int {
        INVALID,
        PIR_WAKEUP,
        KEY_WAKEUP,
        STAY_ALARM,
        BOTTOM
    } type{EventType::INVALID};
    int64_t timestamp_ms;  // gettimeofday_ms
    uint8_t *jpeg{nullptr};
    size_t jpeg_size = 0;
};

#endif /* __COMM_RPC_MSG_H__ */
