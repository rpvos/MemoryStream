/**
 * @file memory_stream.h
 * @author rpvos (mr.rv.asd@gmail.com)
 * @brief Library used to test max485ttl.cpp
 * @version 0.1
 * @date 2023-09-20
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef MEMORY_STREAM_H_
#define MEMORY_STREAM_H_

#include <Stream.h>

class MemoryStream : public Stream
{
public:
    MemoryStream(uint8_t buffersize = 64, bool use_two_buffers = false, bool use_multiple_outputs = false, uint8_t amount_of_outputs = 8);
    ~MemoryStream();

    int available() override;

    int read() override;

    int peek() override;

    size_t write(uint8_t c) override;

    void flush() override;

    uint8_t *GetBuffer();
    uint8_t *GetSecondBuffer();
    void SetOutputLength(uint8_t input_length);
    void SetReadCursor(uint8_t read_cursor);
    void AddOutput(const char *output, uint8_t size);

private:
    bool use_two_buffers_;

    uint8_t *main_buffer_;

    uint8_t *output_buffer_;

    bool use_multiple_outputs_;

    uint8_t output_index_;
    uint8_t output_amount_;
    bool output_has_been_read;

    uint8_t *output_sizes_buffer_;

    uint8_t buffer_size_;

    uint8_t write_cursor_;

    uint8_t read_cursor_;

    uint8_t available_bytes_;
};

#endif // MEMORY_STREAM_H_