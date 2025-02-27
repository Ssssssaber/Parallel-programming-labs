#include <iostream>

#include <svg-cpp-plot.h>
#include "CsvProcessor.hpp"

#include <algorithm>

char* GetOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool OptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

void SeperateXandY(std::vector<Point>& points, std::vector<double>& x, std::vector<double>& y)
{
    uint32_t size = points.size();

    x.resize(size);
    y.resize(size);

    for (int i = 0; i < size; i++)
    {
        Point& point = points[i];
        x[i] = point.X;
        y[i] = point.Y; 
    }
}

int main(int argc, char* argv[]){

    std::string inputFilename = "csv/BD-Patients.csv";
    std::string xColumn = "Creatinine_pvariance";
    std::string yColumn = "HCO3_mean";
    uint32_t maxVectorCount = 5000;
    uint32_t K = 3;
    uint8_t numThreads = 3; 
    std::string outputFilename = "None";
    
    if (OptionExists(argv, argv+argc, "-h"))
    {
        std::cout << "Usage: app_c.exe [--csv input.csv] [--x name] [--y name] [--max uint] [--K uint] [--thrCount uint] [--outSVG out.svg]";
        return EXIT_FAILURE;
    }

    if (OptionExists(argv, argv+argc, "--csv")) inputFilename = GetOption(argv, argv + argc, "--csv");
    if (OptionExists(argv, argv+argc, "--x")) xColumn = GetOption(argv, argv + argc, "--x");
    if (OptionExists(argv, argv+argc, "--y")) yColumn = GetOption(argv, argv + argc, "--y");
    if (OptionExists(argv, argv+argc, "--max")) maxVectorCount = std::stoi(GetOption(argv, argv + argc, "--max"));
    if (OptionExists(argv, argv+argc, "--K")) K = std::stoi(GetOption(argv, argv + argc, "--K"));
    if (OptionExists(argv, argv+argc, "--thrCount")) numThreads = std::stoi(GetOption(argv, argv + argc, "--thrCount"));
    if (OptionExists(argv, argv+argc, "--outSVG")) outputFilename = GetOption(argv, argv + argc, "--outSVG");

    std::cout << "Parameters: " <<
        inputFilename << "; " <<
        xColumn << "; " <<
        yColumn << "; " <<
        maxVectorCount << "; " <<
        K << "; " <<
        numThreads << "; " <<
        outputFilename << std::endl;

    CsvProcessor* processor = new CsvProcessor(inputFilename, xColumn, yColumn, maxVectorCount);

    if (!processor->GetIsReady()) return EXIT_FAILURE;

    processor->PerformClusterization(K, numThreads);

    std::vector clusters = processor->GetCluseters();

    std::cout << "---------------------------------------------------------" << std::endl;
    std::cout << "Cluster centroids info" << std::endl;
    for (auto cluster = clusters.begin(); cluster != clusters.end(); cluster++)
    {   
        Point centroid = cluster->Centroid;
        std::cout << "Cluster id: " << cluster->Id << "; Centroid (x, y): " << centroid.X << "; " << centroid.Y << "; " << std::endl;
    }
    std::cout << "---------------------------------------------------------" << std::endl;

    if (outputFilename == "None") return 0;

    svg_cpp_plot::SVGPlot plt;
    for (auto cluster = clusters.begin(); cluster != clusters.end(); cluster++)
    {   
        std::vector<double> x, y;

        SeperateXandY(cluster->Points, x, y);
        plt.scatter(x, y);
    }

    std::cout << "Saving svg plot..." << std::endl;
    plt.savefig(outputFilename);
    
    std::cout << "Plot saved!" << std::endl;
    return 0;

}
