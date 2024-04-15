/*
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from Arm Technology (China) Co., Ltd.
 *
 *            (C) COPYRIGHT 2021-2021 Arm Technology (China) Co., Ltd.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from Arm Technology (China) Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <algorithm>
#include <cmath>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <unistd.h>

#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#include <linux/videodev2.h>
// #include <linux/ion.h>
#include "ion.h"

#include "mvx_player.hpp"
#include "md5.h"

using namespace std;

/****************************************************************************
 * Defines
 ****************************************************************************/
// Define bit field extract and test macros
// SBFX = signed bitfield extract
//  BFS = bit field set
#define SBFX(Rn,lsb,width)   ((int32_t)((Rn)<<(32-(lsb)-(width)))>>(32-(width)))
#define BFS(Rn,lsb,width)    (((Rn)&((1u<<(width))-1))<<(lsb))
#define UBFX(Rn,lsb,width)   ((uint32_t)((Rn)<<(32-(lsb)-(width)))>>(32-(width)))

// Extract macros as above but with width macro automatically generated
#define GET(lsb,Rn)          UBFX(Rn,lsb,lsb ## _SZ)
#define SET(lsb,Rn)          BFS(Rn,lsb,lsb ## _SZ)
#define SGET(lsb,Rn)         SBFX(Rn,lsb,lsb ## _SZ)

#ifndef V4L2_BUF_FLAG_LAST
#define V4L2_BUF_FLAG_LAST 0x00100000
#endif

#define V4L2_ALLOCATE_BUFFER_ROI 1048576 * 3
#define V4L2_READ_LEN_BUFFER_ROI 1048576 * 2

#ifndef V4L2_EVENT_SOURCE_CHANGE
#define V4L2_EVENT_SOURCE_CHANGE 5
#endif

#ifndef MVX_AD_STATS_THUMBNAIL_SIZE
#define MVX_AD_STATS_THUMBNAIL_SIZE (3*4096)
#endif



typedef reader::result result;
/****************************************************************************
 * Misc
 ****************************************************************************/

template<typename T, typename U>
static T divRoundUp(T value, U round)
{
    return (value + round - 1) / round;
}

template<typename T, typename U>
static T roundUp(T value, U round)
{
    return divRoundUp(value, round) * round;
}

/*
static void swapInt32( int32_t& value)
{
    int32_t tmp =value;
    value = ((tmp & 0x000000FF) << 24) |
           ((tmp & 0x0000FF00) << 8) |
           ((tmp & 0x00FF0000) >> 8) |
           ((tmp & 0xFF000000) >> 24) ;
}
*/

static void swapUint32( uint32_t& value)
{
    uint32_t tmp =value;
    value = ((tmp & 0x000000FF) << 24) |
            ((tmp & 0x0000FF00) << 8) |
            ((tmp & 0x00FF0000) >> 8) |
            ((tmp & 0xFF000000) >> 24) ;
}

static void swapUint16( uint16_t& value)
{
    uint16_t tmp =value;
    value = ((tmp & 0x00FF) << 8) | ((tmp & 0xFF00) >> 8);
}

/****************************************************************************
 * Exception
 ****************************************************************************/

Exception::Exception(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
}

Exception::Exception(const string &str)
{
    strncpy(msg, str.c_str(), sizeof(msg));
}

Exception::~Exception() throw()
{}

const char *Exception::what() const throw()
{
    return msg;
}

/****************************************************************************
 * Input and output
 ****************************************************************************/

const uint32_t IVFHeader::signatureDKIF = v4l2_fourcc('D', 'K', 'I', 'F');
static const  uint8_t startCode[4] = { 0x00, 0x00, 0x00, 0x01 };
static const  uint8_t subStartCode[3] = { 0x00, 0x00, 0x01 };
static std::mutex m_Mutex;

//COnfiguration file maximum line lengths, for use by fgets()
//ROI requires 512
//ERP requires 10+1 + 7+1 + 11+1 + (22+1)*256 = 5919 (8k width)
#define CFG_FILE_LINE_SIZE (6144)
static char cfg_file_line_buf[CFG_FILE_LINE_SIZE];
//static int startcode_find_candidate(char *buf, int size);

IVFHeader::IVFHeader() :
    signature(signatureDKIF),
    version(0),
    length(32),
    codec(0),
    width(0),
    height(0),
    frameRate(30 << 16),
    timeScale(1 << 16),
    frameCount(0),
    padding(0)
{}

IVFHeader::IVFHeader(uint32_t codec, uint16_t width, uint16_t height) :
    signature(signatureDKIF),
    version(0),
    length(32),
    codec(codec),
    width(width),
    height(height),
    frameRate(30 << 16),
    timeScale(1 << 16),
    frameCount(0),
    padding(0)
{}

IVFFrame::IVFFrame() :
    size(0),
    timestamp(0)
{}

IVFFrame::IVFFrame(uint32_t size, uint64_t timestamp) :
    size(size),
    timestamp(timestamp)
{}

const uint8_t VC1SequenceLayerData::magic1 = 0xC5;
const uint32_t VC1SequenceLayerData::magic2 = 4;

VC1SequenceLayerData::VC1SequenceLayerData() :
    numFrames(0),
    signature1(0),
    signature2(0),
    headerC(),
    restOfSLD()
{}

VC1FrameLayerData::VC1FrameLayerData() :
    frameSize(0),
    reserved(0),
    key(0),
    timestamp(0)
{}



RVHeader::RVHeader()
{}

const uint32_t RVHeader::signatureVIDO = v4l2_fourcc('V', 'I', 'D', 'O');
const uint32_t RVHeader::signatureRV30 = v4l2_fourcc('R', 'V', '3', '0');
const uint32_t RVHeader::signatureRV40 = v4l2_fourcc('R', 'V', '4', '0');

RVFrame::RVFrame()
{}

const uint32_t RVFrame::slicestartcode = 0x00000001;

RVSlice::RVSlice()
{}


AFBCHeader::AFBCHeader()
{}

AFBCHeader::AFBCHeader(const v4l2_format &format, size_t frameSize, const v4l2_crop &crop, bool tiled, const int field) :
    magic(MAGIC),
    headerSize(sizeof(AFBCHeader)),
    version(VERSION),
    frameSize(frameSize),
    numComponents(0),
    subsampling(0),
    yuvTransform(false),
    blockSplit(false),
    yBits(0),
    cbBits(0),
    crBits(0),
    alphaBits(0),
    mbWidth(0),
    mbHeight(0),
    width(0),
    height(0),
    cropLeft(crop.c.left),
    cropTop(crop.c.top),
    param(0),
    fileMessage(field)
{
    uint32_t pixelformat;

    if (V4L2_TYPE_IS_MULTIPLANAR(format.type))
    {
        const v4l2_pix_format_mplane &f = format.fmt.pix_mp;

        width = f.width;
        height = f.height;
        pixelformat = f.pixelformat;
    }
    else
    {
        const v4l2_pix_format &f = format.fmt.pix;

        width = f.width;
        height = f.height;
        pixelformat = f.pixelformat;
    }

    if (field != FIELD_NONE)
    {
        height = divRoundUp(height, 2);
    }
    mbWidth = (width + cropLeft + 15) / 16;
    mbHeight = (height + cropTop + 15) / 16;

    switch (pixelformat)
    {
        case V4L2_PIX_FMT_YUV420_AFBC_8:
            numComponents = 3;
            subsampling = 1;
            yBits = 8;
            crBits = 8;
            cbBits = 8;
            alphaBits = 0;
            break;
        case V4L2_PIX_FMT_YUV420_AFBC_10:
            numComponents = 3;
            subsampling = 1;
            yBits = 10;
            crBits = 10;
            cbBits = 10;
            alphaBits = 0;
            break;
        case V4L2_PIX_FMT_YUV422_AFBC_8:
            numComponents = 3;
            subsampling = 2;
            yBits = 8;
            crBits = 8;
            cbBits = 8;
            alphaBits = 0;
            break;
        case V4L2_PIX_FMT_YUV422_AFBC_10:
            numComponents = 3;
            subsampling = 2;
            yBits = 10;
            crBits = 10;
            cbBits = 10;
            alphaBits = 0;
            break;
        default:
            throw Exception("Unsupported AFBC pixel format. pixelformat=0x%x.", pixelformat);
    }

    if (tiled)
    {
        param |= (PARAM_TILED_BODY | PARAM_TILED_HEADER);
    }
}





IO::IO(uint32_t format, size_t width, size_t height, size_t strideAlign) :
    format(format),
    width(width),
    height(height),
    strideAlign(strideAlign)
{}

Input::Input(uint32_t format, size_t width, size_t height, size_t strideAlign) :
    IO(format, width, height, strideAlign)
{
    if (format == V4L2_PIX_FMT_VC1_ANNEX_G)
    {
        profile = 12;
    }
    dir = 0;
}

InputFile::InputFile(istream &input, uint32_t format) :
    Input(format),
    input(input)
{
    state = 7;
    offset=0;
    curlen=0;
    iseof = false;
    naluFmt = 0;
    remaining_bytes = 0;
}

InputFile::InputFile(istream &input, uint32_t format, size_t width, size_t height, size_t strideAlign) :
    Input(format, width, height, strideAlign),
    input(input)
{
    state = 7;
    offset=0;
    curlen=0;
    iseof = false;
    naluFmt = 0;
    remaining_bytes = 0;
}

InputFile::~InputFile()
{
    if (inputBuf) {
        free(inputBuf);
    }
    if (reader) {
        delete reader;
    }
}

void InputFile::prepare(Buffer &buf)
{
    vector<iovec> iov = buf.getImageSize();
    buf.getBuffer().flags = 0;
    if (getNaluFormat() != V4L2_OPT_NALU_FORMAT_START_CODES) {
        reader::result res;
        uint8_t *buffer_data = static_cast<uint8_t *>(iov[0].iov_base);
        uint32_t buffer_size = buf.getBuffer().length;
        uint32_t buf_offset = 0;
        int mode = 0, nalu_val = 0;
        {
            m_Mutex.lock();
            if (!reader) {
                reader = new start_code_reader(getFormat(), &input);
                switch(getNaluFormat()){
                    case V4L2_OPT_NALU_FORMAT_ONE_BYTE_LENGTH_FIELD:
                    case V4L2_OPT_NALU_FORMAT_TWO_BYTE_LENGTH_FIELD:
                    case V4L2_OPT_NALU_FORMAT_FOUR_BYTE_LENGTH_FIELD:
                        mode |= reader::RMODE_DELIMITED;
                        if (V4L2_OPT_NALU_FORMAT_ONE_BYTE_LENGTH_FIELD == getNaluFormat()) {
                            nalu_val = 1;
                        } else if (V4L2_OPT_NALU_FORMAT_TWO_BYTE_LENGTH_FIELD == getNaluFormat()) {
                            nalu_val = 2;
                        } else {
                            nalu_val = 4;
                        }
                        reader->set_metadata(reader::RMETA_DELIMITER_LENGTH, nalu_val);
                        break;
                    case V4L2_OPT_NALU_FORMAT_ONE_NALU_PER_BUFFER:
                        mode |= reader::RMODE_PACKETED;
                        send_end_of_subframe_flag = true;
                        send_end_of_frame_flag = true;
                        break;
                    case V4L2_OPT_NALU_FORMAT_ONE_FRAME_PER_BUFFER:
                        send_end_of_frame_flag = true;
                        break;
                }
                if (send_end_of_subframe_flag || send_end_of_frame_flag) {
                    mode |= reader::RMODE_FLAGS;
                }
                input.seekg(0, ios::end);
                reader->setFileLength((uint64_t)input.tellg());
                input.seekg(0,ios::beg);
                reader->set_mode(mode);
            }
            m_Mutex.unlock();
        }
        do {
            uint32_t filled_size;
            res = reader->read(0, buffer_data+buf_offset, buffer_size-buf_offset, &filled_size);
            buf_offset += filled_size;

            if(res == reader::RR_EOS)
            {
                break;
            }
            if(send_end_of_frame_flag && (res==reader::RR_EOP_FRAME || res==reader::RR_EOP_CODEC_CONFIG))
            {
                break;
            }
            bool eosf = res==reader::RR_EOP || res==reader::RR_EOP_FRAME || res==reader::RR_EOP_CODEC_CONFIG;
            if(send_end_of_subframe_flag && eosf)
            {
                break;
            }
        } while (buf_offset < buffer_size);
        if(res==reader::RR_EOP_CODEC_CONFIG)
        {
            buf.setCodecConfig( true );
        }
        if(res==reader::RR_EOP_FRAME && send_end_of_frame_flag)
        {
            buf.setEndOfFrame( true );
        }
        if((res==reader::RR_EOP || res == reader::RR_EOP_FRAME || res==reader::RR_EOP_CODEC_CONFIG) && send_end_of_subframe_flag)
        {
            buf.setEndOfSubFrame( true );
        }
        if(res==reader::RR_EOS)
        {
            iseof = true;
            buf.setEndOfStream( true );
        }
        iov[0].iov_len = buf_offset;
        buf.setBytesUsed(iov);
    } else {
        for (size_t i = 0; i < iov.size(); ++i)
        {
            input.read(static_cast<char *>(iov[i].iov_base), iov[i].iov_len);
            if ((unsigned int)input.gcount() < iov[i].iov_len) {
                iseof = true;

            }
            iov[i].iov_len = input.gcount();
        }

        buf.setBytesUsed(iov);
    }
}

/*int startcode_find_candidate(char *buf, int size){
    int i = 0;
    for (; i < size; i++){
        if (!buf[i])
            break;
    }
    return i;
}*/
bool InputFile::eof()
{
    //return input.peek() == EOF;
    return iseof;
}

InputIVF::InputIVF(istream &input, uint32_t informat) :
    InputFile(input, 0)
{
    IVFHeader header;
    left_bytes = 0;
    input.read(reinterpret_cast<char *>(&header), sizeof(header));

    if (header.signature != IVFHeader::signatureDKIF)
    {
        char *c = reinterpret_cast<char *>(&header.signature);
        throw Exception("Incorrect DKIF signature. signature=%c%c%c%c.", c[0], c[1], c[2], c[3]);
    }

    if (header.version != 0)
    {
        throw Exception("Incorrect DKIF version. version=%u.", header.version);
    }

    if (header.length != 32)
    {
        throw Exception("Incorrect DKIF length. length=%u.", header.length);
    }
    if (informat != header.codec) {
        format = informat;
    } else {
        format = header.codec;
    }
}

bool InputIVF::eof()
{
    return input.peek() == EOF;
}

void InputIVF::prepare(Buffer &buf)
{
    uint32_t cal_size = 0;
    if (left_bytes == 0) {
        IVFFrame frame;
        input.read(reinterpret_cast<char *>(&frame), sizeof(frame));
        cal_size = frame.size;
        timestamp = frame.timestamp;

    } else {
        cal_size = left_bytes;
    }

    vector<iovec> iov = buf.getImageSize();
    uint32_t read_bytes = cal_size > iov[0].iov_len? iov[0].iov_len:cal_size;
    input.read(static_cast<char *>(iov[0].iov_base), read_bytes);
    if (input.gcount() < read_bytes){
        cout<<"read less than need!."<<endl;
    }
    iov[0].iov_len = input.gcount();
    buf.setEndOfFrame(cal_size == iov[0].iov_len);
    left_bytes = cal_size - iov[0].iov_len;
    buf.setBytesUsed(iov);
    buf.setTimeStamp(timestamp);

}

InputRCV::InputRCV(istream &input) :
    InputFile(input, 0)
{
    input.read(reinterpret_cast<char *>(&sld), sizeof(sld));

    if (sld.signature1 != VC1SequenceLayerData::magic1 ||
        sld.signature2 != VC1SequenceLayerData::magic2)
    {
        cout<<"This is a raw stream of VC1!"<<endl;
        format = V4L2_PIX_FMT_VC1_ANNEX_G;
        profile = 12;
        codecConfigSent = true;
        isRcv = false;
        input.clear();
        input.seekg(0);
    } else {

        /* headerC is serialized in bigendian byteorder. */
        uint32_t q = sld.headerC;
        q = ntohl(q);

        HeaderC hc = reinterpret_cast<HeaderC &>(q);
        profile = hc.profile;

        format = V4L2_PIX_FMT_VC1_ANNEX_L;

        codecConfigSent = false;
        isRcv = true;
    }
}

bool InputRCV::eof()
{
    return input.peek() == EOF;
}

void InputRCV::prepare(Buffer &buf)
{
    vector<iovec> iov = buf.getImageSize();
    if (!isRcv) {
        InputFile::prepare(buf);
        return;
    }
    if (codecConfigSent != false)
    {
        uint32_t cal_size = 0;
        uint32_t read_bytes = 0;
        char * read_addr;
        uint32_t offset = 0;
        VC1FrameLayerData *fld = static_cast<VC1FrameLayerData *>(iov[0].iov_base);
        if (left_bytes == 0) {
            input.read(reinterpret_cast<char *>(fld), sizeof(*fld));
            cal_size = fld->frameSize;
            read_addr = reinterpret_cast<char *>(fld->data);
            offset = sizeof(*fld);
        } else {
            cal_size = left_bytes;
            read_addr = static_cast<char *>(iov[0].iov_base);
        }
        read_bytes = iov[0].iov_len - offset > cal_size?cal_size:iov[0].iov_len - offset;
        input.read(read_addr, read_bytes);
        iov[0].iov_len = input.gcount() + offset;
        left_bytes = cal_size > input.gcount()? cal_size - input.gcount(): 0;
        buf.setEndOfFrame(left_bytes == 0);
    }
    else
    {
        memcpy(iov[0].iov_base, &sld, sizeof(sld));
        iov[0].iov_len = sizeof(sld);

        buf.setCodecConfig(true);
        codecConfigSent = true;
    }

    buf.setBytesUsed(iov);
}


InputRV::InputRV(istream &input) :
    InputFile(input, 0)
{
    b_header_read =false;
    frame_left_bytes =0;
    slice_read_indx =0;
    memset(&frameheader,0,sizeof(RVFrame));
    memset(&header,0,sizeof(RVFrame));
    input.read(reinterpret_cast<char *>(&header), sizeof(header));
    swapUint32(header.metlen);
    cout<<"read a new RVHeader";
    cout<<" metlen:"<<header.metlen<<endl;

    if (header.signature1 != RVHeader::signatureVIDO)
    {
        throw Exception("Incorrect VIDO signature1");
    }

    if(header.signature2 != RVHeader::signatureRV30 && header.signature2 != RVHeader::signatureRV40)
    {
        throw Exception("Incorrect VIDO signature2");
    }
    if(header.metlen < sizeof(header))
    {
        throw Exception("Incorrect metlen:%u",header.metlen);
    }
    //input.seekg(header.metlen- sizeof(header),ios::cur);
    input.seekg(0,ios::beg);

    format = V4L2_PIX_FMT_RV;
}

bool InputRV::eof()
{
    return input.peek() == EOF;
}

void InputRV::prepare_config_data(Buffer &buf)
{
    uint32_t cal_size = header.metlen;
    vector<iovec> iov = buf.getImageSize();
    uint32_t read_bytes = cal_size > iov[0].iov_len ? iov[0].iov_len : cal_size;

    cout<<"read RVHeader config data"<<endl;
    input.read(static_cast<char *>(iov[0].iov_base), read_bytes);
    if (input.gcount() < read_bytes){
        cout<<"read less than need!."<<endl;
    }
    iov[0].iov_len = input.gcount();

    buf.setEndOfSubFrame(1);
    buf.setCodecConfig(1);
    buf.setBytesUsed(iov);
    buf.setTimeStamp(0);
    b_header_read =true;
}

void InputRV::prepare_slice_data(Buffer &buf)
{
    uint32_t i =0;
    uint32_t cal_size = 0;

    if (slice_read_indx == frameheader.slicesnum || frame_left_bytes ==0)
    {
        while(!slice.empty())
        {
            slice.pop_back();
        }

        input.read(reinterpret_cast<char *>(&frameheader), sizeof(frameheader));

        swapUint32(frameheader.size); //only include silce data size
        swapUint32(frameheader.timestamp);
        swapUint16(frameheader.sequencenum);//the same as framecnt
        swapUint16(frameheader.flags);//not need to set to mve
        swapUint32(frameheader.lastpacket);
        swapUint32(frameheader.slicesnum);

        frame_left_bytes = frameheader.size;
        slice_read_indx = 0;

        cout<<"read a new RVFrame header sequencenum:"<<frameheader.sequencenum;
        cout<<" frame size:"<<frameheader.size;
        cout<<" timestamp:"<<frameheader.timestamp;
        cout<<" flags:"<<frameheader.flags;
        cout<<" lastpacket:"<<frameheader.lastpacket;
        cout<<" slicesnum:"<<frameheader.slicesnum<<endl;

        for(i =0;i < frameheader.slicesnum;i++)
        {
            RVSlice rvslice;
            memset(&rvslice,0,sizeof(RVSlice));

            input.read(reinterpret_cast<char *>(&rvslice), sizeof(RVSlice));

            swapUint32(rvslice.startcode);
            swapUint32(rvslice.offset);

            if(rvslice.startcode != RVFrame::slicestartcode)
            {
                throw Exception("Incorrect slicestartcode:%x",rvslice.startcode);
            }

            slice.push_back(rvslice);
            cout<<"read slice header indx:"<< i <<" sliceoffset:"<<rvslice.offset<<endl;
        }

    }

    //just send 1 slice
    if(slice_read_indx != frameheader.slicesnum)
    {
        RVSlice cur_slice;
        RVSlice next_slice;

        cur_slice = slice[slice_read_indx];

        if(slice_read_indx != frameheader.slicesnum - 1)
        {
            next_slice = slice[slice_read_indx +1];
            cal_size =next_slice.offset - cur_slice.offset;
        }
        else
        {
            cal_size = frame_left_bytes;
        }

        vector<iovec> iov = buf.getImageSize();
        uint32_t read_bytes = cal_size > iov[0].iov_len ? iov[0].iov_len : cal_size;
        input.read(static_cast<char *>(iov[0].iov_base), read_bytes);
        cout<<"read slice indx:"<< slice_read_indx <<" read_bytes:"<<read_bytes<<endl;

        slice_read_indx ++;
        frame_left_bytes -= read_bytes;

        if (input.gcount() < read_bytes){
            cout<<"read less than need!."<<endl;
        }
        iov[0].iov_len = input.gcount();

        buf.setEndOfSubFrame(1);
        buf.setEndOfFrame(frame_left_bytes == 0 ? 1 : 0);
        buf.setBytesUsed(iov);
        buf.setTimeStamp(frameheader.timestamp);
    }
}

void InputRV::prepare(Buffer &buf)
{
    if(b_header_read ==false)
    {
        prepare_config_data(buf);
    }
    else
    {
        prepare_slice_data(buf);
    }
}


InputAFBC::InputAFBC(istream &input, uint32_t format, size_t width, size_t height) :
    InputFile(input, format, width, height, 1)
{}

void InputAFBC::prepare(Buffer &buf)
{
    if (eof()) {
        return;
    }
    vector<iovec> iov = buf.getImageSize();
    AFBCHeader header;

    input.read(reinterpret_cast<char *>(&header), sizeof(header));

    if (header.version > AFBCHeader::VERSION) //if (header.version != AFBCHeader::VERSION)
    {
        throw Exception("Incorrect AFBC header version. got=%u, exptected <=%d.", header.version, AFBCHeader::VERSION);
    }

    if (header.headerSize < sizeof(header))
    {
        throw Exception("AFBC header size too small. size=%u.", header.headerSize);
    }

    if (6 == header.version && 64 != header.headerSize)
    {
        throw Exception("Incorrect AFBC headerSize, got=%u, exptected 64 in header.version=%d.", header.headerSize, header.version);
    }

    if (header.frameSize > iov[0].iov_len)
    {
        throw Exception("AFBC buffer too small. header_size=%u, frame_size=%u, buffer_size=%zu.",
                        header.headerSize, header.frameSize, iov[0].iov_len);
    }

    size_t skip = header.headerSize - sizeof(header);
    if (skip > 0)
    {
        input.ignore(skip);
    }

    input.read(reinterpret_cast<char *>(iov[0].iov_base), header.frameSize);
    if (input.gcount() != header.frameSize)
    {
        throw Exception("Too few AFBC bytes read. expected=%u, got=%zu.",
                        header.frameSize, input.gcount());
    }

    iov[0].iov_len = header.frameSize;

    buf.setBytesUsed(iov);

    bool tiled = header.param & AFBCHeader::PARAM_TILED_BODY;
    buf.setTiled(tiled);

    bool superblock = 2 < header.subsampling;
    buf.setSuperblock(superblock);
}

bool InputAFBC::eof()
{
    return input.peek() == EOF;
}


InputFileFrame::InputFileFrame(istream &input, uint32_t format, size_t width, size_t height,
                               size_t strideAlign, size_t __stride[]) :
    InputFile(input, format, width, height, strideAlign),
    nplanes(0)
{
    memcpy(stride, __stride, sizeof(size_t) * 3);
    framesize = Codec::getSize(format, width, height, strideAlign, nplanes, stride, size, heights);
}

bool InputFileFrame::eof()
{
    return input.peek() == EOF || iseof;
}

void InputFileFrame::prepare(Buffer &buf)
{
    if (chr_list){
        if (0 == prepared_frames) {
            chr_cur = chr_list->begin();
        }
        if (chr_list->end() != chr_cur && chr_cur->pic_index == prepared_frames) {
            buf.setChrCfg(*chr_cur);
            buf.setChrflag(true);
            chr_cur++;
        } else {
            buf.setChrflag(false);
        }
    }
    if (gop_list && gop_list->size()){
        struct v4l2_gop_config cfg = gop_list->front();

        if (cfg.gop_pic == prepared_frames) {
            buf.setGopResetCfg(cfg);
            buf.setGopResetflag(true);
            gop_list->pop();
        } else {
            buf.setGopResetflag(false);
        }
    } else {
            buf.setGopResetflag(false);
    }

    if (ltr_list && ltr_list->size()){
        struct v4l2_reset_ltr_peroid_config cfg = ltr_list->front();

        if (cfg.reset_trigger_pic == prepared_frames) {
            buf.setLtrResetCfg(cfg);
            buf.setLtrResetflag(true);
            ltr_list->pop();
        } else {
            buf.setLtrResetflag(false);
        }
    } else {
            buf.setLtrResetflag(false);
    }

     if (enc_stats_list && enc_stats_list->size()){
        struct v4l2_enc_stats_cfg stats_cfg = enc_stats_list->front();

        if (stats_cfg.reset_pic == prepared_frames) {
            buf.setResetStatsMode(stats_cfg.reset_pic, stats_cfg.reset_cfg);
            enc_stats_list->pop();
        } else {
            buf.setResetStatsMode(stats_cfg.reset_pic, 0);
        }
    } else {
            buf.setResetStatsMode(prepared_frames, 0);
    }
    vector<iovec> iov = buf.getImageSize();

    if (nplanes != iov.size())
    {
        throw Exception("Frame format and buffer have different number of planes. format=%zu, buffer=%zu.",
                        nplanes, iov.size());
    }

    for (size_t i = 0; i < nplanes; ++i)
    {
        if (size[i] > iov[i].iov_len)
        {
            throw Exception("Frame plane is larger than buffer plane. plane=%zu, plane_size=%zu, buffer_size=%zu.",
                            i, size[i], iov[0].iov_len);
        }

        input.read(static_cast<char *>(iov[i].iov_base), size[i]);
        if ((unsigned int)input.gcount() < iov[i].iov_len) {
            iseof = true;
        }
        iov[i].iov_len = input.gcount();
    }
    prepared_frames++;
    buf.setBytesUsed(iov);
}

