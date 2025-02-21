#include "BmpProcessor.hpp"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

static std::vector<Pixel>& ImageDataToPixelArray(unsigned char* imageData, int width, int height, int channels)
{
    std::vector<Pixel>* pixelArray = new std::vector<Pixel>();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Pixel px(
                (int)imageData[y * width * channels + x * channels],    
                (int)imageData[y * width * channels + x * channels + 1],
                (int)imageData[y * width * channels + x * channels + 2]
            );
            pixelArray->push_back(px);
        }
    }

    return *pixelArray;
}

static unsigned char* PixelArrayToImageData(const std::vector<Pixel>& pixelArray, int width, int height, int channels)
{
    unsigned char* imageData = (unsigned char *) malloc(height * width * channels);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            imageData[y * width * channels + x * channels] = pixelArray[y * width + x].R;
            imageData[y * width * channels + x * channels + 1] = pixelArray[y * width + x].G;
            imageData[y * width * channels + x * channels + 2] = pixelArray[y * width + x].B;
        }
    }

    return imageData;
}

BmpProcessor::BmpProcessor(const std::string& filename, int intensityThreshold, int erosionStep) :
    _intencityThreshold(intensityThreshold),
    _erosionStep(erosionStep)
{
    unsigned char* imageData = stbi_load(filename.c_str(), &_width, &_height, &_channels, 3);

    if (!imageData)
    {
        std::cout << "Failed to load image";
        return;
    }
    
    std::cout << "Image: " << filename << "; Width: " << _width << "; Height: " << _height << "; Number of channels: " << _channels << "\n";
    
    _initialPixelArray = ImageDataToPixelArray(imageData, _width, _height, _channels);

    _thresholdArray.resize(_height * _width);
    _resultPixelArray.resize(_height * _width);

    stbi_image_free(imageData);

    _ready = true;
}

void BmpProcessor::ProcessImageMultithread(int threadCount)
{
    std::cout << "Started processing with " << threadCount << " thread(s)" << std::endl;
    _tsBegin = std::chrono::steady_clock::now();

    int step = _height / threadCount;
    for (int i = 0; i < threadCount; i++)
    {
        int start = i * step;
        int end = (i + 1) * step + 1;
        _threads.push_back(std::thread(&BmpProcessor::ProcessImage, this, start, end));
    }

    for (auto& thread : _threads)
    {
        if (thread.joinable()) thread.join();
    }

    _tsEnd = std::chrono::steady_clock::now();

    std::cout << "Ended processing. TIme elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(_tsEnd - _tsBegin).count() << " ms" << std::endl;
}

void BmpProcessor::ProcessImage(int startLine, int endLine)
{
    for (int y = startLine; y < endLine && y < _height; y++) 
    {
        for (int x = 0; x < _width; x++) 
        {
            int intensity = (_initialPixelArray[y * _width + x].R +
                             _initialPixelArray[y * _width + x].R +
                             _initialPixelArray[y * _width + x].R) / 3;
            
            if (intensity > _intencityThreshold) _thresholdArray[y * _width + x] = 1;
            else _thresholdArray[y * _width + x] = 0;
        }
    }

    for (int y = startLine; y < endLine && y < _height; y ++) 
    {
        for (int x = 0; x < _width; x ++) 
        {
            if (PerformErosion(x, y)) _resultPixelArray[y * _width + x].SetAll(25);
            else _resultPixelArray[y * _width + x].SetAll(230);
        }
    }
}

bool BmpProcessor::PerformErosion(int x, int y)
{
    for (int coreY = 0; coreY < _erosionStep; coreY++)
    {
        for (int coreX = 0; coreX < _erosionStep; coreX++)
        {
            int pixelX = x + (coreX - _erosionStep / 2);
            int pixelY = y + (coreY - _erosionStep / 2);
            
            if (pixelX < 0 || pixelX >= _width || pixelY < 0 || pixelY >= _height) continue;

            if (_thresholdArray[pixelY * _width + pixelX] == 1) return false;
        }
    }
    return true;
}

void BmpProcessor::ProcessImageSingleThread()
{
    ProcessImage(0, _height);
    
}

void BmpProcessor::SaveFile(const std::string& filename)
{
    unsigned char* imageData = PixelArrayToImageData(_resultPixelArray, _width, _height, _channels);

    stbi_write_bmp((filename).c_str(), _width, _height, 3, (const void*)imageData);
}