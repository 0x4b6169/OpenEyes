#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include "logging/log.h"
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

// https://www.theverge.com/2022/1/20/22892152/google-project-iris-ar-headset-2024 people to network with

// First method is initialization of the FFMpeg libary. This library takes an input device (in this application's case the AV input device is
// programmed to capture the screen of the device). This method should be reusable for any input device for any future projects with varying
// operating systems / input drivers in general.

// Second method is the initialization, input-setting, and rendering of an SDL2 window. This functionality may never be used, but is a cool project to
// have completed in the event that I can paint to the screen. Initially planning to use warning windows that pop-up on screen at the bottom right that
// describe threat level.

// Third method will be a data streaming method that will send frames to a machine learning algorithm. This algorithm will hopefully live on the same
// device but may also be in the cloud initially for PoC.


// Idea - turn this application into a workflow tool with autocomplete ideas based on what the user does on an average day

int thread_exit = 0;

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
    av_dict_set(&options, "framerate", "30", 0);
    av_dict_set(&options, "video_size", "1920x1080", 0);

    AVInputFormat *inputFormat;
    inputFormat = av_find_input_format("avfoundation");

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

        window = SDL_CreateWindow("OpenPlaya", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
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
        rect.w = SCREEN_WIDTH;
        rect.h = SCREEN_HEIGHT;

        AVPacket *packet = NULL;
        packet = av_packet_alloc();

        log_info("before sws context");

        frameYUV->width = frame->width = codecContext->width;
        frameYUV->height = frame->height = codecContext->height;
        frameYUV->channels = frame->channels;
        frameYUV->channel_layout = frame->channel_layout;
        frameYUV->format = AV_PIX_FMT_YUV420P;

        struct SwsContext *img_sws_context = NULL;
        img_sws_context = sws_getCachedContext(img_sws_context, frame->width, frame->height,
                                               codecContext->pix_fmt, frameYUV->width, frameYUV->height,
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

            log_info("wait for event");

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

                        log_info("frameYUV height: %d, frameYUV width: %d, frameYUV format: %d",
                                 frameYUV->height,
                                 frameYUV->width,
                                 frameYUV->format);

                        log_info("Before scale");

                        sws_scale(img_sws_context, (const uint8_t *const *) frame->data, frame->linesize, 0, SCREEN_HEIGHT,
                                  frameYUV->data, frameYUV->linesize);



                        log_info("After scale");

                        SDL_UpdateYUVTexture(texture, &rect,
                                             frameYUV->data[0], frameYUV->linesize[0],
                                             frameYUV->data[1], frameYUV->linesize[1],
                                             frameYUV->data[2], frameYUV->linesize[2]);

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

//            else {
//                log_error("Event type is not SDL_USEREVENT");
//                thread_exit = -1;
//                break;
//            }
        }

        av_packet_free(&packet);
        av_frame_free(&frame);

    }

    SDL_Quit();

    avcodec_close(codecContext);
    avformat_close_input(&formatContext);

    return 0;

}