InputFileMiniFrame::InputFileMiniFrame(std :: istream & input, uint32_t format, size_t width, size_t height,
            size_t strideAlign, uint32_t mini_height, size_t __stride[]):
    InputFileFrame(input, format, width, height, strideAlign, __stride),
        miniframe_height(mini_height)
{
    for (size_t i = 0; i < 3; ++i)
    { offset[i] = 0; is_done[i] = true;}
    cnt_of_miniframe = divRoundUp(height, miniframe_height);
    setReadHeight(miniframe_height);
}

void InputFileMiniFrame::prepare(Buffer &buf)
{
    vector<iovec> iov = buf.getImageSize();
    size_t readlen = 0;
    size_t readheight = 0;
    size_t quad = 0;
    v4l2_buffer &v_buf = buf.getBuffer();
    v_buf.reserved2 |= V4L2_BUF_FLAG_MVX_MINIFRAME;//use this to indicate miniframe mode
    if (nplanes != iov.size())
    {
        throw Exception("Frame format and buffer have different number of planes. format=%zu, buffer=%zu.",
                        nplanes, iov.size());
    }
    if (format == V4L2_PIX_FMT_Y0L2 || format == V4L2_PIX_FMT_AQB1) {
        quad = 2;
    } else {
        quad = 1;
    }
    for (size_t i = 0; i < nplanes; ++i)
    {
        readheight = i >= 1 ? miniframe_height / (2 * quad) : miniframe_height / quad;
        if (offset[i] + readheight * quad >= heights[i]) {
            readheight = (heights[i] - offset[i]) / quad;
        }
        readlen = stride[i] * readheight;
        input.seekg(count * framesize + offset[i] * stride[i] / quad);
        if (i == 1) input.seekg(size[i - 1], ios::cur);
        if (i == 2) input.seekg(size[i - 1] + size[i - 2], ios::cur);
        v_buf.m.planes[i].reserved[10] = offset[i] << 16;  //it's better that don't use miniframe mode with epr together.
        offset[i] += (readheight * quad);
        v_buf.m.planes[i].reserved[10] |= offset[i];
        input.read(static_cast<char *>(iov[i].iov_base), readlen);
        cout<<"read bytes in plane:"<<i<<" all bytes:"<<readlen<<" offset:"<<offset[i]<<endl;
        iov[i].iov_len = input.gcount();
        if (offset[i] == heights[i]) {
            is_done[i] = true;
            offset[i] = 0;
        } else {
            is_done[i] = false;
        }
    }
    if (is_done[0] && is_done[1] && is_done[2]) {
        count++;
        cout<<"frame read done:"<<count<<endl;
    }
    buf.setBytesUsed(iov);
}

bool InputFileMiniFrame::eof()
{
    return input.peek() == EOF;
}


InputFileFrameWithROI::InputFileFrameWithROI(std :: istream & input, uint32_t format, size_t width,
                                size_t height, size_t strideAlign, std :: istream & roi, size_t __stride[]) :
        InputFileFrame(input, format, width, height, strideAlign, __stride),
        roi_is(roi)
{
    load_roi_cfg();
    prepared_frames = 0;
    cur = roi_list->begin();
}

InputFileFrameWithROI::~InputFileFrameWithROI()
{
    if (roi_list) {
        delete roi_list;
    }
}

void InputFileFrameWithROI::prepare(Buffer & buf)
{
    if (roi_list->end() != cur && cur->pic_index == prepared_frames) {
        buf.setRoiCfg(*cur);
        buf.setROIflag(true);
        cur++;
    } else {
        buf.setROIflag(false);
    }
    InputFileFrame::prepare(buf);
}

void InputFileFrameWithROI::load_roi_cfg()
{
    int pic_index, qp, num_roi, prio_valid = 0;
    struct v4l2_mvx_roi_regions roi;
    if (!roi_list) {
        roi_list = new v4l2_roi_list_t;
    }
    while (roi_is.getline(cfg_file_line_buf, CFG_FILE_LINE_SIZE)){
        memset(&roi, 0, sizeof(struct v4l2_mvx_roi_regions));
        if (4 == sscanf(cfg_file_line_buf, "pic=%d qp=%d num_roi=%d prio_valid=%d",&pic_index,&qp,&num_roi, &prio_valid)) {
            roi.pic_index = pic_index;
            roi.qp = qp;
            roi.num_roi = num_roi;
            roi.roi_present  = true;
            roi.qp_present = true;
        } else if (4 == sscanf(cfg_file_line_buf, "pic=%d num_roi=%d qp=%d prio_valid=%d",&pic_index,&num_roi,&qp, &prio_valid)) {
            roi.pic_index = pic_index;
            roi.qp = qp;
            roi.num_roi = num_roi;
            roi.roi_present  = true;
            roi.qp_present = true;
        } else if (2 == sscanf(cfg_file_line_buf, "pic=%d num_roi=%d qp=%d prio_valid=%d",&pic_index,&num_roi,&qp, &prio_valid)) {
            roi.pic_index = pic_index;
            roi.qp = 0;
            roi.num_roi = num_roi;
            roi.roi_present  = true;
            roi.qp_present = false;
        } else if (2 == sscanf(cfg_file_line_buf, "pic=%d qp=%d num_roi=%d prio_valid=%d",&pic_index,&qp,&num_roi, &prio_valid)) {
            roi.pic_index = pic_index;
            roi.qp = qp;
            roi.num_roi = 0;
            roi.roi_present  = false;
            roi.qp_present = true;
        } else {
            cout <<"parse ROI config file ERROR!"<<endl;
        }
        if (roi.num_roi > V4L2_MVX_MAX_FRAME_REGIONS) {
            cout <<"invalid n_regions value:"<<roi.num_roi<<endl;
        }
        if (roi.num_roi > 0){
            char *sub_buf1 = cfg_file_line_buf;
            char *sub_buf2;
            int match = 0;
            for (int i = 0; i < roi.num_roi; i++) {
                sub_buf2 = strstr(sub_buf1," roi=");
                match = sscanf(sub_buf2, " roi={%hu,%hu,%hu,%hu,%hd,%hu,%hu}",
                    &roi.roi[i].mbx_left, &roi.roi[i].mbx_right,
                    &roi.roi[i].mby_top, &roi.roi[i].mby_bottom,
                    &roi.roi[i].qp_delta, &roi.roi[i].prio, &roi.roi[i].force_intra);
                if (match != 7){
                    cout<<"Error while parsing the ROI regions:"<<match<<endl;
                }
                if (!prio_valid) {
                    roi.roi[i].prio = 0;
                }
                cout<<roi.roi[i].mbx_left<<","<<
                roi.roi[i].mbx_right<<","<<
                roi.roi[i].mby_top<<","<<
                roi.roi[i].mby_bottom<<","<<
                roi.roi[i].qp_delta<<","<<
                roi.roi[i].prio<<endl;
                sub_buf1 = sub_buf2 + 5;
            }
        }
        roi_list->push_back(roi);
    }
}

InputFileFrameWithEPR::InputFileFrameWithEPR(std :: istream & input, uint32_t format, size_t width,
                                size_t height, size_t strideAlign, std :: istream & epr, uint32_t oformat, size_t __stride[]) :
        InputFileFrame(input, format, width, height, strideAlign, __stride),
        epr_is(epr),
        outformat(oformat)
{
    prepared_frames = 0;
    load_epr_cfg();
    cur = epr_list->begin();
}

InputFileFrameWithEPR::~InputFileFrameWithEPR(){
    if (epr_list) {
        delete epr_list;
    }
}

void InputFileFrameWithEPR::erp_adjust_bpr_to_64_64(
                                    struct v4l2_buffer_general_rows_uncomp_body* uncomp_body,
                                    int qp_delta,
                                    uint32_t bpr_base_idx,
                                    uint32_t row_off,
                                    uint8_t force,
                                    uint8_t quad_skip,
                                    int local_base)
{
    if (bpr_base_idx < local_base + row_off) {
        uncomp_body->bpr[bpr_base_idx].qp_delta = (uint16_t)(SET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_LEFT_16X16, qp_delta)
                                                            | SET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_RIGHT_16X16, qp_delta)
                                                            | SET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_LEFT_16X16, qp_delta)
                                                            | SET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_RIGHT_16X16, qp_delta)
                                                            | SET(V4L2_BLOCK_PARAM_RECORD_QP_FORCE_FIELD, force)
                                                            | SET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, quad_skip));
    }
    if (bpr_base_idx + 1 < local_base + row_off)
        uncomp_body->bpr[bpr_base_idx + 1] = uncomp_body->bpr[bpr_base_idx];
    if (bpr_base_idx + row_off < local_base + 2 * row_off)
        uncomp_body->bpr[bpr_base_idx + row_off] = uncomp_body->bpr[bpr_base_idx];
    if (bpr_base_idx + row_off + 1 < local_base + 2 * row_off)
        uncomp_body->bpr[bpr_base_idx + row_off + 1] = uncomp_body->bpr[bpr_base_idx];
    //uncomp_body->bpr[bpr_base_idx].force = force;
    //uncomp_body->bpr[bpr_base_idx + 1] = uncomp_body->bpr[bpr_base_idx];
    //uncomp_body->bpr[bpr_base_idx + 1].force = force;
    //uncomp_body->bpr[bpr_base_idx + row_off] = uncomp_body->bpr[bpr_base_idx];
    //uncomp_body->bpr[bpr_base_idx + row_off].force = force;
    //uncomp_body->bpr[bpr_base_idx + row_off + 1] = uncomp_body->bpr[bpr_base_idx];
    //uncomp_body->bpr[bpr_base_idx + row_off + 1].force = force;
}

void InputFileFrameWithEPR::prepare(Buffer & buf)
{
    /*if (prepared_frames % 2 == 0) {
        prepareEPR(buf);
    } else {
        InputFileFrame::prepare(buf);
    }
    prepared_frames++;*/
    if (epr_list->end() != cur && cur->pic_index == prepared_frames) {
        prepareEPR(buf);
        cur++;
    } else {
        InputFileFrame::prepare(buf);
    }
}

void InputFileFrameWithEPR::prepareEPR(Buffer & buf)
{
    vector<iovec> iov = buf.getImageSize();
    v4l2_epr_list_t::iterator iter= cur;
    v4l2_epr_list_t::iterator end= epr_list->end();
    /*for (; iter != end; iter++) {
        if (iter->pic_index == prepared_frames / 2) {
            break;
        }
    }*/
    if (cur != end) {
        if (iter->qp_present) {
            printf("set epr qp:%d, iframe_enabled:%d\n", iter->qp.qp, iter->qp.epr_iframe_enable);
            buf.setQPofEPR(iter->qp);
        }
        if (iter->block_configs_present) {
            struct v4l2_buffer_general_block_configs block_configs;
            struct v4l2_core_buffer_header_general buffer_configs;
            unsigned int blk_cfg_size = 0;
            if (iter->block_configs.blk_cfg_type == V4L2_BLOCK_CONFIGS_TYPE_ROW_UNCOMP) {
                blk_cfg_size += sizeof(block_configs.blk_cfgs.rows_uncomp);

                int max_cols = (getWidth()+31) >> 5;
                int max_rows = (getHeight()+31) >> 5;
                if (getRotation() % 360 == 90 || getRotation() % 360 == 270) {
                    swap(max_cols,max_rows);
                }
                block_configs.blk_cfgs.rows_uncomp.n_cols_minus1 = max_cols - 1;
                block_configs.blk_cfgs.rows_uncomp.n_rows_minus1 = max_rows - 1;
                size_t bpru_body_size;
                if (outformat == V4L2_PIX_FMT_HEVC) {
                    if (getRotation() % 360 == 90 || getRotation() % 360 == 270)
                    {
                        max_rows = (getWidth() + 127) >> 7;
                    }
                    else
                    {
                        max_rows = (getHeight() + 127) >> 7;
                    }
                    max_rows *= 4;
                    bpru_body_size = max_rows * max_cols * sizeof(v4l2_buffer_general_rows_uncomp_body);
                } else {
                    bpru_body_size = max_rows * max_cols * sizeof(v4l2_buffer_general_rows_uncomp_body);
                }
                struct v4l2_buffer_general_rows_uncomp_body *uncomp_body =
                        static_cast<struct v4l2_buffer_general_rows_uncomp_body*>(iov[0].iov_base);
                iov[0].iov_len = bpru_body_size;
                cout<<"plane 0 size:"<<bpru_body_size<<endl;
                int n_cols = iter->block_configs.blk_cfgs.rows_uncomp.n_cols_minus1 + 1;
                int n_rows = iter->block_configs.blk_cfgs.rows_uncomp.n_rows_minus1 + 1;
                for (int j = 0; j < n_rows; j++)
                {
                    for (int i = 0; i < n_cols; i++)
                    {
                        int idx = (j * n_cols) + i;
                        unsigned int quad_qp_delta = iter->bc_row_body.uncomp->bpr[idx].qp_delta;
                        printf("   bpr[%u]={%d,%d,%d,%d,0x%x,0x%x,0x%x}\n", idx,
                            (int)SGET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_LEFT_16X16,  quad_qp_delta),
                            (int)SGET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_RIGHT_16X16, quad_qp_delta),
                            (int)SGET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_LEFT_16X16,  quad_qp_delta),
                            (int)SGET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_RIGHT_16X16, quad_qp_delta),
                            (uint8_t)GET(V4L2_BLOCK_PARAM_RECORD_QP_FORCE_FIELD, quad_qp_delta),
                            (uint8_t)GET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, quad_qp_delta),
                            (uint8_t)GET(V4L2_BLOCK_PARAM_RECORD_QP_ABSOLUTE, quad_qp_delta));
                    }
               }
                //qp-set quad granularity is 32*32, the minimum qp-set granularity
                //is 16*16. These keep the same with H264. Pls not the FS flag in
                //ME QUADCFG register.
                if (true) {
                    for (int row = 0; row < n_rows; row++) {
                        memcpy(&uncomp_body->bpr[row * max_cols],
                               &iter->bc_row_body.uncomp->bpr[row * n_cols],
                               n_cols * sizeof(uncomp_body->bpr[0]));
                        for (int col = n_cols; col < max_cols; col++) {
                            memcpy(&uncomp_body->bpr[(row * max_cols) + col],
                                   &uncomp_body->bpr[(row * max_cols) + n_cols - 1],
                                   sizeof(uncomp_body->bpr[0]));
                            // bpr quad skip flag cant not inherit
                            uncomp_body->bpr[(row * max_cols) + col].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, 1));
                            uncomp_body->bpr[(row * max_cols) + col].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QP_ABSOLUTE, 1));
                            uncomp_body->bpr[(row * max_cols) + col].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QUAD_FORCE_INTRA, 1));
                        }
                    }
                    for (int row = n_rows; row < max_rows; row++) {
                        memcpy(&uncomp_body->bpr[row * max_cols],
                               &uncomp_body->bpr[(n_rows - 1) * max_cols],
                               max_cols * sizeof(uncomp_body->bpr[0]));
                        // bpr quad skip and absolute QP  flag cant not inherit
                        for(int tmpj = 0; tmpj < max_cols; tmpj++) {
                            uncomp_body->bpr[row * max_cols + tmpj].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, 1));
                            uncomp_body->bpr[(row * max_cols) + tmpj].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QP_ABSOLUTE, 1));
                            uncomp_body->bpr[(row * max_cols) + tmpj].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QUAD_FORCE_INTRA, 1));
                        }

                    }
                } else {
                    for (int row = 0; row < n_rows; row++)
                    {
                        for (int k=0; k<n_cols; k++) {
                            unsigned int quad_qp_delta = iter->bc_row_body.uncomp->bpr[row * n_cols + k].qp_delta;
                            int top_left =  (int)SGET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_LEFT_16X16,  quad_qp_delta);
                            int top_right = (int)SGET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_RIGHT_16X16, quad_qp_delta);
                            int bot_left =  (int)SGET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_LEFT_16X16,  quad_qp_delta);
                            int bot_right = (int)SGET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_RIGHT_16X16, quad_qp_delta);
                            uint8_t force = (uint8_t)GET(V4L2_BLOCK_PARAM_RECORD_QP_FORCE_FIELD, quad_qp_delta);
                            uint8_t quad_skip = (uint8_t)GET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, quad_qp_delta);
                            // top left 64*64
                            erp_adjust_bpr_to_64_64(uncomp_body, top_left, 4*row*max_cols + 4*k, max_cols, force, quad_skip, 4*row*max_cols);
                            // top right 64*64
                            erp_adjust_bpr_to_64_64(uncomp_body, top_right, 4*row*max_cols + 4*k + 2, max_cols, force, quad_skip, 4*row*max_cols);
                            // bottom left 64*64
                            erp_adjust_bpr_to_64_64(uncomp_body, bot_left, 4*row*max_cols + 2*max_cols + 4*k, max_cols, force, quad_skip, 4*row*max_cols + 2*max_cols);
                            // bottom right 64*64
                            erp_adjust_bpr_to_64_64(uncomp_body, bot_right, 4*row*max_cols + 2*max_cols + 4*k + 2, max_cols, force, quad_skip, 4*row*max_cols + 2*max_cols);
                        }
                        // extend the end of a row
                        for (int col = 4*n_cols; col < max_cols; col++)
                        {
                            memcpy(&uncomp_body->bpr[(4*row * max_cols) + col],
                                &uncomp_body->bpr[(4*row * max_cols) + 4*n_cols - 1],
                                sizeof(uncomp_body->bpr[0]));
                            memcpy(&uncomp_body->bpr[(4*row * max_cols + max_cols) + col],
                                &uncomp_body->bpr[(4*row * max_cols + max_cols) + 4*n_cols - 1],
                                sizeof(uncomp_body->bpr[0]));
                            memcpy(&uncomp_body->bpr[(4*row * max_cols + 2*max_cols) + col],
                                &uncomp_body->bpr[(4*row * max_cols + 2*max_cols) + 4*n_cols - 1],
                                sizeof(uncomp_body->bpr[0]));
                            memcpy(&uncomp_body->bpr[(4*row * max_cols + 3*max_cols) + col],
                                &uncomp_body->bpr[(4*row * max_cols + 3*max_cols) + 4*n_cols - 1],
                                sizeof(uncomp_body->bpr[0]));
                            // bpr quad skip flag cant not inherit
                            uncomp_body->bpr[(4*row * max_cols) + col].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, 1));
                            uncomp_body->bpr[(4*row * max_cols + max_cols) + col].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, 1));
                            uncomp_body->bpr[(4*row * max_cols + 2*max_cols) + col].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, 1));
                            uncomp_body->bpr[(4*row * max_cols + 3*max_cols) + col].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, 1));
                        }
                    }
                    for (int row = 4*n_rows; row < max_rows; row++) {
                        // extend the last row
                        memcpy(&uncomp_body->bpr[row * max_cols],
                               &uncomp_body->bpr[(row - 4) * max_cols],
                               max_cols * sizeof(uncomp_body->bpr[0]));
                        // bpr quad skip flag cant not inherit
                        for(int tmpj = 0; tmpj < max_cols; tmpj++) {
                            uncomp_body->bpr[row * max_cols + tmpj].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, 1));
                        }
                    }
                }

                block_configs.blk_cfg_type = iter->block_configs.blk_cfg_type;
                blk_cfg_size += sizeof(block_configs.blk_cfg_type);
                blk_cfg_size += sizeof(block_configs.reserved);

                buffer_configs.type = V4L2_BUFFER_GENERAL_TYPE_BLOCK_CONFIGS;
                buffer_configs.buffer_size = bpru_body_size;
                buffer_configs.config_size = blk_cfg_size;
                memcpy(&buffer_configs.config.config, &block_configs, blk_cfg_size);
                memcpy(&buf.getBuffer().m.planes[0].reserved[0], &buffer_configs, sizeof(buffer_configs.buffer_size) +
                        sizeof(buffer_configs.type) + sizeof(buffer_configs.config_size) + blk_cfg_size);

            }
        }
    }
    for (uint32_t i = 1; i < iov.size(); i++) {
        iov[i].iov_len = 0;
    }
    buf.setBytesUsed(iov);
    buf.setEPRflag();
}

void InputFileFrameWithEPR::read_row_cfg(char * buf, int row, int len, epr_config & config)
{
    char *sub_buf1, *sub_buf2;
    int quad_qp_delta[4];
    uint8_t force_tmp, force_qual_skip, absolute_qp, force_intra, select_qp_delta;
    int quad_min_qp[4]={0};
    struct v4l2_block_param_record *bpr = NULL;
    int n_cols = config.block_configs.blk_cfgs.rows_uncomp.n_cols_minus1 + 1;
    if (row == 0)
    {
        config.block_configs.blk_cfgs.rows_uncomp.n_cols_minus1 = len - 1;
    }
    bpr = &config.bc_row_body.uncomp->bpr[row * n_cols];
    sub_buf1 = buf;

    for (int i = 0; i< len; i++) {
        sub_buf2 = strstr(sub_buf1, " bpr=");
        //assert(sub_buf2);
        if (!sub_buf2) {
           printf("Error : the real bpr number is less than num_bpr for picture =%d (%s))\n",
                        config.pic_index, cfg_file_line_buf);
           return;
        }
        if (13 == sscanf(sub_buf2, " bpr={%d,%d,%d,%d,%hhi,%hhi,%hhi,%hhi,%d,%d,%d,%d,%hhi}",
                        &quad_qp_delta[0],
                        &quad_qp_delta[1],
                        &quad_qp_delta[2],
                        &quad_qp_delta[3],
                        &force_tmp,
                        &force_qual_skip,
                        &absolute_qp,
                        &force_intra,
                        &quad_min_qp[0],
                        &quad_min_qp[1],
                        &quad_min_qp[2],
                        &quad_min_qp[3],
                        &select_qp_delta)) {
         } else if(12 == sscanf(sub_buf2, " bpr={%d,%d,%d,%d,%hhi,%hhi,%hhi,%hhi,%d,%d,%d,%d}",
                        &quad_qp_delta[0],
                        &quad_qp_delta[1],
                        &quad_qp_delta[2],
                        &quad_qp_delta[3],
                        &force_tmp,
                        &force_qual_skip,
                        &absolute_qp,
                        &force_intra,
                        &quad_min_qp[0],
                        &quad_min_qp[1],
                        &quad_min_qp[2],
                        &quad_min_qp[3])) {
        } else if(8 == sscanf(sub_buf2, " bpr={%d,%d,%d,%d,%hhi,%hhi,%hhi,%hhi}",
                        &quad_qp_delta[0],
                        &quad_qp_delta[1],
                        &quad_qp_delta[2],
                        &quad_qp_delta[3],
                        &force_tmp,
                        &force_qual_skip,
                        &absolute_qp,
                        &force_intra)){
        } else {
            printf("Error while parsing the row BPR[%d]\n", i);
        }
        assert(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_LEFT_16X16_SZ == V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_RIGHT_16X16_SZ &&
               V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_LEFT_16X16_SZ == V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_LEFT_16X16_SZ &&
               V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_LEFT_16X16_SZ == V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_RIGHT_16X16_SZ);

        const int qpd_min = -(1 << (V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_LEFT_16X16_SZ - 1));
        const int qpd_max =  (1 << (V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_LEFT_16X16_SZ - 1)) - 1;
        if (quad_qp_delta[0] < qpd_min || quad_qp_delta[0] > qpd_max ||
            quad_qp_delta[1] < qpd_min || quad_qp_delta[1] > qpd_max ||
            quad_qp_delta[2] < qpd_min || quad_qp_delta[2] > qpd_max ||
            quad_qp_delta[3] < qpd_min || quad_qp_delta[3] > qpd_max) {

            printf("Error qp_deltas (%d,%d,%d,%d) are out of range (min=%d, max=%d)\n",
                quad_qp_delta[0], quad_qp_delta[1], quad_qp_delta[2], quad_qp_delta[3],
                qpd_min, qpd_max);

        }
        bpr[i].qp_delta  = SET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_LEFT_16X16,  quad_qp_delta[0]);
        bpr[i].qp_delta |= SET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_RIGHT_16X16, quad_qp_delta[1]);
        bpr[i].qp_delta |= SET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_LEFT_16X16,  quad_qp_delta[2]);
        bpr[i].qp_delta |= SET(V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_RIGHT_16X16, quad_qp_delta[3]);
        bpr[i].qp_delta |= SET(V4L2_BLOCK_PARAM_RECORD_QP_FORCE_FIELD, force_tmp);
        bpr[i].qp_delta |= SET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, force_qual_skip);
        bpr[i].qp_delta |= SET(V4L2_BLOCK_PARAM_RECORD_QP_ABSOLUTE, absolute_qp);
        bpr[i].qp_delta |= SET(V4L2_BLOCK_PARAM_RECORD_QUAD_FORCE_INTRA, force_intra);
        if( quad_min_qp[0] < 0 || quad_min_qp[0] > 51 ||
            quad_min_qp[1] < 0 || quad_min_qp[1] > 51 ||
            quad_min_qp[2] < 0 || quad_min_qp[2] > 51 ||
            quad_min_qp[3] < 0 || quad_min_qp[3] > 51)
        {
            printf("Error min_qp (%d,%d,%d,%d) are out of range (min=0, max=51)\n",
                quad_min_qp[0], quad_min_qp[1], quad_min_qp[2], quad_min_qp[3]);
        }
        bpr[i].min_qp  = SET(V4L2_BLOCK_PARAM_RECORD_MIN_QP_TOP_LEFT_16X16,  quad_min_qp[0]);
        bpr[i].min_qp |= SET(V4L2_BLOCK_PARAM_RECORD_MIN_QP_TOP_RIGHT_16X16, quad_min_qp[1]);
        bpr[i].min_qp |= SET(V4L2_BLOCK_PARAM_RECORD_MIN_QP_BOT_LEFT_16X16,  quad_min_qp[2]);
        bpr[i].min_qp |= SET(V4L2_BLOCK_PARAM_RECORD_MIN_QP_BOT_RIGHT_16X16, quad_min_qp[3]);
        bpr[i].min_qp |= SET(V4L2_BLOCK_PARAM_RECORD_QUAD_SELECT_QP_DELTA, select_qp_delta);

        sub_buf1 = sub_buf2 + 5;
    }
    sub_buf2 = strstr(sub_buf1, " bpr=");
    if (sub_buf2) {
        printf("Error : the real bpr number is larger than num_bpr for picture =%d (%s)\n",
                    config.pic_index, cfg_file_line_buf);
    }
    /*
    for (int i=len; i<n_cols; i++) {
      bpr[i] = bpr[len-1];
    }*/
}

