#include <unity.h>
#include "memory_stream.h"
#include <Arduino.h>

MemoryStream *stream;

void test_available(void)
{
    // String used as output
    String s = "Hello world!";
    // Char used as output
    char c = 'A';

    // Test default constructor
    stream = new MemoryStream();
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    stream->write(c);
    TEST_ASSERT_EQUAL_INT(1, stream->available());
    stream->read();
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    stream->print(s);
    TEST_ASSERT_EQUAL_INT(s.length(), stream->available());
    stream->readString();
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    stream->~MemoryStream();

    // Test seperate buffer
    stream = new MemoryStream(64U, true);
    // Buffer should not be filled when written to
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    stream->write(c);
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    stream->read();
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    // Fill buffer
    memcpy(stream->GetSecondBuffer(), s.c_str(), s.length());
    stream->SetOutputLength(s.length());

    // Available should be equal to amount of letters in second buffer
    TEST_ASSERT_EQUAL_INT(s.length(), stream->available());
    stream->readString();
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    stream->~MemoryStream();

    // Test multiple outputs
    stream = new MemoryStream(64U, true, true);
    // Buffer should not be filled when written to
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    stream->write(c);
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    stream->read();
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    // Fill buffer with letters
    stream->AddOutput(s.c_str(), s.length());

    // Available should be equal to amount of letters in second buffer
    TEST_ASSERT_EQUAL_INT(s.length(), stream->available());
    stream->readString();
    TEST_ASSERT_EQUAL_INT(0, stream->available());

    // Fill buffer with 2 strings
    stream->AddOutput(s.c_str(), s.length());
    stream->AddOutput(s.c_str(), s.length());
    TEST_ASSERT_EQUAL_INT(s.length(), stream->available());
    stream->readString();
    TEST_ASSERT_EQUAL_INT(s.length(), stream->available());
    stream->readString();
    TEST_ASSERT_EQUAL_INT(0, stream->available());

    stream->~MemoryStream();
}

void setUp(void)
{
}

void tearDown(void)
{
    if (stream != nullptr)
    {
        stream->~MemoryStream();
    }
}

int RunAllTests(void)
{
    UNITY_BEGIN(); // Start unit testing

    RUN_TEST(test_available);
    // RUN_TEST(test_read);
    // RUN_TEST(test_peek);
    // RUN_TEST(test_write);
    // RUN_TEST(test_GetBuffer);
    // RUN_TEST(test_GetSecondBuffer);
    // RUN_TEST(test_AddOutput);

    return UNITY_END(); // Stop unit testing
}

#ifdef ARDUINO
#include <Arduino.h>

/**
 * @brief Entry point to start all tests
 *
 */
void setup()
{
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    RunAllTests();
}

/**
 * @brief Do nothing after all tests have succeeded
 *
 */
void loop()
{
    delay(2000);
}
#else
#include <time.h>

int main(void)
{
    return RunAllTests();
}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = number_of_seconds;

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

#endif