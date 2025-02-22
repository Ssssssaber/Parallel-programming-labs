#include "CsvProcessor.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

bool is_number(const std::string& s)
{
    char* end = nullptr;
    double val = strtod(s.c_str(), &end);
    return end != s.c_str() && *end == '\0';
}

CsvProcessor::CsvProcessor(const std::string& filename, const std::vector<std::string>& neededColumns)
{
    ReadFile(filename, neededColumns);

    for (auto it = _titleMap.begin(); it!= _titleMap.end(); it++)
    {
        ClampToOne(it->first);
    }
}

void CsvProcessor::ReadFile(const std::string& filename, const std::vector<std::string>& neededColumns)
{
    std::ifstream input{filename};

    if (!input.is_open()) 
    {
        std::cerr << "Couldn't read file: " << filename << "\n";
        return;
    }

    for (std::string line; std::getline(input, line);)
    {
        std::istringstream ss(std::move(line));

        // find needed columns ids
        if (_columnMap.empty())
        {
            uint32_t columnId = 0;
            for (std::string columnName; std::getline(ss, columnName, ',');)
            {
                if (std::find(neededColumns.begin(), neededColumns.end(), columnName) != neededColumns.end())
                {
                    _titleMap[columnName] = columnId;
                    _columnMap[columnId] = CsvColumn();
                    std::cout << columnId << ": " << columnName << std::endl;
                }
                columnId++;
            }
        }
        
        // parsing needed columns
        uint32_t columnId = 0;
        for (std::string value; std::getline(ss, value, ',');)
        {
            if ((_columnMap.find(columnId) != _columnMap.end()) && (is_number(value))) 
            {
                _columnMap[columnId].push_back(std::stod(value));
            }
            columnId++;
        }
    }
}

void PerformClusterization(uint32_t count)
{

}

void CsvProcessor::ClampToOne(const std::string& columnName)
{
    CsvColumn& column = GetColumn(columnName);
    double maximum = *std::max_element(column.begin(), column.end());
    for (auto it = column.begin(); it != column.end(); it++)
    {
        *it /= maximum; 
    }
}