#include <iostream>
#include <Base64.hh>
#include <iomanip>
#include "h265LiveFrameSource.h"

H265LiveFrameSource* H265LiveFrameSource::createNew(UsageEnvironment &env, size_t queue_size) {
    return new H265LiveFrameSource(env, queue_size);
}

H265LiveFrameSource::H265LiveFrameSource(UsageEnvironment &env, size_t queue_size) : LiveFrameSource(env, queue_size) 
{}

const uint8_t H265marker[] = {0, 0, 0, 1};
uint8_t *H265LiveFrameSource::extractFrame(uint8_t *frame, size_t &size, size_t &outsize) {
    unsigned char *outFrame = NULL;
    outsize = 0;
    if ((size >= sizeof(H265marker)) && (memcmp(frame, H265marker, sizeof(H265marker)) == 0)) {
        size -= sizeof(H265marker);
        outFrame = &frame[sizeof(H265marker)];
        outsize = size;
        for (int i = 0; i + sizeof(H265marker) < size; ++i) {
            if (memcmp(&outFrame[i], H265marker, sizeof(H265marker)) == 0) {
                outsize = (size_t) i;
                break;
            }
        }
        size -= outsize;
    }
    return outFrame;
}


#define H265_NAL(v)	 ((v>> 1) & 0x3f)

std::list<LiveFrameSource::FramePacket>
H265LiveFrameSource::parseFrame(std::shared_ptr<uint8_t> data, size_t data_size, const struct timeval &ref) {
    std::list<FramePacket> packetList;

    size_t bufSize = data_size;
    size_t size = 0;
    uint8_t *buffer = this->extractFrame(data.get(), bufSize, size);
    while (buffer != NULL) {
        int nal_type = H265_NAL(buffer[0]);
        switch (nal_type) {
            case 32: // VPS
                fAuxLine.clear();
                fVps.reset(), fSps.reset(), fPps.reset();
                fVps = make_shared_array<uint8_t>(size);
                memcpy(fVps.get(), buffer, size);
                vps_size = size;
                break;
            case 33: // SPS
                fSps = make_shared_array<uint8_t>(size);
                memcpy(fSps.get(), buffer, size);
                sps_size = size;
                break;
            case 34: // PPS
                fPps = make_shared_array<uint8_t>(size);
                memcpy(fPps.get(), buffer, size);
                pps_size = size;
                break;
            case 19:  // IDR_W_RADL 
            case 20:  // IDR_N_LP
                if (fRepeatConfig && fVps && fSps && fPps) {
                    FramePacket vps(fVps, 0, vps_size, ref);
                    packetList.push_back(vps);
                    FramePacket sps(fSps, 0, sps_size, ref);
                    packetList.push_back(sps);
                    FramePacket pps(fPps, 0, pps_size, ref);
                    packetList.push_back(pps);
                }
                break;
            default:
                break;
        }

        if (fAuxLine.empty() && fVps && fSps && fPps) {
            u_int32_t profile_level_id = 0;
            if (sps_size >= 4)
                profile_level_id = (u_int32_t) ((fSps.get()[1] << 16) | (fSps.get()[2] << 8) | fSps.get()[3]);
            char *vps_base64 = base64Encode((char*)fVps.get(), vps_size);
            char *sps_base64 = base64Encode((char*)fSps.get(), sps_size);
            char *pps_base64 = base64Encode((char*)fPps.get(), pps_size);
            std::ostringstream os;
            os << "profile-level-id=" << std::hex << std::setw(6) << profile_level_id;
            os << ";sprop-parameter-sets=" << vps_base64 << "," << sps_base64 << "," << pps_base64 << ";";
            fAuxLine.assign(os.str());

            free(vps_base64);
            free(sps_base64);
            free(pps_base64);
            // std::cout << "H265 SDP-aux-line: " << fAuxLine.c_str() << std::endl;
        }
        FramePacket packet(data, buffer - data.get(), size, ref);
        packetList.push_back(packet);
        buffer = this->extractFrame(&buffer[size], bufSize, size);
    }
    return packetList;
}
