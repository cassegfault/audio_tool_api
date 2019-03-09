//
//  processor.cpp
//  audio_tool_api
//
//  Created by Chris Pauley on 12/8/18.
//  Copyright © 2018 Chris Pauley. All rights reserved.
//

#include "processor.h"


void AudioProcessor::execute(){
    int ret = AVERROR_EXIT;
    
    /* Open the input file for reading. */
    if (open_input_file())
        goto cleanup;
    /* Open the output file for writing. */
    if (open_output_file())
        goto cleanup;
    /* Initialize the resampler to be able to convert audio sample formats. */
    if (init_resampler())
        goto cleanup;
    /* Initialize the FIFO buffer to store audio samples to be encoded. */
    if (init_fifo())
        goto cleanup;
    /* Write the header of the output file container. */
    if (write_output_file_header())
        goto cleanup;
    /* Loop as long as we have input samples to read or output samples
     * to write; abort as soon as we have neither. */
    while (1) {
        /* Use the encoder's desired frame size for processing. */
        const int output_frame_size = 1024 * 1024 * 20;//output_codec_context->frame_size;
        int finished                = 0;
        /* Make sure that there is one frame worth of samples in the FIFO
         * buffer so that the encoder can do its work.
         * Since the decoder's and the encoder's frame size may differ, we
         * need to FIFO buffer to store as many frames worth of input samples
         * that they make up at least one frame worth of output samples. */
        while (av_audio_fifo_size(fifo) < output_frame_size) {
            /* Decode one frame worth of audio samples, convert it to the
             * output sample format and put it into the FIFO buffer. */
            if (read_decode_convert_and_store(&finished))
                goto cleanup;
            /* If we are at the end of the input file, we continue
             * encoding the remaining audio samples to the output file. */
            if (finished)
                break;
        }
        /* If we have enough samples for the encoder, we encode them.
         * At the end of the file, we pass the remaining samples to
         * the encoder. */
        while (av_audio_fifo_size(fifo) >= output_frame_size ||
               (finished && av_audio_fifo_size(fifo) > 0))
        /* Take one frame worth of audio samples from the FIFO buffer,
         * encode it and write it to the output file. */
            if (load_encode_and_write())
                goto cleanup;
        /* If we are at the end of the input file and have encoded
         * all remaining samples, we can exit this loop and finish. */
        if (finished) {
            int data_written;
            /* Flush the encoder as it may have delayed frames. */
            do {
                data_written = 0;
                if (encode_audio_frame(NULL, &data_written))
                    goto cleanup;
            } while (data_written);
            break;
        }
    }
    /* Write the trailer of the output file container. */
    if (write_output_file_trailer())
        goto cleanup;
    ret = 0;
cleanup:
    if (fifo)
        av_audio_fifo_free(fifo);
    swr_free(&resample_context);
    if (output_codec_context)
        avcodec_free_context(&output_codec_context);
    if (input_codec_context)
        avcodec_free_context(&input_codec_context);
    if (input_format_context)
        avformat_close_input(&input_format_context);
    
}
AudioProcessor::~AudioProcessor(){
    //av_freep(input_buffer);
    //av_free(output_buffer);
    //input_buffer = nullptr;
    //output_buffer = nullptr;
    /*obd.buf = obd.ptr = nullptr;
    ibd.ptr = nullptr;*/
    //av_freep(obd.ptr);
    //av_freep(obd.buf);
    //av_freep(output_buffer);
    //av_freep(input_buffer);
    //delete input_buffer;
}

int AudioProcessor::read_packet(void *opaque, uint8_t *buf, int buf_size){
    struct input_buffer_data *bd = (struct input_buffer_data *)opaque;
    buf_size = fmin(buf_size, bd->size);
    if (!buf_size)
        return AVERROR_EOF;
    //printf("ptr:%p size:%zu\n", bd->ptr, bd->size);
    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;
    bd->size -= buf_size;
    return buf_size;
}

