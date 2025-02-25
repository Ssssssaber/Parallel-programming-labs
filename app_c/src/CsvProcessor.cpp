#include "CsvProcessor.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "float.h"

bool is_number(const std::string& s)
{
    char* end = nullptr;
    double val = strtod(s.c_str(), &end);
    return end != s.c_str() && *end == '\0';
}

std::vector<std::string> split(std::string& s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

CsvProcessor::CsvProcessor(const std::string& filename, const std::string& columnXName, const std::string& columnYName)
{
    _graphInfo.LabelX = columnXName;
    _graphInfo.LabelY = columnYName;

    std::cout << "Reading file: " << filename << std::endl;
    ReadFileAndNormalize(filename, _points, _graphInfo);
}

void CsvProcessor::PerformClusterization(uint32_t K)
{
    PerformClusterization(K, _points, _clusters);
}

void CsvProcessor::ReadFileAndNormalize(const std::string& filename, std::vector<Point>& points, GraphInfo& info)
{
    if (info.LabelX == "None" || info.LabelY == "None") 
    {
        std::cout << "Incorrect label info" << std::endl;
        return;
    }

    static double maxX, maxY;
    static bool initialized = false;
    
    std::ifstream input{filename};

    if (!input.is_open()) 
    {
        std::cerr << "Couldn't read file: " << filename << "\n";
        return;
    }

    for (std::string line; std::getline(input, line);)
    {
        // find needed columns ids
        if (!initialized)
        {
            uint32_t columnId = 0;
            for (std::string columnName : split(line, ","))
            {
                if (columnName == info.LabelX)
                {
                    info.XId = columnId;    
                }
                else if (columnName == info.LabelY)
                {
                    info.YId = columnId;    
                }
                columnId++;
            }
        }

        if (info.XId == -1 || info.YId == -1) 
        {
            std::cout << "Labels were not found" << std::endl;
            return;
        }
        
        std::vector<std::string> tokens = split(line, ",");
        if (!is_number(tokens[info.XId]) || !is_number(tokens[info.YId])) continue;
        double x = std::stod(tokens[info.XId]);
        double y = std::stod(tokens[info.YId]);
        _points.push_back(
            Point(x, y)
        );

        if (maxX < x) maxX = x;
        if (maxY < y) maxY = y;
    }

    ClampToOne(points, maxX, maxY);

    _ready = true;
}

int CsvProcessor::GetNearestClusterId(Point& point, std::vector<Cluster>& clusters)
{
    if (clusters.empty())
    {
        std::cerr << "Clusters empty" << std::endl;
    }

    double minDist = DBL_MAX;
    int nearestClusterId;

    for (auto cluster = clusters.begin(); cluster != clusters.end(); cluster++)
    {
        double dist = point.Distance(cluster->Centroid);
        if (dist < minDist) 
        {
            minDist = dist;
            nearestClusterId = cluster->Id;
        }
    }

    return nearestClusterId;
}

void CsvProcessor::ClearClusterPoints(std::vector<Cluster>& clusters)
{
    for (Cluster& cluster : clusters)
    {
        cluster.Points.clear();
    }
}

void CsvProcessor::PerformClusterization(uint32_t K, std::vector<Point>& points, std::vector<Cluster>& clusters)
{
    std::cout << "Started processing " << std::endl;
    _tsBegin = std::chrono::steady_clock::now();

    uint32_t pointsCount = points.size();

    // // Initializing Clusters
    std::vector<int> usedPointIds;
    static int iters = 10;
    for (int i = 1; i <= K; i++)
    {
        while (true)
        {
            int index = rand() % pointsCount;

            if (find(usedPointIds.begin(), usedPointIds.end(), index) ==
                usedPointIds.end())
            {
                usedPointIds.push_back(index);
                points[index].ClusterId = i;
                Cluster cluster(i, points[index]);
                clusters.push_back(cluster);
                break;
            }
        }
    }
    std::cout << "Clusters initialized = " << clusters.size() << std::endl;

    std::cout << "Running K-Means Clustering.." << std::endl;

    int iter = 1;
    while (true)
    {
        std::cout << "Iter - " << iter << "/" << iters << std::endl;
        bool done = true;

        // Add all points to their nearest cluster
        // #pragma omp parallel for reduction(&&: done) num_threads(16)
        for (int i = 0; i < pointsCount; i++)
        {
            int currentClusterId = _points[i].ClusterId;
            int nearestClusterId = GetNearestClusterId(points[i], clusters);

            if (currentClusterId != nearestClusterId)
            {
                points[i].ClusterId = nearestClusterId;
                done = false;
            }
        }

        // clear all existing clusters
        ClearClusterPoints(clusters);

        // reassign points to their new clusters
        for (int i = 0; i < pointsCount; i++)
        {
            // cluster index is ID-1
            clusters[points[i].ClusterId - 1].Points.push_back(points[i]);
        }

        // Recalculating the center of each cluster
        for (int i = 0; i < K; i++)
        {
            int clusterSize = clusters[i].Points.size();

            if (clusterSize < 0) 
            {
                continue;
            }
            
            Point newCentroid;
            for (int p = 0; p < clusterSize; p++)
            {
                Point& point = clusters[i].Points[p];
                newCentroid.X += point.X;
                newCentroid.Y += point.Y;
            }
            newCentroid.X /= clusterSize;
            newCentroid.Y /= clusterSize;
            
            clusters[i].Centroid = newCentroid;
        }

        if (done || iter >= iters)
        {
            std::cout << "Clustering completed in iteration : " << iter << std::endl;
            break;
        }
        iter++;
    }
    _tsEnd = std::chrono::steady_clock::now();
    std::cout << "Ended processing. Time elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(_tsEnd - _tsBegin).count() << " ms" << std::endl;
    
}

void CsvProcessor::ClampToOne(std::vector<Point>& points, double maxX, double maxY)
{
    for (auto it = points.begin(); it != points.end(); it++)
    {
        it->X /= maxX;
        it->Y /= maxX;
    }
}