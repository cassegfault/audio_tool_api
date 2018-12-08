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
class AudioProcessor {
public:
    AudioProcessor(){};
    ~AudioProcessor(){};
    
    AVFormatContext * input_format_context = NULL;
    AVFormatContext * output_format_context = NULL;
    AVCodecContext  * input_codec_context = NULL;
    AVCodecContext  * output_codec_context = NULL;
    SwrContext * resample_context = NULL;
    AVAudioFifo * fifo = NULL;
    
    void set_input_content(void * data, size_t size);
    string get_output_content();
    
    void execute();
    
    int write_output_file_trailer(AVFormatContext *output_format_context);
    int convert_samples(const uint8_t **input_data, uint8_t **converted_data, const int frame_size, SwrContext *resample_context);
    int load_encode_and_write(AVAudioFifo *fifo, AVFormatContext *output_format_context, AVCodecContext *output_codec_context);
    int encode_audio_frame(AVFrame *frame, AVFormatContext *output_format_context, AVCodecContext *output_codec_context, int *data_present);
    int init_output_frame(AVFrame **frame, AVCodecContext *output_codec_context, int frame_size);
    int read_decode_convert_and_store(AVAudioFifo *fifo, AVFormatContext *input_format_context, AVCodecContext *input_codec_context, AVCodecContext *output_codec_context, SwrContext *resampler_context, int *finished);
    int add_samples_to_fifo(AVAudioFifo *fifo, uint8_t **converted_input_samples, const int frame_size);
    int init_converted_samples(uint8_t ***converted_input_samples, AVCodecContext *output_codec_context, int frame_size);
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
    
    int OUTPUT_BIT_RATE = 96000;
    int OUTPUT_CHANNELS = 2;
    
    uint8_t * input_buffer;
    size_t input_buffer_size;
    
    uint8_t * output_buffer;
    size_t output_buffer_size;
    
    static int read_packet(void *opaque, uint8_t *buf, int buf_size);
    static int write_packet(void *opaque, uint8_t *buf, int buf_size);
};

struct buffer_data {
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

#endif /* processor_h */