int AudioProcessor::write_packet(void *opaque, uint8_t *buf, int buf_size){
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


void AudioProcessor::set_input_content(uint8_t * data, size_t size) {
    this->input_buffer = data;
    this->input_buffer_size = size;
}

string AudioProcessor::get_output_content(){
    return string((const char *)this->obd.buf.get(), this->obd.size - this->obd.room);
}

int AudioProcessor::open_input_file()
{
    AVCodecContext *avctx;
    AVCodec *input_codec;
    AVIOContext *avio_ctx = NULL;
    int error;
    uint8_t *avio_ctx_buffer = NULL;
    int avio_ctx_buffer_size = 4096;
    int stream_number;
    //ibd.ptr.reset((uint8_t *)input_buffer.get());
    ibd.ptr = (uint8_t *) input_buffer;
    ibd.size = input_buffer_size;
    
    if (!(input_format_context = avformat_alloc_context())) {
        error = AVERROR(ENOMEM);
        return error;
    }
    
    avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
    if (!avio_ctx_buffer) {
        error = AVERROR(ENOMEM);
        return error;
    }
    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, &ibd, &AudioProcessor::read_packet, NULL, NULL);
    if (!avio_ctx) {
        error = AVERROR(ENOMEM);
        return error;
    }
    input_format_context->pb = avio_ctx;
    
    
    
    error = avformat_open_input(&input_format_context, NULL, NULL, NULL);
    if (error < 0) {
        fprintf(stderr, "Could not open input\n");
        return error;
    }
    
    /* Get information on the input file (number of streams etc.). */
    if ((error = avformat_find_stream_info(input_format_context, NULL)) < 0) {
        fprintf(stderr, "Could not open find stream info (error '%s')\n",
                av_err2str(error));
        avformat_close_input(&input_format_context);
        return error;
    }
    /* select the audio stream */
    stream_number = -1;
    int audio_stream_count = 0;
    for(int i=0; i < input_format_context->nb_streams; i++) {
        if(input_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_count++;
            stream_number = i;
            //break;
        }
    }
    /*error = av_find_best_stream(input_format_context, AVMEDIA_TYPE_AUDIO, -1, -1, &input_codec, 0);
    if (error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find an audio stream in the input file\n");
        return error;
    }
     */
    stream_number = error;
    /* Find a decoder for the audio stream. */
    if (!(input_codec = avcodec_find_decoder(input_format_context->streams[stream_number]->codecpar->codec_id))) {
        fprintf(stderr, "Could not find input codec\n");
        avformat_close_input(&input_format_context);
        return AVERROR_EXIT;
    }
    /* Allocate a new decoding context. */
    avctx = avcodec_alloc_context3(input_codec);
    if (!avctx) {
        fprintf(stderr, "Could not allocate a decoding context\n");
        avformat_close_input(&input_format_context);
        return AVERROR(ENOMEM);
    }
    /* Initialize the stream parameters with demuxer information. */
    error = avcodec_parameters_to_context(avctx, input_format_context->streams[stream_number]->codecpar);
    if (error < 0) {
        avformat_close_input(&input_format_context);
        avcodec_free_context(&avctx);
        return error;
    }
    /* Open the decoder for the audio stream to use it later. */
    if ((error = avcodec_open2(avctx, input_codec, NULL)) < 0) {
        fprintf(stderr, "Could not open input codec (error '%s')\n",
                av_err2str(error));
        avcodec_free_context(&avctx);
        avformat_close_input(&input_format_context);
        return error;
    }
    /* Save the decoder context for easier access later. */
    input_codec_context = avctx;
    return 0;
}
/**
 * Open an output file and the required encoder.
 * Also set some basic encoder parameters.
 * Some of these parameters are based on the input file's parameters.
 * @return Error code (0 if successful)
 */
