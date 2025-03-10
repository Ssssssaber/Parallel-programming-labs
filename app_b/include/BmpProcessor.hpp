#pragma once

#include <string>
#include <vector>
#include <thread>
#include <chrono>

struct Pixel {
    int R;
    int G;
    int B;

    Pixel() = default;
    Pixel(int initial) : R(initial), G(initial), B(initial) {} 
    Pixel(int r, int g, int b) : R(r), G(g), B(b) {}

    void SetAll(int value)
    {
        R = value;
        G = value;
        B = value;
    }
};

static std::vector<Pixel>& ImageDataToPixelArray(unsigned char* imageData, int width, int height, int channels);

static unsigned char* PixelArrayToImageData(const std::vector<Pixel>& pixelArray, int width, int height, int channels);
class BmpProcessor
{
    public:
        BmpProcessor(const std::string& filename, int threshold = 160, int erosionStep = 1);
        ~BmpProcessor() = default;

        bool GetIsReady() { return _ready; }

        void ProcessImageMultithread(int threadCount = 1);

        void ProcessImageSingleThread();

        void SaveFile(const std::string& filename);

    private:
        void ProcessImage(int startLine, int endLine);
        bool PerformErosion(int x, int y);

    private:
        bool _ready = false;
        // threading and time
        std::vector<std::thread> _threads;
        std::chrono::steady_clock::time_point _tsBegin;
        std::chrono::steady_clock::time_point _tsEnd;

        // image processing
        int _width, _height, _channels;
        int _intencityThreshold;
        int _erosionStep;

        std::vector<Pixel> _initialPixelArray;
        std::vector<int> _thresholdArray;
        std::vector<Pixel> _resultPixelArray;
};