void InputFileFrameWithEPR::read_efp_cfg(char * buf, int num_epr, epr_config * config)
{
    char *sub_buf1;
    int epr_count = 0;
    // qp
    sub_buf1 = strstr(buf, " qp=");
    if (sub_buf1)
    {
        if (1 != sscanf(sub_buf1, " qp={%u}",
                        &config->qp.qp))
        {
            cout<<"Error while parsing qp:"<<sub_buf1<<endl;
        }
        config->qp_present = true;
        epr_count++;
    }
        // iframe enable
    config->qp.epr_iframe_enable = 1; // default is on
    sub_buf1 = strstr(buf, " iframe_enable=");
    if (sub_buf1)
    {
        if (1 != sscanf(sub_buf1, " iframe_enable=%u",
                        &config->qp.epr_iframe_enable))
        {
            printf("Error while parsing qp %s\n", sub_buf1);
        }
    }
    if (epr_count != num_epr)
    {
        cout<<"Error not parsed enough EPRs"<<endl;
    }

}
void InputFileFrameWithEPR::load_epr_cfg()
{
    unsigned int num;
    unsigned int num2;
    int pic_num;
    int last_pic_num = -1; // -1 to handle first time
    int epr_num_row = 0;
    int epr_real_row = 0;
    if (!epr_list) {
        epr_list = new v4l2_epr_list_t;
    }
    const size_t max_bprf_body_size = ((getWidth() +31) >> 5) * ((getHeight() +31) >> 5) *
                                        sizeof(struct v4l2_block_param_record);
    epr_config config(max_bprf_body_size);
    while (epr_is.getline(cfg_file_line_buf, CFG_FILE_LINE_SIZE)) {
        if (1 != sscanf(cfg_file_line_buf, "pic=%d", &pic_num)) {
            cout<<"Line:"<<cfg_file_line_buf<<endl;
            return;
        }
        if (pic_num < 0) {
            cout<<"pic index must not be less than zero!"<<endl;
            return;
        }
        if (last_pic_num != pic_num) {
            // not the first time
            if (last_pic_num != -1)
            {
                epr_list->push_back(config);
            }
            config.clear();
            epr_num_row = 0;
            epr_real_row = 0;
            memset(config.bc_row_body.uncomp->bpr, 0, max_bprf_body_size);
        }
        if (2 == sscanf(cfg_file_line_buf,"pic=%d num_efp=%d", &config.pic_index, &num)) {
            read_efp_cfg(cfg_file_line_buf, num, &config);
        } else if (3 == sscanf(cfg_file_line_buf, "pic=%d num_row=%d type=%i", &config.pic_index, &num, &num2)) {
            if (num == 0) {
                cout<<"num_row must be greater than 0"<<endl;
                return;
            }
            if (config.block_configs_present) {
                cout<<" block_configs_present flag already set to region for picture!"<<endl;
            }
            config.block_configs_present = true;

            if (num2 == V4L2_BLOCK_CONFIGS_TYPE_ROW_UNCOMP)
            {
                config.block_configs.blk_cfg_type = V4L2_BLOCK_CONFIGS_TYPE_ROW_UNCOMP;
                config.block_configs.blk_cfgs.rows_uncomp.n_rows_minus1 = num - 1;
            }
            else
            {
                cout<<"Error - Unsupported block_param_rows_format"<<endl;
                return;
            }
            epr_num_row = num;
        }else if (3 ==
                sscanf(cfg_file_line_buf, "pic=%d row=%u num_bpr=%u", &config.pic_index, &num, &num2)) {
            if (!config.block_configs_present)
            {
                cout<<"Error - block_configs_present is not set for picture" <<endl;
            }
            if (config.block_configs.blk_cfg_type == V4L2_BLOCK_CONFIGS_TYPE_ROW_UNCOMP) {
                static unsigned int last_row = 0;
                if (num <= last_row && num != 0) {
                    cout<<"Error : current row number is less than last_row for picture"<<endl;
                }
                while (num > (last_row + 1)) {
                    //duplicate row
                    int n_cols = config.block_configs.blk_cfgs.rows_uncomp.n_cols_minus1 + 1;
                    memcpy(&config.bc_row_body.uncomp->bpr[(last_row + 1) * n_cols],
                           &config.bc_row_body.uncomp->bpr[last_row * n_cols],
                           n_cols * sizeof(struct v4l2_block_param_record));
                                    // bpr quad skip flag cant not inherit
                    for (int tmpi = 0; tmpi < n_cols; tmpi++) {
                      config.bc_row_body.uncomp->bpr[(last_row + 1) * n_cols + tmpi].qp_delta &= ~((uint32_t)SET(V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP, 1));
                    }
                    config.block_configs.blk_cfgs.rows_uncomp.n_rows_minus1++;
                    last_row++;
                }
                epr_real_row++;
                read_row_cfg(cfg_file_line_buf, num, num2, config);
                last_row = num;
            } else {
            cout<<"Error - block_configs_type is set to an unsupported value"<<endl;
            }
        }else {
            cout<<"parse config file ERROR:"<<cfg_file_line_buf<<endl;
        }
        last_pic_num = pic_num;
    }

    if (epr_real_row != epr_num_row) {
         cout<<"Error: num_row [%d] in epr-cfg-file is not equal to the real-line-number"<<endl;
    }
    epr_list->push_back(config);
}

InputFrame::InputFrame(uint32_t format, size_t width, size_t height,
                       size_t strideAlign, size_t nframes) :
    Input(format, width, height, strideAlign),
    nframes(nframes),
    count(0)
{
    Codec::getSize(format, width, height, strideAlign, nplanes, stride, size, heights);
}

