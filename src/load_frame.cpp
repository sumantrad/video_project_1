extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <inttypes.h>
#include <libswscale/swscale.h>
}

// av_err2str returns a temporary array. This doesn't work in gcc.
// This function can be used as a replacement for av_err2str.
static const char* av_make_error(int errnum) {
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}



bool load_frame(const char* filename, int* width_out, int* height_out, unsigned char** data_out){

    AVFormatContext* av_format_ctx = avformat_alloc_context();
    if(!av_format_ctx){
        printf("couldn't create AVFormatContext\n");
        return false;
    }
    if(avformat_open_input(&av_format_ctx,filename,NULL,NULL)!=0){
        printf("couldn't open video file\n");
        return false;
    }

    int video_stream_index = -1;
    AVCodecParameters* av_codec_params;
    AVCodec* av_codec;

    //find the first valid video stream
    for(int i =0;i<av_format_ctx->nb_streams;++i){
        auto stream = av_format_ctx->streams[i];
        av_codec_params = av_format_ctx->streams[i]->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);

        if(!av_codec){
            continue;
        }
        if(av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            break;
        }

    }

    if(video_stream_index ==-1){
        printf("couldn't find valid video stream inside file\n");
        return false;
    }

    //set up a codef context for the decoder
    AVCodecContext* av_codec_ctx = avcodec_alloc_context3(av_codec);
    if(!av_codec_ctx){
        printf("couldn't create AVCodecContext\n");
        return false;
    }

    if(avcodec_parameters_to_context(av_codec_ctx,av_codec_params)){
        printf("couldn't initialize avcodec context\n");
        return false;
    }
    if(avcodec_open2(av_codec_ctx,av_codec,NULL)<0){
        printf("couldn't open codec\n");
        return false;
    }

    AVFrame* av_frame = av_frame_alloc();
    if(!av_frame){
        printf("couldn't allocate avframe\n");
        return false;
    }

    AVPacket* av_packet=av_packet_alloc();
    if(!av_packet){
        printf("couldn't allocate avpacket\n");
        return false;
    }
    int response;
    while(av_read_frame(av_format_ctx,av_packet)>=0){
        if(av_packet->stream_index!=video_stream_index){
            continue;
        }
        response=avcodec_send_packet (av_codec_ctx,av_packet);
        if(response<0){
            printf("failed to decode packet %s\n",av_make_error(response));
            return false;
        }
        response = avcodec_receive_frame(av_codec_ctx,av_frame);
        if(response==AVERROR(EAGAIN) || response == AVERROR_EOF){
            continue;
        } else if(response<0){
            printf("failed to decode packet %s\n",av_make_error(response));
            return false;
        }
        av_packet_unref(av_packet);
        break;
    }

    uint8_t* data = new uint8_t[av_frame->width*av_frame->height*4];

    SwsContext* sws_scalar_ctx = sws_getContext(av_frame->width,
                                               av_frame->height,
                                               av_codec_ctx->pix_fmt,
                                               av_frame->width,
                                               av_frame->height,
                                               AV_PIX_FMT_RGB0,
                                               SWS_BILINEAR,
                                               NULL,
                                               NULL,
                                               NULL);
    if(!sws_scalar_ctx){
        printf("couldn't initialize swscalar\n");
        return false;
    }
    uint8_t* dest[4] ={data,NULL,NULL,NULL};
    int dest_linesize[4]={av_frame->width*4,0,0,0};
    sws_scale(sws_scalar_ctx,av_frame->data,av_frame->linesize,0,av_frame->height,dest, dest_linesize);
    sws_freeContext(sws_scalar_ctx);

    /*unsigned char* data = new unsigned char[av_frame->width* av_frame->height * 3];
    for(int x=0;x<av_frame->width;++x){
        for(int y=0;y<av_frame->height;++y){
            data[y*av_frame->width*3 +x*3] =av_frame->data[0][y*av_frame->linesize[0]+x];
            data[y*av_frame->width*3 +x*3+1] =av_frame->data[0][y*av_frame->linesize[0]+x];
            data[y*av_frame->width*3 +x*3+2] =av_frame->data[0][y*av_frame->linesize[0]+x];
        }
    }*/

    *width_out = av_frame->width;
    *height_out = av_frame->height;
    *data_out = data;

    avformat_close_input(&av_format_ctx);
    avformat_free_context(av_format_ctx);
    av_frame_free(&av_frame);
    av_packet_free(&av_packet);
    avcodec_free_context(&av_codec_ctx);

    return true;



    /**width = 100;
    *height = 100;
    *data = new unsigned char[100*100*3];

    auto ptr = *data;
    for(int x=0;x<100;++x){
        for(int y=0;y<100;++y){
            *ptr++ = 0xff;
            *ptr++ = 0x00;
            *ptr++ = 0x00;
        }
    }
    return true;*/
}