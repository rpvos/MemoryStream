/**
 * @file memory_stream.cpp
 * @author rpvos (mr.rv.asd@gmail.com)
 * @brief Library used to test arduino projects
 * @version 1.0
 * @date 2023-09-20
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "memory_stream.h"
#include <Arduino.h>

const uint16_t kTimedReadTimeout = 1000U;
const char kStringEliminator = '\0';

/**
 * @brief Construct a new Memory Stream object
 *
 * @param seperate_input_output_buffer bool if two buffers are used, one for input and one for output
 * @param buffersize is the size of the buffers that will be used
 * @param maximum_amount_of_entries is the size of the buffer that will store the size of every entrie (outputs and inputs)
 */
MemoryStream::MemoryStream(bool seperate_input_output_buffer, uint8_t buffer_size, uint8_t maximum_amount_of_entries)
{
    this->buffer_size_ = buffer_size;
    this->main_buffer_ = new uint8_t[buffer_size_];

    this->use_two_buffers_ = seperate_input_output_buffer;

    if (this->use_two_buffers_)
    {
        this->maximum_amount_of_entries_ = maximum_amount_of_entries;
        InitTwoBuffers_();
    }
    else
    {
        this->output_buffer_ = nullptr;
        this->output_sizes_buffer_ = nullptr;
        this->maximum_amount_of_entries_ = 0;
    }

    this->write_cursor_ = 0;
    this->read_cursor_ = 0;
    this->available_bytes_ = 0;
}

void MemoryStream::InitTwoBuffers_(void)
{
    this->output_buffer_ = new uint8_t[buffer_size_];

    this->output_has_been_read = true;
    this->output_sizes_buffer_ = new uint8_t[this->maximum_amount_of_entries_];
    this->input_sizes_buffer_ = new uint8_t[this->maximum_amount_of_entries_];
    this->output_cursor_ = 0;
    this->input_cursor_ = 0;
    this->output_amount_ = 0;
    this->input_amount_ = 0;
    this->last_write_cursor_index_ = 0;
}

MemoryStream::~MemoryStream()
{
    delete[] this->main_buffer_;
    if (this->use_two_buffers_)
    {
        delete[] this->output_buffer_;
        delete[] this->output_sizes_buffer_;
        delete[] this->input_sizes_buffer_;
    }
}

int MemoryStream::available()
{
    // Return the amount of bytes that are available
    if (available_bytes_)
    {
        return available_bytes_;
    }

    // If no bytes are available check if multiple outputs are used
    if (!use_two_buffers_)
    {
        return 0;
    }

    // Check to make sure the timedRead does not consume all strings
    if (output_has_been_read)
    {
        // If there are more outputs defined then that have been read
        if (output_amount_)
        {
            // available bytes is not initialized yet so initialise it
            available_bytes_ = output_sizes_buffer_[output_cursor_ % maximum_amount_of_entries_];
            output_amount_--;
            output_cursor_++;
            output_has_been_read = false;
            return available_bytes_;
        }
    }
    else
    {
        // Delay for timed read
        delay(kTimedReadTimeout);
        output_has_been_read = true;
    }

    return 0;
}

int8_t MemoryStream::AddOutput(const char *output, uint8_t size)
{
    // Check if multiple outputs are eneabled
    if (!this->use_two_buffers_)
    {
        return kUseTwoBuffersNotEnabled;
    }

    // Calculate the used size of buffer
    uint8_t used_size = 0;
    for (size_t i = 0; i < output_amount_; i++)
    {
        used_size += output_sizes_buffer_[(output_cursor_ + i) % maximum_amount_of_entries_];
    }

    // Check if there is space for the new output
    if (buffer_size_ - used_size < size)
    {
        return kBufferOverflow;
    }

    // Add the new output to the list
    int index = (output_cursor_ + output_amount_) % maximum_amount_of_entries_;
    output_sizes_buffer_[index] = size;
    output_amount_++;

    // Calculate the amount of space in front of the current index
    int remaining_space_in_back = buffer_size_ - (read_cursor_ + used_size);

    // Copy new data to output buffer in the correct location
    if (size > remaining_space_in_back)
    {
        int space_needed_in_front = size - remaining_space_in_back;
        memcpy((output_buffer_ + read_cursor_ + used_size), output, remaining_space_in_back);
        memcpy(output_buffer_, (output + remaining_space_in_back), space_needed_in_front);
    }
    else
    {
        memcpy(output_buffer_ + read_cursor_ + used_size, output, size);
    }

    return kSucces;
}

