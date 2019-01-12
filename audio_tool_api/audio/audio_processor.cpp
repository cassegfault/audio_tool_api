//
//  audio_processor.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/13/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//
// Steals much from https://www.ffmpeg.org/doxygen/trunk/transcoding_8c-example.html

#include "audio_processor.h"

int AP::read_packet(void *opaque, uint8_t *buf, int buf_size){
    struct input_buffer_data *bd = (struct input_buffer_data *)opaque;
    buf_size = fmin(buf_size, bd->size);
    if (!buf_size)
        return AVERROR_EOF;
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;
    bd->size -= buf_size;
    return buf_size;
}

int AP::write_packet(void *opaque, uint8_t *buf, int buf_size){
    struct output_buffer_data *bd = (struct output_buffer_data *)opaque;
    while (buf_size > bd->room) {
        int64_t offset = bd->ptr - bd->buf.get();
        uint8_t * tmp = (uint8_t*)malloc(2 * bd->size);
        memcpy(tmp, bd->buf.get(), bd->size);
        bd->buf.reset(tmp);
        if (!bd->buf)
            return AVERROR(ENOMEM);
        bd->size *= 2;
        bd->ptr = bd->buf.get() + offset;
        bd->room = bd->size - offset;
    }
    buf_size = fmin(buf_size, bd->size);
    memcpy(bd->ptr, buf, buf_size);
    bd->ptr  += buf_size;
    bd->room -= buf_size;
    return 0;
}

void AP::set_input_content(uint8_t * data, size_t size) {
    this->input_buffer = data;
    this->input_buffer_size = size;
}

string AP::get_output_content(){
    return string((const char *)this->obd.buf.get(), this->obd.size - this->obd.room);
}

void AP::init_input() {
    int error = 0;
    /*
        Setup custom avio input context
    */
        AVIOContext *avio_ctx = NULL;
        int avio_ctx_buffer_size = 4096;
        uint8_t * avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);

        // refactor this to be a file_buffer_data
        ibd.ptr = input_buffer;
        ibd.size = input_buffer_size;
    
        in_fmt_ctx = avformat_alloc_context();
    
        if (!in_fmt_ctx) {
            throw "Error allocating memory";
        }
    
        if (!avio_ctx_buffer) {
            throw "Error allocating memory";
        }
    
        // Potentially implement seek (last parameter)
        avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, &ibd, &AP::read_packet, NULL, NULL);
        if (!avio_ctx) {
            throw "Error allocating memory";
        }
        in_fmt_ctx->pb = avio_ctx;
    
    /*
        Set up decoders and decoder contexts
    */
    
    avformat_open_input(&in_fmt_ctx, NULL, NULL, NULL);
    
    error = (avformat_find_stream_info(in_fmt_ctx, NULL));
    if (error < 0){
        throw string("Could not find stream info, Error: ") + av_err2str(error);
    }
    cout << "Check stream_ctx size:" << sizeof(stream_ctx) << endl;
    stream_ctx = (StreamContext*)av_mallocz_array(in_fmt_ctx->nb_streams, sizeof(*stream_ctx));
    
    // Iterate every stream, set up decoder if its an audio stream
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream = in_fmt_ctx->streams[i];
        AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
        AVCodecContext * codec_ctx;
        
        if (!dec || dec == nullptr) {
            throw "Failed to find decoder for stream";
        }
        
        codec_ctx = avcodec_alloc_context3(dec);
        
        if (!codec_ctx || codec_ctx == nullptr) {
            throw "Failed to allocate decoder context for stream";
        }
        
        error = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
        if (error < 0) {
            throw "Failed to copy decoder codec paramters to input decoder context";
        }
        
        // Set up decoder contexts for each stream
        if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            error = avcodec_open2(codec_ctx, dec, NULL);
            if (error < 0) {
                throw "Failed to open decoder for stream";
            }
        }
        stream_ctx[i].dec_ctx = codec_ctx;
    }
}

