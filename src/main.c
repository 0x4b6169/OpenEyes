#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#include "util/logging/log.h"

#define INITIAL_SCREEN_WIDTH 3584
#define INITIAL_SCREEN_HEIGHT 2240
#define FINAL_SCREEN_WIDTH 480
#define FINAL_SCREEN_HEIGHT 300

#define MBP_SCREEN_RES "3584x2240"

int thread_exit = 0;

static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
    FILE *f;
    int i;
    char file[512];
    strcpy(file, ""); // change this to output to file external drive
    strcat(file, filename);
    f = fopen(file,"w");

    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecCtx, AVFrame *pFrame)
{
    log_info("Frame %d (type=%c, size=%d, format=%d) pts %d key_frame %d [DTS %d]",
             pCodecCtx->frame_number,
             pFrame->pkt_size,
             pFrame->format,
             pFrame->pts,
             pFrame->key_frame,
             pFrame->coded_picture_number);

    log_info("pFrame height: %d, frameYUV width: %d, frameYUV format: %d",
             pFrame->height,
             pFrame->width,
             pFrame->format);

        char frame_filename[1024];
        snprintf(frame_filename, sizeof(frame_filename), "%s-%d.pgm", "frame", pCodecCtx->frame_number);

        if (pFrame->format != AV_PIX_FMT_YUV420P)
        {
            log_debug("The generated file may not be a grayscale image");
        }

        save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
    return 0;
}

int sfp_refresh_thread(void *opaque)
{
    int ret;
    while (thread_exit == 0) {
        SDL_Event event;
        event.type = SDL_USEREVENT;
        ret = SDL_PushEvent(&event);
        if (ret < 0)
        {
            log_error( "Could not push event - %s", SDL_GetError());
            return -1;
        }
        SDL_Delay(40);
    }
    return 0;
}

