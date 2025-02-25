#include <iostream>

#include <svg-cpp-plot.h>
#include "CsvProcessor.hpp"

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

    std::string filename;

    filename = "csv/BD-Patients.csv";
    CsvProcessor* processor = new CsvProcessor(filename, "HCO3_mean", "BUN_percentil_75");

    if (!processor->GetIsReady()) return EXIT_FAILURE;

    processor->PerformClusterization(3);

    std::vector clusters = processor->GetCluseters();
    svg_cpp_plot::SVGPlot plt;
    std::cout << "Creating svg plot...";
    for (auto cluster = clusters.begin(); cluster != clusters.end(); cluster++)
    {
        std::vector<double> x, y;

        SeperateXandY(cluster->Points, x, y);
        plt.scatter(x, y);
    }

    std::cout << "Saving svg plot..." << std::endl;
    plt.savefig("teaser.svg");
    
    std::cout << "Plot saved!" << std::endl;
    return 0;

}