void AP::init_output() {
    int error = 0;
    AVStream *out_stream = NULL;
    AVStream *in_stream = NULL;
    AVCodecContext *dec_ctx, *enc_ctx;
    AVCodec *encoder = NULL;
    
    /*
        Set up custom IO context
    */
        AVIOContext *output_io_context = NULL;
        int avio_ctx_buffer_size = 4096;
        uint8_t * avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
    
        obd.buf.reset((uint8_t *)av_malloc(output_buffer_size));
        obd.ptr = obd.buf.get();
        obd.size = obd.room = output_buffer_size;
    
        output_io_context = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 1, &obd, NULL, &write_packet, NULL);
        if (!output_io_context) {
            throw "Could not allocate output IO context";
        }
        out_fmt_ctx.reset( avformat_alloc_context() );
        if (out_fmt_ctx.get() == nullptr) {
            throw "Could not allocate output format context";
        }
        out_fmt_ctx->pb = output_io_context;
    
        out_fmt_ctx->oformat = av_guess_format("wav", NULL, NULL);
        if (!out_fmt_ctx->oformat) {
            throw "Could not set up output format context";
        }
    
    /*
        Set up output streams for each input stream
    */
    for (int i=0; i < in_fmt_ctx->nb_streams; i++) {
        out_stream = avformat_new_stream(out_fmt_ctx.get(), NULL);
        if (!out_stream) {
            throw "Could not allocate output stream";
        }
        
        in_stream = in_fmt_ctx->streams[i];
        dec_ctx = stream_ctx[i].dec_ctx;
        error = avcodec_parameters_to_context(dec_ctx, in_stream->codecpar);
        if (error < 0) {
            throw "Failed to build codec context from paramters for stream";
        }
        enc_ctx = avcodec_alloc_context3(encoder);
        if(!enc_ctx) {
            throw "Failed to allocate the encoder context";
        }
        
        if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            encoder = avcodec_find_encoder(AV_CODEC_ID_PCM_F32LE);
            if (!encoder) {
                throw "Could not find necessary encoder";
            }
            
            // we're going to need a resampler and to change all these to be un
            enc_ctx->sample_rate = dec_ctx->sample_rate;
            enc_ctx->channel_layout = dec_ctx->channel_layout;
            enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
            enc_ctx->sample_fmt = encoder->sample_fmts[0];
            enc_ctx->time_base = (AVRational){1, enc_ctx->sample_rate};
            
            if (out_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            
            error = avcodec_open2(enc_ctx, encoder, NULL);
            if (error < 0) {
                throw "Could not open codec";
            }
            
            error = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
            if (error < 0) {
                throw "Failed to copy codec parameters from encoding context";
            }
            
            out_stream->time_base = in_stream->time_base;
            stream_ctx[i].enc_ctx = enc_ctx;
        }
    }
    
    // All of our streams are set up, write out the header
    error = avformat_write_header(out_fmt_ctx.get(), &out_fmt_ctx->metadata);
    if (error < 0) {
        throw "Error occurred when writing header";
    }
}

