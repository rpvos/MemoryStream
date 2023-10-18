/**
 * @file memory_stream.cpp
 * @author rpvos (mr.rv.asd@gmail.com)
 * @brief Library used to test max485ttl.cpp
 * @version 0.1
 * @date 2023-09-20
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "memory_stream.h"
#include <Arduino.h>

uint16_t kTimedReadTimeout = 1000U;

// Include string.h for memcpy
#ifndef ARDUINO
#include <string.h>

void delay(int number_of_millis)
{

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + number_of_millis)
        ;
}
#endif

/**
 * @brief Construct a new Memory Stream object
 *
 * @param buffersize is the size of the buffers that will be used
 * @param seperate_input_output_buffer bool if two buffers are used, one for input and one for output
 * @param use_multiple_outputs bool that is used to determine if multiple outputs are used
 * @param maximum_amount_of_outputs is the size of the buffer that will store the size of every output
 */
MemoryStream::MemoryStream(uint8_t buffersize, bool seperate_input_output_buffer, bool use_multiple_outputs, uint8_t maximum_amount_of_outputs)
{
    this->buffer_size_ = buffersize;
    this->main_buffer_ = new uint8_t[buffersize];

    this->use_two_buffers_ = seperate_input_output_buffer;
    if (this->use_two_buffers_)
    {
        this->output_buffer_ = new uint8_t[buffersize];
    }
    else
    {
        this->output_buffer_ = nullptr;
    }

    this->use_multiple_outputs_ = use_multiple_outputs;
    this->output_index_ = 0;
    this->output_amount_ = 0;
    this->output_has_been_read = true;
    if (this->use_multiple_outputs_)
    {
        this->output_sizes_buffer_ = new uint8_t[maximum_amount_of_outputs];
    }
    else
    {
        this->output_sizes_buffer_ = nullptr;
    }

    this->write_cursor_ = 0;
    this->read_cursor_ = 0;
    this->available_bytes_ = 0;
}

MemoryStream::~MemoryStream()
{
    delete[] this->main_buffer_;
    if (this->output_buffer_ != nullptr)
    {
        delete[] this->output_buffer_;
    }

    if (this->output_sizes_buffer_ != nullptr)
    {
        delete[] this->output_sizes_buffer_;
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
    if (!use_multiple_outputs_)
    {
        return 0;
    }

    // Check to make sure the timedRead does not consume all strings
    if (output_has_been_read)
    {
        // If there are more outputs defined then that have been read
        if (output_amount_ - output_index_)
        {
            // available bytes is not initialized yet so initialise it
            available_bytes_ = output_sizes_buffer_[output_index_];
            output_index_++;
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

void MemoryStream::AddOutput(const char *output, uint8_t size)
{
    // Check if multiple outputs are eneabled
    if (!this->use_multiple_outputs_)
    {
        return;
    }

    // Calculate the used size of buffer
    uint8_t used_size = 0;
    for (size_t i = 0; i < output_amount_; i++)
    {
        used_size += output_sizes_buffer_[i];
    }

    // Check if there is space for the new output
    if (buffer_size_ - used_size < size)
    {
        return;
    }

    // Add the new output to the list
    output_amount_++;
    output_sizes_buffer_[output_amount_ - 1] = size;
    // Copy new data to output buffer in the correct location
    memcpy(output_buffer_ + used_size, output, size);
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
    return;
}

uint8_t *MemoryStream::GetBuffer()
{
    return this->main_buffer_;
}

uint8_t *MemoryStream::GetSecondBuffer()
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