//
//  processor.hpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/8/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#ifndef processor_h
#define processor_h

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavformat/avio.h>
    #include <libavutil/file.h>
    #include <libavfilter/buffersink.h>
    #include <libavfilter/buffersrc.h>
    #include <libavutil/opt.h>
    #include <libavutil/avassert.h>
    #include <libavutil/audio_fifo.h>
    
    #include <libswresample/swresample.h>
}
using namespace std;

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

class AudioProcessor {
public:
    AudioProcessor(){};
    ~AudioProcessor();
    
    AVFormatContext * input_format_context = NULL;
    AVFormatContext * output_format_context = NULL;
    AVCodecContext  * input_codec_context = NULL;
    AVCodecContext  * output_codec_context = NULL;
    SwrContext * resample_context = NULL;
    AVAudioFifo * fifo = NULL;
    
    void set_input_content(uint8_t *data, size_t size);
    string get_output_content();
    
    void execute();
    
    int write_output_file_trailer();
    int convert_samples(const uint8_t **input_data, uint8_t **converted_data, const int frame_size);
    int load_encode_and_write();
    int encode_audio_frame(AVFrame *frame, int *data_present);
    int init_output_frame(AVFrame **frame, int frame_size);
    int read_decode_convert_and_store(int *finished);
    int add_samples_to_fifo(uint8_t **converted_input_samples, const int frame_size);
    int init_converted_samples(uint8_t ***converted_input_samples, int frame_size);
    int decode_audio_frame(AVFrame *frame, int *data_present, int *finished);
    int write_output_file_header();
    int init_fifo();
    int init_resampler();
    
    int init_input_frame(AVFrame **frame);
    void init_packet(AVPacket *packet);
    
    int open_output_file();
    int open_input_file();
    
    
    
    
private:
    int64_t pts = 0;
    
    /* This may need to be configurable down the road */
    int OUTPUT_BIT_RATE = 96000;
    int OUTPUT_CHANNELS = 2;
    
    typedef struct FilteringContext {
        AVFilterContext *buffersink_ctx;
        AVFilterContext *buffersrc_ctx;
        AVFilterGraph *filter_graph;
    } FilteringContext;
    FilteringContext *filter_ctx;
    
    uint8_t * input_buffer;
    size_t input_buffer_size;
    
    //unique_ptr<uint8_t> output_buffer;
    size_t output_buffer_size = 1024 ;
    
    output_buffer_data obd { 0 };
    input_buffer_data ibd { 0 };
    
    static int read_packet(void *opaque, uint8_t *buf, int buf_size);
    static int write_packet(void *opaque, uint8_t *buf, int buf_size);
};


#endif /* processor_h */