void AP::init_filter(FilteringContext * fctx, const char * filter_spec, AVCodecContext * dec_ctx, AVCodecContext * enc_ctx) {
    int error = 0;
    char args[512];
    const AVFilter *buffersrc = NULL;
    const AVFilter *buffersink = NULL;
    AVFilterContext *buffersrc_ctx = NULL;
    AVFilterContext *buffersink_ctx = NULL;
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVFilterGraph *filter_graph = avfilter_graph_alloc();
    
    if (!outputs || !inputs || !filter_graph) {
        throw "Could not allocate reources for filter";
    }
    
    buffersrc = avfilter_get_by_name("abuffer");
    buffersink = avfilter_get_by_name("abuffersink");
    if (!buffersrc || !buffersink) {
        throw "Could not find filter for buffer source or sink";
    }
    
    if (!dec_ctx->channel_layout) {
        dec_ctx->channel_layout = av_get_default_channel_layout(dec_ctx->channels);
    }
    
    snprintf(args, sizeof(args),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
             dec_ctx->time_base.num, dec_ctx->time_base.den, dec_ctx->sample_rate,
             av_get_sample_fmt_name(dec_ctx->sample_fmt),
             dec_ctx->channel_layout);
    
    error = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, filter_graph);
    if(error < 0) {
        throw "Could not create buffer source";
    }
    
    avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, NULL, filter_graph);
    if(error < 0) {
        throw "Could not create buffer sink";
    }
    
    error = av_opt_set_bin(buffersink_ctx, "sample_fmts", (uint8_t*)&enc_ctx->sample_fmt, sizeof(enc_ctx->sample_fmt), AV_OPT_SEARCH_CHILDREN);
    if(error < 0) {
        throw "Could not set output sample format";
    }
    
    av_opt_set_bin(buffersink_ctx, "channel_layouts", (uint8_t*)&enc_ctx->channel_layout, sizeof(enc_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
    if(error < 0) {
        throw "Could not set output channel layout";
    }
    
    av_opt_set_bin(buffersink_ctx, "sample_rates", (uint8_t*)&enc_ctx->sample_rate, sizeof(enc_ctx->sample_rate), AV_OPT_SEARCH_CHILDREN);
    if(error < 0) {
        throw "Could not set output sample rate";
    }
    
    /* Endpoints for the filter graph. */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;
    
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;
    
    if (!outputs->name || !inputs->name) {
        throw "Could not set up filter graph endpoints";
    }
    
    error = avfilter_graph_parse_ptr(filter_graph, filter_spec, &inputs, &outputs, NULL);
    if(error < 0) {
        throw "";
    }
    
    error = avfilter_graph_config(filter_graph, NULL);
    if(error < 0) {
        throw "";
    }
    
    fctx->buffersrc_ctx = buffersrc_ctx;
    fctx->buffersink_ctx = buffersink_ctx;
    fctx->filter_graph = filter_graph;
    
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
}

void AP::init_filters() {
    filter_ctx.reset((FilteringContext*)av_malloc_array(in_fmt_ctx->nb_streams, sizeof(*filter_ctx)));

    if (filter_ctx.get() == nullptr){
        throw "Could not allocate filter context";
    }
    
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        filter_ctx.get()[i].buffersrc_ctx  = NULL;
        filter_ctx.get()[i].buffersink_ctx = NULL;
        filter_ctx.get()[i].filter_graph   = NULL;
        
        init_filter(&filter_ctx.get()[i], "anull", stream_ctx[i].dec_ctx, stream_ctx[i].enc_ctx);
    }
}

