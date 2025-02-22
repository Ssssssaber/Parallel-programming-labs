#include <iostream>

#include <svg-cpp-plot.h>
#include "CsvProcessor.hpp"

int main(int argc, char* argv[]){

    std::string filename;
    // if (argc != 2)
    // {
    //     std::cout << "Usage: app_c.exe {input.csv}";
    //     return EXIT_FAILURE;
    // }

    // filename = argv[1];
    
    filename = "csv/BD-Patients.csv";
    CsvProcessor* processor = new CsvProcessor(filename, {"Creatinine_pvariance", "HCO3_mean"});

    auto first = processor->GetColumn("Creatinine_pvariance");
    auto second = processor->GetColumn("HCO3_mean");

    std::cout << *std::max_element(first.begin(), first.end()) << "; " << *std::max_element(second.begin(), second.end());
    svg_cpp_plot::SVGPlot plt;
    plt.subplot(1, 1, 0).xlabel("keke").ylabel("keke").scatter(first, second);
    plt.savefig("teaser.svg");


    // if (processor->getIs
}
