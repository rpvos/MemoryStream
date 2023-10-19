#include <Arduino.h>
#include <unity.h>
#include "memory_stream.h"

MemoryStream *stream;

// String used as output
const String kTestString = "Hello world!";
// Char used as output
const char kTestCharacter = 'A';

void test_ReadInput(void)
{
    stream = new MemoryStream(true);

    // Test one input
    TEST_ASSERT_EQUAL_INT(kTestString.length(), stream->print(kTestString));
    uint8_t buffer_size = 64;
    char buffer[buffer_size];
    TEST_ASSERT_EQUAL_INT(MemoryStream::kSucces, stream->ReadInput(buffer, buffer_size));
    TEST_ASSERT_EQUAL_STRING(kTestString.c_str(), buffer);

    TEST_ASSERT_EQUAL_INT(kTestString.length(), stream->print(kTestString));
    stream->flush();
    TEST_ASSERT_EQUAL_INT(kTestString.length(), stream->print(kTestString));
    stream->flush();
    TEST_ASSERT_EQUAL_INT(MemoryStream::kSucces, stream->ReadInput(buffer, buffer_size));
    TEST_ASSERT_EQUAL_STRING(kTestString.c_str(), buffer);
    TEST_ASSERT_EQUAL_INT(MemoryStream::kSucces, stream->ReadInput(buffer, buffer_size));
    TEST_ASSERT_EQUAL_STRING(kTestString.c_str(), buffer);

    TEST_ASSERT_EQUAL_INT(MemoryStream::kSucces, stream->ReadInput(buffer, buffer_size));
    TEST_ASSERT_EQUAL_STRING("", buffer);
}

void test_AddOutput(void)
{
    uint8_t buffer_size = 64U;
    stream = new MemoryStream(true, buffer_size);

    // Fill buffer with 2 strings
    TEST_ASSERT_EQUAL_INT(MemoryStream::kSucces, stream->AddOutput(kTestString.c_str(), kTestString.length()));
    TEST_ASSERT_EQUAL_INT(MemoryStream::kSucces, stream->AddOutput(kTestString.c_str(), kTestString.length()));
    TEST_ASSERT_EQUAL_INT(kTestString.length(), stream->available());
    stream->readString();
    TEST_ASSERT_EQUAL_INT(kTestString.length(), stream->available());
    stream->readString();
    TEST_ASSERT_EQUAL_INT(0, stream->available());

    // Test for edge case buffer overflow
    char buffer_overflow[buffer_size];
    for (uint8_t i = 0; i < buffer_size - 1; i++)
    {
        buffer_overflow[i] = ('A' + i);
    }
    buffer_overflow[buffer_size - 1] = '\0';

    TEST_ASSERT_EQUAL_INT(MemoryStream::kSucces, stream->AddOutput(buffer_overflow, buffer_size));
    TEST_ASSERT_EQUAL_INT(MemoryStream::kBufferOverflow, stream->AddOutput(buffer_overflow, buffer_size));

    TEST_ASSERT_EQUAL_STRING(buffer_overflow, stream->readString().c_str());
    TEST_ASSERT_EQUAL_STRING("", stream->readString().c_str());

    stream->~MemoryStream();

    // Test for edge case no two buffers
    stream = new MemoryStream(false);
    TEST_ASSERT_EQUAL_INT(MemoryStream::kUseTwoBuffersNotEnabled, stream->AddOutput(buffer_overflow, buffer_size));
}

void test_BasicFunctionsMultipleBuffers(void)
{
    // Test multiple buffers and outputs
    stream = new MemoryStream(true);

    // Test seperate buffer
    // Buffer should not be filled when written to
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    TEST_ASSERT_EQUAL_CHAR(-1, stream->peek());
    memcpy(stream->GetOutputBuffer(), &kTestCharacter, sizeof(kTestCharacter));
    stream->SetReadCursor(0);
    stream->SetOutputLength(1);
    TEST_ASSERT_EQUAL_CHAR(kTestCharacter, stream->peek());
    TEST_ASSERT_EQUAL_INT(1, stream->available());
    stream->read();
    TEST_ASSERT_EQUAL_INT(0, stream->available());

    // Test second buffer
    memcpy(stream->GetOutputBuffer(), kTestString.c_str(), kTestString.length());
    stream->SetReadCursor(0);
    stream->SetOutputLength(kTestString.length());

    // Available should be equal to amount of letters in second buffer
    TEST_ASSERT_EQUAL_INT(kTestString.length(), stream->available());
    TEST_ASSERT_EQUAL_STRING(kTestString.c_str(), stream->readString().c_str());
    TEST_ASSERT_EQUAL_INT(0, stream->available());
}

/**
 * @brief Test containing available, write, print, peek, read and read string functions
 *
 */
void test_BasicFunctionsDefault(void)
{
    // Test default constructor
    stream = new MemoryStream();
    // No char is available
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    TEST_ASSERT_EQUAL_CHAR(-1, stream->peek());
    stream->write(kTestCharacter);
    TEST_ASSERT_EQUAL_CHAR(kTestCharacter, stream->peek());
    TEST_ASSERT_EQUAL_INT(1, stream->available());
    stream->read();
    TEST_ASSERT_EQUAL_INT(0, stream->available());
    stream->print(kTestString);
    stream->flush();
    TEST_ASSERT_EQUAL_INT(kTestString.length(), stream->available());
    TEST_ASSERT_EQUAL_STRING(kTestString.c_str(), stream->readString().c_str());
    TEST_ASSERT_EQUAL_INT(0, stream->available());

    // Test main buffer getter
    memcpy(stream->GetBuffer(), kTestString.c_str(), kTestString.length());
    stream->SetReadCursor(0);
    stream->SetOutputLength(kTestString.length());
    TEST_ASSERT_EQUAL_INT(kTestString.length(), stream->available());
    TEST_ASSERT_EQUAL_STRING(kTestString.c_str(), stream->readString().c_str());
}

void setUp(void)
{
}

void tearDown(void)
{
    if (stream != nullptr)
    {
        delete stream;
        stream = nullptr;
    }
}

int RunAllTests(void)
{
    UNITY_BEGIN(); // Start unit testing

    RUN_TEST(test_BasicFunctionsDefault);
    RUN_TEST(test_BasicFunctionsMultipleBuffers);
    RUN_TEST(test_AddOutput);
    RUN_TEST(test_ReadInput);

    return UNITY_END(); // Stop unit testing
}

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