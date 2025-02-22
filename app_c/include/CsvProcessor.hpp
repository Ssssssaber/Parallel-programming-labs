#pragma once

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
class CsvProcessor
{
    using CsvColumn = std::vector<double>; 
    public:
        CsvProcessor(const std::string& filename, const std::vector<std::string>& neededCollumns = {"Creatinine_pvariance", "HCO3_mean"});
        ~CsvProcessor() = default;

        bool GetIsReady() { return _ready; }

        void PerformClusterization(uint32_t count);

        std::map<uint32_t, CsvColumn>& GetData() { return _columnMap; }        
        CsvColumn& GetColumn(const std::string& columnName) { return _columnMap[_titleMap[columnName]]; }

    private:
        void ReadFile(const std::string& filename, const std::vector<std::string>& neededCollumns);
        void ClampToOne(const std::string& columnName);
        // void 

    private:
        bool _ready = false;
        // threading and time
        std::vector<std::thread> _threads;
        std::chrono::steady_clock::time_point _tsBegin;
        std::chrono::steady_clock::time_point _tsEnd;

        // csv data
        // std::vector
        std::map<uint32_t, CsvColumn> _columnMap;
        std::map<std::string, uint32_t> _titleMap;
};