#include <iostream>

#include "BmpProcessor.hpp"

int main(int argc, char* argv[]){

    std::string inputFilename;
    std::string outputFilename;
    int numThreads;

    if (argc != 4) 
    {
        std::cout << "Usage: app_b.exe {input.bmp} {output.bmp} {numThreads}";
        return EXIT_FAILURE;
	}
    else 
    {
        inputFilename = argv[1];
        outputFilename = argv[2];
        numThreads = std::stoi(argv[3]);
    }

    BmpProcessor* processor = new BmpProcessor(inputFilename);

    if (!processor->GetIsReady()) return EXIT_FAILURE;

    processor->ProcessImageMultithread(numThreads);
    processor->SaveFile(outputFilename);
    
    std::cout << "File (" << outputFilename << ") saved \n";
}
