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
            Pixel px {
                .r = imageData[y * width * channels + x * channels],    
                .g = imageData[y * width * channels + x * channels + 1],
                .b = imageData[y * width * channels + x * channels + 2],
            };
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
            imageData[y * width * channels + x * channels] = pixelArray[y * width + x].r;
            imageData[y * width * channels + x * channels + 1] = pixelArray[y * width + x].g;
            imageData[y * width * channels + x * channels + 2] = pixelArray[y * width + x].b;
        }
    }

    return imageData;
}

BmpProcessor::BmpProcessor(const std::string& filename)
{
    unsigned char* imageData = stbi_load(filename.c_str(), &_width, &_height, &_channels, 3);

    if (!imageData)
    {
        std::cout << "Failed to load image";
        return;
    }
    
    std::cout << "Image: " << filename << "; Width: " << _width << "; Height: " << _height << "; Number of channels: " << _channels << "\n";
    
    _initialPixelArray = ImageDataToPixelArray(imageData, _width, _height, _channels);

    _convPixelArray.resize(_height * _width);

    _minimizedWidth = _width / 2;
    _minimizedHeight = _height / 2;
    
    _resultPixelArray.resize(_minimizedHeight * _minimizedWidth);

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
            PerformIntencityStep(x, y);
        }
    }

    for (int y = startLine; y < endLine && y < _height; y += _minimizationScale) 
    {
        for (int x = 0; x < _width; x += _minimizationScale) 
        {
            PerformMinimizationStep(x, y);
        }
    }
}

void BmpProcessor::ProcessImageSingleThread()
{
    ProcessImage(0, _height);
    
}

void BmpProcessor::PerformIntencityStep(int x, int y)
{
    int rSum = 0, gSum = 0, bSum = 0, weightSum = 0;
    for (int convY = 0; convY < _kernelHeight; convY++)
    {
        for (int convX = 0; convX < _kernelWidth; convX++)
        {
            int pixelX = x + (convX - (_kernelWidth / 2));
            int pixelY = y + (convY - (_kernelHeight / 2));
            
            if (pixelX < 0 || pixelX >= _width || pixelY < 0 || pixelY >= _height) continue;

            rSum += _initialPixelArray[pixelY * _width + pixelX].r * _convCore[convY][convX];
            gSum += _initialPixelArray[pixelY * _width + pixelX].g * _convCore[convY][convX];
            bSum += _initialPixelArray[pixelY * _width + pixelX].b * _convCore[convY][convX];
            weightSum += _convCore[convY][convX];
        }
    }

    if (weightSum == 0) weightSum = 1;

    rSum /= weightSum;
    if (rSum < 0) rSum = 0;
    if (rSum > 255) rSum = 255;

    gSum /= weightSum;
    if (gSum < 0) gSum = 0;
    if (gSum > 255) gSum = 255;

    bSum /= weightSum;
    if (bSum < 0) bSum = 0;
    if (bSum > 255) bSum = 255;
    
    _convPixelArray[y * _width + x].r = rSum;
    _convPixelArray[y * _width + x].g = gSum;
    _convPixelArray[y * _width + x].b = bSum;
}


void BmpProcessor::PerformMinimizationStep(int x, int y)
{
    int rSum = 0, gSum = 0, bSum = 0, weightSum = 0;
    for (int minimY = 0; minimY < _minimizationScale; minimY++)
    {
        for (int minimX = 0; minimX < _minimizationScale; minimX++)
        {
            int pixelX = x + (minimX - _minimizationScale / 2);
            int pixelY = y + (minimY - _minimizationScale / 2);
            
            if (pixelX < 0 || pixelX >= _width || pixelY < 0 || pixelY >= _height) continue;

            rSum += _convPixelArray[pixelY * _width + pixelX].r;
            gSum += _convPixelArray[pixelY * _width + pixelX].g;
            bSum += _convPixelArray[pixelY * _width + pixelX].b;

            weightSum += 1;
        }
    }

    rSum /= weightSum;
    if (rSum < 0) rSum = 0;
    if (rSum > 255) rSum = 255;

    gSum /= weightSum;
    if (gSum < 0) gSum = 0;
    if (gSum > 255) gSum = 255;

    bSum /= weightSum;
    if (bSum < 0) bSum = 0;
    if (bSum > 255) bSum = 255;
    
    int minimY = y / _minimizationScale;
    int minimX = x / _minimizationScale;
    _resultPixelArray[minimY * _minimizedWidth + minimX].r = rSum;
    _resultPixelArray[minimY * _minimizedWidth + minimX].g = gSum;
    _resultPixelArray[minimY * _minimizedWidth + minimX].b = bSum;
}

void BmpProcessor::SaveFile(const std::string& filename)
{
    unsigned char* imageData = PixelArrayToImageData(_resultPixelArray, _minimizedWidth, _minimizedHeight, _channels);

    stbi_write_bmp((filename).c_str(), _minimizedWidth, _minimizedHeight, 3, (const void*)imageData);
}