void InputFrame::prepare(Buffer &buf)
{
    vector<iovec> iov = buf.getImageSize();

    unsigned int color = (0xff << 24) | (count * 10);
    unsigned int rgba[4];
    for (int i = 0; i < 3; ++i)
    {
        rgba[i] = (color >> (i * 8)) & 0xff;
    }

    unsigned int yuv[3];
    rgb2yuv(yuv, rgba);

    unsigned int y = yuv[0];
    unsigned int u = yuv[1];
    unsigned int v = yuv[2];

    switch (format)
    {
        case V4L2_PIX_FMT_YUV420M:
        {
            if (iov.size() != 3)
            {
                throw Exception("YUV420 has 3 planes. planes=%zu.", iov.size());
            }

            /* Y plane. */
            if (iov[0].iov_len < size[0])
            {
                throw Exception("YUV420 Y plane has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            memset(iov[0].iov_base, y, size[0]);
            iov[0].iov_len = size[0];

            /* U plane. */
            if (iov[1].iov_len < size[1])
            {
                throw Exception("YUV420 U plane has incorrect size. size=%zu, expected=%zu.",
                                iov[1].iov_len, size[1]);
            }

            memset(iov[1].iov_base, u, size[1]);
            iov[1].iov_len = size[1];

            /* V plane. */
            if (iov[2].iov_len < size[2])
            {
                throw Exception("YUV420 V plane has incorrect size. size=%zu, expected=%zu.",
                                iov[2].iov_len, size[2]);
            }

            memset(iov[2].iov_base, v, size[2]);
            iov[2].iov_len = size[2];

            break;
        }
        case V4L2_PIX_FMT_NV12:
        {
            if (iov.size() != 2)
            {
                throw Exception("YUV420 NV12 has 2 planes. planes=%zu.", iov.size());
            }

            /* Y plane. */
            if (iov[0].iov_len < size[0])
            {
                throw Exception("YUV420 NV12 Y plane has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            memset(iov[0].iov_base, y, size[0]);
            iov[0].iov_len = size[0];

            /* UV plane. */
            if (iov[1].iov_len < size[1])
            {
                throw Exception("YUV420 NV12 UV plane has incorrect size. size=%zu, expected=%zu.",
                                iov[1].iov_len, size[1]);
            }

            uint8_t *p = static_cast<uint8_t *>(iov[1].iov_base);
            for (size_t i = 0; i < iov[1].iov_len; i += 2)
            {
                *p++ = u;
                *p++ = v;
            }

            break;
        }
        case V4L2_PIX_FMT_NV21:
        {
            if (iov.size() != 2)
            {
                throw Exception("YUV420 NV12 has 2 planes. planes=%zu.", iov.size());
            }

            /* Y plane. */
            if (iov[0].iov_len < size[0])
            {
                throw Exception("YUV420 NV12 Y plane has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            memset(iov[0].iov_base, y, size[0]);
            iov[0].iov_len = size[0];

            /* UV plane. */
            if (iov[1].iov_len < size[1])
            {
                throw Exception("YUV420 NV12 UV plane has incorrect size. size=%zu, expected=%zu.",
                                iov[1].iov_len, size[1]);
            }

            uint8_t *p = static_cast<uint8_t *>(iov[1].iov_base);
            for (size_t i = 0; i < iov[1].iov_len; i += 2)
            {
                *p++ = v;
                *p++ = u;
            }

            break;
        }
        case V4L2_PIX_FMT_YUYV:
        {
            if (iov.size() != 1)
            {
                throw Exception("YUYV has 1 plane. planes=%zu.", iov.size());
            }

            if (iov[0].iov_len < size[0])
            {
                throw Exception("YUYV plane 1 has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            uint8_t *p = static_cast<uint8_t *>(iov[0].iov_base);
            for (size_t i = 0; i < size[0]; i += 4)
            {
                *p++ = y;
                *p++ = u;
                *p++ = y;
                *p++ = v;
            }

            iov[0].iov_len = size[0];

            break;
        }
        case V4L2_PIX_FMT_UYVY:
        {
            if (iov.size() != 1)
            {
                throw Exception("UYVY has 1 plane. planes=%zu.", iov.size());
            }

            if (iov[0].iov_len < size[0])
            {
                throw Exception("UYVY plane 1 has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            uint8_t *p = static_cast<uint8_t *>(iov[0].iov_base);
            for (size_t i = 0; i < size[0]; i += 4)
            {
                *p++ = u;
                *p++ = y;
                *p++ = v;
                *p++ = y;
            }

            iov[0].iov_len = size[0];

            break;
        }
        case V4L2_PIX_FMT_Y210:
        {
            uint16_t y16 = y << 8;
            uint16_t u16 = u << 8;
            uint16_t v16 = v << 8;

            if (iov.size() != 1)
            {
                throw Exception("Y210 has 1 plane. planes=%zu.", iov.size());
            }

            if (iov[0].iov_len < size[0])
            {
                throw Exception("Y210 plane 1 has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            uint16_t *p = static_cast<uint16_t *>(iov[0].iov_base);
            for (size_t i = 0; i < (size[0] / sizeof(*p)); i += 4)
            {
                *p++ = y16;
                *p++ = u16;
                *p++ = y16;
                *p++ = v16;
            }

            iov[0].iov_len = size[0];

            break;
        }
        case V4L2_PIX_FMT_P010:
        {
            uint16_t y16 = y << 8;
            uint16_t u16 = u << 8;
            uint16_t v16 = v << 8;

            if (iov.size() != 2)
            {
                throw Exception("P010 has 2 planes. planes=%zu.", iov.size());
            }

            /* Y plane. */
            if (iov[0].iov_len < size[0])
            {
                throw Exception("P010 Y plane has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            uint16_t *p = static_cast<uint16_t *>(iov[0].iov_base);
            for (size_t i = 0; i < (size[0] / sizeof(*p)); ++i)
            {
                *p++ = y16;
            }

            iov[0].iov_len = size[0];

            /* UV plane. */
            if (iov[1].iov_len < size[1])
            {
                throw Exception("P010 UV plane has incorrect size. size=%zu, expected=%zu.",
                                iov[1].iov_len, size[1]);
            }

            p = static_cast<uint16_t *>(iov[1].iov_base);
            for (size_t i = 0; i < (size[1] / sizeof(*p)); i += 2)
            {
                *p++ = u16;
                *p++ = v16;
            }

            iov[1].iov_len = size[1];

            break;
        }
        case V4L2_PIX_FMT_Y0L2:
        {
            if (iov.size() != 1)
            {
                throw Exception("Y0L2 has 1 plane. planes=%zu.", iov.size());
            }

            if (iov[0].iov_len < size[0])
            {
                throw Exception("Y0L2 plane 1 has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            uint64_t a = 3;
            uint64_t y10 = y << 2;
            uint64_t u10 = u << 2;
            uint64_t v10 = v << 2;
            uint64_t w = (a << 62) | (y10 << 52) | (v10 << 42) | (y10 << 32) | (a << 30) | (y10 << 20) | (u10 << 10) | y10;
            uint8_t *p = static_cast<uint8_t *>(iov[0].iov_base);
            for (size_t i = 0; i < iov[0].iov_len; i += 8)
            {
                *p++ = w & 0xff;
                *p++ = (w >> 8) & 0xff;
                *p++ = (w >> 16) & 0xff;
                *p++ = (w >> 24) & 0xff;
                *p++ = (w >> 32) & 0xff;
                *p++ = (w >> 40) & 0xff;
                *p++ = (w >> 48) & 0xff;
                *p++ = (w >> 56) & 0xff;
            }

            iov[0].iov_len = size[0];

            break;
        }
        case DRM_FORMAT_ABGR8888:
        {
            if (iov.size() != 1)
            {
                throw Exception("RGBA has 1 plane. planes=%zu.", iov.size());
            }

            if (iov[0].iov_len < size[0])
            {
                throw Exception("RGBA plane 1 has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            uint8_t *p = static_cast<uint8_t *>(iov[0].iov_base);
            for (size_t i = 0; i < size[0]; i += 4)
            {
                *p++ = rgba[0];
                *p++ = rgba[1];
                *p++ = rgba[2];
                *p++ = rgba[3];
            }

            break;
        }
        case DRM_FORMAT_ARGB8888:
        {
            if (iov.size() != 1)
            {
                throw Exception("BGRA has 1 plane. planes=%zu.", iov.size());
            }

            if (iov[0].iov_len < size[0])
            {
                throw Exception("BGRA plane 1 has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            uint8_t *p = static_cast<uint8_t *>(iov[0].iov_base);
            for (size_t i = 0; i < size[0]; i += 4)
            {
                *p++ = rgba[2];
                *p++ = rgba[1];
                *p++ = rgba[0];
                *p++ = rgba[3];
            }

            break;
        }
        case DRM_FORMAT_BGRA8888:
        {
            if (iov.size() != 1)
            {
                throw Exception("ARGB has 1 plane. planes=%zu.", iov.size());
            }

            if (iov[0].iov_len < size[0])
            {
                throw Exception("ABGR plane 1 has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            uint8_t *p = static_cast<uint8_t *>(iov[0].iov_base);
            for (size_t i = 0; i < size[0]; i += 4)
            {
                *p++ = rgba[3];
                *p++ = rgba[0];
                *p++ = rgba[1];
                *p++ = rgba[2];
            }

            break;
        }
        case DRM_FORMAT_RGBA8888:
        {
            if (iov.size() != 1)
            {
                throw Exception("ABGR has 1 plane. planes=%zu.", iov.size());
            }

            if (iov[0].iov_len < size[0])
            {
                throw Exception("ABGR plane 1 has incorrect size. size=%zu, expected=%zu.",
                                iov[0].iov_len, size[0]);
            }

            uint8_t *p = static_cast<uint8_t *>(iov[0].iov_base);
            for (size_t i = 0; i < size[0]; i += 4)
            {
                *p++ = rgba[3];
                *p++ = rgba[2];
                *p++ = rgba[1];
                *p++ = rgba[0];
            }

            break;
        }
        default:
            throw Exception("Unsupport input frame format.");
    }

    ++count;
    buf.setBytesUsed(iov);
    buf.setTimeStamp(count);
}

bool InputFrame::eof()
{
    return count >= nframes;
}

InputFileOsd::InputFileOsd(const char  * filename, uint32_t format, size_t width, size_t height, size_t strideAlign, size_t __stride[]):
        osd_is(filename)
{
    if (__stride) {
        memcpy(stride, __stride, sizeof(size_t) * 3);
    } else {
        memset(stride, 0, sizeof(size_t) * 3);
    }
    framesize = Codec::getSize(format, width, height, strideAlign, nplanes, stride, size, heights);
}

void InputFileOsd::prepare(Buffer & buf)
{
    vector<iovec> iov = buf.getImageSize();

    for (size_t i = 0; i < nplanes; ++i)
    {
        osd_is.read(static_cast<char *>(iov[i].iov_base), size[i]);
        iov[i].iov_len = osd_is.gcount();
    }
    buf.setBytesUsed(iov);
}

bool InputFileOsd::eof()
{
    return osd_is.peek() == EOF;
}

InputFileFrameOSD::InputFileFrameOSD(istream &input, uint32_t format, size_t width,
                    size_t height, size_t strideAlign, size_t __stride[]) :
    InputFileFrame(input, format, width, height, strideAlign, __stride)
{
    refresh_index = 0;
    osd_file_1 = NULL;
    osd_file_2 = NULL;
}

void InputFileFrameOSD::prepare(Buffer & buf)
{
    bool prepared = false;
    if (osd_list){
        if (0 == prepared_frames && refresh_index == 0) {
            osd_cur = osd_list->begin();
        }
        if (osd_list->end() != osd_cur && osd_cur->pic_index == prepared_frames) {
            buf.setOsdCfg(*osd_cur);
            buf.setOsdCfgEnable(true);
            refresh_index = osd_cur->num_osd;
            osd_cur++;
        } else {
            buf.setOsdCfgEnable(false);
        }
    }
    while (!prepared) {
        switch (refresh_index & 0x3) {
            case 0x1:
            case 0x3:
                if (osd_file_1) {
                    if (!osd_file_1->eof()) {
                        osd_file_1->prepare(buf);
                        prepared = true;
                        buf.setOsdBufferflag(refresh_index);
                    }
                    refresh_index &= ~0x1;
                } else {
                    cerr<<"osd file 1 not exist"<< endl;
                    exit(1);
                }
                break;
            case 0x2:
                if (osd_file_2) {
                    if (!osd_file_2->eof()) {
                        osd_file_2->prepare(buf);
                        prepared = true;
                        buf.setOsdBufferflag(refresh_index);
                    }
                    refresh_index &= ~0x2;
                } else {
                    cerr<<"osd file 2 not exist"<< endl;
                    exit(1);
                }
                break;
            case 0x0:
                InputFileFrame::prepare(buf);
                prepared = true;
                buf.setOsdBufferflag(refresh_index);
                break;
            default:
                cerr<<"Unkonw refresh index:"<< refresh_index << endl;
                exit(1);
        }
    }
}

void InputFrame::rgb2yuv(unsigned int yuv[3], const unsigned int rgb[3])
{
    /* Y = Kr*R + Kg*G + Kb*B */
    /* U = (B-Y)/(1-Kb) = - R * Kr/(1-Kb) - G * Kg/(1-Kb) + B */
    /* V = (R-Y)/(1-Kr) = R - G * Kg/(1-Kr) - B * Kb/(1-Kr) */

    /* BT601 { 0.2990, 0.5870, 0.1140 } */
    /* BT709 { 0.2125, 0.7154, 0.0721 }; */

    float Kr = 0.299;
    float Kg = 0.587;
    float Kb = 0.114;

    float r = rgb[0] / 255.0;
    float g = rgb[1] / 255.0;
    float b = rgb[2] / 255.0;

    float y;
    float u;
    float v;

    /* RGB to YUV. */
    y = Kr * r + Kg * g + Kb * b;
    u = (b - y) / (1 - Kb);
    v = (r - y) / (1 - Kr);

    /* Map YUV to limited color space. */
    yuv[0] = y * 219 + 16;
    yuv[1] = u * 112 + 128;
    yuv[2] = v * 112 + 128;
}

Output::Output(uint32_t format) :
    IO(format),
    timestamp(0),
    totalSize(0)
{
    dir = 1;
    packed = false;
}

Output::Output(uint32_t format, bool packed) :
    IO(format),
    timestamp(0),
    totalSize(0),
    packed(packed)
{
    dir = 1;
}

Output::~Output()
{
    cout << "Total size " << totalSize << endl;
}

void Output::prepare(Buffer &buf)
{
    buf.clearBytesUsed();
}

void Output::finalize(Buffer &buf)
{
    v4l2_buffer &b = buf.getBuffer();
    if (V4L2_TYPE_IS_MULTIPLANAR(b.type) &&
            ((b.flags & V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT) != V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT ||
            (b.flags & V4L2_BUF_FLAG_MVX_DECODE_ONLY) == V4L2_BUF_FLAG_MVX_DECODE_ONLY)){
        return;
    }
    if (b.timestamp.tv_usec != 0
        && b.timestamp.tv_usec != timestamp
        && b.timestamp.tv_usec != (timestamp + 1)
        && b.timestamp.tv_usec != (timestamp + 100))
    {
        cerr << "Incorrect timestamp. got=" << b.timestamp.tv_usec << ", expected=" << timestamp << endl;
    }

    if (!V4L2_TYPE_IS_MULTIPLANAR(b.type)) {
        uint8_t frame_type = (b.reserved2 & 0xFF000000) >> 24;
        uint8_t src_transform  = (b.reserved2 & 0xFF0000) >> 16;
        uint16_t remaining_bytes = b.reserved2;
        switch(frame_type){
            case 0:
                printf("I frame.\n");
            break;
            case 1:
                printf("P frame.\n");
            break;
            case 2:
                printf("B frame.\n");
            break;
            case 3:
                printf("LOWER B frame.\n");
            break;
            case 4:
                printf("P key frame.\n");
            break;
            case 5:
                printf("No ref P frame.\n");
            break;
            case 6:
                printf("GDR frame.\n");
            break;
            default:
                printf("unrecognized frame type.\n");
            break;
        }
        switch(src_transform){
            case 0:
                printf("none src transform.\n");
            break;
            case 1:
                printf("rotate 90  degrees.\n");
            break;
            case 2:
                printf("rotate 180  degrees.\n");
            break;
            case 3:
                printf("rotate 270  degrees.\n");
            break;
            case 4:
                printf("vertical flip (no rotation).\n");
            break;
            case 5:
                printf("rotate 90  degrees and vertical flip.\n");
            break;
            case 6:
                printf("rotate 180  degrees and vertical flip.\n");
            break;
            case 7:
                printf("rotate 270  degrees and vertical flip.\n");
            break;
            default:
                printf("unrecognized src transform.\n");
            break;
        }
        printf("remaining bytes of this buffer:%d\n", remaining_bytes);
    }
    timestamp = b.timestamp.tv_usec;

    vector<iovec> iov;
    vector<char> img_buf;
    size_t y_width, y_height, frame_size, nplanes = 0;
    size_t src_stride[3] = {0}, dst_stride[3] = {0}, dst_heights[3] = {0}, dst_size[3] = {0};
    if (packed) {
        if (V4L2_TYPE_IS_MULTIPLANAR(b.type)) {
            y_width = b.m.planes[0].reserved[1] >> 16;
            y_height = b.m.planes[0].reserved[1] & 0xFFFF;

            for (uint32_t i = 0; i < b.length; i++) {
                src_stride[i] = b.m.planes[i].reserved[0];
            }
            frame_size = Codec::getSize(getFormat(), y_width, y_height, 1, nplanes, dst_stride, dst_size, dst_heights);
            img_buf.resize(frame_size);
            buf.getPackedBuffer(static_cast<char*>(&img_buf[0]), nplanes, dst_heights, dst_stride, src_stride);
            if (getFormat() == V4L2_PIX_FMT_P010) {
                iov = buf._convert10Bit((unsigned short*)(&img_buf[0]), (unsigned short*)(&img_buf[dst_size[0]]),
                                dst_size[0], dst_size[1]);
            } else {
                iovec vec = { .iov_base = static_cast<char *>(&img_buf[0]), .iov_len = frame_size };
                iov.push_back(vec);
            }
        }
    } else {
        iov = buf.getBytesUsed();
    }
    const struct v4l2_mvx_seamless_target  &seamless = buf.getSeamless();
    for (size_t i = 0; i < iov.size(); ++i)
    {
        if (V4L2_TYPE_IS_MULTIPLANAR(b.type)&& seamless.seamless_mode !=0)
        {
            write(iov[i].iov_base, b.m.planes[i].length);
            memset(iov[i].iov_base,0,b.m.planes[i].length);
            totalSize += b.m.planes[i].length;
            //cerr << b.m.planes[i].length <<endl;
        }
        else
        {
            write(iov[i].iov_base, iov[i].iov_len);
            totalSize += iov[i].iov_len;
        }

    }

}

OutputFile::OutputFile(ostream &output, uint32_t format) :
    Output(format),
    output(output)
{}

OutputFile::OutputFile(ostream &output, uint32_t format, bool packed) :
    Output(format, packed),
    output(output)
{}

void OutputFile::write(void *ptr, size_t nbytes)
{
    output.write(static_cast<char *>(ptr), nbytes);
    output.flush();
}

OutputIVF::OutputIVF(ofstream &output,
                     uint32_t format,
                     uint16_t width,
                     uint16_t height) :
    OutputFile(output, format)
{
    IVFHeader header(format, width, height);
    write(reinterpret_cast<char *>(&header), sizeof(header));
}

void OutputIVF::finalize(Buffer &buf)
{
    vector<iovec> iov = buf.getBytesUsed();
    v4l2_buffer &b = buf.getBuffer();
    if (V4L2_TYPE_IS_MULTIPLANAR(b.type) &&
            (b.flags & V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT) != V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT){
        return;
    }

    for (size_t i = 0; i < iov.size(); ++i)
    {
        size_t n = temp.size();
        temp.resize(temp.size() + iov[i].iov_len);

        char *p = static_cast<char *>(iov[i].iov_base);
        copy(p, p + iov[i].iov_len, &temp[n]);
    }

    if (b.flags & V4L2_BUF_FLAG_KEYFRAME)
    {
        IVFFrame frame(temp.size(), b.timestamp.tv_usec);
        write(&frame, sizeof(frame));

        write(&temp[0], temp.size());
        temp.clear();
    }
}

OutputAFBC::OutputAFBC(std::ofstream &output, uint32_t format, bool tiled) :
    OutputFile(output, format),
    tiled(tiled)
{}

void OutputAFBC::prepare(Buffer &buf)
{
    buf.clearBytesUsed();
    buf.setTiled(tiled);
}

void OutputAFBC::finalize(Buffer &buf)
{
    vector<iovec> iov = buf.getBytesUsed();
    v4l2_buffer &b = buf.getBuffer();
    if (V4L2_TYPE_IS_MULTIPLANAR(b.type) &&
            (b.flags & V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT) != V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT){
        return;
    }

    if (iov[0].iov_len > 0)
    {
        AFBCHeader header(buf.getFormat(), iov[0].iov_len, buf.getCrop(), tiled);
        write(&header, sizeof(header));
        OutputFile::finalize(buf);
    }
}

OutputAFBCInterlaced::OutputAFBCInterlaced(std::ofstream &output, uint32_t format, bool tiled) :
    OutputAFBC(output, format, tiled)
{}

void OutputAFBCInterlaced::finalize(Buffer &buf)
{
    vector<iovec> iov = buf.getBytesUsed();
    v4l2_buffer &b = buf.getBuffer();
    if (V4L2_TYPE_IS_MULTIPLANAR(b.type) &&
            (b.flags & V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT) == 0){
        return;
    }

    if (iov[0].iov_len == 0)
    {
        return;
    }

    size_t top_len = roundUp(iov[0].iov_len / 2, 32);
    AFBCHeader top_header(buf.getFormat(), top_len, buf.getCrop(), tiled, AFBCHeader::FIELD_TOP);

    write(&top_header, sizeof(top_header));
    write(iov[0].iov_base, top_len);

    size_t bot_len = iov[0].iov_len - top_len;
    AFBCHeader bot_header(buf.getFormat(), bot_len, buf.getCrop(), tiled, AFBCHeader::FIELD_BOTTOM);

    write(&bot_header, sizeof(bot_header));
    write(static_cast<char *>(iov[0].iov_base) + top_len, bot_len);
}

OutputFileWithMD5::OutputFileWithMD5(std::ofstream &output, uint32_t format, std::ofstream &output_md5, bool packed) :
    OutputFile(output, format, packed),
    output_md5(output_md5)
{}

void OutputFileWithMD5::finalize(Buffer &buf)
{
#define HASH_DIGEST_LENGTH 16
#define STR_HASH_SIZE (HASH_DIGEST_LENGTH * 2 + 2)
    MD5_CTX ctx;
    char str_hash[STR_HASH_SIZE];
    uint32_t data[HASH_DIGEST_LENGTH/sizeof(uint32_t)];
    uint8_t *hash = (uint8_t *)data;
    static char const slookup[] = {
                '0', '1', '2', '3',
                '4', '5', '6', '7',
                '8', '9', 'a', 'b',
                'c', 'd', 'e', 'f'
    };
    int i;
    vector<iovec> iov;
    vector<char> img_buf;
    size_t y_width, y_height, frame_size, nplanes = 0;
    size_t src_stride[3] = {0}, dst_stride[3] = {0}, dst_heights[3] = {0}, dst_size[3] = {0};
    v4l2_buffer &b = buf.getBuffer();
    if (V4L2_TYPE_IS_MULTIPLANAR(b.type) &&
            (b.flags & V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT) != V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT){
        return;
    }
    if (packed) {
        if (V4L2_TYPE_IS_MULTIPLANAR(b.type)) {
            y_width = (b.m.planes[0].reserved[1] & 0xFFFF0000) >> 16;
            y_height = b.m.planes[0].reserved[1] & 0xFFFF;

            for (uint32_t i = 0; i < b.length; i++) {
                src_stride[i] = b.m.planes[i].reserved[0];
            }
            frame_size = Codec::getSize(getFormat(), y_width, y_height, 1, nplanes, dst_stride, dst_size, dst_heights);
            img_buf.resize(frame_size);
            buf.getPackedBuffer(static_cast<char*>(&img_buf[0]), nplanes, dst_heights, dst_stride, src_stride);
            if (getFormat() == V4L2_PIX_FMT_P010) {
                iov = buf._convert10Bit((unsigned short*)(&img_buf[0]), (unsigned short*)(&img_buf[dst_size[0]]),
                                dst_size[0], dst_size[1]);
            } else {
                iovec vec = { .iov_base = static_cast<char *>(&img_buf[0]), .iov_len = frame_size };
                iov.push_back(vec);
            }
        }
    } else {
        if(getFormat() == V4L2_PIX_FMT_P010){
            iov = buf.convert10Bit();
        } else {
            iov = buf.getBytesUsed();
        }
    }
    for (size_t i = 0; i < iov.size(); ++i)
    {
        write(iov[i].iov_base, iov[i].iov_len);
        totalSize += iov[i].iov_len;
    }
    //OutputFile::finalize(buf);

    if(iov[0].iov_len){
        MD5_Init(&ctx);
        for(i = 0; (size_t)i < iov.size(); ++i){
            MD5_Update(&ctx, iov[i].iov_base, iov[i].iov_len);
        }
        MD5_Finalize(&ctx);
        MD5_GetHash(&ctx, hash);

        for(i = 0; i < HASH_DIGEST_LENGTH; ++i){
            str_hash[i << 1] = slookup[hash[i] >>  4];
            str_hash[(i << 1) + 1] = slookup[hash[i] & 0xF];
        }
        str_hash[sizeof(str_hash) - 2] = '\r';
        str_hash[sizeof(str_hash) - 1] = '\n';

        output_md5.write(static_cast<char *>(str_hash), STR_HASH_SIZE);
        output_md5.flush();
#if 0
        printf("hash: %02x,%02x,%02x,%02x,"
                    "%02x,%02x,%02x,%02x,"
                    "%02x,%02x,%02x,%02x,"
                    "%02x,%02x,%02x,%02x.\n",
                    hash[0], hash[1], hash[2], hash[3],
                    hash[4], hash[5], hash[6], hash[7],
                    hash[8], hash[9], hash[10], hash[11],
                    hash[12], hash[13], hash[14], hash[15]);
#endif
    }
}

OutputFileWithADStats::OutputFileWithADStats(std::ofstream &output, uint32_t format,
    const std::string& ad_stats_filename,const std::string&thumbnail_filename, bool packed) :
    OutputFile(output, format, packed)
{
    char str_ad_stats[256];

    output_ad_stats.open(ad_stats_filename.c_str(), std::ios_base::trunc );
    printf( "Output: AD statistics file, %s\n", ad_stats_filename.c_str() );

    memset(str_ad_stats,0,sizeof(str_ad_stats));
    sprintf(str_ad_stats,"%s","Assertive Display Frame Average Statistics (Frame number, Y 12-bit, Cb 10-bit, Cr 10-bit)\n");
    output_ad_stats.write(str_ad_stats, strlen(str_ad_stats));

    if(thumbnail_filename != std::string("dev/null"))
    {
        output_thumbnail.open( thumbnail_filename.c_str(), std::ios_base::binary );
        printf( "Output: thumbnail file, %s\n", thumbnail_filename.c_str() );
    }
}

OutputFileWithADStats::~OutputFileWithADStats()
{
    if (output_ad_stats.is_open())
    {
        output_ad_stats.close();
    }
    if (output_thumbnail.is_open())
    {
        output_thumbnail.close();
    }
}

void OutputFileWithADStats::finalize(Buffer &buf)
{
    char str_ad_stats[256];
    unsigned int thumbnail_size=0;
    struct v4l2_core_buffer_header_general buffer_configs;
    v4l2_buffer &b = buf.getBuffer();
    vector<iovec> iov;

    if (V4L2_TYPE_IS_MULTIPLANAR(b.type) &&
        ((b.flags & V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT) != V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT ||
        (b.flags & V4L2_BUF_FLAG_MVX_DECODE_ONLY) == V4L2_BUF_FLAG_MVX_DECODE_ONLY)){
        return;
    }

    if ((b.reserved2 & V4L2_BUF_FLAG_MVX_AD_STATS) != V4L2_BUF_FLAG_MVX_AD_STATS)
    {
        OutputFile::finalize(buf);
        return;
    }

    buffer_configs.buffer_size = b.m.planes[0].reserved[2];
    buffer_configs.type        = (b.m.planes[0].reserved[3] & 0xFFFF0000) >> 16;
    buffer_configs.config_size = b.m.planes[0].reserved[3] & 0XFFFF;

    buffer_configs.config.ad_stats.frame_averages  = b.m.planes[0].reserved[4];
    buffer_configs.config.ad_stats.thumbnail_width = (b.m.planes[0].reserved[5] & 0xFFFF0000)  >> 16;
    buffer_configs.config.ad_stats.thumbnail_height= b.m.planes[0].reserved[5]  & 0XFFFF;
    buffer_configs.config.ad_stats.ad_stats_flags  = b.m.planes[0].reserved[6];

    if(buffer_configs.config.ad_stats.ad_stats_flags == 1)
    {
        thumbnail_size = 2*buffer_configs.config.ad_stats.thumbnail_width*buffer_configs.config.ad_stats.thumbnail_height;
    }
    else
    {
        thumbnail_size = buffer_configs.config.ad_stats.thumbnail_width*buffer_configs.config.ad_stats.thumbnail_height;
    }

    printf("<-: ad_stats info: frame count:%d, frame_averages:%u, thumbnail_width:%u, thumbnail_height:%u, ad_stats_flags:%u thumbnail_size:%u\n",
    buf.getFrameProcessCount(),
    buffer_configs.config.ad_stats.frame_averages,
    buffer_configs.config.ad_stats.thumbnail_width,
    buffer_configs.config.ad_stats.thumbnail_height,
    buffer_configs.config.ad_stats.ad_stats_flags,
    thumbnail_size);

    memset(str_ad_stats,0,sizeof(str_ad_stats));
    sprintf(str_ad_stats,"%d, %d, %d, %d\n",
    buf.getFrameProcessCount(),
    buffer_configs.config.ad_stats.frame_averages  & 0xfff,
    (buffer_configs.config.ad_stats.frame_averages & 0x003ff000) >> 12,
    (buffer_configs.config.ad_stats.frame_averages & 0xffc00000) >>22);

    output_ad_stats.write(str_ad_stats, strlen(str_ad_stats));

    if (output_thumbnail.is_open())
    {
        iov = buf.getImageSize();
        output_thumbnail.write(static_cast<char *>(iov[0].iov_base) + iov[0].iov_len - MVX_AD_STATS_THUMBNAIL_SIZE,thumbnail_size);
        memset(static_cast<char *>(iov[0].iov_base) + iov[0].iov_len - MVX_AD_STATS_THUMBNAIL_SIZE,0,MVX_AD_STATS_THUMBNAIL_SIZE);
    }

    OutputFile::finalize(buf);

}

OutputFileFrameStats::OutputFileFrameStats(std::ostream &output, uint32_t format,
            uint32_t stats_mode, const std::string& filename, uint32_t width, uint32_t height):
    OutputFile(output, format)
{
    queued_buffer = 0;
    //open all enc stats file by default for convenience
    std::string file_name_mms = std::string( filename) + ".mms";
    file_mms.open( file_name_mms.c_str(), std::ios_base::binary );
    printf( "Output: statistics(SATD, MSE, MAD) file, %s\n", file_name_mms.c_str() );

    std::string file_name_bitcost = std::string( filename) + ".bc";
    file_bitcost.open( file_name_bitcost.c_str(), std::ios_base::binary );
    printf( "Output: statistics(bitcost) file, %s\n", file_name_bitcost.c_str() );

    std::string file_name_qp = std::string( filename) + ".qp";
    file_qp.open( file_name_qp.c_str(), std::ios_base::binary );
    printf( "Output: statistics(bitcost) file, %s\n", file_name_qp.c_str() );

}

void OutputFileFrameStats::finalize(Buffer & buf)
{
    if ((buf.getBuffer().flags & V4L2_BUF_FLAG_MVX_BUFFER_ENC_STATS) != V4L2_BUF_FLAG_MVX_BUFFER_ENC_STATS)
    {
        OutputFile::finalize(buf);
        return;
    }

    vector<iovec> iov = buf.getBytesUsed();
    struct v4l2_buffer_param_enc_stats *stats =static_cast<struct v4l2_buffer_param_enc_stats *>(iov[0].iov_base);
    uint32_t offset = sizeof(struct v4l2_buffer_param_enc_stats);
    if (stats->flags & V4L2_BUFFER_ENC_STATS_FLAG_DROP)
    {
        return;
    }

    if ( stats->flags & V4L2_BUFFER_ENC_STATS_FLAG_MMS )
    {
        file_mms.write(static_cast<char *>(iov[0].iov_base) + offset,
                       stats->mms_buffer_size);
        file_mms << std::flush;
    }
    if ( stats->flags & V4L2_BUFFER_ENC_STATS_FLAG_BITCOST )
    {
        file_bitcost.write(static_cast<char *>(iov[0].iov_base) + offset + stats->mms_buffer_size,
                       stats->bitcost_buffer_size);
        file_bitcost << std::flush;
    }
    if ( stats->flags & V4L2_BUFFER_ENC_STATS_FLAG_QP )
    {
        file_qp.write(static_cast<char *>(iov[0].iov_base) + offset + stats->mms_buffer_size + stats->bitcost_buffer_size,
                       stats->qp_buffer_size);
        file_qp << std::flush;
    }
    printf("stats mb size of this buffer, width:%u, height:%u\n", (stats->pic_index_or_mb_size & 0xFFFF), (stats->pic_index_or_mb_size & 0xFFFF0000)>>16);
}

OutputFileFrameStats::~OutputFileFrameStats()
{
    if (file_mms.is_open())
    {
        file_mms.close();
    }
    if (file_bitcost.is_open())
    {
        file_bitcost.close();
    }
    if (file_qp.is_open())
    {
        file_qp.close();
    }
}

/****************************************************************************
 * Buffer
 ****************************************************************************/

Buffer::Buffer(const v4l2_format &format) :
    format(format)
{
    isRoiCfg = false;
    epr_qp.qp = 0;
    frames_processed=0;
    memset(&seamless,0,sizeof(seamless));
}

Buffer::Buffer(v4l2_buffer &buf, int fd, const v4l2_format &format, enum v4l2_memory memory_type) :
    buf(buf),
    format(format),
    memory_type(memory_type)
{
    memset(ptr, 0, sizeof(ptr));

    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        memcpy(planes, buf.m.planes, sizeof(planes[0]) * buf.length);
        this->buf.m.planes = planes;
    }
    isRoiCfg = false;
    epr_qp.qp = 0;
    frames_processed=0;
    memset(&seamless,0,sizeof(seamless));
    //memoryMap(fd);
}

Buffer::~Buffer()
{
    memoryUnmap();
}

v4l2_buffer &Buffer::getBuffer()
{
    return buf;
}

const v4l2_format &Buffer::getFormat() const
{
    return format;
}

void Buffer::setCrop(const v4l2_crop &crop)
{
    this->crop = crop;
}

const v4l2_crop &Buffer::getCrop() const
{
    return crop;
}

void Buffer::setSeamless(const struct v4l2_mvx_seamless_target &seamless)
{
    this->seamless = seamless;
}

const v4l2_mvx_seamless_target &Buffer::getSeamless()const
{
    return seamless;
}

void Buffer::setFrameProcessCount(int count)
{
    this->frames_processed = count;
}

const int Buffer::getFrameProcessCount()const
{
    return this->frames_processed;
}


vector<iovec> Buffer::getImageSize() const
{
    vector<iovec> iova;

    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        for (unsigned int i = 0; i < buf.length; ++i)
        {
            iovec iov = { .iov_base = ptr[i], .iov_len = buf.m.planes[i].length };
            iova.push_back(iov);
        }
    }
    else
    {
        iovec iov = { .iov_base = ptr[0], .iov_len = buf.length };
        iova.push_back(iov);
    }

    return iova;
}

vector<iovec> Buffer::convert10Bit(){
    return _convert10Bit(static_cast<unsigned short*>(ptr[0]) + buf.m.planes[0].data_offset,
        static_cast<unsigned short *>(ptr[1]) + buf.m.planes[1].data_offset,
        buf.m.planes[0].bytesused - buf.m.planes[0].data_offset,
        buf.m.planes[1].bytesused - buf.m.planes[1].data_offset);
}

vector<iovec> Buffer::_convert10Bit(unsigned short* ptr_y, unsigned short* ptr_uv, size_t size_y, size_t size_uv){
    unsigned int i=0;
    unsigned short *y, *tmp_uv, *u, *v;
    size_t y_size, uv_size;
    vector<iovec> iova;
    //v4l2_plane &y_p = buf.m.planes[0];
    //y = static_cast<unsigned short *>(ptr[0]) + y_p.data_offset;
    //y_size = y_p.bytesused - y_p.data_offset;
    //v4l2_plane &u_p = buf.m.planes[1];
    //u = static_cast<unsigned short *>(ptr[1]) + u_p.data_offset;
    //uv_size = u_p.bytesused - u_p.data_offset;
    y = ptr_y;
    y_size = size_y;
    u = ptr_uv;
    uv_size = size_uv;


    v = u + uv_size/(2*sizeof(short));
    //buf.length = 3;
    iovec iov;

    tmp_uv = static_cast<unsigned short *>(calloc(uv_size / sizeof(short), sizeof(short)));
    if(NULL == tmp_uv){
        memset(y, 0xcc, y_size);
        iov = { .iov_base = y, .iov_len = y_size};
        iova.push_back(iov);
    } else {
        memcpy(tmp_uv, u , uv_size);
        memset(u, 0, uv_size);
        for( i=0; i < y_size / sizeof(short); i++){
            y[i] = y[i] >> 6;
        }
        iov = { .iov_base = y, .iov_len = y_size};
        iova.push_back(iov);
        for(i = 0; i < uv_size / (2*sizeof(short)); i++){
            u[i] = tmp_uv[2*i] >> 6;
            v[i] = tmp_uv[2*i+1] >> 6;
        }
        iov = { .iov_base = u, .iov_len = (uv_size)/2};
        iova.push_back(iov);
        iov = { .iov_base = v, .iov_len = (uv_size)/2};
        iova.push_back(iov);
        free(tmp_uv);
    }
    return iova;
}
vector<iovec> Buffer::getBytesUsed() const
{
    vector<iovec> iova;

    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        for (unsigned int i = 0; i < buf.length; ++i)
        {
            v4l2_plane &p = buf.m.planes[i];

            iovec iov = { .iov_base = static_cast<char *>(ptr[i]) + p.data_offset, .iov_len = p.bytesused - p.data_offset };

            if (p.bytesused < p.data_offset)
            {
                iov.iov_len = 0;
            }

            iova.push_back(iov);
        }
    }
    else
    {
        iovec iov = { .iov_base = ptr[0], .iov_len = buf.bytesused };

        /*
         * Single planar buffers has no support for offset, but for HEVC and VP9
         * encode we must find a way to relay the offset from the code.
         *
         * For MMAP we use the lower 12 bits (assuming 4k page size) to relay
         * the offset.
         *
         * For userptr the actual pointer is updated to point at the first byte
         * of the data.
         *
         * Because there is no offset 'bytesused' does not have to be adjusted
         * similar to multi planar buffers.
         */
        switch (buf.memory)
        {
            case V4L2_MEMORY_MMAP:
                iov.iov_base = static_cast<char *>(iov.iov_base) + (buf.m.offset & ((1 << 12) - 1));
                break;
            case V4L2_MEMORY_USERPTR:
                iov.iov_base = reinterpret_cast<void *>(buf.m.userptr);
                break;
            default:
                break;
        }

        iova.push_back(iov);
    }

    return iova;
}

void Buffer::getPackedBuffer(char* img, size_t nplanes, size_t dst_heights[], size_t dst_stride[], size_t src_stride[])
{
    char *dst = img;
    for (uint32_t i = 0; i < nplanes; i++) {
        char *src = static_cast<char*>(ptr[i]);
        for (uint32_t row = 0; row < dst_heights[i]; row++) {
            memcpy(dst, src, dst_stride[i]);
            dst += dst_stride[i];
            src += src_stride[i];
        }
    }

}

void Buffer::setBytesUsed(vector<iovec> &iov)
{
    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        if (iov.size() > buf.length)
        {
            throw Exception("iovec vector size is larger than V4L2 buffer number of planes. size=%u, planes=%u",
                            iov.size(), buf.length);
        }

        size_t i;
        for (i = 0; i < iov.size(); ++i)
        {
            buf.m.planes[i].bytesused = iov[i].iov_len;
        }

        for (; i < buf.length; ++i)
        {
            buf.m.planes[i].bytesused = 0;
        }
    }
    else
    {
        buf.bytesused = 0;

        for (size_t i = 0; i < iov.size(); ++i)
        {
            buf.bytesused += iov[i].iov_len;

            if (buf.bytesused > buf.length)
            {
                throw Exception("V4L2 buffer size too small. length=%u, buyteused=%u.", buf.length, buf.bytesused);
            }
        }
    }
}

void Buffer::clearBytesUsed()
{
    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        for (size_t i = 0; i < buf.length; ++i)
        {
            buf.m.planes[i].bytesused = 0;
        }
    }
    else
    {
        buf.bytesused = 0;
    }
}

void Buffer::resetVendorFlags()
{
    buf.flags &= ~V4L2_BUF_FLAG_MVX_MASK;
}

void Buffer::setCodecConfig(bool codecConfig)
{
    buf.flags &= ~V4L2_BUF_FLAG_MVX_CODEC_CONFIG;
    buf.flags |= codecConfig ? V4L2_BUF_FLAG_MVX_CODEC_CONFIG : 0;
}

void Buffer::setTimeStamp(unsigned int timeUs)
{
    buf.flags |= V4L2_BUF_FLAG_TIMESTAMP_COPY;
    buf.timestamp.tv_sec = timeUs / 1000000;
    buf.timestamp.tv_usec = timeUs % 1000000;
}

void Buffer::setEndOfFrame(bool eof)
{
    buf.flags &= ~V4L2_BUF_FLAG_KEYFRAME;
    buf.flags |= eof ? V4L2_BUF_FLAG_KEYFRAME : 0;
}

void Buffer::setEndOfSubFrame(bool eosf)
{
    buf.flags &= ~V4L2_BUF_FLAG_END_OF_SUB_FRAME;
    buf.flags |= eosf ? V4L2_BUF_FLAG_END_OF_SUB_FRAME : 0;
}

void Buffer::setRotation(int rotation)
{
    if( rotation%90 !=0) {
        return;
    }

    switch (rotation%360) {
        case 90:
            buf.flags &= ~V4L2_BUF_FRAME_FLAG_ROTATION_MASK;
            buf.flags |= V4L2_BUF_FRAME_FLAG_ROTATION_90;
            break;
        case 180:
            buf.flags &= ~V4L2_BUF_FRAME_FLAG_ROTATION_MASK;
            buf.flags |= V4L2_BUF_FRAME_FLAG_ROTATION_180;
            break;
        case 270:
            buf.flags &= ~V4L2_BUF_FRAME_FLAG_ROTATION_MASK;
            buf.flags |= V4L2_BUF_FRAME_FLAG_ROTATION_270;
            break;
        default:
            break;
    }
    return;
}

void Buffer::setEncRotation(int rotation)
{
    if( rotation%90 !=0) {
        return;
    }

    switch (rotation%360) {
        case 90:
            buf.flags &= ~V4L2_BUF_ENCODE_FLAG_ROTATION_MASK;
            buf.flags |= V4L2_BUF_ENCODE_FLAG_ROTATION_90;
            break;
        case 180:
            buf.flags &= ~V4L2_BUF_ENCODE_FLAG_ROTATION_MASK;
            buf.flags |= V4L2_BUF_ENCODE_FLAG_ROTATION_180;
            break;
        case 270:
            buf.flags &= ~V4L2_BUF_ENCODE_FLAG_ROTATION_MASK;
            buf.flags |= V4L2_BUF_ENCODE_FLAG_ROTATION_270;
            break;
        default:
            break;
    }
    return;
}

void Buffer::setDownScale(int scale)
{
    if (scale == 1) {
        return;
    }
    switch (scale) {
        case 2:
            buf.flags &= ~V4L2_BUF_FRAME_FLAG_SCALING_MASK;
            buf.flags |= V4L2_BUF_FRAME_FLAG_SCALING_2;
            break;
        case 4:
            buf.flags &= ~V4L2_BUF_FRAME_FLAG_SCALING_MASK;
            buf.flags |= V4L2_BUF_FRAME_FLAG_SCALING_4;
            break;
        default:
            printf("didnot support this scale factor :%d",scale);
            break;
    }
    return;
}

void Buffer::setMirror(int mirror)
{
    if (mirror == 0) {
        return;
    } else {
        if (mirror == 1) {
            buf.flags &= ~V4L2_BUF_FRAME_FLAG_MIRROR_MASK;
            buf.flags |= V4L2_BUF_FRAME_FLAG_MIRROR_HORI;
        } else if (mirror == 2) {
            buf.flags &= ~V4L2_BUF_FRAME_FLAG_MIRROR_MASK;
            buf.flags |= V4L2_BUF_FRAME_FLAG_MIRROR_VERT;
        }
    }
    return;
}

void Buffer::setEndOfStream(bool eos)
{
    buf.flags &= ~V4L2_BUF_FLAG_LAST;
    buf.flags |= eos ? V4L2_BUF_FLAG_LAST : 0;
}

void Buffer::set_force_idr_flag(bool idr)
{
    buf.flags &= ~V4L2_BUF_FLAG_KEYFRAME;
    buf.flags |= idr ? V4L2_BUF_FLAG_KEYFRAME : 0;
}

void Buffer::setROIflag(bool roi_valid){
    buf.flags &= ~V4L2_BUF_FLAG_MVX_BUFFER_ROI;
    if (roi_valid) {
        buf.flags |= V4L2_BUF_FLAG_MVX_BUFFER_ROI;
        isRoiCfg = true;
    } else {
        isRoiCfg = false;
    }
}

void Buffer::setChrflag(bool chr_valid){
    buf.flags &= ~V4L2_BUF_FLAG_MVX_BUFFER_CHR;
    if (chr_valid) {
        buf.flags |= V4L2_BUF_FLAG_MVX_BUFFER_CHR;
        isChrCfg = true;
    } else {
        isChrCfg = false;
    }
}

void Buffer::setGopResetflag(bool valid){
    buf.flags &= ~V4L2_BUF_FLAG_MVX_BUFFER_GOP_RESET;
    if (valid) {
        buf.flags |= V4L2_BUF_FLAG_MVX_BUFFER_GOP_RESET;
        isGopCfg = true;
    } else {
        isGopCfg = false;
    }
}

void Buffer::setLtrResetflag(bool valid){
    buf.flags &= ~V4L2_BUF_FLAG_MVX_BUFFER_LTR_RESET;
    if (valid) {
        buf.flags |= V4L2_BUF_FLAG_MVX_BUFFER_LTR_RESET;
        isLtrCfg = true;
    } else {
        isLtrCfg = false;
    }
}

void Buffer::setEPRflag(){
    buf.flags &= ~V4L2_BUF_FLAG_MVX_BUFFER_EPR;
    buf.flags |= V4L2_BUF_FLAG_MVX_BUFFER_EPR;
}

void Buffer::setOsdBufferflag(uint32_t index)
{
    buf.reserved2 &= ~V4L2_BUF_FLAG_MVX_OSD_MASK;
    if (index & 0x1) {
        buf.reserved2 |= V4L2_BUF_FLAG_MVX_OSD_1;
        return;
    }
    if (index & 0x2) {
        buf.reserved2 |= V4L2_BUF_FLAG_MVX_OSD_2;
        return;
    }
}


void Buffer::setAdStatsGeneralBuffer(int ad_stats)
{
    struct v4l2_core_buffer_header_general buffer_configs;

    buf.reserved2 &= ~V4L2_BUF_FLAG_MVX_AD_STATS;

    if (ad_stats == 0)
    {
        return;
    }

    if(buf.m.planes[0].length < MVX_AD_STATS_THUMBNAIL_SIZE)
    {
        printf("frame size is less than %d,can not support AdStats\n",MVX_AD_STATS_THUMBNAIL_SIZE);
        return;
    }

    buffer_configs.type = V4L2_BUFFER_GENERAL_TYPE_AD_STATS;
    buffer_configs.buffer_size = MVX_AD_STATS_THUMBNAIL_SIZE;
    buffer_configs.config_size = sizeof(v4l2_buffer_general_ad_stats);
    memset(&buffer_configs.config.ad_stats,0,sizeof(buffer_configs.config.ad_stats));
    memcpy(&buf.m.planes[0].reserved[0], &buffer_configs,sizeof(buffer_configs.type) +  sizeof(buffer_configs.config_size) + sizeof(buffer_configs.buffer_size)
    + sizeof(buffer_configs.config.ad_stats));

    buf.reserved2 |= V4L2_BUF_FLAG_MVX_AD_STATS;

    printf("setAdStatsGeneralBuffer ad_stats:%d\n",ad_stats);
}

void Buffer::update(v4l2_buffer &b)
{
    buf = b;

    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        buf.m.planes = planes;
        for (size_t i = 0; i < buf.length; ++i)
        {
            buf.m.planes[i] = b.m.planes[i];
        }
    }
}

void Buffer::memoryMap(int fd)
{
    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        for (uint32_t i = 0; i < buf.length; ++i)
        {
            v4l2_plane &p = buf.m.planes[i];

            if (p.length > 0)
            {
                ptr[i] = mmap(NULL,
                              p.length,
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED,
                              fd,
                              p.m.mem_offset);
                if (ptr[i] == MAP_FAILED)
                {
                    throw Exception("Failed to mmap multi memory.");
                }
            }
        }
    }
    else
    {
        if (buf.length > 0)
        {
            ptr[0] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, buf.m.offset);
            if (ptr[0] == MAP_FAILED)
            {
                throw Exception("Failed to mmap memory.");
            }
        }
    }
}

void Buffer::memoryUnmap()
{
    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        for (uint32_t i = 0; i < buf.length; ++i)
        {
            if (buf.memory == V4L2_MEMORY_DMABUF)
            {
                close(buf.m.planes[i].m.fd);
            }
            if (ptr[i] != 0)
            {
                munmap(ptr[i], buf.m.planes[i].length);
            }
        }
    }
    else
    {
        if (ptr[0])
        {
            if (buf.memory == V4L2_MEMORY_DMABUF)
            {
                close(buf.m.fd);
            }
            munmap(ptr[0], buf.length);
        }
    }
}

size_t Buffer::getLength(unsigned int plane)
{
    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        if (buf.length <= plane)
        {
            return 0;
        }

        return buf.m.planes[plane].length;
    }
    else
    {
        if (plane > 0)
        {
            return 0;
        }

        return buf.length;
    }
}

void Buffer::setInterlaced(bool interlaced)
{
    buf.field = interlaced ? V4L2_FIELD_SEQ_TB : V4L2_FIELD_NONE;
}

void Buffer::setTiled(bool tiled)
{
    buf.flags &= ~(V4L2_BUF_FLAG_MVX_AFBC_TILED_HEADERS | V4L2_BUF_FLAG_MVX_AFBC_TILED_BODY);
    if (tiled)
    {
        buf.flags |= V4L2_BUF_FLAG_MVX_AFBC_TILED_HEADERS;
        buf.flags |= V4L2_BUF_FLAG_MVX_AFBC_TILED_BODY;
    }
}

void Buffer::setRoiCfg(struct v4l2_mvx_roi_regions roi)
{
    roi_cfg = roi;
    isRoiCfg = true;
}

void Buffer::setChrCfg(struct v4l2_mvx_chr_config chr)
{
    chr_cfg = chr;
    isChrCfg = true;
}

void Buffer::setOsdCfg(struct v4l2_osd_config osd)
{
    osd_cfg = osd;
}

void Buffer::setOsdCfgEnable(bool enable)
{
    isOsdCfg = enable;
}
void Buffer::setGopResetCfg(struct v4l2_gop_config cfg)
{
    gop_cfg = cfg;
    isGopCfg = true;
}

void Buffer::setLtrResetCfg(struct v4l2_reset_ltr_peroid_config cfg)
{
    ltr_cfg = cfg;
    isLtrCfg = true;
}

void Buffer::setSuperblock(bool superblock)
{
    buf.flags &= ~(V4L2_BUF_FLAG_MVX_AFBC_32X8_SUPERBLOCK);
    if (superblock)
    {
        buf.flags |= V4L2_BUF_FLAG_MVX_AFBC_32X8_SUPERBLOCK;
    }
}

void Buffer::setDmaFd(int fd, const unsigned int plane, const unsigned offset)
{
    if (buf.memory != V4L2_MEMORY_DMABUF)
    {
        throw Exception("Failed to set file descriptor. Illegal memory type. memory=%d.", buf.memory);
    }

    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        buf.m.planes[plane].m.fd = fd;
        buf.m.planes[plane].data_offset = offset;
    }
    else
    {
        buf.m.fd = fd;
    }
}

void Buffer::setLength(const unsigned int length, const unsigned int plane)
{
    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        buf.m.planes[plane].length = length;
    }
    else
    {
        buf.length = length;
    }
}

void Buffer::dmaMemoryMap(int dma_fd, const unsigned int plane)
{
    unsigned int length = getLength(plane);
    ptr[plane] = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, dma_fd, 0);
    if (ptr[plane] == MAP_FAILED)
    {
        throw Exception("Failed to mmap buffer.");
    }
}

void Buffer::dmaMemoryunMap(void *p, const unsigned int plane)
{
    unsigned int length = getLength(plane);
    munmap(p, length);
}

unsigned int Buffer::getNumPlanes() const
{
    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        return buf.length;
    }
    else
    {
        return 1;
    }
}

/****************************************************************************
 * Transcoder, decoder, encoder
 ****************************************************************************/

Codec::Codec(const char *dev,
             enum v4l2_buf_type inputType,
             enum v4l2_buf_type outputType,
             ostream &log,
             bool nonblock) :
    input(fd, inputType, log),
    output(fd, outputType, log),
    log(log),
    nonblock(nonblock)
{
    openDev(dev);
    mini_frame_height = 0;
    memory_type = V4L2_MEMORY_MMAP;
}

Codec::Codec(const char *dev,
             Input &input,
             enum v4l2_buf_type inputType,
             Output &output,
             enum v4l2_buf_type outputType,
             ostream &log,
             bool nonblock) :
    input(fd, input, inputType, log),
    output(fd, output, outputType, log),
    log(log),
    csweo(false),
    fps(0),
    bps(0),
    minqp(0),
    maxqp(0),
    fixedqp(0),
    nonblock(nonblock)
{
    openDev(dev);
    mini_frame_height = 0;
    memory_type = V4L2_MEMORY_MMAP;
}

Codec::~Codec()
{
    closeDev();
}

int Codec::stream()
{
    /* Set NALU. */
    if (isVPx(input.io->getFormat()))
    {
        input.setNALU(NALU_FORMAT_ONE_NALU_PER_BUFFER);
    }

    if (input.io->getFormat() == V4L2_PIX_FMT_RV)
    {
       input.setNALU(NALU_FORMAT_ONE_NALU_PER_BUFFER);
    }
    if (input.io->getFormat() == V4L2_PIX_FMT_VC1_ANNEX_L)
    {
        input.setNALU(NALU_FORMAT_ONE_NALU_PER_BUFFER);
    }
    if (input.io->getNaluFormat() == NALU_FORMAT_ONE_NALU_PER_BUFFER
            || input.io->getNaluFormat() == NALU_FORMAT_ONE_BYTE_LENGTH_FIELD
            || input.io->getNaluFormat() == NALU_FORMAT_TWO_BYTE_LENGTH_FIELD
            || input.io->getNaluFormat() == NALU_FORMAT_FOUR_BYTE_LENGTH_FIELD){
        input.setNALU((NaluFormat)input.io->getNaluFormat());
    }
    if ((input.io->getFormat() == V4L2_PIX_FMT_VC1_ANNEX_L) ||
        (input.io->getFormat() == V4L2_PIX_FMT_VC1_ANNEX_G))
    {
        struct v4l2_control control;
        int profile = 0xff;

        switch (input.io->getProfile())
        {
            case 0:
            {
                profile = 0;
                break;
            }
            case 4:
            {
                profile = 1;
                break;
            }
            case 12:
            {
                profile = 2;
                break;
            }
            default:
            {
                throw Exception("Unsupported VC1 profile.\n");
            }
        }

        log << "VC1 decoding profile( " << profile << " )" << endl;

        memset(&control, 0, sizeof(control));

        control.id = V4L2_CID_MVE_VIDEO_VC1_PROFILE;
        control.value = profile;

        if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
        {
            throw Exception("Failed to set profile=%u for fmt: %u .", profile, input.io->getFormat());
        }
    }

    /* Add VPx file header. */
    if (isVPx(output.io->getFormat()))
    {
        output.setNALU(NALU_FORMAT_ONE_NALU_PER_BUFFER);
    }

    try
    {
        queryCapabilities();
        /* enumerateFormats(); */
        enumerateFramesizes(output.io->getFormat());
        setFormats();
        subscribeEvents();
        allocateBuffers(memory_type);
        queueBuffers();
        streamon();

        if (nonblock)
        {
            runPoll();
        }
        else
        {
            runThreads();
        }
        streamoff();
    }
    catch (Exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}

uint32_t Codec::to4cc(const string &str)
{
    if (str.compare("yuv420_afbc_8") == 0)
    {
        return v4l2_fourcc('Y', '0', 'A', '8');
    }
    else if (str.compare("yuv420_afbc_10") == 0)
    {
        return v4l2_fourcc('Y', '0', 'A', 'A');
    }
    else if (str.compare("yuv422_afbc_8") == 0)
    {
        return v4l2_fourcc('Y', '2', 'A', '8');
    }
    else if (str.compare("yuv422_afbc_10") == 0)
    {
        return v4l2_fourcc('Y', '2', 'A', 'A');
    }
    else if (str.compare("yuv420") == 0)
    {
        return V4L2_PIX_FMT_YUV420M;
    }
    else if (str.compare("yuv420_nv12") == 0)
    {
        return V4L2_PIX_FMT_NV12;
    }
    else if (str.compare("yuv420_nv21") == 0)
    {
        return V4L2_PIX_FMT_NV21;
    }
    else if (str.compare("yuv420_p010") == 0)
    {
        return V4L2_PIX_FMT_P010;
    }
    else if (str.compare("yuv420_y0l2") == 0)
    {
        return V4L2_PIX_FMT_Y0L2;
    }
    else if (str.compare("yuv420_aqb1") == 0)
    {
        return V4L2_PIX_FMT_AQB1;
    }
    else if (str.compare("yuv422_yuy2") == 0)
    {
        return V4L2_PIX_FMT_YUYV;
    }
    else if (str.compare("yuv422_uyvy") == 0)
    {
        return V4L2_PIX_FMT_UYVY;
    }
    else if (str.compare("yuv422_y210") == 0)
    {
        return V4L2_PIX_FMT_Y210;
    }
    else if (str.compare("rgba") == 0)
    {
        return DRM_FORMAT_ABGR8888;
    }
    else if (str.compare("bgra") == 0)
    {
        return DRM_FORMAT_ARGB8888;
    }
    else if (str.compare("argb") == 0)
    {
        return DRM_FORMAT_BGRA8888;
    }
    else if (str.compare("abgr") == 0)
    {
        return DRM_FORMAT_RGBA8888;
    }
    else if (str.compare("bgr") == 0)
    {
        return V4L2_PIX_FMT_BGR24;
    }
    else if (str.compare("rgb") == 0)
    {
        return V4L2_PIX_FMT_RGB24;
    }
    else if (str.compare("rgb3p") == 0)
    {
        return V4L2_PIX_FMT_RGB_3P;
    }
    else if (str.compare("argb1555") == 0)
    {
        return V4L2_PIX_FMT_ARGB555;
    }
    else if (str.compare("argb4444") == 0)
    {
        return V4L2_PIX_FMT_ARGB444;
    }
    else if (str.compare("rgb565") == 0)
    {
        return V4L2_PIX_FMT_RGB565;
    }
    else if (str.compare("gray") == 0)
    {
        return V4L2_PIX_FMT_GREY;
    }
    else if (str.compare("gray_10") == 0)
    {
        return V4L2_PIX_FMT_Y10_LE;
    }
    else if (str.compare("yuv444") == 0)
    {
        return V4L2_PIX_FMT_YUV444M;
    }
    else if (str.compare("yuv444_10") == 0)
    {
        return V4L2_PIX_FMT_YUV444_10;
    }
    else if (str.compare("yuv420_2p10") == 0)
    {
        return V4L2_PIX_FMT_YUV420_2P_10;
    }
    else if (str.compare("yuv422_1p10") == 0)
    {
        return V4L2_PIX_FMT_YUV422_1P_10;
    }
    else if (str.compare("yuv_i42010") == 0)
    {
        return V4L2_PIX_FMT_YUV420_I420_10;
    }
    else if (str.compare("avs2") == 0)
    {
        return V4L2_PIX_FMT_AVS2;
    }
    else if (str.compare("avs") == 0)
    {
        return V4L2_PIX_FMT_AVS;
    }
    else if (str.compare("h263") == 0)
    {
        return V4L2_PIX_FMT_H263;
    }
    else if (str.compare("h264") == 0)
    {
        return V4L2_PIX_FMT_H264;
    }
    else if (str.compare("h264_mvc") == 0)
    {
        return V4L2_PIX_FMT_H264_MVC;
    }
    else if (str.compare("h264_no_sc") == 0)
    {
        return V4L2_PIX_FMT_H264_NO_SC;
    }
    else if (str.compare("hevc") == 0)
    {
        return V4L2_PIX_FMT_HEVC;
    }
    else if (str.compare("mjpeg") == 0)
    {
        return V4L2_PIX_FMT_MJPEG;
    }
    else if (str.compare("jpeg") == 0)
    {
        return V4L2_PIX_FMT_JPEG;
    }
    else if (str.compare("mpeg2") == 0)
    {
        return V4L2_PIX_FMT_MPEG2;
    }
    else if (str.compare("mpeg4") == 0)
    {
        return V4L2_PIX_FMT_MPEG4;
    }
    else if (str.compare("rv") == 0)
    {
        return V4L2_PIX_FMT_RV;
    }
    else if (str.compare("vc1") == 0)
    {
        return V4L2_PIX_FMT_VC1_ANNEX_G;
    }
    else if (str.compare("vc1_l") == 0)
    {
        return V4L2_PIX_FMT_VC1_ANNEX_L;
    }
    else if (str.compare("vp8") == 0)
    {
        return V4L2_PIX_FMT_VP8;
    }
    else if (str.compare("vp9") == 0)
    {
        return V4L2_PIX_FMT_VP9;
    }
    else if (str.compare("av1") == 0)
    {
        return V4L2_PIX_FMT_AV1;
    }
    else
    {
        throw Exception("Not a valid format '%s'.\n", str.c_str());
    }

    return 0;
}

bool Codec::isVPx(uint32_t format)
{
    return format == V4L2_PIX_FMT_VP8 || format == V4L2_PIX_FMT_VP9 || format == V4L2_PIX_FMT_AV1;
}

bool Codec::isAFBC(uint32_t format)
{
    switch (format)
    {
        case V4L2_PIX_FMT_YUV420_AFBC_8:
        case V4L2_PIX_FMT_YUV420_AFBC_10:
        case V4L2_PIX_FMT_YUV422_AFBC_8:
        case V4L2_PIX_FMT_YUV422_AFBC_10:
            return true;
        default:
            return false;
    }
}

bool Codec::isYUV422(uint32_t format)
{
    switch (format)
    {
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
        case V4L2_PIX_FMT_Y210:
        case V4L2_PIX_FMT_YUV422_1P_10:
        case V4L2_PIX_FMT_YUV422_AFBC_8:
        case V4L2_PIX_FMT_YUV422_AFBC_10:
            return true;
        default:
            return false;
    }
}

size_t Codec::getBytesUsed(v4l2_buffer &buf)
{
    size_t size = 0;

    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        for (uint32_t i = 0; i < buf.length; ++i)
        {
            size += buf.m.planes[i].bytesused;
        }
    }
    else
    {
        size = buf.bytesused;
    }

    return size;
}

void Codec::openDev(const char *dev)
{
    int flags = O_RDWR;

    log << "Opening '" << dev << "'." << endl;

    if (nonblock)
    {
        flags |= O_NONBLOCK;
    }

    /* Open the video device in read/write mode. */
    fd = open(dev, flags);
    if (fd < 0)
    {
        throw Exception("Failed to open device.");
    }
}

void Codec::closeDev()
{
    log << "Closing fd " << fd << "." << endl;
    close(fd);
    fd = -1;
}

void Codec::queryCapabilities()
{
    struct v4l2_capability cap;
    int ret;

    /* Query capabilities. */
    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret != 0)
    {
        throw Exception("Failed to query for capabilities");
    }

    if ((cap.capabilities &
         (V4L2_CAP_VIDEO_M2M | V4L2_CAP_VIDEO_M2M_MPLANE)) == 0)
    {
        throw Exception("Device is missing m2m support.");
    }
}

void Codec::enumerateFormats()
{
    input.enumerateFormats();
    output.enumerateFormats();
}

void Codec::Port::enumerateFormats()
{
    struct v4l2_fmtdesc fmtdesc;
    int ret;

    fmtdesc.index = 0;
    fmtdesc.type = type;

    while (1)
    {
        ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc);
        if (ret != 0)
        {
            break;
        }

        log << "fmt: index=" << fmtdesc.index <<
        ", type=" << fmtdesc.type <<
        " , flags=" << hex << fmtdesc.flags <<
        ", pixelformat=" << fmtdesc.pixelformat <<
        ", description=" << fmtdesc.description <<
        endl;

        fmtdesc.index++;
    }

    printf("\n");
}

void Codec::enumerateFramesizes(uint32_t format)
{
    struct v4l2_frmsizeenum frmsize;

    frmsize.index = 0;
    frmsize.pixel_format = format;

    int ret = ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize);
    if (ret != 0)
    {
        throw Exception("Failed to enumerate frame sizes. ret=d.\n",
                        ret);
    }

    log << "Enumerate frame size." <<
    " index=" << frmsize.index <<
    ", pixel_format=" << hex << frmsize.pixel_format << dec;

    switch (frmsize.type)
    {
        case V4L2_FRMIVAL_TYPE_DISCRETE:
            break;
        case V4L2_FRMIVAL_TYPE_CONTINUOUS:
        case V4L2_FRMIVAL_TYPE_STEPWISE:
            log << ", min_width=" << frmsize.stepwise.min_width <<
            ", max_width=" << frmsize.stepwise.max_width <<
            ", step_width=" << frmsize.stepwise.step_width <<
            ", min_height=" << frmsize.stepwise.min_height <<
            ", max_height=" << frmsize.stepwise.max_height <<
            ", step_height=" << frmsize.stepwise.step_height;
            break;
        default:
            throw Exception("Unsupported enumerate frame size type. type=d.\n",
                            frmsize.type);
    }

    log << endl;
}

const v4l2_format &Codec::Port::getFormat()
{
    /* Get and print format. */
    format.type = type;
    int ret = ioctl(fd, VIDIOC_G_FMT, &format);
    if (ret != 0)
    {
        throw Exception("Failed to get format.");
    }

    return format;
}

void Codec::Port::tryFormat(v4l2_format &format)
{
    int ret = ioctl(fd, VIDIOC_TRY_FMT, &format);
    if (ret != 0)
    {
        throw Exception("Failed to try format.");
    }
}

void Codec::Port::setFormat(v4l2_format &format)
{
    int ret = ioctl(fd, VIDIOC_S_FMT, &format);
    if (ret != 0)
    {
        throw Exception("Failed to set format.");
    }

    this->format = format;
}

void Codec::Port::getTrySetFormat()
{
    size_t width = 0, height = 0;

    v4l2_format fmt = getFormat();
    if (V4L2_TYPE_IS_MULTIPLANAR(type))
    {
        struct v4l2_pix_format_mplane &f = fmt.fmt.pix_mp;

        f.pixelformat = io->getFormat();
        f.width = io->getWidth();
        f.height = io->getHeight();
        f.num_planes = 3;
        f.field = interlaced ? V4L2_FIELD_SEQ_TB : V4L2_FIELD_NONE;

        for (int i = 0; i < 3; ++i)
        {
            f.plane_fmt[i].bytesperline = __stride[i];
            f.plane_fmt[i].sizeimage = 0;
        }
    }
    else
    {
        struct v4l2_pix_format &f = fmt.fmt.pix;

        f.pixelformat = io->getFormat();
        f.width = io->getWidth();
        f.height = io->getHeight();
        f.bytesperline = 0;
        f.sizeimage = 1 * 1024 * 1024;
        f.field = interlaced ? V4L2_FIELD_SEQ_TB : V4L2_FIELD_NONE;
    }

    /* Try format. */
    tryFormat(fmt);

    if (V4L2_TYPE_IS_MULTIPLANAR(type))
    {
        struct v4l2_pix_format_mplane &f = fmt.fmt.pix_mp;
        width = f.width;
        height = f.height;
        if (mini_frame_height >= 64) {
            for (int i = 0; i < 3; ++i)
            {
                f.plane_fmt[i].sizeimage = f.plane_fmt[i].bytesperline * io->getReadHeight();
                if (i > 0) f.plane_fmt[i].sizeimage /= 2;
            }
        }
    }
    else
    {
        struct v4l2_pix_format &f = fmt.fmt.pix;
        width = f.width;
        height = f.height;
    }
    // for dsl frame case, this is not suitable, remove this.
    if (V4L2_TYPE_IS_OUTPUT(type) && (width != io->getWidth() || height != io->getHeight()))
    {
        //throw Exception("Selected resolution is not supported for this format width:%d, io width:%d", width, io->getWidth());
    }
    setFormat(fmt);
    printFormat(fmt);
}

void Codec::setFormats()
{
    input.getTrySetFormat();
    output.getTrySetFormat();
}

void Codec::Port::printFormat(const struct v4l2_format &format)
{
    if (V4L2_TYPE_IS_MULTIPLANAR(format.type))
    {
        const struct v4l2_pix_format_mplane &f = format.fmt.pix_mp;

        log << "Format:" << dec <<
        " type=" << format.type <<
        ", format=" << f.pixelformat <<
        ", width=" << f.width <<
        ", height=" << f.height <<
        ", nplanes=" << int(f.num_planes) <<
        ", bytesperline=[" << f.plane_fmt[0].bytesperline <<
        ", " << f.plane_fmt[1].bytesperline <<
        ", " << f.plane_fmt[2].bytesperline << "]" <<
        ", sizeimage=[" << f.plane_fmt[0].sizeimage <<
        ", " << f.plane_fmt[1].sizeimage <<
        ", " << f.plane_fmt[2].sizeimage << "]" <<
        ", interlaced:" << interlaced <<
        endl;
    }
    else
    {
        const struct v4l2_pix_format &f = format.fmt.pix;

        log << "Format:" << dec <<
        " type=" << format.type <<
        ", format=" << f.pixelformat <<
        ", width=" << f.width <<
        ", height=" << f.height <<
        ", sizeimage=" << f.sizeimage <<
        ", bytesperline=" << f.bytesperline <<
        ", interlaced:" << interlaced << endl;
    }
}

const v4l2_crop Codec::Port::getCrop()
{
    v4l2_crop crop = { .type = type };

    int ret = ioctl(fd, VIDIOC_G_CROP, &crop);
    if (ret != 0)
    {
        throw Exception("Failed to get crop.");
    }

    return crop;
}

void Codec::Port::setInterlaced(bool interlaced)
{
    this->interlaced = interlaced;
}

void Codec::Port::tryEncStopCmd(bool tryStop)
{
    this->tryEncStop = tryStop;
}

void Codec::Port::tryDecStopCmd(bool tryStop)
{
    this->tryDecStop = tryStop;
}

v4l2_mvx_color_desc Codec::getColorDesc()
{
    v4l2_mvx_color_desc color;

    int ret = ioctl(fd, VIDIOC_G_MVX_COLORDESC, &color);
    if (ret != 0)
    {
        throw Exception("Failed to get color description.");
    }

    return color;
}

void Codec::printColorDesc(const v4l2_mvx_color_desc &color)
{
    log << "Color desc. range=" << static_cast<unsigned int>(color.range) <<
    ", primaries=" << static_cast<unsigned int>(color.primaries) <<
    ", transfer=" << static_cast<unsigned int>(color.transfer) <<
    ", matrix=" << static_cast<unsigned int>(color.matrix);

    if (color.flags & V4L2_MVX_COLOR_DESC_DISPLAY_VALID)
    {
        log << ", display={" <<
        "r={x=" << color.display.r.x << ", y=" << color.display.r.y << "}" <<
        ", g={x=" << color.display.g.x << ", y=" << color.display.g.y << "}" <<
        ", b={x=" << color.display.b.x << ", y=" << color.display.b.y << "}" <<
        ", w={x=" << color.display.w.x << ", y=" << color.display.w.y << "}";
    }

    if (color.flags & V4L2_MVX_COLOR_DESC_CONTENT_VALID)
    {
        log << ", luminance_min=" << color.display.luminance_min * 0.00002 <<
        ", lumiance_max=" <<  color.display.luminance_max * 0.00002 << "}" <<
        ", content={luminance_max=" << color.content.luminance_max * 0.00002 <<
        ", luminance_average=" << color.content.luminance_average * 0.00002 << "}";
    }

    log << endl;
}

void Codec::subscribeEvents()
{
    subscribeEvents(V4L2_EVENT_EOS);
    subscribeEvents(V4L2_EVENT_SOURCE_CHANGE);
    subscribeEvents(V4L2_EVENT_MVX_COLOR_DESC);
}

void Codec::subscribeEvents(uint32_t event)
{
    struct v4l2_event_subscription sub = {
        .type = event, .id = 0
    };
    int ret;

    ret = ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret != 0)
    {
        throw Exception("Failed to subscribe for event.");
    }
}

void Codec::unsubscribeEvents()
{
    unsubscribeEvents(V4L2_EVENT_ALL);
}

void Codec::unsubscribeEvents(uint32_t event)
{
    struct v4l2_event_subscription sub;
    int ret;

    sub.type = event;
    ret = ioctl(fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
    if (ret != 0)
    {
        throw Exception("Failed to unsubscribe for event.");
    }
}
void Codec::setInputThread(int input_thread)
{
    if (input_thread)
    {
       input.startInputThread();
    }
}

void Codec::setMemoryType(enum v4l2_memory mem_type)
{
    memory_type = mem_type;
    input.setPortMemoryType(mem_type);
    output.setPortMemoryType(mem_type);
}

void Codec::allocateBuffers(enum v4l2_memory m_type)
{
    //0 is default value, let port handle buffer cnt
    input.allocateBuffers(input.getBufferCnt() ? input.getBufferCnt() : 6, m_type);
    output.allocateBuffers(output.getBufferCnt()? output.getBufferCnt() : 6, m_type);
}

uint32_t Codec::Port::getInputBufferIdx(pthread_mutex_t *mutex, pthread_cond_t *cond, std::queue<uint32_t> *input_queue)
{
    uint32_t index;
    pthread_mutex_lock(mutex);
    while(input_queue->size() == 0)
    {
        pthread_cond_wait(cond, mutex);
    }
    index = input_queue->front();
    input_queue->pop();
    pthread_mutex_unlock(mutex);
    return index;
}

void Codec::Port::appendInputBufferIdx(pthread_mutex_t *mutex, pthread_cond_t *cond, std::queue<uint32_t> *input_queue, uint32_t index)
{
    pthread_mutex_lock(mutex);
    input_queue->push(index);
    pthread_cond_broadcast(cond);
    pthread_mutex_unlock(mutex);
}

void* Codec::Port::fillInputThread(void *arg)
{
    Codec::Port *input = static_cast<Codec::Port *>(arg);
    pthread_detach(pthread_self());
    input->_fillInputThread();
    pthread_exit((void *)NULL);
}

void Codec::Port::_fillInputThread()
{
    while(!io->eof()){
        uint32_t index = getInputBufferIdx(&input_producer_mutex, &input_producer_cond, &input_producer_queue);
        Buffer &buffer = *(buffers.at(index));
        io->prepare(buffer);
        buffer.setEndOfStream(io->eof());
        appendInputBufferIdx(&input_consumer_mutex, &input_consumer_cond, &input_consumer_queue, index);
    }
}

void Codec::Port::startInputThread()
{
    int ret;
    isInputThread = true;
    pthread_mutex_init(&input_consumer_mutex, NULL);
    pthread_mutex_init(&input_producer_mutex, NULL);
    pthread_cond_init(&input_consumer_cond, NULL);
    pthread_cond_init(&input_producer_cond, NULL);
    ret = pthread_create(&tid, NULL, fillInputThread, this);
    if (ret != 0)
    {
        throw Exception("Failed to create input thread.");
    }
}

void Codec::Port::setPortMemoryType(enum v4l2_memory mem_type)
{
    memory_type_port = mem_type;
}

int Codec::Port::allocateDMABuf(size_t size)
{
    int ionfd = open("/dev/ion", O_RDWR);
    int dma_fd;
    if (ionfd < 0)
    {
        throw Exception("Failed to open ion device.");
    }

    struct ion_allocation_data allocData = { .len = size };
    allocData.heap_id_mask = 1 << (ION_HEAP_TYPE_CUSTOM + 1);
    if (ioctl(ionfd, ION_IOC_ALLOC, &allocData) < 0)
    {
        throw Exception("ION_IOC_ALLOC failed. errno=%d (%s).", errno, strerror(errno));
    }

#ifdef ION_IOC_MAP
    ion_user_handle_t handle = allocData.handle;

    struct ion_fd_data fdData= { .handle = handle, .fd = -1 };
    if (ioctl(ionfd, ION_IOC_MAP, &fdData) < 0)
    {
        throw Exception("ION_IOC_MAP failed. errno=%d (%s).", errno, strerror(errno));
    }

    dma_fd = fdData.fd;
    struct ion_handle_data handleData = { .handle = handle };
    if (ioctl(ionfd, ION_IOC_FREE, &handleData) < 0)
    {
        throw Exception("ION_IOC_FREE failed. errno=%d (%s).", errno, strerror(errno));
    }
#else
    dma_fd = allocData.fd;
#endif
    close(ionfd);
    return dma_fd;
}

void Codec::Port::allocateBuffers(size_t count, enum v4l2_memory mem_type)
{
    struct v4l2_requestbuffers reqbuf;
    uint32_t i;
    int ret;
    /* Free existing meta buffer. */
    freeBuffers();
    /* Request new buffer to be allocated. */
    reqbuf.count = io->needDoubleCount()?count * 2 : count;
    reqbuf.type = type;
    reqbuf.memory = mem_type;
    ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
    if (ret != 0)
    {
        throw Exception("Failed to request buffers.%d", mem_type);
    }

    log << "Request buffers." <<
    " type=" << reqbuf.type <<
    ", count=" << reqbuf.count <<
    ", memory=" << reqbuf.memory << endl;

    /* Reset number of buffers queued to driver. */
    pending = 0;

    /* Query each buffer and create a new meta buffer. */
    for (i = 0; i < reqbuf.count; ++i)
    {
        v4l2_buffer buf;
        struct v4l2_plane planes[VIDEO_MAX_PLANES];

        buf.type = type;
        buf.memory = mem_type;
        buf.index = i;
        buf.length = 3;
        buf.m.planes = planes;
        ret = ioctl(fd, VIDIOC_QUERYBUF, &buf);
        if (ret != 0)
        {
            throw Exception("Failed to query buffer.ret=%d", ret);
        }

        printBuffer(buf, "Query");

        buffers[buf.index] = new Buffer(buf, fd, format, mem_type);
        size_t nplanes = buffers[buf.index]->getNumPlanes();
        if (mem_type == V4L2_MEMORY_MMAP) {
            buffers[buf.index]->memoryMap(fd);
        } else if (mem_type == V4L2_MEMORY_DMABUF) {
            for (unsigned int plane = 0; plane < nplanes; ++plane)
            {
                unsigned int length = nplanes == 1 ? buf.length : buf.m.planes[plane].length;
                int dma_fd = allocateDMABuf(length);

                buffers[buf.index]->setDmaFd(dma_fd, plane);
                buffers[buf.index]->setLength(length, plane);
                buffers[buf.index]->dmaMemoryMap(dma_fd, plane);
            }
        } else {
            cerr<<"didnot support this v4l2 memory type on this mode:"<<mem_type<<endl;
        }
    }
}

void Codec::freeBuffers()
{
    input.freeBuffers();
    output.freeBuffers();
}

void Codec::Port::freeBuffers()
{
    while (!buffers.empty())
    {
        BufferMap::iterator it = buffers.begin();
        delete(it->second);
        buffers.erase(it);
    }
}

unsigned int Codec::Port::getBufferCount()
{
    struct v4l2_control control;
    uint32_t value;

    control.id = V4L2_TYPE_IS_OUTPUT(type) ? V4L2_CID_MIN_BUFFERS_FOR_OUTPUT : V4L2_CID_MIN_BUFFERS_FOR_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_G_CTRL, &control))
    {
        throw Exception("Failed to get minimum buffers.");
    }
    value = control.value;
    return value > getBufferCnt() ? value : getBufferCnt();
}

void Codec::queueBuffers()
{
    output.queueBuffers();
    input.queueBuffers();
}

void Codec::Port::queueBuffers()
{
    for (BufferMap::iterator it = buffers.begin();
         it != buffers.end(); ++it)
    {
        Buffer &buffer = *(it->second);
        if (!io->eof())
        {
            /* Remove vendor custom flags. */
            buffer.resetVendorFlags();

            if (isInputThread) {
                appendInputBufferIdx(&input_producer_mutex, &input_producer_cond, &input_producer_queue, buffer.getBuffer().index);
                uint32_t index = getInputBufferIdx(&input_consumer_mutex, &input_consumer_cond, &input_consumer_queue);
                Buffer &buffer_input = *(buffers.at(index));
                queueBuffer(buffer_input);
            } else {
                io->prepare(buffer);
                buffer.setEndOfStream(io->eof());
                queueBuffer(buffer);
            }
        }
    }
}

void Codec::Port::queueBuffer(Buffer &buf)
{
    v4l2_buffer &b = buf.getBuffer();
    int ret;

    buf.setFrameProcessCount(frames_processed);
    buf.setInterlaced(interlaced);
    if(io->getDir() == 0 && V4L2_TYPE_IS_MULTIPLANAR(b.type)){
        buf.setEncRotation(rotation);
    }
    if(io->getDir() == 1 && V4L2_TYPE_IS_MULTIPLANAR(b.type)){
        buf.setRotation(rotation);
    }
    buf.setMirror(mirror);
    buf.setDownScale(scale);

    if (buf.getRoiCfgflag() && getBytesUsed(b) != 0) {
        struct v4l2_mvx_roi_regions roi = buf.getRoiCfg();
        ret = ioctl(fd, VIDIOC_S_MVX_ROI_REGIONS, &roi);
        if (ret != 0)
        {
            throw Exception("Failed to queue roi param.");
        }
    }

    if (buf.getChrCfgflag() && getBytesUsed(b) != 0) {
        struct v4l2_mvx_chr_config chr = buf.getChrCfg();
        ret = ioctl(fd, VIDIOC_S_MVX_CHR_CFG, &chr);
        if (ret != 0)
        {
            throw Exception("Failed to queue chr param.");
        }
    }

    if (buf.getGopResetCfgflag() && getBytesUsed(b) != 0) {
        struct v4l2_gop_config gop = buf.getGopResetCfg();
        setGopResetPframes(gop.gop_pframes);
    }

    if (buf.getLtrResetCfgflag() && getBytesUsed(b) != 0) {
        struct v4l2_reset_ltr_peroid_config ltr = buf.getLtrResetCfg();
        setLtrResetPeriod(ltr.reset_ltr_peroid);
    }
    if (buf.getResetStatsMode() > 0  && getBytesUsed(b) != 0) {
        setStatsMode(buf.getResetStatsMode(), buf.getResetStatsPicIndex());
    }

    if (buf.getOsdCfgEnable() > 0  && getBytesUsed(b) != 0) {
        setOsdCfg(buf.getOsdCfg());
    }

    if (buf.getQPofEPR().qp > 0) {
        struct v4l2_buffer_param_qp _epr_qp = buf.getQPofEPR();
        ret = ioctl(fd, VIDIOC_S_MVX_QP_EPR, &_epr_qp);
        if (ret != 0)
        {
            throw Exception("Failed to queue roi param.");
        }
        _epr_qp.qp = 0;
        buf.setQPofEPR(_epr_qp);
    }
    /* Mask buffer offset. */
    if (!V4L2_TYPE_IS_MULTIPLANAR(b.type))
    {
        switch (b.memory)
        {
            case V4L2_MEMORY_MMAP:
                b.m.offset &= ~((1 << 12) - 1);
                break;
            default:
                break;
        }
    }
    //encoder specfied frames count to be processed
    if (io->getDir() == 0 && frames_count > 0 && frames_processed >= frames_count - 1 && !buf.isGeneralBuffer() && !buf.isOsdBuffer()) {
        if (frames_processed >= frames_count) {
            buf.clearBytesUsed();
            buf.resetVendorFlags();
            return;
        }
        buf.setEndOfStream(true);
    }
    if (io->getDir() == 0 && V4L2_TYPE_IS_MULTIPLANAR(b.type) &&
           !buf.isGeneralBuffer() && !buf.isOsdBuffer()) {
        Input *is = dynamic_cast<Input*>(io);
        if (is->idr_list.size())
        {
            if (is->idr_list.front() == frames_processed){
                is->idr_list.pop();
                buf.set_force_idr_flag(true);
            } else {
                buf.set_force_idr_flag(false);
            }
        }
        frames_processed++;
    }
    if (seamless.seamless_mode !=0 && io->getDir() == 1 && V4L2_TYPE_IS_MULTIPLANAR(b.type)) {
        buf.setAdStatsGeneralBuffer(ad_stats);
    }
    printBuffer(b, "->");
    ret = ioctl(fd, VIDIOC_QBUF, &b);
    if (ret != 0)
    {
        throw Exception("Failed to queue buffer.");
    }

    ++pending;
}

Buffer &Codec::Port::dequeueBuffer()
{
    v4l2_plane planes[VIDEO_MAX_PLANES];
    v4l2_buffer buf;
    buf.m.planes = planes;
    int ret;

    buf.type = type;
    buf.memory = memory_type_port;
    buf.length = 3;

    ret = ioctl(fd, VIDIOC_DQBUF, &buf);
    if (ret != 0)
    {
        throw Exception("Failed to dequeue buffer. type=%u, memory=%u",
                        buf.type, buf.memory);
    }

    --pending;
    printBuffer(buf, "<-");

    Buffer &buffer = *(buffers.at(buf.index));
    buffer.update(buf);

    buffer.setCrop(getCrop());
    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        buffer.setSeamless(this->seamless);
    }
    return buffer;
}

void Codec::Port::printBuffer(const v4l2_buffer &buf, const char *prefix)
{
    log << prefix << ": " <<
    "type=" << buf.type <<
    ", index=" << buf.index <<
    ", sequence=" << buf.sequence <<
    ", timestamp={" << buf.timestamp.tv_sec << ", " << buf.timestamp.tv_usec << "}" <<
    ", flags=" << hex << buf.flags << dec;

    if (V4L2_TYPE_IS_MULTIPLANAR(buf.type))
    {
        const char *delim;

        log << ", num_planes=" << buf.length;

        delim = "";
        log << ", bytesused=[";
        for (unsigned int i = 0; i < buf.length; ++i)
        {
            log << delim << buf.m.planes[i].bytesused;
            delim = ", ";
        }
        log << "]";

        delim = "";
        log << ", length=[";
        for (unsigned int i = 0; i < buf.length; ++i)
        {
            log << delim << buf.m.planes[i].length;
            delim = ", ";
        }
        log << "]";

        delim = "";
        log << ", offset=[";
        for (unsigned int i = 0; i < buf.length; ++i)
        {
            log << delim << buf.m.planes[i].data_offset;
            delim = ", ";
        }
        log << "]";
        if(seamless.seamless_mode != 0 && buf.m.planes[0].bytesused >0)
        {
            log << ", out video w:h=[";
            log<<((buf.m.planes[0].reserved[1] & 0xffff0000) >> 16);
            delim = ", ";
            log << delim << (buf.m.planes[0].reserved[1] & 0xffff);
            log << "]";
            log << ", stride=[";
            delim ="";
            for (unsigned int i = 0; i < buf.length; ++i)
            {
                log << delim << buf.m.planes[i].reserved[0];
                delim = ", ";
            }
            log << "]";
        }
    }
    else
    {
        log << ", bytesused=" << buf.bytesused <<
        ", length=" << buf.length;
    }

    log << endl;
}

void Codec::streamon()
{
    input.streamon();
    output.streamon();
}

void Codec::Port::streamon()
{
    log << "Stream on " << dec << type << endl;

    int ret = ioctl(fd, VIDIOC_STREAMON, &type);
    if (ret != 0)
    {
        throw Exception("Failed to stream on.");
    }
}

void Codec::streamoff()
{
    input.streamoff();
    output.streamoff();
}

void Codec::Port::streamoff()
{
    log << "Stream off " << dec << type << endl;

    int ret = ioctl(fd, VIDIOC_STREAMOFF, &type);
    if (ret != 0)
    {
        throw Exception("Failed to stream off.");
    }
}

void Codec::Port::sendEncStopCommand()
{
    v4l2_encoder_cmd cmd = { .cmd = V4L2_ENC_CMD_STOP };

    if (tryEncStop)
    {
        if (0 != ioctl(fd, VIDIOC_TRY_ENCODER_CMD, &cmd))
        {
            throw Exception("Failed to send try encoder stop command.");
        }
        if (0 != ioctl(fd, VIDIOC_ENCODER_CMD, &cmd))
        {
           throw Exception("Failed to send encoding stop command.");
        }
    }
}

void Codec::Port::sendDecStopCommand()
{
    v4l2_decoder_cmd cmd = { .cmd = V4L2_DEC_CMD_STOP };

    if (tryDecStop)
    {
        if (0 != ioctl(fd, VIDIOC_TRY_DECODER_CMD, &cmd))
        {
            throw Exception("Failed to send try decoder stop command.");
        }
    }

    if (0 != ioctl(fd, VIDIOC_DECODER_CMD, &cmd))
    {
        throw Exception("Failed to send decoding stop command.");
    }
}

void Codec::Port::setH264DecIntBufSize(uint32_t ibs)
{
    log << "setH264DecIntBufSize( " << ibs << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_INTBUF_SIZE;
    control.value = ibs;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set H264 ibs=%u.", ibs);
    }
}

void Codec::Port::setDecFrameReOrdering(uint32_t fro)
{
    log << "setDecFrameReOrdering( " << fro << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_FRAME_REORDERING;
    control.value = fro;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set decoding fro=%u.", fro);
    }
}

