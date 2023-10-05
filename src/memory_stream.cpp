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

MemoryStream::MemoryStream(uint8_t buffersize, bool use_two_buffers)
{
    this->buffer_size_ = buffersize;
    this->buffer_ = new uint8_t[buffersize];
    this->use_two_buffers_ = use_two_buffers;
    if (this->use_two_buffers_)
    {
        this->second_buffer_ = new uint8_t[buffersize];
    }
    else
    {
        this->second_buffer_ = nullptr;
    }

    this->write_cursor_ = 0;
    this->read_cursor_ = 0;
    this->input_length_ = 0;
}

MemoryStream::~MemoryStream()
{
    delete[] this->buffer_;
    delete[] this->second_buffer_;
}

int MemoryStream::read()
{
    int c = peek();
    if (c >= 0)
    {
        // Make read cursor loop
        read_cursor_ = (read_cursor_ + 1) % buffer_size_;
        // Subtract 1 from length of chars that need to be read
        input_length_--;
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
            return second_buffer_[read_cursor_];
        }
        else
        {
            return buffer_[read_cursor_];
        }
    }
}

size_t MemoryStream::write(uint8_t c)
{
    if (input_length_ >= buffer_size_)
        return 0; // buffer is full
    else
    {
        // Put char in buffer
        buffer_[write_cursor_] = c;
        // Make write cursor loop
        write_cursor_ = (write_cursor_ + 1) % buffer_size_;
        if (!use_two_buffers_)
        {
            // Add 1 to length of chars that need to be read
            input_length_++;
        }

        // Return 1 because 1 char is written
        return 1;
    }
}

uint8_t *MemoryStream::GetBuffer()
{
    return this->buffer_;
}

uint8_t *MemoryStream::GetSecondBuffer()
{
    return this->second_buffer_;
}

void MemoryStream::SetInputLength(uint8_t input_length)
{
    this->input_length_ = input_length;
}

void MemoryStream::SetReadCursor(uint8_t read_cursor)
{
    this->read_cursor_ = read_cursor;
}