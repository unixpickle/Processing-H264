//
//  main.c
//  h264decoder
//
//  Created by Alex Nichol on 1/13/13.
//
//

#include "socket_command.h"

#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#define DEBUG_ENABLED 1

typedef struct {
    int width;
    int height;
    struct SwsContext * converter;
    AVCodecContext * avContext;
    AVCodec * avCodec;
} DecodeContext;

int client_initialization(FILE * client, int * width, int * height);
int client_handler(FILE * client);

DecodeContext decode_context_create(int width, int height);
int decode_context_frame(DecodeContext context, uint8_t * h264Data, int dataLen, char ** rgbDataOut);
void decode_context_destroy(DecodeContext context);

int main(int argc, const char * argv[]) {
    int client = main_listen_client(argc, argv);
    if (client < 0) return -client;
    
    av_register_all();
    
    FILE * clientFp = fdopen(client, "r+");
    int result = client_handler(clientFp);
    fclose(clientFp);
    
    return result;
}

#pragma mark - Clients -

int client_initialization(FILE * client, int * width, int * height) {
    if (DEBUG_ENABLED) printf("client_initialization: doing negotiation\n");
    uint32_t widthBuffer;
    uint32_t heightBuffer;
    if (fwrite("v001", 1, 4, client) != 4) return -1;
    if (fflush(client) != 0) return -1;
    if (fread(&widthBuffer, 1, 4, client) != 4) return -1;
    if (fread(&heightBuffer, 1, 4, client) != 4) return -1;
    
    *width = (int)htonl(widthBuffer);
    *height = (int)htonl(heightBuffer);
    return 0;
}

int client_handler(FILE * client) {
    int width, height;
    if (client_initialization(client, &width, &height) != 0) {
        perror("client_initialization");
        return 3;
    }
    
    if (DEBUG_ENABLED) printf("client_handler: creating context\n");
    DecodeContext context = decode_context_create(width, height);
    
    int result = 0;
    while (1) {
        uint32_t bigEndianSize = 0;
        if (fread(&bigEndianSize, 1, 4, client) != 4) {
            break;
        }
        int realSize = htonl(bigEndianSize);
        if (DEBUG_ENABLED) printf("supposed encoded size: %d\n", realSize);
        uint8_t * h264Buffer = (uint8_t *)malloc(realSize);
        size_t got = fread(h264Buffer, 1, realSize, client);
        if (got != realSize) break;
        char * decoded;
        if (DEBUG_ENABLED) printf("client_handler: about to decode the RGB frame\n");
        if (decode_context_frame(context, h264Buffer, realSize, &decoded) != 0) {
            free(h264Buffer);
            result = 3;
            break;
        }
        free(h264Buffer);
        if (DEBUG_ENABLED) printf("client_handler: sending decoded RGB frame\n");
        if (fwrite(decoded, 1, context.width * context.height * 3, client) != context.width * context.height * 3) {
            free(decoded);
            break;
        }
        if (DEBUG_ENABLED) printf("client_handler: sent data\n");
        free(decoded);
    }
    
    decode_context_destroy(context);
    return result;
}

#pragma mark - Decoding -

DecodeContext decode_context_create(int width, int height) {
    DecodeContext context;
    context.width = width;
    context.height = height;
    context.converter = sws_getContext(width, height,
                                       PIX_FMT_YUV420P, width, height,
                                       PIX_FMT_RGB24, SWS_FAST_BILINEAR, 0, 0, 0);
    context.avCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    context.avContext = avcodec_alloc_context3(context.avCodec);
    avcodec_open2(context.avContext, context.avCodec, NULL);
    return context;
}

int decode_context_frame(DecodeContext context, uint8_t * h264Data, int dataLen, char ** rgbDataOut) {
    AVFrame * pFrame = avcodec_alloc_frame();
    AVPacket packet;
    bzero(&packet, sizeof(packet));
    packet.size = dataLen;
    packet.data = (uint8_t *)h264Data;
    int frameFinished = 0;
    avcodec_decode_video2(context.avContext, pFrame, &frameFinished, &packet);
    if (!frameFinished) {
        avcodec_free_frame(&pFrame);
        return -1;
    }
    uint8_t * dataOut = (uint8_t *)malloc(context.width * context.height * 3);
    int stride = 3 * context.width;
    sws_scale(context.converter, (const uint8_t **)&pFrame->data, pFrame->linesize, 0, context.height,
              &dataOut, &stride);
    *rgbDataOut = (char *)dataOut;
    avcodec_free_frame(&pFrame);
    return 0;
}

void decode_context_destroy(DecodeContext context) {
    avcodec_close(context.avContext);
    sws_freeContext(context.converter);
}