int8_t MemoryStream::ReadInput(char *buffer, uint8_t buffer_size)
{
    // Check if multiple outputs are eneabled
    if (!this->use_two_buffers_)
    {
        return kUseTwoBuffersNotEnabled;
    }

    // Flush so an entry is made of current amount of written characters
    if (input_amount_ == 0)
    {
        flush();
    }

    // Calculate the used size of buffer
    uint8_t used_size = 0;
    for (size_t i = 1; i <= input_amount_; i++)
    {
        used_size += input_sizes_buffer_[(input_cursor_ - i) % maximum_amount_of_entries_];
    }

    if (used_size == 0)
    {

        memcpy(buffer, &kStringEliminator, 1);
        return kSucces;
    }

    int index = (input_cursor_ - input_amount_) % maximum_amount_of_entries_;
    int message_size = input_sizes_buffer_[index];

    // Check if there is space for the new output
    if (buffer_size < message_size)
    {
        return kBufferOverflow;
    }

    // Calculate the amount of space in front of the current index
    int begin_of_messages = (write_cursor_ - used_size) % buffer_size_;

    // Copy data from behind and infront of the cursor
    if (begin_of_messages + message_size > buffer_size_)
    {
        int space_needed_from_start = begin_of_messages + message_size - buffer_size_;
        memcpy(buffer, main_buffer_ + begin_of_messages, message_size - space_needed_from_start);
        memcpy((buffer + message_size - space_needed_from_start), main_buffer_, space_needed_from_start);
    }
    else
    {
        memcpy(buffer, main_buffer_ + begin_of_messages, message_size);
    }
    memcpy(buffer + message_size, &kStringEliminator, 1);

    // Note that first message has been read
    input_amount_--;

    return kSucces;
}

int MemoryStream::read()
{
    int c = peek();
    if (c >= 0)
    {
        // Make read cursor loop
        read_cursor_ = (read_cursor_ + 1) % buffer_size_;
        // Subtract 1 from length of chars that need to be read
        available_bytes_--;
    }
    return c;
}

int MemoryStream::peek()
{
    if (available() == 0)
    {
        // Input buffer empty
        return -1;
    }
    else
    {
        if (use_two_buffers_)
        {
            return output_buffer_[read_cursor_];
        }
        else
        {
            return main_buffer_[read_cursor_];
        }
    }
}

size_t MemoryStream::write(uint8_t c)
{
    if (available_bytes_ >= buffer_size_)
        return 0; // buffer is full
    else
    {
        // Put char in buffer
        main_buffer_[write_cursor_] = c;
        // Make write cursor loop
        write_cursor_ = (write_cursor_ + 1) % buffer_size_;
        if (!use_two_buffers_)
        {
            // Add 1 to length of chars that need to be read
            available_bytes_++;
        }

        // Return 1 because 1 char is written
        return 1;
    }
}

void MemoryStream::flush()
{
    if (use_two_buffers_)
    {
        // Check that message amount cant exceed maximum amount of entries
        if (input_amount_ == maximum_amount_of_entries_ - 1)
        {
            return;
        }

        if (write_cursor_ == last_write_cursor_index_)
        {
            return;
        }

        // Note index of message end
        input_sizes_buffer_[input_cursor_ % maximum_amount_of_entries_] = (write_cursor_ - last_write_cursor_index_) % buffer_size_;
        last_write_cursor_index_ = write_cursor_;
        input_amount_++;
        input_cursor_++;
    }

    return;
}

uint8_t *MemoryStream::GetBuffer()
{
    return this->main_buffer_;
}

uint8_t *MemoryStream::GetOutputBuffer()
{
    return this->output_buffer_;
}

void MemoryStream::SetOutputLength(uint8_t output_length)
{
    this->available_bytes_ = output_length;
}

void MemoryStream::SetReadCursor(uint8_t read_cursor)
{
    this->read_cursor_ = read_cursor;
}