#ifndef DATA_PACKET_H
#define DATA_PACKET_H
#include "pico.h"

#define N_CHAR_PER_WORD      2
#define N_LINE_PER_DATA      2
#define W_GUARDBAND          2
#define W_PREAMBLE           8
#define W_DATA_PACKET        32

#define W_DATA_ISLAND        W_GUARDBAND * 2 + W_DATA_PACKET
#define N_DATA_ISLAND_WORDS  W_DATA_ISLAND / N_CHAR_PER_WORD

typedef enum {
    SCAN_INFO_NO_DATA,
    OVERSCAN,
    UNDERSCAN
} ScanInfo;

typedef enum {
    RGB,
    YCBCR422,
    YCBCR444
} PixelFormat;

typedef enum {
    COLORIMETRY_NO_DATA,
    ITU601,
    ITU709,
    EXTENDED
} Colorimetry;

typedef enum {
    PIC_ASPECT_RATIO_NO_DATA,
    PIC_ASPECT_RATIO_4_3,
    PIC_ASPECT_RATIO_16_9
} PixtureAspectRatio;

typedef enum {
    ACTIVE_FORMAT_ASPECT_RATIO_NO_DATA = -1,
    SAME_AS_PAR = 8,
    ACTIVE_FORMAT_ASPECT_RATIO_4_3,
    ACTIVE_FORMAT_ASPECT_RATIO_16_9,
    ACTIVE_FORMAT_ASPECT_RATIO_14_9
} ActiveFormatAspectRatio;

typedef enum {
    DEFAULT,
    LIMITED,
    FULL
} RGBQuantizationRange;

typedef enum {
    _640x480P60 = 1,
    _720x480P60 = 2,
    _1280x720P60 = 4,
    _1920x1080I60 = 5,
} VideoCode;

typedef struct data_packet {
    uint8_t header[4];
    uint8_t subpacket[4][8];
} data_packet_t;

typedef struct data_island_stream {
    uint32_t data[3][N_DATA_ISLAND_WORDS];
} data_island_stream_t;

// Functions related to the data_packet (requires a data_packet instance)
void computeHeaderParity(data_packet_t *data_packet);
void computeSubPacketParity(data_packet_t *data_packet, int i);
void computeParity(data_packet_t *data_packet);
void computeInfoFrameCheckSum(data_packet_t *data_packet);
void encodeHeader(const data_packet_t *data_packet, uint32_t *dst, int hv, bool firstPacket);
void encodeSubPacket(const data_packet_t *data_packet, uint32_t *dst1, uint32_t *dst2);
void setNull(data_packet_t *data_packet);
int  setAudioSample(data_packet_t *data_packet, const int16_t **p, int n, int frameCt);
void setAudioClockRegeneration(data_packet_t *data_packet, int CTS, int N);
void setAudioInfoFrame(data_packet_t *data_packet, int freq);
void setAVIInfoFrame(data_packet_t *data_packet, ScanInfo s, PixelFormat y, Colorimetry c, PixtureAspectRatio m,
    ActiveFormatAspectRatio r, RGBQuantizationRange q, VideoCode vic);

// Public Functions
extern uint32_t defaultDataPacket12_[N_DATA_ISLAND_WORDS];
inline uint32_t *getDefaultDataPacket12() {
    return defaultDataPacket12_;
}
uint32_t *getDefaultDataPacket0(bool vsync, bool hsync);
void encode(data_island_stream_t *dst, const data_packet_t *packet, bool vsync, bool hsync);
#endif