void Codec::Port::setDecIgnoreStreamHeaders(uint32_t ish)
{
    log << "setDecIgnoreStreamHeaders( " << ish << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_IGNORE_STREAM_HEADERS;
    control.value = ish;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set decoding ish=%u.", ish);
    }
}

void Codec::Port::setNALU(NaluFormat nalu)
{
    log << "Set NALU " << nalu << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_NALU_FORMAT;
    control.value = nalu;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set NALU. nalu=%u.", nalu);
    }
}

void Codec::Port::setEncFramerate(uint32_t frame_rate)
{
    log << "setEncFramerate( " << frame_rate << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_FRAME_RATE;
    control.value = frame_rate;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set frame_rate=%u.", frame_rate);
    }
}

void Codec::Port::setEncBitrate(uint32_t bit_rate)
{
    log << "setEncBitrate( " << bit_rate << " )" << endl;
    log << "setRctype( " << rc_type << " )" << endl;
    if (bit_rate == 0 && rc_type == 0) {
        return;
    }
    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MPEG_VIDEO_BITRATE;
    control.value = bit_rate;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set bit_rate=%u.", bit_rate);
    }
}

void Codec::Port::setRateControl(struct v4l2_rate_control *rc)
{
    log << "setRateControl( " << rc->rc_type << ",";
    log << rc->target_bitrate << "," << rc->maximum_bitrate << ")" << endl;

    int ret = ioctl(fd, VIDIOC_S_MVX_RATE_CONTROL, rc);
    if (ret != 0)
    {
        throw Exception("Failed to set rate control.");
    }

    return;

}

