/**
 * @file memory_stream.h
 * @author rpvos (mr.rv.asd@gmail.com)
 * @brief Library used to test embedded software without making a mockup
 * @version 1.0
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
    MemoryStream(bool use_two_buffers = false, uint8_t buffersize = 64U, uint8_t amount_of_outputs = 8U);
    ~MemoryStream();

    int available() override;

    int read() override;

    int peek() override;

    size_t write(uint8_t c) override;

    void flush() override;

    uint8_t *GetBuffer();
    uint8_t *GetOutputBuffer();
    void SetOutputLength(uint8_t input_length);
    void SetReadCursor(uint8_t read_cursor);
    int8_t AddOutput(const char *output, uint8_t size);
    int8_t ReadInput(char *buffer, uint8_t buffer_size);

    enum Errorcode
    {
        kSucces = 1,
        kBufferOverflow = -1,
        kUseTwoBuffersNotEnabled = -2,
    };

private:
    void InitTwoBuffers_(void);
    uint8_t *main_buffer_;

    uint8_t buffer_size_;

    uint8_t write_cursor_;

    uint8_t read_cursor_;

    uint8_t available_bytes_;

    bool use_two_buffers_;
    uint8_t *output_buffer_;
    uint8_t output_cursor_;
    uint8_t input_cursor_;

    /**
     * @brief Maximum slots for outputs
     *
     */
    uint8_t maximum_amount_of_entries_;

    /**
     * @brief Amount of outputs used
     *
     */
    uint8_t output_amount_;

    /**
     * @brief Amount of inputs
     *
     */
    uint8_t input_amount_;

    /**
     * @brief Bool used to make sure a empty symbol is send when a output has been read fully
     *
     */
    bool output_has_been_read;

    /**
     * @brief Array used to set the size of the outputs
     *
     */
    uint8_t *output_sizes_buffer_;

    /**
     * @brief Array used to set the size of the inputs
     *
     */
    uint8_t *input_sizes_buffer_;

    /**
     * @brief Keeping last index of write cursor for calculating size
     *
     */
    uint8_t last_write_cursor_index_;
};

#endif // MEMORY_STREAM_H_