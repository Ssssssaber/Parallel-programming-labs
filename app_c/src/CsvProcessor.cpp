#include "CsvProcessor.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "float.h"
#include <numeric>

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

CsvProcessor::CsvProcessor(const std::string& filename, const std::string& columnXName, const std::string& columnYName, uint64_t maxVectorCount)
{
    _graphInfo.LabelX = columnXName;
    _graphInfo.LabelY = columnYName;

    std::cout << "Starting to parse file " << filename << " for columns " << columnXName << " and " << columnYName << std::endl;

    ReadFileAndNormalize(filename, _points, _graphInfo);
    CutToVectorCount(maxVectorCount);
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

void CsvProcessor::CutToVectorCount(uint64_t vectorCount)
{
    _points.resize(vectorCount);
}

double MeanDistanceToCluster(Point& point, Cluster& cluster)
{
    double mean;
    for (auto it = cluster.Points.begin(); it != cluster.Points.end(); it++)
    {
        mean += point.Distance(*it);
    }

    return mean / cluster.Points.size();
}

void CsvProcessor::CalculateDissimalarityAndSimilarity(uint32_t start, uint32_t end, uint32_t K, int pointsCount)
{
    std::vector<double> temp;
    temp.resize(K - 1);
    for (int i = start; i < end; i++)
    {
        int jindex = 0;
        for (auto cluster = _clusters.begin(); cluster != _clusters.end(); cluster++)
        {
            if (_points[i].ClusterId == cluster->Id)
            {
                a[i] = MeanDistanceToCluster(_points[i], *cluster);
            }
            else
            {
                temp[jindex++] = MeanDistanceToCluster(_points[i], *cluster);
            }
        }

        b[i] = *std::min_element(temp.begin(), temp.end());
    }
}

double CsvProcessor::CalculateSilhouette(uint32_t K, int pointsCount, uint8_t threadCount)
{
    a.resize(pointsCount);
    b.resize(pointsCount);

    if (threadCount == 1)
    {
        CalculateDissimalarityAndSimilarity(0, pointsCount, K, pointsCount);
    }
    else
    {
        int step = pointsCount / threadCount;
        for (int i = 0; i < threadCount; i++)
        {
            int start = i * step;
            int end = (i + 1) * step + 1;
            if (i == threadCount - 1) end = pointsCount;
            _threads.push_back(std::thread(&CsvProcessor::CalculateDissimalarityAndSimilarity, this, start, end, K, pointsCount));
        }
    
        for (auto& thread : _threads)
        {
            if (thread.joinable()) thread.join();
        }

        _threads.clear();
    }
   
    
    std::vector<double> s;
    s.resize(pointsCount);
    double maxim;
    for (int i = 0; i < pointsCount; i++)
    {
        if (a[i] > b[i]) maxim = a[i];
        else maxim = b[i];

        s[i] = (a[i] - b[i]) / maxim;
    }

    return std::accumulate(s.begin(), s.end(), 0.0) / pointsCount;
}

int CsvProcessor::GetNearestClusterId(Point& point)
{
    if (_clusters.empty())
    {
        std::cerr << "Clusters empty" << std::endl;
    }

    double minDist = DBL_MAX;
    int nearestClusterId;

    for (auto cluster = _clusters.begin(); cluster != _clusters.end(); cluster++)
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

void CsvProcessor::ClearClusterPoints()
{
    for (Cluster& cluster : _clusters)
    {
        cluster.Points.clear();
    }
}

void CsvProcessor::CalculateNearestClusterForDots(uint32_t start, uint32_t end)
{
    for (int i = start; i < end; i++)
    {
        int currentClusterId = _points[i].ClusterId;
        int nearestClusterId = GetNearestClusterId(_points[i]);

        if (currentClusterId != nearestClusterId)
        {
            _points[i].ClusterId = nearestClusterId;
            // _clusterizationDone = false;
        }
    }
}

void CsvProcessor::RecalculateClusterCentroids(uint32_t clusterId, uint32_t K, uint32_t pointsCount)
{
    // Recalculating the center of each cluster
    for (int i = 0; i < K; i++)
    {
        int clusterSize = _clusters[clusterId].Points.size();

        if (clusterSize < 0) 
        {
            continue;
        }
        
        Point newCentroid;
        for (int p = 0; p < clusterSize; p++)
        {
            Point& point = _clusters[clusterId].Points[p];
            newCentroid.X += point.X;
            newCentroid.Y += point.Y;
        }
        newCentroid.X /= clusterSize;
        newCentroid.Y /= clusterSize;
        
        _clusters[clusterId].Centroid = newCentroid;
    }
}

void CsvProcessor::PerformClusterization(uint32_t K, uint8_t threadCount)
{
    std::cout << "---------------------------------------------------------" << std::endl;
    std::cout << "Started processing with " << (int)threadCount << " thread(s)" << std::endl;
    _tsBegin = std::chrono::steady_clock::now();

    uint32_t pointsCount = _points.size();

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
                _points[index].ClusterId = i;
                Cluster cluster(i, _points[index]);
                _clusters.push_back(cluster);
                break;
            }
        }
    }
    std::cout << "Clusters initialized = " << _clusters.size() << std::endl;

    std::cout << "Running K-Means Clustering.." << std::endl;

    int iter = 1;
    while (true)
    {
        std::cout << "Iter - " << iter << "/" << iters << std::endl;
        // _clusterizationDone = true;

        // Add all points to their nearest cluster
        // #pragma omp parallel for reduction(&&: _clusterizationDne) num_threads(16)
       if (threadCount == 1)
       {
            CalculateNearestClusterForDots(0, pointsCount);
       }
       else
       {
            int step = pointsCount / threadCount;
            for (int i = 0; i < threadCount; i++)
            {
                int start = i * step;
                int end = (i + 1) * step + 1;
                if (i == threadCount - 1) end = pointsCount;
                _threads.push_back(std::thread(&CsvProcessor::CalculateNearestClusterForDots, this, start, end));
            }
        
            for (auto& thread : _threads)
            {
                if (thread.joinable()) thread.join();
            }

            _threads.clear();
       }

        // clear all existing clusters
        ClearClusterPoints();


        // reassign points to their new clusters
        for (int i = 0; i < pointsCount; i++)
        {
            // cluster index is ID-1
            _clusters[_points[i].ClusterId - 1].Points.push_back(_points[i]);
        }

        if (threadCount == 1)
        {
            for (int i = 0; i < K; i++)
            {
                RecalculateClusterCentroids(i, K, pointsCount);
            }
        }
        else
        {
            for (int i = 0; i < K; i++)
            {
                _threads.push_back(std::thread(&CsvProcessor::RecalculateClusterCentroids, this, i, K, pointsCount));
            }

            for (auto& thread : _threads)
            {
                if (thread.joinable()) thread.join();
            }

            _threads.clear();
        }

        if ( iter >= iters)
        {
            std::cout << "Clustering completed in iteration : " << iter << std::endl;
            break;
        }
        iter++;
    }

    std::cout << "---------------------------------------------------------" << std::endl;
    std::cout << "Silhouette: " << CalculateSilhouette(K, pointsCount, threadCount) << std::endl;
    _tsEnd= std::chrono::steady_clock::now();
    std::cout << "Ended processing. Time elapsed: " << 
        std::chrono::duration_cast<std::chrono::milliseconds>(_tsEnd - _tsBegin).count() << " ms (" <<
        std::chrono::duration_cast<std::chrono::nanoseconds>(_tsEnd - _tsBegin).count() << ")" << std::endl;
}

void CsvProcessor::ClampToOne(std::vector<Point>& points, double maxX, double maxY)
{
    for (auto it = points.begin(); it != points.end(); it++)
    {
        it->X /= maxX;
        it->Y /= maxY;
    }
}