int AudioProcessor::open_output_file()
{
    AVCodecContext *avctx          = NULL;
    AVIOContext *output_io_context = NULL;
    AVStream *stream               = NULL;
    AVCodec *output_codec          = NULL;
    int error = 0;
    uint8_t *avio_ctx_buffer = NULL;
    int avio_ctx_buffer_size = 4096;
    //output_buffer = (uint8_t *)av_malloc(output_buffer_size);
    obd.buf.reset((uint8_t *)av_malloc(output_buffer_size));
    obd.ptr = obd.buf.get();
    obd.size = obd.room = output_buffer_size;
    
    avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
    if (!avio_ctx_buffer) {
        error = AVERROR(ENOMEM);
        return error;
    }
    output_io_context = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 1, &obd, NULL, &write_packet, NULL);
    if (!output_io_context) {
        error = AVERROR(ENOMEM);
        return error;
    }
    
    /* Create a new format context for the output container format. */
    if (!(output_format_context = avformat_alloc_context())) {
        fprintf(stderr, "Could not allocate output format context\n");
        return AVERROR(ENOMEM);
    }
    /* Associate the output file (pointer) with the container format context. */
    output_format_context->pb = output_io_context;
    /* Guess the desired container format based on the file extension. */
    if (!(output_format_context->oformat = av_guess_format("wav", NULL,
                                                              NULL))) {
        fprintf(stderr, "Could not find output file format\n");
        goto cleanup;
    }
    /* Find the encoder to be used by its name. */
    if (!(output_codec = avcodec_find_encoder(AV_CODEC_ID_PCM_F32LE))) {
        fprintf(stderr, "Could not find an 32Bit Floating Point PCM encoder.\n");
        goto cleanup;
    }
    /* Create a new audio stream in the output file container. */
    if (!(stream = avformat_new_stream(output_format_context, NULL))) {
        fprintf(stderr, "Could not create new stream\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }
    avctx = avcodec_alloc_context3(output_codec);
    if (!avctx) {
        fprintf(stderr, "Could not allocate an encoding context\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }
    /* Set the basic encoder parameters.
     * The input file's sample rate is used to avoid a sample rate conversion. */
    avctx->channels       = OUTPUT_CHANNELS;
    avctx->channel_layout = av_get_default_channel_layout(OUTPUT_CHANNELS);
    avctx->sample_rate    = input_codec_context->sample_rate;
    avctx->sample_fmt     = output_codec->sample_fmts[0];
    avctx->bit_rate       = OUTPUT_BIT_RATE;
    /* Allow the use of the experimental AAC encoder. */
    avctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    /* Set the sample rate for the container. */
    stream->time_base.den = input_codec_context->sample_rate;
    stream->time_base.num = 1;
    /* Some container formats (like MP4) require global headers to be present.
     * Mark the encoder so that it behaves accordingly. */
    if (output_format_context->oformat->flags & AVFMT_GLOBALHEADER)
        avctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    /* Open the encoder for the audio stream to use it later. */
    if ((error = avcodec_open2(avctx, output_codec, NULL)) < 0) {
        fprintf(stderr, "Could not open output codec (error '%s')\n",
                av_err2str(error));
        goto cleanup;
    }
    error = avcodec_parameters_from_context(stream->codecpar, avctx);
    if (error < 0) {
        fprintf(stderr, "Could not initialize stream parameters\n");
        goto cleanup;
    }
    /* Save the encoder context for easier access later. */
    output_codec_context = avctx;
    return 0;
cleanup:
    avcodec_free_context(&avctx);
    avio_closep(&output_format_context->pb);
    avformat_free_context(output_format_context);
    output_format_context = NULL;
    return error < 0 ? error : AVERROR_EXIT;
}
/**
 * Initialize one data packet for reading or writing.
 * @param packet Packet to be initialized
 */
void AudioProcessor::init_packet(AVPacket *packet)
{
    av_init_packet(packet);
    /* Set the packet data and size so that it is recognized as being empty. */
    packet->data = NULL;
    packet->size = 0;
}
/**
 * Initialize one audio frame for reading from the input file.
 * @param[out] frame Frame to be initialized
 * @return Error code (0 if successful)
 */
int AudioProcessor::init_input_frame(AVFrame **frame)
{
    if (!(*frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate input frame\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}
/**
 * Initialize the audio resampler based on the input and output codec settings.
 * If the input and output sample formats differ, a conversion is required
 * libswresample takes care of this, but requires initialization.
 * @return Error code (0 if successful)
 */
int AudioProcessor::init_resampler()
{
    int error;
    /*
     * Create a resampler context for the conversion.
     * Set the conversion parameters.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity (they are sometimes not detected
     * properly by the demuxer and/or decoder).
     */
    resample_context = swr_alloc_set_opts(NULL,
                                           av_get_default_channel_layout(output_codec_context->channels),
                                           output_codec_context->sample_fmt,
                                           output_codec_context->sample_rate,
                                           av_get_default_channel_layout(input_codec_context->channels),
                                           input_codec_context->sample_fmt,
                                           input_codec_context->sample_rate,
                                           0, NULL);
    if (!resample_context) {
        fprintf(stderr, "Could not allocate resample context\n");
        return AVERROR(ENOMEM);
    }
    /*
     * Perform a sanity check so that the number of converted samples is
     * not greater than the number of samples to be converted.
     * If the sample rates differ, this case has to be handled differently
     */
    av_assert0(output_codec_context->sample_rate == input_codec_context->sample_rate);
    /* Open the resampler with the specified parameters. */
    if ((error = swr_init(resample_context)) < 0) {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(&resample_context);
        return error;
    }
    return 0;
}
/**
 * Initialize a FIFO buffer for the audio samples to be encoded.
 * @return Error code (0 if successful)
 */
int AudioProcessor::init_fifo()
{
    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(fifo = av_audio_fifo_alloc(output_codec_context->sample_fmt,
                                      output_codec_context->channels, 1))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}
/**
 * Write the header of the output file container.
 * @return Error code (0 if successful)
 */
int AudioProcessor::write_output_file_header()
{
    int error;
    /* see https://stackoverflow.com/questions/31371190/create-a-44-byte-header-with-ffmpeg */
    if ((error = avformat_write_header(output_format_context, &output_format_context->metadata)) < 0) {
        fprintf(stderr, "Could not write output file header (error '%s')\n",
                av_err2str(error));
        return error;
    }
    return 0;
}
/**
 * Decode one audio frame from the input file.
 * @param      frame                Audio frame to be decoded
 * @param[out] data_present         Indicates whether data has been decoded
 * @param[out] finished             Indicates whether the end of file has
 *                                  been reached and all data has been
 *                                  decoded. If this flag is false, there
 *                                  is more data to be decoded, i.e., this
 *                                  function has to be called again.
 * @return Error code (0 if successful)
 */
int AudioProcessor::decode_audio_frame(AVFrame *frame,
                                       int *data_present, int *finished)
{
    /* Packet used for temporary storage. */
    AVPacket input_packet;
    int error;
    init_packet(&input_packet);
    /* Read one audio frame from the input file into a temporary packet. */
    if ((error = av_read_frame(input_format_context, &input_packet)) < 0) {
        /* If we are at the end of the file, flush the decoder below. */
        if (error == AVERROR_EOF)
            *finished = 1;
        else {
            fprintf(stderr, "Could not read frame (error '%s')\n",
                    av_err2str(error));
            return error;
        }
    }
    /* Send the audio frame stored in the temporary packet to the decoder.
     * The input audio stream decoder is used to do this. */
    if ((error = avcodec_send_packet(input_codec_context, &input_packet)) < 0) {
        fprintf(stderr, "Could not send packet for decoding (error '%s')\n",
                av_err2str(error));
        return error;
    }
    /* Receive one frame from the decoder. */
    error = avcodec_receive_frame(input_codec_context, frame);
    /* If the decoder asks for more data to be able to decode a frame,
     * return indicating that no data is present. */
    if (error == AVERROR(EAGAIN)) {
        error = 0;
        goto cleanup;
        /* If the end of the input file is reached, stop decoding. */
    } else if (error == AVERROR_EOF) {
        *finished = 1;
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        fprintf(stderr, "Could not decode frame (error '%s')\n",
                av_err2str(error));
        goto cleanup;
        /* Default case: Return decoded data. */
    } else {
        *data_present = 1;
        goto cleanup;
    }
cleanup:
    av_packet_unref(&input_packet);
    return error;
}
/**
 * Initialize a temporary storage for the specified number of audio samples.
 * The conversion requires temporary storage due to the different format.
 * The number of audio samples to be allocated is specified in frame_size.
 * @param[out] converted_input_samples Array of converted samples. The
 *                                     dimensions are reference, channel
 *                                     (for multi-channel audio), sample.
 * @param      frame_size              Number of samples to be converted in
 *                                     each round
 * @return Error code (0 if successful)
 */
int AudioProcessor::init_converted_samples(uint8_t ***converted_input_samples,
                                           int frame_size)
{
    int error;
    /* Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */
    if (!(*converted_input_samples = (uint8_t **) calloc(output_codec_context->channels,
                                                         sizeof(**converted_input_samples)))) {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }
    /* Allocate memory for the samples of all channels in one consecutive
     * block for convenience. */
    if ((error = av_samples_alloc(*converted_input_samples, NULL,
                                  output_codec_context->channels,
                                  frame_size,
                                  output_codec_context->sample_fmt, 0)) < 0) {
        fprintf(stderr,
                "Could not allocate converted input samples (error '%s')\n",
                av_err2str(error));
        av_freep(&(*converted_input_samples)[0]);
        free(*converted_input_samples);
        return error;
    }
    return 0;
}
/**
 * Convert the input audio samples into the output sample format.
 * The conversion happens on a per-frame basis, the size of which is
 * specified by frame_size.
 * @param      input_data       Samples to be decoded. The dimensions are
 *                              channel (for multi-channel audio), sample.
 * @param[out] converted_data   Converted samples. The dimensions are channel
 *                              (for multi-channel audio), sample.
 * @param      frame_size       Number of samples to be converted
 * @return Error code (0 if successful)
 */
int AudioProcessor::convert_samples(const uint8_t **input_data,
                                    uint8_t **converted_data, const int frame_size)
{
    int error;
    /* Convert the samples using the resampler. */
    if ((error = swr_convert(resample_context,
                             converted_data, frame_size,
                             input_data    , frame_size)) < 0) {
        fprintf(stderr, "Could not convert input samples (error '%s')\n",
                av_err2str(error));
        return error;
    }
    return 0;
}
/**
 * Add converted input audio samples to the FIFO buffer for later processing.
 * @param converted_input_samples Samples to be added. The dimensions are channel
 *                                (for multi-channel audio), sample.
 * @param frame_size              Number of samples to be converted
 * @return Error code (0 if successful)
 */
int AudioProcessor::add_samples_to_fifo(uint8_t **converted_input_samples,
                                        const int frame_size)
{
    int error;
    /* Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples. */
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }
    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void **)converted_input_samples,
                            frame_size) < frame_size) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}
/**
 * Read one audio frame from the input file, decode, convert and store
 * it in the FIFO buffer.

 * @param[out] finished             Indicates whether the end of file has
 *                                  been reached and all data has been
 *                                  decoded. If this flag is false,
 *                                  there is more data to be decoded,
 *                                  i.e., this function has to be called
 *                                  again.
 * @return Error code (0 if successful)
 */
int AudioProcessor::read_decode_convert_and_store(int *finished)
{
    /* Temporary storage of the input samples of the frame read from the file. */
    AVFrame *input_frame = NULL;
    /* Temporary storage for the converted input samples. */
    uint8_t **converted_input_samples = NULL;
    int data_present = 0;
    int ret = AVERROR_EXIT;
    /* Initialize temporary storage for one input frame. */
    if (init_input_frame(&input_frame))
        goto cleanup;
    /* Decode one frame worth of audio samples. */
    if (decode_audio_frame(input_frame, &data_present, finished))
        goto cleanup;
    /* If we are at the end of the file and there are no more samples
     * in the decoder which are delayed, we are actually finished.
     * This must not be treated as an error. */
    if (*finished) {
        ret = 0;
        goto cleanup;
    }
    /* If there is decoded data, convert and store it. */
    if (data_present) {
        /* Initialize the temporary storage for the converted input samples. */
        if (init_converted_samples(&converted_input_samples, input_frame->nb_samples))
            goto cleanup;
        /* Convert the input samples to the desired output sample format.
         * This requires a temporary storage provided by converted_input_samples. */
        if (convert_samples((const uint8_t**)input_frame->extended_data, converted_input_samples, input_frame->nb_samples))
            goto cleanup;
        /* Add the converted input samples to the FIFO buffer for later processing. */
        if (add_samples_to_fifo(converted_input_samples, input_frame->nb_samples))
            goto cleanup;
        ret = 0;
    }
    ret = 0;
cleanup:
    if (converted_input_samples) {
        av_freep(&converted_input_samples[0]);
        free(converted_input_samples);
    }
    av_frame_free(&input_frame);
    return ret;
}
/**
 * Initialize one input frame for writing to the output file.
 * The frame will be exactly frame_size samples large.
 * @param[out] frame                Frame to be initialized
 * @param      frame_size           Size of the frame
 * @return Error code (0 if successful)
 */
int AudioProcessor::init_output_frame(AVFrame **frame, int frame_size)
{
    int error;
    /* Create a new frame to store the audio samples. */
    if (!(*frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate output frame\n");
        return AVERROR_EXIT;
    }

    /* Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity. */
    (*frame)->nb_samples     = frame_size;
    (*frame)->channel_layout = output_codec_context->channel_layout;
    (*frame)->format         = output_codec_context->sample_fmt;
    (*frame)->sample_rate    = output_codec_context->sample_rate;
    /* Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified. */
    if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
        fprintf(stderr, "Could not allocate output frame samples (error '%s')\n",
                av_err2str(error));
        av_frame_free(frame);
        return error;
    }
    return 0;
}
/* Global timestamp for the audio frames. */

/**
 * Encode one frame worth of audio to the output file.
 * @param      frame                 Samples to be encoded
 * @param[out] data_present          Indicates whether data has been
 *                                   encoded
 * @return Error code (0 if successful)
 */
int AudioProcessor::encode_audio_frame(AVFrame *frame,
                                       int *data_present)
{
    /* Packet used for temporary storage. */
    AVPacket output_packet;
    int error;
    init_packet(&output_packet);
    /* Set a timestamp based on the sample rate for the container. */
    if (frame) {
        frame->pts = pts;
        pts += frame->nb_samples;
    }
    /* Send the audio frame stored in the temporary packet to the encoder.
     * The output audio stream encoder is used to do this. */
    error = avcodec_send_frame(output_codec_context, frame);
    /* The encoder signals that it has nothing more to encode. */
    if (error == AVERROR_EOF) {
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        fprintf(stderr, "Could not send packet for encoding (error '%s')\n",
                av_err2str(error));
        return error;
    }
    /* Receive one encoded frame from the encoder. */
    error = avcodec_receive_packet(output_codec_context, &output_packet);
    /* If the encoder asks for more data to be able to provide an
     * encoded frame, return indicating that no data is present. */
    if (error == AVERROR(EAGAIN)) {
        error = 0;
        goto cleanup;
        /* If the last frame has been encoded, stop encoding. */
    } else if (error == AVERROR_EOF) {
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        fprintf(stderr, "Could not encode frame (error '%s')\n",
                av_err2str(error));
        goto cleanup;
        /* Default case: Return encoded data. */
    } else {
        *data_present = 1;
    }
    /* Write one audio frame from the temporary packet to the output file. */
    if (*data_present &&
        (error = av_write_frame(output_format_context, &output_packet)) < 0) {
        fprintf(stderr, "Could not write frame (error '%s')\n",
                av_err2str(error));
        goto cleanup;
    }
cleanup:
    av_packet_unref(&output_packet);
    return error;
}
/**
 * Load one audio frame from the FIFO buffer, encode and write it to the
 * output file.
 * @return Error code (0 if successful)
 */
int AudioProcessor::load_encode_and_write()
{
    /* Temporary storage of the output samples of the frame written to the file. */
    AVFrame *output_frame;
    /* Use the maximum number of possible samples per frame.
     * If there is less than the maximum possible frame size in the FIFO
     * buffer use this number. Otherwise, use the maximum possible frame size. */
    const int frame_size = FFMIN(av_audio_fifo_size(fifo),
                                 1024 * 1024 * 20);
    int data_written;
    /* Initialize temporary storage for one output frame. */
    if (init_output_frame(&output_frame, frame_size))
        return AVERROR_EXIT;
    /* Read as many samples from the FIFO buffer as required to fill the frame.
     * The samples are stored in the frame temporarily. */
    if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size) {
        fprintf(stderr, "Could not read data from FIFO\n");
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    /* Encode one frame worth of audio samples. */
    if (encode_audio_frame(output_frame, &data_written)) {
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    av_frame_free(&output_frame);
    return 0;
}
/**
 * Write the trailer of the output file container.
 * @return Error code (0 if successful)
 */
int AudioProcessor::write_output_file_trailer()
{
    int error;
    if ((error = av_write_trailer(output_format_context)) < 0) {
        fprintf(stderr, "Could not write output file trailer (error '%s')\n",
                av_err2str(error));
        return error;
    }
    return 0;
}
