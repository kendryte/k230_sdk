#include <iostream>
#include "Base64.hh"
#include <iomanip>
#include "h264LiveFrameSource.h"

H264LiveFrameSource* H264LiveFrameSource::createNew(UsageEnvironment &env, size_t queue_size) {
    return new H264LiveFrameSource(env, queue_size);
}

H264LiveFrameSource::H264LiveFrameSource(UsageEnvironment &env, size_t queue_size) : LiveFrameSource(env, queue_size) 
{}

const uint8_t H264marker[] = {0, 0, 0, 1};
uint8_t *H264LiveFrameSource::extractFrame(uint8_t *frame, size_t &size, size_t &outsize) {
    unsigned char *outFrame = NULL;
    outsize = 0;
    if ((size >= sizeof(H264marker)) && (memcmp(frame, H264marker, sizeof(H264marker)) == 0)) {
        size -= sizeof(H264marker);
        outFrame = &frame[sizeof(H264marker)];
        outsize = size;
        for (int i = 0; i + sizeof(H264marker) < size; ++i) {
            if (memcmp(&outFrame[i], H264marker, sizeof(H264marker)) == 0) {
                outsize = (size_t) i;
                break;
            }
        }
        size -= outsize;
    }
    return outFrame;
}

std::list<LiveFrameSource::FramePacket>
H264LiveFrameSource::parseFrame(std::shared_ptr<uint8_t> data, size_t data_size, const struct timeval &ref) {
    std::list<FramePacket> packetList;

    size_t bufSize = data_size;
    size_t size = 0;
    uint8_t *buffer = this->extractFrame(data.get(), bufSize, size);
    while (buffer != NULL) {
        switch (buffer[0] & 0x1F) {
            case 7:
                fAuxLine.clear();
                fSps.reset(), fPps.reset();
                fSps = make_shared_array<uint8_t>(size);;
                memcpy(fSps.get(), buffer, size);
                sps_size = size;
                break;
            case 8:
                fPps = make_shared_array<uint8_t>(size);
                memcpy(fPps.get(), buffer, size);
                pps_size = size;
                break;
            case 5:
                if (fRepeatConfig && fSps && fPps) {
                    FramePacket sps(fSps, 0, sps_size, ref);
                    packetList.push_back(sps);
                    FramePacket pps(fPps, 0, pps_size, ref);
                    packetList.push_back(pps);
                }
                break;
            default:
                break;
        }

        if (fAuxLine.empty() && fSps && fPps) {
            u_int32_t profile_level_id = 0;
            if (sps_size >= 4)
                profile_level_id = (u_int32_t) ((fSps.get()[1] << 16) | (fSps.get()[2] << 8) | fSps.get()[3]);
            char *sps_base64 = base64Encode((char*)fSps.get(), sps_size);
            char *pps_base64 = base64Encode((char*)fPps.get(), pps_size);

            std::ostringstream os;
            os << "profile-level-id=" << std::hex << std::setw(6) << profile_level_id;
            os << ";sprop-parameter-sets=" << sps_base64 << "," << pps_base64 << ";";
            fAuxLine.assign(os.str());

            free(sps_base64);
            free(pps_base64);
            // std::cout << "H264 SDP-aux-line: " << fAuxLine.c_str() << std::endl;
        }
        FramePacket packet(data, buffer - data.get(), size, ref);
        packetList.push_back(packet);
        buffer = this->extractFrame(&buffer[size], bufSize, size);
    }
    return packetList;
}