void Codec::Port::setEncPFrames(uint32_t pframes)
{
    log << "setEncPFrames( " << pframes << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_P_FRAMES;
    control.value = pframes;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set pframes=%u.", pframes);
    }
}

void Codec::Port::setEncBFrames(uint32_t bframes)
{
    log << "setEncBFrames( " << bframes << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MPEG_VIDEO_B_FRAMES;
    control.value = bframes;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set bframes=%u.", bframes);
    }
}

void Codec::Port::setEncSliceSpacing(uint32_t spacing)
{
    log << "setEncSliceSpacing( " << spacing << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB;
    control.value = spacing;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set slice spacing=%u.", spacing);
    }

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE;
    control.value = spacing != 0;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set slice mode.");
    }
}

void Codec::Port::setEncForceChroma(uint32_t fmt)
{
    log << "setEncForceChroma( " << fmt << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_FORCE_CHROMA_FORMAT;
    control.value = fmt;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set Force chroma fmt=%u.", fmt);
    }
}

void Codec::Port::setEncBitdepth(uint32_t bd)
{
    log << "setEncBitdepth( " << bd << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_BITDEPTH_LUMA;
    control.value = bd;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set encode video frame luma bd=%u.", bd);
    }

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_BITDEPTH_CHROMA;
    control.value = bd;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set encode video frame chroma bd=%u.", bd);
    }
}

void Codec::Port::setH264EncIntraMBRefresh(uint32_t period)
{
    log << "setH264EncIntraMBRefresh( " << period << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB;
    control.value = period;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set H264 period=%u.", period);
    }
}

void Codec::Port::setEncProfile(uint32_t profile)
{
    log << "setEncProfile( " << profile << " )" << endl;

    bool setProfile = false;
    struct v4l2_control control;

    memset(&control, 0, sizeof(control));

    if (io->getFormat() == to4cc("h264"))
    {
        setProfile = true;
        control.id = V4L2_CID_MPEG_VIDEO_H264_PROFILE;
        control.value = profile;
    }
    else if (io->getFormat() == to4cc("hevc"))
    {
        setProfile = true;
        control.id = V4L2_CID_MVE_VIDEO_H265_PROFILE;
        control.value = profile;
    }

    if (setProfile)
    {
        if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
        {
            throw Exception("Failed to set profile=%u for fmt: %u .", profile, io->getFormat());
        }
    }
    else
    {
        log << "Profile cannot be set for this codec" << endl;
    }
}

void Codec::Port::setEncLevel(uint32_t level)
{
    log << "setEncLevel( " << level << " )" << endl;

    bool setLevel = false;
    struct v4l2_control control;

    memset(&control, 0, sizeof(control));

    if (io->getFormat() == to4cc("h264"))
    {
        setLevel = true;
        control.id = V4L2_CID_MPEG_VIDEO_H264_LEVEL;
        control.value = level;
    }
    else if (io->getFormat() == to4cc("hevc"))
    {
        setLevel = true;
        control.id = V4L2_CID_MVE_VIDEO_H265_LEVEL;
        control.value = level;
    }

    if (setLevel)
    {
        if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
        {
            throw Exception("Failed to set level=%u for fmt: %u .", level, io->getFormat());
        }
    }
    else
    {
        log << "Level cannot be set for this codec" << endl;
    }
}

void Codec::Port::setEncConstrainedIntraPred(uint32_t cip)
{
    log << "setEncConstrainedIntraPred( " << cip << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_CONSTR_IPRED;
    control.value = cip;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set encoding cip=%u.", cip);
    }
}

void Codec::Port::setH264EncEntropyMode(uint32_t ecm)
{
    log << "setH264EncEntropyMode( " << ecm << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE;
    control.value = ecm;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set H264 ecm=%u.", ecm);
    }
}

void Codec::Port::setH264EncGOPType(uint32_t gop)
{
    log << "setH264EncGOPType( " << gop << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_GOP_TYPE;
    control.value = gop;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set H264 gop=%u.", gop);
    }
}

void Codec::Port::setEncMinQP(uint32_t minqp)
{
    log << "setH264EncMinQP( " << minqp << " )" << endl;

    struct v4l2_control control;
    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE;
    control.value = 1;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to enable/disable rate control.");
    }

    memset(&control, 0, sizeof(control));
    if (io->getFormat() == V4L2_PIX_FMT_H264) {
        control.id = V4L2_CID_MPEG_VIDEO_H264_MIN_QP;
    } else if (io->getFormat() == V4L2_PIX_FMT_HEVC) {
        control.id = V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP;
    } else if (io->getFormat() == V4L2_PIX_FMT_VP9) {
        control.id = V4L2_CID_MPEG_VIDEO_VPX_MIN_QP;
    } else {
        cerr<<"This format is not supported for min QP"<<endl;
    }
    control.value = minqp;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set H264 minqp=%u.", minqp);
    }
}

void Codec::Port::setEncMaxQP(uint32_t maxqp)
{
    log << "setH264EncMaxQP( " << maxqp << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE;
    control.value = 1;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to enable/disable rate control.");
    }

    memset(&control, 0, sizeof(control));
    if (io->getFormat() == V4L2_PIX_FMT_H264) {
        control.id = V4L2_CID_MPEG_VIDEO_H264_MAX_QP;
    } else if (io->getFormat() == V4L2_PIX_FMT_HEVC) {
        control.id = V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP;
    } else if (io->getFormat() == V4L2_PIX_FMT_VP9) {
        control.id = V4L2_CID_MPEG_VIDEO_VPX_MAX_QP;
    } else {
        cerr<<"This format is not supported for max QP"<<endl;
    }
    control.value = maxqp;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set H264 maxqp=%u.", maxqp);
    }
}

void Codec::Port::setEncFixedQP(uint32_t fqp)
{
    log << "setH264EncFixedQP( " << fqp << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_QP_FIXED;
    control.value = fqp;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set fixed qp=%u.", fqp);
    }
}

void Codec::Port::setEncFixedQPI(uint32_t fqp)
{
    log << "setH264EncFixedQPI( " << fqp << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    if (io->getFormat() == V4L2_PIX_FMT_H264) {
        control.id = V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP;
    } else if (io->getFormat() == V4L2_PIX_FMT_HEVC) {
        control.id = V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP;
    } else if (io->getFormat() == V4L2_PIX_FMT_VP9) {
        control.id = V4L2_CID_MPEG_VIDEO_VPX_I_FRAME_QP;
    } else {
        cerr<<"This format is not supported for I frames fixed QP"<<endl;
    }
    control.value = fqp;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set H264 I frame fqp=%u.", fqp);
    }
}

void Codec::Port::setEncFixedQPP(uint32_t fqp)
{
    log << "setH264EncFixedQPP( " << fqp << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    if (io->getFormat() == V4L2_PIX_FMT_H264) {
        control.id = V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP;
    } else if (io->getFormat() == V4L2_PIX_FMT_HEVC) {
        control.id = V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP;
    } else if (io->getFormat() == V4L2_PIX_FMT_VP9) {
        control.id = V4L2_CID_MPEG_VIDEO_VPX_P_FRAME_QP;
    } else {
        cerr<<"This format is not supported for P frames fixed QP"<<endl;
    }
    control.value = fqp;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set H264 P frame fqp=%u.", fqp);
    }

}

void Codec::Port::setEncFixedQPB(uint32_t fqp)
{
    log << "setH264EncFixedQPB( " << fqp << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    if (io->getFormat() == V4L2_PIX_FMT_H264) {
        control.id = V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP;
    } else if (io->getFormat() == V4L2_PIX_FMT_HEVC) {
        control.id = V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP;
    } else if (io->getFormat() == V4L2_PIX_FMT_VP9) {
        control.id = V4L2_CID_MVE_VIDEO_VPX_B_FRAME_QP;
    } else {
        cerr<<"This format is not supported for B frames fixed QP"<<endl;
    }
    control.value = fqp;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set H264 B frame fqp=%u.", fqp);
    }

}

void Codec::Port::setEncMinQPI(uint32_t nQpMinI)
{
    log << "setEncMinQPI( " << nQpMinI << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_MIN_QP_I;
    control.value = nQpMinI;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set nQpMinI=%u.", nQpMinI);
    }
}

void Codec::Port::setEncMaxQPI(uint32_t nQpMaxI)
{
    log << "setEncMaxQPI( " << nQpMaxI << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_MAX_QP_I;
    control.value = nQpMaxI;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set nQpMaxI=%u.", nQpMaxI);
    }
}

void Codec::Port::setEncInitQPI(uint32_t init_qpi)
{
    log << "setEncInitQPI( " << init_qpi << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_INIT_QP_I;
    control.value = init_qpi;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set init_qpi=%u.", init_qpi);
    }
}

void Codec::Port::setEncInitQPP(uint32_t init_qpp)
{
    log << "setEncInitQPP( " << init_qpp << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_INIT_QP_P;
    control.value = init_qpp;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set init_qpp=%u.", init_qpp);
    }
}

