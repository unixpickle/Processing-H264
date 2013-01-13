//
//  main.c
//  h264encoder
//
//  Created by Alex Nichol on 1/13/13.
//
//

#include "socket_command.h"

#include <libswscale/swscale.h>
#include "x264.h"

#define DEBUG_ENABLED 1

typedef struct {
    int width;
    int height;
    struct SwsContext * converter;
    x264_t * encoder;
} EncodeContext;

int client_initialization(FILE * client, int * width, int * height);
int client_handler(FILE * client);

// encode context
EncodeContext encode_context_create(int width, int height);
int encode_context_frame(EncodeContext context, const char * frameData, char ** nalData, int * dataSize);
void encode_context_destroy(EncodeContext encodeContext);


int main(int argc, const char * argv[]) {
    int client = main_listen_client(argc, argv);
    if (client < 0) return -client;
    
    FILE * clientFp = fdopen(client, "r+");
    int result = client_handler(clientFp);
    fclose(clientFp);
    
    return result;
}

#pragma mark - Client Protocol -

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
    EncodeContext context = encode_context_create(width, height);
    
    int frameSize = width * height * 3;
    char * frameBuffer = (char *)malloc(frameSize);
    int result = 0;
    
    while (1) {
        size_t got = fread(frameBuffer, 1, frameSize, client);
        if (got != frameSize) break;
        char * encoded;
        int length;
        if (encode_context_frame(context, frameBuffer, &encoded, &length) != 0) {
            result = 3;
            break;
        }
        if (DEBUG_ENABLED) printf("client_handler: sending encoded frame (len %d)\n", length);
        uint32_t sizeInfo = htonl(length);
        if (fwrite(&sizeInfo, 1, 4, client) != 4) {
            free(encoded);
            break;
        }
        if (fwrite(encoded, 1, length, client) != length) {
            free(encoded);
            break;
        }
        free(encoded);
    }
    
    free(frameBuffer);
    encode_context_destroy(context);
    return result;
}

#pragma mark - Encoding -

EncodeContext encode_context_create(int width, int height) {
    EncodeContext context;
    context.width = width;
    context.height = height;
    
    x264_param_t param;
    x264_param_default_preset(&param, "veryfast", "zerolatency");
    param.i_threads = 1;
    param.i_width = width;
    param.i_height = height;
    param.i_fps_num = 20; // the FPS doesn't matter
    param.i_fps_den = 1;
    // Intra refres:
    param.i_keyint_max = 20; // once every twenty frames
    param.b_intra_refresh = 1;
    //Rate control:
    param.rc.i_rc_method = X264_RC_CRF;
    param.rc.f_rf_constant = 25;
    param.rc.f_rf_constant_max = 35;
    //For streaming:
    param.b_repeat_headers = 1;
    param.b_annexb = 1;
    x264_param_apply_profile(&param, "baseline");
    
    context.encoder = x264_encoder_open(&param);
    context.converter = sws_getContext(width, height, PIX_FMT_RGB24,
                                       width, height, PIX_FMT_YUV420P,
                                       SWS_FAST_BILINEAR, NULL, NULL, NULL);
    
    return context;
}

int encode_context_frame(EncodeContext context, const char * frameData, char ** nalData, int * dataSize) {
    x264_picture_t pictureOut, pictureIn;
    x264_picture_alloc(&pictureIn, X264_CSP_I420, context.width, context.height);
    int rgbStride = context.width * 3;
    sws_scale(context.converter, (const uint8_t **)&frameData, &rgbStride, 0, context.height,
              pictureIn.img.plane, pictureIn.img.i_stride);
    
    x264_nal_t * nals;
    int i_nals;
    if (DEBUG_ENABLED) printf("encode_context_frame: passing data to x264\n");
    int frameSize = x264_encoder_encode(context.encoder, &nals, &i_nals, &pictureIn, &pictureOut); // TODO: figure out if picture_out can be NULL
    x264_picture_clean(&pictureIn);
    if (frameSize <= 0) return -1;
    
    if (DEBUG_ENABLED) printf("encode_context_frame: joining the frames\n");
    
    int totalSize = 0;
    for (int i = 0; i < i_nals; i++) {
        totalSize += nals[i].i_payload;
    }
    
    char * returnData = (char *)malloc(totalSize);
    int offset = 0;
    for (int i = 0; i < i_nals; i++) {
        memcpy(&returnData[offset], nals[i].p_payload, nals[i].i_payload);
        offset += nals[i].i_payload;
    }
    
    *nalData = returnData;
    *dataSize = totalSize;
    
    return 0;
}

void encode_context_destroy(EncodeContext encodeContext) {
    x264_encoder_close(encodeContext.encoder);
    sws_freeContext(encodeContext.converter);
}
