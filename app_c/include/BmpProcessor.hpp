#pragma once

#include <string>
#include <vector>
#include <thread>
#include <chrono>

struct Pixel {
    int r;
    int g;
    int b;
};

static std::vector<Pixel>& ImageDataToPixelArray(unsigned char* imageData, int width, int height, int channels);

static unsigned char* PixelArrayToImageData(const std::vector<Pixel>& pixelArray, int width, int height, int channels);
class BmpProcessor
{
    public:
        BmpProcessor(const std::string& filename);
        ~BmpProcessor() = default;

        bool GetIsReady() { return _ready; }

        void ProcessImageMultithread(int threadCount = 1);

        void ProcessImageSingleThread();

        void SaveFile(const std::string& filename);

    private:
        void ProcessImage(int startLine, int endLine);
        void PerformIntencityStep(int x, int y);
        void PerformMinimizationStep(int x, int y);

    private:
        bool _ready = false;
        // threading and time
        std::vector<std::thread> _threads;
        std::chrono::steady_clock::time_point _tsBegin;
        std::chrono::steady_clock::time_point _tsEnd;

        // conv stuff
        int _convCore[3][3] = {
            /* relief matrix */
            {-2, -1, 0},
            {-1,  1, 1},
            { 0,  1, 2}
            /* edge detection */
            // {0,  1, 0},
            // {1, -4, 1},
            // {0,  1, 0}
            /* nothing kekw */
            // {0, 0, 0},
            // {0, 1, 0},
            // {0, 0, 0}
        };
        int _kernelHeight = 3;
        int _kernelWidth = 3;
        
        // image processing
        int _width, _height, _channels;
        int _minimizationScale = 2;
        int _minimizedWidth, _minimizedHeight;

        std::vector<Pixel> _initialPixelArray;
        std::vector<Pixel> _convPixelArray;
        std::vector<Pixel> _minimizedPixelArray;
};