void Codec::Port::setEncSAOluma(uint32_t sao_luma_dis)
{
    log << "setEncSAOluma( " << sao_luma_dis << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_SAO_LUMA;
    control.value = sao_luma_dis;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set sao_luma_dis=%u.", sao_luma_dis);
    }
}

void Codec::Port::setEncSAOchroma(uint32_t sao_chroma_dis)
{
    log << "setEncSAOchroma( " << sao_chroma_dis << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_SAO_CHROMA;
    control.value = sao_chroma_dis;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set sao_chroma_dis=%u.", sao_chroma_dis);
    }
}

void Codec::Port::setEncQPDeltaIP(uint32_t qp_delta_i_p)
{
    log << "setEncQPDeltaIP( " << qp_delta_i_p << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_QP_DELTA_I_P;
    control.value = qp_delta_i_p;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set qp_delta_i_p=%u.", qp_delta_i_p);
    }
}

void Codec::Port::setEncRefRbEn(uint32_t ref_rb_en)
{
    log << "setEncRefRbEn( " << ref_rb_en << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_QP_REF_RB_EN;
    control.value = ref_rb_en;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set ref_rb_en=%u.", ref_rb_en);
    }
}

void Codec::Port::setEncRCClipTop(uint32_t rc_qp_clip_top)
{
    log << "setEncRCClipTop( " << rc_qp_clip_top << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_RC_CLIP_TOP;
    control.value = rc_qp_clip_top;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set rc_qp_clip_top=%u.", rc_qp_clip_top);
    }
}

void Codec::Port::setEncRCClipBot(uint32_t rc_qp_clip_bottom)
{
    log << "setEncRCClipBot( " << rc_qp_clip_bottom << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_RC_CLIP_BOT;
    control.value = rc_qp_clip_bottom;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set rc_qp_clip_bottom=%u.", rc_qp_clip_bottom);
    }
}

void Codec::Port::setEncQpmapClipTop(uint32_t qpmap_clip_top)
{
    log << "setEncQpmapClipTop( " << qpmap_clip_top << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_QP_MAP_CLIP_TOP;
    control.value = qpmap_clip_top;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set qpmap_clip_top=%u.", qpmap_clip_top);
    }
}

void Codec::Port::setEncQpmapClipBot(uint32_t qpmap_clip_bottom)
{
    log << "setEncQpmapClipBot( " << qpmap_clip_bottom << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_QP_MAP_CLIP_BOT;
    control.value = qpmap_clip_bottom;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set qpmap_clip_bottom=%u.", qpmap_clip_bottom);
    }
}

void Codec::Port::setPortProfiling(uint32_t enable)
{
    log << "setPortProfiling( " << enable << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_FW_PROFILING;
    control.value = enable;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set debug profiling=%u.", enable);
    }

}

void Codec::Port::setH264EncBandwidth(uint32_t bw)
{
    log << "setH264EncBandwidth( " << bw << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_BANDWIDTH_LIMIT;
    control.value = bw;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set H264 bw=%u.", bw);
    }
}

void Codec::Port::setHEVCEncEntropySync(uint32_t es)
{
    log << "setHEVCEncEntropySync( " << es << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_ENTROPY_SYNC;
    control.value = es;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set HEVC es=%u.", es);
    }
}

void Codec::Port::setHEVCEncTemporalMVP(uint32_t tmvp)
{
    log << "setHEVCEncTemporalMVP( " << tmvp << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_TEMPORAL_MVP;
    control.value = tmvp;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set HEVC tmvp=%u.", tmvp);
    }
}

void Codec::Port::setEncStreamEscaping(uint32_t sesc)
{
    log << "setEncStreamEscaping( " << sesc << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_STREAM_ESCAPING;
    control.value = sesc;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set encoding sesc=%u.", sesc);
    }
}

void Codec::Port::setEncHorizontalMVSearchRange(uint32_t hmvsr)
{
    log << "setEncHorizontalMVSearchRange( " << hmvsr << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MPEG_VIDEO_MV_H_SEARCH_RANGE;
    control.value = hmvsr;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set encoding hmvsr=%u.", hmvsr);
    }
}

void Codec::Port::setEncVerticalMVSearchRange(uint32_t vmvsr)
{
    log << "setEncVerticalMVSearchRange( " << vmvsr << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MPEG_VIDEO_MV_V_SEARCH_RANGE;
    control.value = vmvsr;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set encoding vmvsr=%u.", vmvsr);
    }
}

void Codec::Port::setVP9EncTileCR(uint32_t tcr)
{
    log << "setVP9EncTileCR( " << tcr << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_TILE_COLS;
    control.value = tcr;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set VP9 tile cols=%u.", tcr);
    }

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_TILE_ROWS;
    control.value = tcr;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set VP9 tile rows=%u.", tcr);
    }
}

void Codec::Port::setJPEGEncRefreshInterval(uint32_t r)
{
    log << "setJPEGEncRefreshInterval( " << r << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_JPEG_RESTART_INTERVAL;
    control.value = r;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set JPEG refresh interval=%u.", r);
    }
}

void Codec::Port::setJPEGHufftable(struct v4l2_mvx_huff_table * table)
{
    log << "setJPEGHufftable type( " << table->type<< ")" << endl;

    struct v4l2_mvx_huff_table huff_table;
    memcpy(&huff_table, table, sizeof(struct v4l2_mvx_huff_table));
    int ret = ioctl(fd, VIDIOC_S_MVX_HUFF_TABLE, &huff_table);
    if (ret != 0)
    {
        throw Exception("Failed to set JPEG huff table.");
    }
}

void Codec::Port::setSeamlessTarget(struct v4l2_mvx_seamless_target * seamless)
{
    log << "setSeamlessTarget seamless_mode( " << seamless->seamless_mode<< ")" << endl;
    log << "setSeamlessTarget target_width( " << seamless->target_width<< ")" << endl;
    log << "setSeamlessTarget target_height( " << seamless->target_height<< ")" << endl;
    log << "setSeamlessTarget target_stride( " << seamless->target_stride[0]<< ")" << endl;
    log << "setSeamlessTarget target_stride( " << seamless->target_stride[1]<< ")" << endl;
    log << "setSeamlessTarget target_stride( " << seamless->target_stride[2]<< ")" << endl;
    log << "setSeamlessTarget target_stride( " << seamless->target_size[0]<< ")" << endl;
    log << "setSeamlessTarget target_stride( " << seamless->target_size[1]<< ")" << endl;
    log << "setSeamlessTarget target_stride( " << seamless->target_size[2]<< ")" << endl;

    memcpy(&this->seamless, seamless, sizeof(struct v4l2_mvx_seamless_target));

    int ret = ioctl(fd, VIDIOC_S_MVX_SEAMLESS_TARGET, &this->seamless);
    if (ret != 0)
    {
        throw Exception("Failed to set Seamless Target.");
    }
}
void Codec::Port::setJPEGEncQuality(uint32_t q)
{
    log << "setJPEGEncQuality( " << q << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_JPEG_COMPRESSION_QUALITY;
    control.value = q;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set JPEG compression quality=%u.", q);
    }
}

void Codec::Port::setJPEGEncQualityLuma(uint32_t q)
{
    log << "setJPEGEncQualityLuma( " << q << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_JPEG_QUALITY_LUMA;
    control.value = q;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set JPEG compression luma quality=%u.", q);
    }
}

void Codec::Port::setJPEGEncQualityChroma(uint32_t q)
{
    log << "setJPEGEncQualityChroma( " << q << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_JPEG_QUALITY_CHROMA;
    control.value = q;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set JPEG compression chroma quality=%u.", q);
    }
}

void Codec::Port::setAdStats(int ad_stats)
{
    this->ad_stats =ad_stats;
}


void Codec::Port::setRotation(int rotation)
{
    this->rotation = rotation;
    io->setRotation(rotation);
}

void Codec::Port::setMirror(int mirror)
{
    this->mirror = mirror;
}

void Codec::Port::setDownScale(int scale)
{
    this->scale = scale;
}

void Codec::Port::setDSLFrame(int width, int height)
{
    log<<"setDSLFrame( "<<width<<" ,"<<height<<")"<<endl;

    struct v4l2_mvx_dsl_frame dsl_frame;
    memset(&dsl_frame, 0, sizeof(dsl_frame));
    dsl_frame.width = width;
    dsl_frame.height = height;
    int ret = ioctl(fd, VIDIOC_S_MVX_DSL_FRAME, &dsl_frame);
    if (ret != 0)
    {
        throw Exception("Failed to set DSL frame width/height.");
    }

    return;
}

void Codec::Port::setDSLRatio(int hor, int ver)
{
    log<<"setDSLRatio( "<<hor<<" ,"<<ver<<")"<<endl;

    struct v4l2_mvx_dsl_ratio dsl_ratio;
    memset(&dsl_ratio, 0, sizeof(dsl_ratio));
    dsl_ratio.hor = hor;
    dsl_ratio.ver = ver;
    int ret = ioctl(fd, VIDIOC_S_MVX_DSL_RATIO, &dsl_ratio);
    if (ret != 0)
    {
        throw Exception("Failed to set DSL frame hor/ver.");
    }

    return;
}

void Codec::Port::setDSLMode(int mode)
{
    log<<"setDSLMode("<<mode<<")"<<endl;
    int dsl_pos_mode = mode;
    int ret = ioctl(fd, VIDIOC_S_MVX_DSL_MODE, &dsl_pos_mode);
    if (ret != 0) {
        throw Exception("Failed to set dsl mode.");
    }
}

void Codec::Port::setDSLInterpMode(int mode)
{
    log << "setDSLInterpMode (" << mode << ")" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_DSL_INTERP_MODE;
    control.value = mode;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set dsl interpolation mode=%u.", control.value);
    }
}

void Codec::Port::setDisabledFeatures(int val)
{
    log << "setDisabledFeatures (" << val << ")" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_DISABLED_FEATURES;
    control.value = val;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set disabled features=%u.", control.value);
    }
}

void Codec::Port::setColorConversion(uint32_t mode)
{
    log << "setColorConversion (" << mode << ")" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_COLOR_CONVERSION;
    control.value = mode;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to setColorConversion mode =%u.", control.value);
    }
}

void Codec::Port::setRGBToYUVMode(uint32_t mode)
{
    log << "setRGBToYUVMode (" << mode << ")" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_RGB_TO_YUV_MODE;
    control.value = mode;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to setRGBToYUVMode mode =%u.", control.value);
    }
}

void Codec::Port::setCustColorConvCoef(struct v4l2_mvx_color_conv_coef * color_conv_coef)
{
    log << "setCustColorConvCoef coef[0][0]( " << color_conv_coef->coef[0][0]<< ")" << endl;
    log << "setCustColorConvCoef coef[0][1]( " << color_conv_coef->coef[0][1]<< ")" << endl;
    log << "setCustColorConvCoef coef[0][2]( " << color_conv_coef->coef[0][2]<< ")" << endl;
    log << "setCustColorConvCoef coef[1][0]( " << color_conv_coef->coef[1][0]<< ")" << endl;
    log << "setCustColorConvCoef coef[1][1]( " << color_conv_coef->coef[1][1]<< ")" << endl;
    log << "setCustColorConvCoef coef[1][2]( " << color_conv_coef->coef[1][2]<< ")" << endl;
    log << "setCustColorConvCoef coef[2][0]( " << color_conv_coef->coef[2][0]<< ")" << endl;
    log << "setCustColorConvCoef coef[2][1]( " << color_conv_coef->coef[2][1]<< ")" << endl;
    log << "setCustColorConvCoef coef[2][2]( " << color_conv_coef->coef[2][2]<< ")" << endl;
    log << "setCustColorConvCoef offset[0] ( " << color_conv_coef->offset[0]<< ")" << endl;
    log << "setCustColorConvCoef offset[1] ( " << color_conv_coef->offset[1]<< ")" << endl;
    log << "setCustColorConvCoef offset[2] ( " << color_conv_coef->offset[2]<< ")" << endl;

    int ret = ioctl(fd, VIDIOC_S_MVX_COLOR_CONV_COEF, color_conv_coef);
    if (ret != 0)
    {
        throw Exception("Failed to set Cust Color Conv Coef.");
    }
}

void Codec::Port::setRGBConvertYUV(struct v4l2_mvx_rgb2yuv_color_conv_coef * color_conv_coef)
{
    int ret = ioctl(fd, VIDIOC_S_MVX_RGB2YUV_COLOR_CONV_COEF, color_conv_coef);
    if (ret != 0)
    {
        throw Exception("Failed to set RGB To YUV Conv Coef.");
    }
}

void Codec::Port::setDecDstCrop(struct v4l2_mvx_crop_cfg * dst_crop)
{
    log << "setDecDstCrop crop_en( " << dst_crop->crop_en<< ")" << endl;
    log << "setDecDstCrop       x( " << dst_crop->x<< ")" << endl;
    log << "setDecDstCrop       y( " << dst_crop->y<< ")" << endl;
    log << "setDecDstCrop   width( " << dst_crop->width<< ")" << endl;
    log << "setDecDstCrop  height( " << dst_crop->height<< ")" << endl;

    int ret = ioctl(fd, VIDIOC_S_MVX_DEC_DST_CROP, dst_crop);
    if (ret != 0)
    {
        throw Exception("Failed to setDecDstCrop.");
    }
}


void Codec::Port::setVisibleWidth(uint32_t v_width)
{
    log<<"setVisibleWidth("<<v_width<<")"<<endl;
    struct v4l2_control control;
    if (v_width > io->getWidth()) {
        cerr<<"visible width should not greater than frame width!"<<endl;
        return;
    }
    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_VISIBLE_WIDTH;
    control.value = v_width;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set visible width =%u.", v_width);
    }

}

void Codec::Port::setVisibleHeight(uint32_t v_height)
{
    log<<"setVisibleHeight("<<v_height<<")"<<endl;
    struct v4l2_control control;
    if (v_height > io->getHeight()) {
        cerr<<"visible height should not greater than frame height!"<<endl;
        return;
    }

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_VISIBLE_HEIGHT;
    control.value = v_height;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set visible height =%u.", v_height);
    }
}

void Codec::Port::setLongTermRef(uint32_t mode, uint32_t period)
{
    log<<"setLongTermRef( "<<mode<<" ,"<<period<<")"<<endl;
    struct v4l2_mvx_long_term_ref ltr;
    memset(&ltr, 0, sizeof(ltr));
    ltr.mode = mode;
    ltr.period = period;
    int ret = ioctl(fd, VIDIOC_S_MVX_LONG_TERM_REF, &ltr);
    if (ret != 0)
    {
        throw Exception("Failed to set long term mode/period.");
    }

}

void Codec::Port::setMiniHeight(uint32_t height)
{
    log<<"set miniFrameBuffer cnt ( "<<height<<" )"<<endl;
    mini_frame_height = height;
    int ret = ioctl(fd, VIDIOC_S_MVX_MINI_FRAME_HEIGHT, &mini_frame_height);
    if (ret != 0) {
        throw Exception("Failed to mini Frame Buf cnt.");
    }
}

void Codec::Port::setFrameCount(int frames)
{
    this->frames_count = frames;
}

void Codec::Port::setCropLeft(int left){
    log << "setCropLeft( " << left << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_CROP_LEFT;
    control.value = left;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set crop left=%u.", left);
    }
}

void Codec::Port::setCropRight(int right){
    log << "setCropRight( " << right << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_CROP_RIGHT;
    control.value = right;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set crop right=%u.", right);
    }
}

void Codec::Port::setCropTop(int top){
    log << "setCropTop( " << top << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_CROP_TOP;
    control.value = top;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set crop top=%u.", top);
    }
}

void Codec::Port::setCropBottom(int bottom){
    log << "setCropBottom( " << bottom << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_CROP_BOTTOM;
    control.value = bottom;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set crop bottom=%u.", bottom);
    }
}

void Codec::Port::setStatsMode(int mode, int index){
    int stats_mode = mode;

    struct v4l2_buffer_param_enc_stats stats;
    memset(&stats, 0, sizeof(struct v4l2_buffer_param_enc_stats));
    stats.pic_index_or_mb_size = index;
    stats.mms_buffer_size = stats_mode & 0x1? mms_buffer_size : 0;
    stats.bitcost_buffer_size = stats_mode & 0x2? bitcost_buffer_size : 0;
    stats.qp_buffer_size = stats_mode & 0x4? qp_buffer_size : 0;
    stats.flags |= stats_mode & 0x1? V4L2_BUFFER_ENC_STATS_FLAG_MMS     : 0;
    stats.flags |= stats_mode & 0x2? V4L2_BUFFER_ENC_STATS_FLAG_BITCOST : 0;
    stats.flags |= stats_mode & 0x4? V4L2_BUFFER_ENC_STATS_FLAG_QP      : 0;

    int ret = ioctl(fd, VIDIOC_S_MVX_STATS_MODE, &stats);
    if (ret != 0)
    {
        throw Exception("Failed to set color description.");
    }

    return;
}

void Codec::Port::setOsdCfg(struct v4l2_osd_config osd){
    log << "setOsdCfg, for pic_index " << osd.pic_index << endl;
    struct v4l2_osd_config osd_cfg = osd;
    int ret = ioctl(fd, VIDIOC_S_MVX_OSD_CONFIG, &osd_cfg);
    if (ret != 0)
    {
        throw Exception("Failed to set osd cfg.");
    }
}

void Codec::Port::setEncOSDinfo(struct v4l2_osd_info* info){
    log << "setOsdinfo, osd 1 size " << info->width_osd[0] <<"x"<< info->height_osd[0] << endl;
    log << "setOsdinfo, osd 2 size " << info->width_osd[1] <<"x"<< info->height_osd[1] << endl;
    int ret = ioctl(fd, VIDIOC_S_MVX_OSD_INFO, info);
    if (ret != 0)
    {
        throw Exception("Failed to set osd info.");
    }
}

void Codec::Port::setVuiColourDesc(struct v4l2_mvx_color_desc *color){
    log << "setVuiColourDesc( " << color->content.luminance_average << ",";
    log << color->content.luminance_max << ")" << endl;

    int ret = ioctl(fd, VIDIOC_S_MVX_COLORDESC, color);
    if (ret != 0)
    {
        throw Exception("Failed to set color description.");
    }

    return;
}

void Codec::Port::setSeiUserData(struct v4l2_sei_user_data *sei_user_data){
    log << "setSeiUserData( " << sei_user_data->user_data<< ")" << endl;

    int ret = ioctl(fd, VIDIOC_S_MVX_SEI_USERDATA, sei_user_data);
    if (ret != 0)
    {
        throw Exception("Failed to set color description.");
    }

    return;

}

void Codec::Port::setHRDBufferSize(int size) {
    log << "setHRDBufferSize( " << size << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_HRD_BUFFER_SIZE;
    control.value = size;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set crop bottom=%u.", size);
    }
}

void Codec::Port::setFrameStride(size_t* stride){
    __stride[0] = stride[0];
    __stride[1] = stride[1];
    __stride[2] = stride[2];
}

void Codec::Port::setRcBitIMode(uint32_t mode)
{
    log << "set mode for I frame of rate control( " << mode << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_RC_I_MODE;
    control.value = mode;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set rc bit i frame mode=%u.", mode);
    }
}

void Codec::Port::setChangePos(uint32_t pos)
{
    log << "set change pos for jpeg rc( " << pos << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_CHANGE_POS;
    control.value = pos;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set jpeg rc change pos=%u.", pos);
    }
}

void Codec::Port::setRcBitRationI(uint32_t ratio)
{
    log << "set ratio for I frame of rate control( " << ratio << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_RC_I_RATIO;
    control.value = ratio;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set rc i frame ratio=%u.", ratio);
    }
}

void Codec::Port::setMultiSPSPPS(uint32_t sps_pps)
{
    log << "Support multi SPS PSS for h264 and hevc ( " << sps_pps << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_MULTI_SPS_PPS;
    control.value = sps_pps;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to support multi SPS PSS=%u.", sps_pps);
    }
}

void Codec::Port::setEnableVisual(uint32_t enable)
{
    log << "Enable Visual ( " << enable << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_ENABLE_VISUAL;
    control.value = enable;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to enable visual =%u.", enable);
    }
}

void Codec::Port::setAdaptiveIntraBlock(uint32_t enable)
{
    log << "setAdaptiveIntraBlock (" << enable << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_ENABLE_ADAPTIVE_INTRA_BLOCK;
    control.value = enable;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to adaptive intra block =%u.", enable);
    }
}

void Codec::Port::setEnableSCD(uint32_t scd_enable)
{
    log << "Enable SCD ( " << scd_enable << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_SCD_ENABLE;
    control.value = scd_enable;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to enable SCD =%u.", scd_enable);
    }
}

void Codec::Port::setScdPercent(uint32_t scd_percent)
{
    log << "SCD Percent ( " << scd_percent << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_SCD_PERCENT;
    control.value = scd_percent;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set SCD percent =%u.", scd_percent);
    }
}

void Codec::Port::setScdThreshold(uint32_t scd_threshold)
{
    log << "SCD Threshold ( " << scd_threshold << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_SCD_THRESHOLD;
    control.value = scd_threshold;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set scd threshold =%u.", scd_threshold);
    }
}

void Codec::Port::setEnableAQSsim(uint32_t aq_ssim_en)
{
    log << "Enable AQ SSIM ( " << aq_ssim_en << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_AQ_SSIM_EN;
    control.value = aq_ssim_en;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to enable AQ SSIM =%u.", aq_ssim_en);
    }
}

void Codec::Port::setAQNegRatio(uint32_t aq_neg_ratio)
{
    log << "AQ Negaive Ratio ( " << aq_neg_ratio << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_AQ_NEG_RATIO;
    control.value = aq_neg_ratio;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set AQ negaive ratio =%u.", aq_neg_ratio);
    }
}

void Codec::Port::setAQPosRatio(uint32_t aq_pos_ratio)
{
    log << "AQ Positive Ratio ( " << aq_pos_ratio << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_AQ_POS_RATIO;
    control.value = aq_pos_ratio;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set AQ positive ratio =%u.", aq_pos_ratio);
    }
}

void Codec::Port::setAQQPDeltaLmt(uint32_t aq_qpdelta_lmt)
{
    log << "AQ QPDelta LMT ( " << aq_qpdelta_lmt << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_AQ_QPDELTA_LMT;
    control.value = aq_qpdelta_lmt;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to set AQ QPDelta LMT =%u.", aq_qpdelta_lmt);
    }
}

void Codec::Port::setAQInitFrmAvgSvar(uint32_t aq_init_frm_avg_svar)
{
    log << "Initial Frame Variance ( " << aq_init_frm_avg_svar << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_AQ_INIT_FRM_AVG_SVAR;
    control.value = aq_init_frm_avg_svar;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to initial frame variance =%u.", aq_init_frm_avg_svar);
    }
}

void Codec::Port::setIntermediateBufSize(uint32_t size)
{
    log << "setIntermediateBufSize( " << size << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_INTER_MED_BUF_SIZE;
    control.value = size;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to setIntermediateBufSize=%u.", size);
    }
}

void Codec::Port::setSvct3Level1Period(uint32_t period)
{
    log << "setSvct3Level1Period( " << period << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_SVCT3_LEVEL1_PERIOD;
    control.value = period;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to setSvct3Level1Period=%u.", period);
    }
}

void Codec::Port::setGopResetPframes(int pframes)
{
    log << "setGopResetPframes( " << pframes << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_GOP_RESET_PFRAMES;
    control.value = pframes;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to setGopResetPframes=%u.", pframes);
    }

}

void Codec::Port::setLtrResetPeriod(int period)
{
    log << "setLtrResetPeriod( " << period << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_LTR_RESET_PERIOD;
    control.value = period;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to setLtrResetPeriod=%u.", period);
    }

}

void Codec::Port::setGDRnumber(uint32_t numbder)
{
    log << "setGDRnumber( " << numbder << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_GDR_NUMBER;
    control.value = numbder;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to setGDRnumber=%u.", numbder);
    }
}

void Codec::Port::setGDRperiod(uint32_t period)
{
    log << "setGDRperiod( " << period << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_GDR_PERIOD;
    control.value = period;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to setGDRperiod=%u.", period);
    }
}

void Codec::Port::setForcedUVvalue(uint32_t uv_value)
{
    log << "setForcedUVvalue( " << uv_value << " )" << endl;

    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_MVE_VIDEO_FORCED_UV_VALUE;
    control.value = uv_value;

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control))
    {
        throw Exception("Failed to setForcedUVvalue=%u.", uv_value);
    }
}

void Codec::Port::setEncSrcCrop(struct v4l2_mvx_crop_cfg * src_crop)
{
    log << "setEncSrcCrop crop_en( " << src_crop->crop_en<< ")" << endl;
    log << "setEncSrcCrop       x( " << src_crop->x<< ")" << endl;
    log << "setEncSrcCrop       y( " << src_crop->y<< ")" << endl;
    log << "setEncSrcCrop   width( " << src_crop->width<< ")" << endl;
    log << "setEncSrcCrop  height( " << src_crop->height<< ")" << endl;

    int ret = ioctl(fd, VIDIOC_S_MVX_ENC_SRC_CROP, src_crop);
    if (ret != 0)
    {
        throw Exception("Failed to setEncSrcCrop.");
    }
}

void Codec::Port::setPortStatsSize(uint32_t mms, uint32_t bc, uint32_t qp)
{
    mms_buffer_size = mms;
    bitcost_buffer_size = bc;
    qp_buffer_size = qp;
}

void Codec::runPoll()
{
    bool eos = false;

    while (!eos)
    {
        struct pollfd p = {
            .fd = fd, .events = POLLPRI
        };

        if (input.pending > 0)
        {
            p.events |= POLLOUT;
        }

        if (output.pending > 0)
        {
            p.events |= POLLIN;
        }

        int ret = poll(&p, 1, 60000);
        if (ret < 0)
        {
            throw Exception("Poll returned error code.");
        }

        if (p.revents & POLLERR)
        {
            throw Exception("Poll returned error event.");
        }

        if (ret == 0)
        {
            throw Exception("Poll timed out.");
        }

        if (p.revents & POLLOUT)
        {
            input.handleBuffer();
        }
        if (p.revents & POLLIN)
        {
            if (csweo)
            {
                log << "Changing settings while encoding." << endl;
                if (fps != 0)
                {
                    output.setEncFramerate(fps);
                    fps = 0;
                }
                if (bps != 0)
                {
                    /* output.setEncBitrate(bps); */
                    output.setEncBitrate(0); /* only for coverage */
                    bps = 0;
                }
                /* Set maxQP before minQP, otherwise FW rejects */
                if (maxqp != 0)
                {
                    output.setEncMaxQP(maxqp);
                    maxqp = 0;
                }
                if (minqp != 0)
                {
                    output.setEncMinQP(minqp);
                    minqp = 0;
                }
                if (fixedqp != 0)
                {
                    output.setEncFixedQP(fixedqp);
                    fixedqp = 0;
                }

                csweo = false;
            }

            eos = output.handleBuffer();
        }
        if (p.revents & POLLPRI)
        {
            handleEvent();
        }
    }
}