int main(int argc, char *argv[]) {

    AVFormatContext *formatContext = NULL;
    formatContext = avformat_alloc_context();

    AVCodecContext *codecContext = NULL;
    AVCodec *codec = NULL;
    AVCodecParameters *codecParameters = NULL;


    int i, ret, videoIndex;

    avdevice_register_all();

    AVDictionary *options = NULL;
    av_dict_set(&options, "framerate", "1", 0);
    av_dict_set(&options, "video_size", MBP_SCREEN_RES, 0);

    AVInputFormat *inputFormat;
    inputFormat = av_find_input_format("avfoundation");

    // 1:none - 1 returns screen capture for video input and none ignores audio
    ret = avformat_open_input(&formatContext, "1:none", inputFormat, &options);
    if (ret < 0)
    {
        log_error("Error opening the input stream - %s", av_err2str(ret));
        return -1;
    }

    ret = avformat_find_stream_info(formatContext, NULL);
    if (ret < 0)
    {
        log_error("Error retrieving stream info - %s", av_err2str(ret));
        return -1;
    }

    // Get the parameters so you can access details about the codec in the AVFormat
    AVCodecParameters *localCodecParameters = NULL;
    AVCodec *localCodec = NULL;
    videoIndex = -1;

    for (i = 0; i < formatContext->nb_streams; i++)
    {
        localCodecParameters = formatContext->streams[i]->codecpar;
        localCodec = avcodec_find_decoder(localCodecParameters->codec_id);
        if (localCodec == NULL)
        {
            log_error("Unsupported codec.");
            continue;
        }

        if (localCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if (videoIndex == -1)
            {
                videoIndex = i;
                log_info("Video index = %s, video codec type = %s", videoIndex, localCodecParameters->codec_type);
                codecParameters = localCodecParameters;
                codec = localCodec;
            }
            log_info("Video Codec: resolution %d x %d", codecParameters->width, codecParameters->height);
        }

        if (videoIndex == -1)
        {
            log_error("A video input stream was not found.");
            return -1;
        }

        codecContext = avcodec_alloc_context3(codec);
        if (codecContext == NULL)
        {
            log_error("Could not allocate memory for the codec.");
            return -1;
        }

        ret = avcodec_parameters_to_context(codecContext, codecParameters);
        if (ret < 0)
        {
            log_error("Failed to copy codec params to codec context - %s", av_err2str(ret));
            return -1;
        }

        ret = avcodec_open2(codecContext, codec, NULL);
        if (ret < 0)
        {
            log_error("Could not open the codec.");
            return -1;
        }


        /*
         *
         * THIS IS WHERE THE FRAME AND WINDOW BEGINS
         *
         */
        AVFrame *frame = NULL;
        AVFrame *frameYUV = NULL;
        frame = av_frame_alloc();
        frameYUV = av_frame_alloc();

        ret = SDL_Init(SDL_INIT_VIDEO);
        if (ret < 0)
        {
            log_error( "Could not initialize SDL - %s", SDL_GetError());
            return -1;
        }

        SDL_Window *window = NULL;
        SDL_Renderer *renderer = NULL;

        window = SDL_CreateWindow("OpenPlaya", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  FINAL_SCREEN_HEIGHT, FINAL_SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
        if (window == NULL)
        {
            log_error( "Could not create SDL window - %s", SDL_GetError());
            return -1;
        }

        renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer == NULL)
        {
            log_error( "Could not create SDL renderer - %s", SDL_GetError());
            return -1;
        }

        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = INITIAL_SCREEN_WIDTH;
        rect.h = INITIAL_SCREEN_HEIGHT;

        AVPacket *packet = NULL;
        packet = av_packet_alloc();

        log_info("before sws context");

        /* Must uncomment below, and comment the corresponding lines, to make video work right now... */
//        frameYUV->width = frame->width = codecContext->width;
//        frameYUV->height = frame->height = codecContext->height;
        frame->width = codecContext->width;
        frame->height = codecContext->height;
        frameYUV->width = FINAL_SCREEN_WIDTH;
        frameYUV->height = FINAL_SCREEN_HEIGHT;
        frameYUV->channels = frame->channels;
        frameYUV->channel_layout = frame->channel_layout;
        frameYUV->format = AV_PIX_FMT_YUV420P;

        struct SwsContext *img_sws_context = NULL;
        img_sws_context = sws_getCachedContext(img_sws_context, frame->width, frame->height,
                                               codecContext->pix_fmt, FINAL_SCREEN_WIDTH, FINAL_SCREEN_HEIGHT,
                                               AV_PIX_FMT_YUV420P, 0, NULL, NULL, NULL);

        // Allocate destination memory for frameYUV (target) buffer
        ret = av_image_alloc(frameYUV->data, frameYUV->linesize,
                             frameYUV->width, frameYUV->height, frameYUV->format, 1);
        if (ret < 0)
        {
            fprintf(stderr, "Could not allocate destination image\n");
            return -1;
        }

        log_info("after sws context");

        SDL_Thread *thread = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);
        if (thread == NULL)
        {
            log_error( "Could not create new SDL thread - %s", SDL_GetError());
            return -1;
        }

        SDL_Event *event;

        for (;;)
        {
            ret = SDL_WaitEvent(event);
            if (ret < 0)
            {
                log_error( "Did not find event - %s", SDL_GetError());
                return -1;
            }

            if (event->type == SDL_USEREVENT)
            {
                log_info("sdluserevent");
                if (av_read_frame(formatContext, packet) >= 0)
                {
                    log_info("read frame");
                    if (packet->stream_index == videoIndex)
                    {
                        SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STATIC,
                                                                 codecContext->width, codecContext->height);

                        ret = avcodec_send_packet(codecContext, packet);
                        if (ret < 0)
                        {
                            log_error("Error sending packet to decoder - %s", av_err2str(ret));
                            return -1;
                        }

                        ret = avcodec_receive_frame(codecContext, frame);
                        if (ret < 0)
                        {
                            log_error("Error receiving decoded output from avcodec_send_packet - %s", av_err2str(ret));
                            return -1;
                        }

                        log_info("frame height: %d, frame width: %d, frame format: %d",
                                 frame->height,
                                 frame->width,
                                 frame->format);

                        sws_scale(img_sws_context, (const uint8_t *const *) frame->data, frame->linesize, 0, INITIAL_SCREEN_HEIGHT,
                                  frameYUV->data, frameYUV->linesize);

                        log_info("frameYUV height: %d, frameYUV width: %d, frameYUV format: %d",
                                 frameYUV->height,
                                 frameYUV->width,
                                 frameYUV->format);

//                        decode_packet(packet, codecContext, frameYUV);

                        ret = SDL_UpdateYUVTexture(texture, &rect,
                                             frameYUV->data[0], frameYUV->linesize[0],
                                             frameYUV->data[1], frameYUV->linesize[1],
                                             frameYUV->data[2], frameYUV->linesize[2]);
                        if (ret < 0)
                        {
                            log_error( "Did not find event - %s", SDL_GetError());
                            return -1;
                        }

                        SDL_RenderClear(renderer);
                        SDL_RenderCopy(renderer, texture, NULL, &rect);
                        SDL_RenderPresent(renderer);
                    }
                }
            }

            if (event->type == SDL_QUIT)
            {
                log_info("Quitting application.");
                thread_exit = -1;
                break;
            }
        }

        av_packet_free(&packet);
        av_frame_free(&frame);
        av_frame_free(&frameYUV);
    }

    SDL_Quit();

    avcodec_close(codecContext);
    avformat_close_input(&formatContext);

    return 0;

}