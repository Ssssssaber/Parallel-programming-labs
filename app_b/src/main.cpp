#include <iostream>

#include "BmpProcessor.hpp"

int main(int argc, char* argv[]){

    std::string inputFilename;
    std::string outputFilename;
    int numThreads;
    int intencityThreshold;
    int erosionStep;
    
    if (argc != 4 && argc != 6) 
    {
        std::cout << "Usage: img_process.exe {input.bmp} {output.bmp} {numThreads} [intencityThreshold] [erosionStep]";
        return EXIT_FAILURE;
	}
    else
    {
        inputFilename = argv[1];
        outputFilename = argv[2];
        numThreads = std::stoi(argv[3]);
    }

    if (argc == 6)
    {
        intencityThreshold = std::atoi(argv[4]);
        erosionStep = std::atoi(argv[5]);
    }

    // inputFilename = "images/tales.bmp";
    // outputFilename = "res.bmp";
    // numThreads = 16;
    // intencityThreshold = 100;
    // erosionStep = 10;

    BmpProcessor* processor = new BmpProcessor(inputFilename, intencityThreshold, erosionStep);

    if (!processor->GetIsReady()) return EXIT_FAILURE;

    processor->ProcessImageMultithread(numThreads);
    processor->SaveFile(outputFilename);
    
    std::cout << "File (" << outputFilename << ") saved \n";
}