void AP::process() {
    int error = 0;
    AVPacket packet = { .data = NULL, .size = 0 };
    AVFrame *frame = NULL;
    
    init_input();
    init_output();
    init_filters();
    
    av_dump_format(out_fmt_ctx.get(), NULL, NULL, 1);
    
    
    // This is the loop that needs to be converted for stream processing.
    // Possible it could be done in the custom input IO context of the
    while(true) {
        error = av_read_frame(in_fmt_ctx, &packet);
        if(error < 0) {
            // No more packets to read
            break;
        }
        
        int stream_index = packet.stream_index;
        
        if (filter_ctx.get()[stream_index].filter_graph) {
            frame = av_frame_alloc();
            if(!frame) {
                throw "Could not allocate frame";
            }
            
            av_packet_rescale_ts(&packet, in_fmt_ctx->streams[stream_index]->time_base, stream_ctx[stream_index].dec_ctx->time_base);
            
            //avcodec_decode_audio4(stream_ctx[stream_index].dec_ctx, frame, &got_frame, &packet);
            error = avcodec_send_packet(stream_ctx[stream_index].dec_ctx, &packet);
            if(error < 0) {
                throw "Problem sending packet";
            }
            
            error = avcodec_receive_frame(stream_ctx[stream_index].dec_ctx, frame);
            
            if (error == AVERROR(EAGAIN)) {
                // We need to send additional packets before we can receive a frame
            } else if(error < 0) {
                throw "Problem receiving frame";
            } else {
                // At this point we should have a frame
                frame->pts = frame->best_effort_timestamp;
                //pts += frame->nb_samples;
                
                
                // The meat of the function:
                filter_encode_write(frame, stream_index);
            }
            
            av_frame_free(&frame);
        } else {
            av_packet_rescale_ts(&packet, in_fmt_ctx->streams[stream_index]->time_base, out_fmt_ctx->streams[stream_index]->time_base);
            error = av_write_frame(out_fmt_ctx.get(), &packet);
            if(error < 0) {
                throw "Error writing frame";
                break;
            }
        }
        
        // Done with this packet, move to the next one
        av_packet_unref(&packet);
    }
    
    // Flush
    for(int i=0; i < in_fmt_ctx->nb_streams; i++) {
        if (!filter_ctx.get()[i].filter_graph) {
            continue;
        }
        
        filter_encode_write(NULL, i);
        
        // Flush the encoder
        if (!(stream_ctx[i].enc_ctx->codec->capabilities & AV_CODEC_CAP_DELAY)) {
            continue;
        }
        
        while (true) {
            encode_write(NULL, i);
        }
    }
    
    // Finish off writing
    av_write_trailer(out_fmt_ctx.get());
    
    // Cleanup
    av_packet_unref(&packet);
    av_frame_free(&frame);
    
    for (int i=0; i < in_fmt_ctx->nb_streams; i++) {
        avcodec_free_context(&stream_ctx[i].dec_ctx);
        if (out_fmt_ctx.get() != NULL && out_fmt_ctx->nb_streams > i && out_fmt_ctx->streams[i] && stream_ctx[i].enc_ctx) {
            avcodec_free_context(&stream_ctx[i].enc_ctx);
        }
        if (filter_ctx && filter_ctx.get()[i].filter_graph) {
            avfilter_graph_free(&filter_ctx.get()[i].filter_graph);
        }
    }
}

void AP::filter_encode_write(AVFrame *frame, unsigned int stream_index) {
    int error = 0;
    error = av_buffersrc_add_frame_flags(filter_ctx.get()[stream_index].buffersrc_ctx, frame, 0);
    if(error < 0) {
        throw "Error while feeding the filtergraph";
    }
    
    AVFrame *filt_frame;
    while (true) {
        filt_frame = av_frame_alloc();
        if(!filt_frame) {
            throw "Could not allocate the filter frame";
        }
        error = av_buffersink_get_frame(filter_ctx.get()[stream_index].buffersink_ctx,
                                        filt_frame);
        if(error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
            // Nothing more to process, break
            av_frame_free(&filt_frame);
            break;
        } else if (error < 0) {
            throw;
        }
        // We received a frame from the filterbuffer
        // Encode and write the frame
        encode_write(filt_frame, stream_index);
        
    }
}

void AP::encode_write(AVFrame *filt_frame, unsigned int stream_index) {
    int ret = 0;
    AVPacket enc_pkt;
    
    enc_pkt.data = NULL;
    enc_pkt.size = 0;
    
    av_init_packet(&enc_pkt);
    avcodec_send_frame(stream_ctx[stream_index].enc_ctx, filt_frame);
    //free the frame
    av_frame_free(&filt_frame);
    while (true) {
        ret = avcodec_receive_packet(stream_ctx[stream_index].enc_ctx, &enc_pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_packet_unref(&enc_pkt);
            break;
        } else if (ret < 0) {
            throw "error encoding audio frame";
        }
        // output the packet
        enc_pkt.stream_index = stream_index;
        av_packet_rescale_ts(&enc_pkt, stream_ctx[stream_index].enc_ctx->time_base, out_fmt_ctx->streams[stream_index]->time_base);
        ret = av_write_frame(out_fmt_ctx.get(), &enc_pkt);
        if(ret < 0) {
            throw "Error writing frame";
        }
        //free the packet
        av_packet_unref(&enc_pkt);
    }
    
}