void Codec::runThreads()
{
    int ret;
    void *retval;

    ret = pthread_create(&input.tid, NULL, runThreadInput, this);
    if (ret != 0)
    {
        throw Exception("Failed to create input thread.");
    }

    ret = pthread_create(&output.tid, NULL, runThreadOutput, this);
    if (ret != 0)
    {
        throw Exception("Failed to create output thread.");
    }

    pthread_join(input.tid, &retval);
    pthread_join(output.tid, &retval);
}

void *Codec::runThreadInput(void *arg)
{
    Codec *_this = static_cast<Codec *>(arg);
    bool eos = false;

    while (!eos)
        eos = _this->input.handleBuffer();

    return NULL;
}

void *Codec::runThreadOutput(void *arg)
{
    Codec *_this = static_cast<Codec *>(arg);
    bool eos = false;

    while (!eos)
        eos = _this->output.handleBuffer();

    return NULL;
}

bool Codec::Port::handleBuffer()
{
    Buffer &buffer = dequeueBuffer();
    v4l2_buffer &b = buffer.getBuffer();
    if (io->getDir() == 0 && isInputThread) {
        appendInputBufferIdx(&input_producer_mutex, &input_producer_cond, &input_producer_queue, b.index);
    }

    buffer.setFrameProcessCount(frames_processed);
    io->finalize(buffer);
    if (io->eof())
    {
        if (tryDecStop)
        {
            sendDecStopCommand();
        }
        return true;
    }

    /* EOS on capture port. */
    if (!V4L2_TYPE_IS_OUTPUT(b.type) && b.flags & V4L2_BUF_FLAG_LAST)
    {
        log << "Capture EOS." << endl;
        return true;
    }

    /* Resolution change. we should only handle this on decode output:V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE*/
    if (!V4L2_TYPE_IS_OUTPUT(b.type) && V4L2_TYPE_IS_MULTIPLANAR(b.type) &&
            (getBytesUsed(b) == 0 ||
            (b.flags & V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT) != V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT) &&
            (b.flags & V4L2_BUF_FLAG_ERROR) == 0)
    {
        v4l2_format fmt = format;
        getFormat();
        bool isResChange = true;
        if (V4L2_TYPE_IS_MULTIPLANAR(type))
        {
            struct v4l2_pix_format_mplane &f = fmt.fmt.pix_mp;
            isResChange = ((f.width != format.fmt.pix_mp.width) ||
                    (f.height != format.fmt.pix_mp.height) )&&
                    (f.width * f.height) < (format.fmt.pix_mp.height * format.fmt.pix_mp.width);
        } else {
            struct v4l2_pix_format &f = fmt.fmt.pix;
            isResChange = ((f.width != format.fmt.pix.width) ||
                    (f.height != format.fmt.pix.height)) &&
                    (f.width * f.height) < (format.fmt.pix.height * format.fmt.pix.width);
        }
        if ((b.flags & V4L2_BUF_FLAG_MVX_BUFFER_NEED_REALLOC) == V4L2_BUF_FLAG_MVX_BUFFER_NEED_REALLOC) {
            log << "Resolution changed:" << isResChange <<endl;
            handleResolutionChange();
            return false;
        }
    }

    /* Remove vendor custom flags. */
    //decoder specfied frames count to be processed
    if (io->getDir() == 1 && V4L2_TYPE_IS_MULTIPLANAR(b.type)
            && (b.flags & V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT) == V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT) {
        frames_processed++;
    }
    buffer.resetVendorFlags();
    if (io->getDir() == 1 && frames_count > 0 && frames_processed >= frames_count) {
        buffer.clearBytesUsed();
        buffer.setEndOfStream(true);
        queueBuffer(buffer);
        return true;
    } else {
        if (io->getDir() == 0 && isInputThread) {
            uint32_t index = getInputBufferIdx(&input_consumer_mutex, &input_consumer_cond, &input_consumer_queue);
            Buffer& buffer_input = *(buffers.at(index));
            buffer_input.setEndOfFrame(io->eof());
            queueBuffer(buffer_input);
            if (io->eof())
            {
                sendEncStopCommand();
            }
            return false;
        }
        io->prepare(buffer);
        buffer.setEndOfStream(io->eof());
    }
    queueBuffer(buffer);

    if (io->eof())
    {
        sendEncStopCommand();
    }

    return false;
}

void Codec::Port::handleResolutionChange()
{
    streamoff();
    getFormat();
    allocateBuffers(0, memory_type_port);
    allocateBuffers(getBufferCount(), memory_type_port);
    queueBuffers();
    //frames_processed = 0;
    streamon();
}

bool Codec::handleEvent()
{
    struct v4l2_event event;
    int ret;

    ret = ioctl(fd, VIDIOC_DQEVENT, &event);
    if (ret != 0)
    {
        throw Exception("Failed to dequeue event.");
    }

    log << "Event. type=" << event.type << "." << endl;

    if (event.type == V4L2_EVENT_MVX_COLOR_DESC)
    {
        v4l2_mvx_color_desc color = getColorDesc();
        printColorDesc(color);
    }

    if (event.type == V4L2_EVENT_EOS)
    {
        return true;
    }

    return false;
}

void Codec::getStride(uint32_t format, size_t &nplanes, size_t stride[3][2])
{
    switch (format)
    {
        case V4L2_PIX_FMT_YUV420M:
            nplanes = 3;
            stride[0][0] = 2;
            stride[0][1] = 2;
            stride[1][0] = 1;
            stride[1][1] = 1;
            stride[2][0] = 1;
            stride[2][1] = 1;
            break;
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
            nplanes = 2;
            stride[0][0] = 2;
            stride[0][1] = 2;
            stride[1][0] = 2;
            stride[1][1] = 1;
            stride[2][0] = 0;
            stride[2][1] = 0;
            break;
        case V4L2_PIX_FMT_P010:
        case V4L2_PIX_FMT_YUV420_2P_10:
            nplanes = 2;
            stride[0][0] = 4;
            stride[0][1] = 2;
            stride[1][0] = 4;
            stride[1][1] = 1;
            stride[2][0] = 0;
            stride[2][1] = 0;
            break;
        case V4L2_PIX_FMT_Y0L2:
        case V4L2_PIX_FMT_AQB1:
            nplanes = 1;
            stride[0][0] = 8;
            stride[0][1] = 1;
            stride[1][0] = 0;
            stride[1][1] = 0;
            stride[2][0] = 0;
            stride[2][1] = 0;
            break;
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
            nplanes = 1;
            stride[0][0] = 4;
            stride[0][1] = 2;
            stride[1][0] = 0;
            stride[1][1] = 0;
            stride[2][0] = 0;
            stride[2][1] = 0;
            break;
        case V4L2_PIX_FMT_Y210:
        case V4L2_PIX_FMT_YUV422_1P_10:
        case DRM_FORMAT_ARGB8888:
        case DRM_FORMAT_ABGR8888:
        case DRM_FORMAT_RGBA8888:
        case DRM_FORMAT_BGRA8888:
            nplanes = 1;
            stride[0][0] = 8;
            stride[0][1] = 2;
            stride[1][0] = 0;
            stride[1][1] = 0;
            stride[2][0] = 0;
            stride[2][1] = 0;
            break;
        case V4L2_PIX_FMT_RGB24:
        case V4L2_PIX_FMT_BGR24:
            nplanes = 1;
            stride[0][0] = 6;
            stride[0][1] = 2;
            stride[1][0] = 0;
            stride[1][1] = 0;
            stride[2][0] = 0;
            stride[2][1] = 0;
            break;
        case V4L2_PIX_FMT_RGB_3P:
        case V4L2_PIX_FMT_YUV444M:
            nplanes = 3;
            stride[0][0] = 2;
            stride[0][1] = 2;
            stride[1][0] = 2;
            stride[1][1] = 2;
            stride[2][0] = 2;
            stride[2][1] = 2;
            break;
        case V4L2_PIX_FMT_GREY:
            nplanes = 1;
            stride[0][0] = 2;
            stride[0][1] = 2;
            stride[1][0] = 0;
            stride[1][1] = 0;
            stride[2][0] = 0;
            stride[2][1] = 0;
            break;
        case V4L2_PIX_FMT_Y10_LE:
            nplanes = 1;
            stride[0][0] = 4;
            stride[0][1] = 2;
            stride[1][0] = 0;
            stride[1][1] = 0;
            stride[2][0] = 0;
            stride[2][1] = 0;
            break;
        case V4L2_PIX_FMT_YUV444_10:
            nplanes = 3;
            stride[0][0] = 4;
            stride[0][1] = 2;
            stride[1][0] = 4;
            stride[1][1] = 2;
            stride[2][0] = 4;
            stride[2][1] = 2;
            break;
        case V4L2_PIX_FMT_YUV420_I420_10:
            nplanes = 3;
            stride[0][0] = 4;
            stride[0][1] = 2;
            stride[1][0] = 2;
            stride[1][1] = 1;
            stride[2][0] = 2;
            stride[2][1] = 1;
            break;
        case V4L2_PIX_FMT_ARGB555:
        case V4L2_PIX_FMT_ARGB444:
        case V4L2_PIX_FMT_RGB565:
            nplanes = 1;
            stride[0][0] = 4;
            stride[0][1] = 2;
            stride[1][0] = 0;
            stride[1][1] = 0;
            stride[2][0] = 0;
            stride[2][1] = 0;
            break;
        default:
            throw Exception("Unsupported buffer format.");
    }
}

size_t Codec::getSize(uint32_t format, size_t width, size_t height,
                      size_t strideAlign, size_t &nplanes, size_t stride[3], size_t size[3], size_t heights[3])
{
    size_t s[3][2];
    size_t frameSize = 0;
    size_t tmp_witdh, tmp_height;
    getStride(format, nplanes, s);

    for (int i = 0; i < 3; ++i)
    {
        tmp_witdh = i == 0? width : roundUp(width, 2);
        tmp_height = i == 0? height : roundUp(height, 2);
        stride[i] = max(stride[i], roundUp(divRoundUp(tmp_witdh * s[i][0], 2), strideAlign));
        heights[i] = divRoundUp(tmp_height * s[i][1], 2);
        size[i] = stride[i] * heights[i];//divRoundUp(height * stride[i] * s[i][1], 4);
        frameSize += size[i];
    }

    return frameSize;
}

Decoder::Decoder(const char *dev, Input &input, Output &output, bool nonblock, ostream &log) :
    Codec(dev, input, V4L2_BUF_TYPE_VIDEO_OUTPUT, output,
          V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, log, nonblock)
{}

void Decoder::setH264IntBufSize(uint32_t ibs)
{
    output.setH264DecIntBufSize(ibs);
}

void Decoder::setInterlaced(bool interlaced)
{
    output.setInterlaced(interlaced);
}

void Decoder::setFrameReOrdering(uint32_t fro)
{
    output.setDecFrameReOrdering(fro);
}

void Decoder::setIgnoreStreamHeaders(uint32_t ish)
{
    output.setDecIgnoreStreamHeaders(ish);
}

void Decoder::tryStopCmd(bool tryStop)
{
    input.tryDecStopCmd(tryStop);
}

void Decoder::setNaluFormat(int nalu)
{
    input.io->setNaluFormat(nalu);
    naluFmt = nalu;
}

void Decoder::setAdStats(int ad_stats)
{
    output.setAdStats(ad_stats);
}

void Decoder::setRotation(int rotation)
{
    output.setRotation(rotation);
}

void Decoder::setDownScale(int scale)
{
    output.setDownScale(scale);
}

void Decoder::setFrameCount(int frames) {
    output.setFrameCount(frames);
}

void Decoder::setDSLFrame(int width, int height){
    output.setDSLFrame(width, height);
}

void Decoder::setDSLRatio(int hor, int ver){
    output.setDSLRatio(hor, ver);
}

void Decoder::setDSLMode(int mode){
    output.setDSLMode(mode);
}

void Decoder::setDSLInterpMode(int mode){
    output.setDSLInterpMode(mode);
}

void Decoder::setDisabledFeatures(int val){
    output.setDisabledFeatures(val);
}

void Decoder::setColorConversion(uint32_t mode){
    output.setColorConversion(mode);
}

void Decoder::setCustColorConvCoef(struct v4l2_mvx_color_conv_coef *coef){
    output.setCustColorConvCoef(coef);
}

void Decoder::setDecDstCrop(struct v4l2_mvx_crop_cfg *dst_crop){
    output.setDecDstCrop(dst_crop);
}

void Decoder::setStride(size_t *stride){
    output.setFrameStride(stride);
}


void Decoder::setSeamlessTarget(uint32_t format, struct v4l2_mvx_seamless_target *seamless)
{
    size_t nplanes;
    size_t stride[3];
    size_t size[3];
    size_t heights[3];
    unsigned int framesize;

    memset(stride,0,sizeof(stride));
    memset(size,0,sizeof(size));
    memset(heights,0,sizeof(heights));
    framesize = Codec::getSize(format, (size_t)seamless->target_width, (size_t)seamless->target_height,(size_t)16, nplanes, stride, size, heights);
    seamless->target_stride[0] = stride[0];
    seamless->target_stride[1] = stride[1];
    seamless->target_stride[2] = stride[2];

    seamless->target_size[0]  = size[0];
    seamless->target_size[1]  = size[1];
    seamless->target_size[2]  = size[2];

    cout<<"setSeamlessTarget framesize= %u" <<framesize<<endl;
    output.setSeamlessTarget(seamless);
}
void Decoder::setFrameBufCnt(uint32_t count)
{
    output.setBufferCnt(count);
}

void Decoder::setBitBufCnt(uint32_t count)
{
    input.setBufferCnt(count);
}

Encoder::Encoder(const char *dev, Input &input, Output &output, bool nonblock, ostream &log) :
    Codec(dev, input, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, output,
          V4L2_BUF_TYPE_VIDEO_CAPTURE, log, nonblock)
{
    //this->output.setEncFramerate(30 << 16);
    //this->output.setEncBitrate(input.getWidth() * input.getHeight() * 30 / 2);
}

void Encoder::changeSWEO(uint32_t csweo)
{
    this->csweo = (csweo == 1);
}

void Encoder::setFramerate(uint32_t fps)
{
    if (!csweo)
    {
        output.setEncFramerate(fps);
    }
    else
    {
        this->fps = fps;
    }
}

void Encoder::setBitrate(uint32_t bps)
{
    if (!csweo)
    {
        output.setEncBitrate(bps);
    }
    else
    {
        output.setEncBitrate(bps);
        this->bps = bps - 500;
    }
}

void Encoder::setPFrames(uint32_t pframes)
{
    output.setEncPFrames(pframes);
}

void Encoder::setBFrames(uint32_t bframes)
{
    output.setEncBFrames(bframes);
}

void Encoder::setSliceSpacing(uint32_t spacing)
{
    output.setEncSliceSpacing(spacing);
}

void Encoder::setHorizontalMVSearchRange(uint32_t hmvsr)
{
    output.setEncHorizontalMVSearchRange(hmvsr);
}

void Encoder::setVerticalMVSearchRange(uint32_t vmvsr)
{
    output.setEncVerticalMVSearchRange(vmvsr);
}

void Encoder::setEncForceChroma(uint32_t fmt)
{
    input.setEncForceChroma(fmt);
}

void Encoder::setEncBitdepth(uint32_t bd)
{
    input.setEncBitdepth(bd);
}

void Encoder::setH264IntraMBRefresh(uint32_t period)
{
    output.setH264EncIntraMBRefresh(period);
}

void Encoder::setProfile(uint32_t profile)
{
    output.setEncProfile(profile);
}

void Encoder::setLevel(uint32_t level)
{
    output.setEncLevel(level);
}

void Encoder::setConstrainedIntraPred(uint32_t cip)
{
    output.setEncConstrainedIntraPred(cip);
}

void Encoder::setH264EntropyCodingMode(uint32_t ecm)
{
    output.setH264EncEntropyMode(ecm);
}

void Encoder::setH264GOPType(uint32_t gop)
{
    output.setH264EncGOPType(gop);
}

void Encoder::setEncMinQP(uint32_t minqp)
{
    if (!csweo)
    {
        output.setEncMinQP(minqp);
    }
    else
    {
        this->minqp = minqp;
    }
}

void Encoder::setEncMaxQP(uint32_t maxqp)
{
    if (!csweo)
    {
        output.setEncMaxQP(maxqp);
    }
    else
    {
        this->maxqp = maxqp;
    }
}

void Encoder::setEncFixedQP(uint32_t fqp)
{
    if (!csweo)
    {
        output.setEncFixedQP(fqp);
    }
    else
    {
        output.setEncFixedQP(fqp);
        this->fixedqp = fqp + 2;
    }
}

void Encoder::setEncFixedQPI(uint32_t fqp)
{
    output.setEncFixedQPI(fqp);
}

void Encoder::setEncFixedQPP(uint32_t fqp)
{
    output.setEncFixedQPP(fqp);
}

void Encoder::setEncFixedQPB(uint32_t fqp)
{
    output.setEncFixedQPB(fqp);
}

void Encoder::setEncMinQPI(uint32_t nQpMinI)
{
    output.setEncMinQPI(nQpMinI);
}

void Encoder::setEncMaxQPI(uint32_t nQpMaxI)
{
    output.setEncMaxQPI(nQpMaxI);
}

void Encoder::setEncInitQPI(uint32_t init_qpi)
{
    output.setEncInitQPI(init_qpi);
}

void Encoder::setEncInitQPP(uint32_t init_qpp)
{
    output.setEncInitQPP(init_qpp);
}

void Encoder::setEncSAOluma(uint32_t sao_luma_dis)
{
    output.setEncSAOluma(sao_luma_dis);
}

void Encoder::setEncSAOchroma(uint32_t sao_chroma_dis)
{
    output.setEncSAOchroma(sao_chroma_dis);
}

void Encoder::setEncQPDeltaIP(uint32_t qp_delta_i_p)
{
    output.setEncQPDeltaIP(qp_delta_i_p);
}

void Encoder::setEncRefRbEn(uint32_t ref_rb_en)
{
    output.setEncRefRbEn(ref_rb_en);
}

void Encoder::setEncRCClipTop(uint32_t rc_qp_clip_top)
{
    output.setEncRCClipTop(rc_qp_clip_top);
}

void Encoder::setEncRCClipBot(uint32_t rc_qp_clip_bottom)
{
    output.setEncRCClipBot(rc_qp_clip_bottom);
}

void Encoder::setEncQpmapClipTop(uint32_t qpmap_clip_top)
{
    output.setEncQpmapClipTop(qpmap_clip_top);
}

void Encoder::setEncQpmapClipBot(uint32_t qpmap_clip_bottom)
{
    output.setEncQpmapClipBot(qpmap_clip_bottom);
}

void Encoder::setH264Bandwidth(uint32_t bw)
{
    output.setH264EncBandwidth(bw);
}

void Encoder::setEncProfiling(uint32_t enable)
{
    output.setPortProfiling(enable);
}

void Encoder::setVP9TileCR(uint32_t tcr)
{
    output.setVP9EncTileCR(tcr);
}

void Encoder::setJPEGRefreshInterval(uint32_t r)
{
    output.setJPEGEncRefreshInterval(r);
}

void Encoder::setJPEGQuality(uint32_t q)
{
    output.setJPEGEncQuality(q);
}

void Encoder::setJPEGQualityLuma(uint32_t q)
{
    output.setJPEGEncQualityLuma(q);
}

void Encoder::setJPEGQualityChroma(uint32_t q)
{
    output.setJPEGEncQualityChroma(q);
}

void Encoder::setJPEGHufftable(struct v4l2_mvx_huff_table *table)
{
    output.setJPEGHufftable(table);
}

void Encoder::setHEVCEntropySync(uint32_t es)
{
    output.setHEVCEncEntropySync(es);
}

void Encoder::setHEVCTemporalMVP(uint32_t tmvp)
{
    output.setHEVCEncTemporalMVP(tmvp);
}

void Encoder::setStreamEscaping(uint32_t sesc)
{
    output.setEncStreamEscaping(sesc);
}

void Encoder::tryStopCmd(bool tryStop)
{
    input.tryEncStopCmd(tryStop);
}

void Encoder::setMirror(int mirror)
{
    input.setMirror(mirror);
}

void Encoder::setRotation(int rotation)
{
    input.setRotation(rotation);
}

void Encoder::setFrameCount(int frames) {
    input.setFrameCount(frames);
}

void Encoder::setMiniHeight(uint32_t mini_height)
{
    mini_frame_height = mini_height;
    input.setMiniHeight(mini_height);
}

void Encoder::setStride(size_t *stride){
    input.setFrameStride(stride);
}

void Encoder::setRateControl(const std::string &rc, int target_bitrate, int maximum_bitrate){

    struct v4l2_rate_control v4l2_rc;
    memset(&v4l2_rc, 0, sizeof(v4l2_rc));
    if (rc.compare("standard") == 0) {
        v4l2_rc.rc_type = V4L2_OPT_RATE_CONTROL_MODE_STANDARD;
    } else if (rc.compare("constant") == 0) {
        v4l2_rc.rc_type = V4L2_OPT_RATE_CONTROL_MODE_CONSTANT;
    } else if (rc.compare("variable") == 0) {
        v4l2_rc.rc_type = V4L2_OPT_RATE_CONTROL_MODE_VARIABLE;
    } else if (rc.compare("cvbr") == 0) {
        v4l2_rc.rc_type = V4L2_OPT_RATE_CONTROL_MODE_C_VARIABLE;
    } else if (rc.compare("off") == 0){
        v4l2_rc.rc_type = V4L2_OPT_RATE_CONTROL_MODE_OFF;
    } else {
        v4l2_rc.rc_type = V4L2_OPT_RATE_CONTROL_MODE_OFF;
    }
    if (v4l2_rc.rc_type) {
        v4l2_rc.target_bitrate = target_bitrate;
    }
    if (v4l2_rc.rc_type == V4L2_OPT_RATE_CONTROL_MODE_C_VARIABLE) {
        v4l2_rc.maximum_bitrate = maximum_bitrate;
    }
    output.setRateControl(&v4l2_rc);
}

void Encoder::setCropLeft(int left){
    input.setCropLeft(left);
}

void Encoder::setCropRight(int right){
    input.setCropRight(right);
}

void Encoder::setCropTop(int top){
    input.setCropTop(top);
}

void Encoder::setCropBottom(int bottom){
    input.setCropBottom(bottom);
}

void Encoder::setStatsMode(int mode){
    input.setStatsMode(mode);
}

void Encoder::setVuiColourDesc(struct v4l2_mvx_color_desc *color){
    input.setVuiColourDesc(color);
}

void Encoder::setSeiUserData(struct v4l2_sei_user_data *sei_user_data){
    input.setSeiUserData(sei_user_data);
}

void Encoder::setHRDBufferSize(int size) {
    input.setHRDBufferSize(size);
}

void Encoder::setLongTermRef(uint32_t mode, uint32_t period){
    input.setLongTermRef(mode, period);
}

void Encoder::setVisibleWidth(uint32_t v_width)
{
    input.setVisibleWidth(v_width);
}

void Encoder::setVisibleHeight(uint32_t v_height)
{
    input.setVisibleHeight(v_height);
}

void Encoder::setRcBitIMode(uint32_t mode)
{
    output.setRcBitIMode(mode);
}

void Encoder::setChangePos(uint32_t pos)
{
    output.setChangePos(pos);
}

void Encoder::setRcBitRationI(uint32_t ratio)
{
    output.setRcBitRationI(ratio);
}

void Encoder::setMultiSPSPPS(uint32_t sps_pps){
    output.setMultiSPSPPS(sps_pps);
}

void Encoder::setEnableSCD(uint32_t scd_enable){
    output.setEnableSCD(scd_enable);
}

void Encoder::setScdPercent(uint32_t scd_percent){
    output.setScdPercent(scd_percent);
}

void Encoder::setScdThreshold(uint32_t scd_threshold){
    output.setScdThreshold(scd_threshold);
}

void Encoder::setEnableAQSsim(uint32_t aq_ssim_en){
    output.setEnableAQSsim(aq_ssim_en);
}

void Encoder::setAQNegRatio(uint32_t aq_neg_ratio){
    output.setAQNegRatio(aq_neg_ratio);
}

void Encoder::setAQPosRatio(uint32_t aq_pos_ratio){
    output.setAQPosRatio(aq_pos_ratio);
}

void Encoder::setAQQPDeltaLmt(uint32_t aq_qpdelta_lmt){
    output.setAQQPDeltaLmt(aq_qpdelta_lmt);
}

void Encoder::setAQInitFrmAvgSvar(uint32_t aq_init_frm_avg_svar){
    output.setAQInitFrmAvgSvar(aq_init_frm_avg_svar);
}

void Encoder::setEnableVisual(uint32_t enable){
    output.setEnableVisual(enable);
}

void Encoder::setAdaptiveIntraBlock(uint32_t enable){
    output.setAdaptiveIntraBlock(enable);
}

void Encoder::setSvct3Level1Period(uint32_t period)
{
    input.setSvct3Level1Period(period);
}

void Encoder::setIntermediateBufSize(uint32_t size)
{
    input.setIntermediateBufSize(size);
}

void Encoder::setStatsSize(uint32_t mms, uint32_t bc, uint32_t qp)
{
    input.setPortStatsSize(mms,bc,qp);
}

void Encoder::setGDRnumber(uint32_t numbder)
{
    input.setGDRnumber(numbder);
}

void Encoder::setGDRperiod(uint32_t period)
{
    input.setGDRperiod(period);
}

void Encoder::setForcedUVvalue(uint32_t uv_value)
{
    input.setForcedUVvalue(uv_value);
}

void Encoder::setEncSrcCrop(struct v4l2_mvx_crop_cfg * src_crop)
{
    input.setEncSrcCrop(src_crop);
}

void Encoder::setRGBToYUVMode(uint32_t mode){
    input.setRGBToYUVMode(mode);
}

void Encoder::setRGBConvertYUV(struct v4l2_mvx_rgb2yuv_color_conv_coef *coef){
    input.setRGBConvertYUV(coef);
}

void Encoder::setFrameBufCnt(uint32_t count)
{
    input.setBufferCnt(count);
}

void Encoder::setBitBufCnt(uint32_t count)
{
    output.setBufferCnt(count);
}

void Encoder::setEncOSDinfo(struct v4l2_osd_info* info)
{
    input.setEncOSDinfo(info);
}

Info::Info(const char *dev, ostream &log) :
    Codec(dev,
          V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
          V4L2_BUF_TYPE_VIDEO_CAPTURE,
          log,
          true)
{}

void Info::enumerateFormats()
{
    try
    {
        Codec::enumerateFormats();
    }
    catch (Exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }
}
