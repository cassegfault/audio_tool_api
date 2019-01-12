//
//  audio_processor.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/13/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#ifndef audio_processor_h
#define audio_processor_h

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cinttypes>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}


using namespace std;
class AP {
public:
    
    void process();
    void set_input_content(uint8_t * data, size_t size);
    string get_output_content();
    
private:
    // Initialization functions
    void init_input();
    void init_output();
    void init_filters();
    
    // Processing functions
    void filter_encode_write(AVFrame *frame, unsigned int stream_index);
    void encode_write(AVFrame *filt_frame, unsigned int stream_index);
    
    // IO functions
    static int read_packet(void *opaque, uint8_t *buf, int buf_size);
    static int write_packet(void *opaque, uint8_t *buf, int buf_size);
    
    AVFormatContext * in_fmt_ctx;
    shared_ptr<AVFormatContext> out_fmt_ctx;
    
    unsigned int pts = 0;
    
    typedef struct FilteringContext {
        AVFilterContext *buffersink_ctx;
        AVFilterContext *buffersrc_ctx;
        AVFilterGraph *filter_graph;
    } FilteringContext;
    shared_ptr<FilteringContext> filter_ctx;
    void init_filter(FilteringContext * fctx, const char * filter_spec, AVCodecContext * dec_ctx, AVCodecContext * enc_ctx);
    
    typedef struct StreamContext {
        AVCodecContext *dec_ctx;
        AVCodecContext *enc_ctx;
    } StreamContext;
    StreamContext * stream_ctx;
    
    struct input_buffer_data {
        uint8_t * ptr;
        size_t size; ///< size left in the buffer
    };
    
    struct output_buffer_data {
        unique_ptr<uint8_t> buf;
        size_t size;
        uint8_t * ptr;
        size_t room; ///< size left in the buffer
    };
    
    uint8_t * input_buffer;
    size_t input_buffer_size;
    
    //unique_ptr<uint8_t> output_buffer;
    size_t output_buffer_size = 1024 ;
    
    output_buffer_data obd { 0 };
    input_buffer_data ibd { 0 };
    
    
};

#endif /* audio_processor_h */
