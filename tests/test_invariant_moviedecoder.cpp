#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include "ffmpegthumbnailer/moviedecoder.h"

class BufferOverflowTest : public ::testing::TestWithParam<std::pair<std::string, int>> {};

TEST_P(BufferOverflowTest, NoBufferOverreadOnOversizedFrames) {
    // Invariant: Buffer reads never exceed the declared length
    auto [video_path, expected_behavior] = GetParam();
    
    ffmpegthumbnailer::MovieDecoder decoder(video_path, ffmpegthumbnailer::AVFormatContextPtr());
    
    try {
        decoder.initialize();
        ffmpegthumbnailer::VideoFrame frame;
        decoder.getScaledVideoFrame(10, true, frame);
        
        // If we reach here without crash, verify frame data size is bounded
        ASSERT_LE(frame.frameData.size(), 1920 * 1080 * 4 * 10) 
            << "Frame data exceeds reasonable bounds for decoded frame";
        
        if (expected_behavior == 0) {
            SUCCEED() << "Valid input processed correctly";
        }
    } catch (const std::exception& e) {
        if (expected_behavior == 1) {
            SUCCEED() << "Malicious input rejected: " << e.what();
        } else {
            FAIL() << "Unexpected exception on valid input: " << e.what();
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    BufferOverflowTest,
    ::testing::Values(
        std::make_pair("test_data/malicious_oversized_frame.mp4", 1),
        std::make_pair("test_data/boundary_max_resolution.mp4", 1),
        std::make_pair("test_data/valid_small.mp4", 0)
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}