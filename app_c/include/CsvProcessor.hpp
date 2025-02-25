#pragma once

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <cmath>
#include <atomic>

struct GraphInfo {
    std::string LabelX = "None";
    std::string LabelY = "None";
    int XId = -1;
    int YId = -1;
};

struct Point {
    double X, Y;     // coordinates
    int ClusterId;     // no default cluster
    
    Point() : 
        X(0.0), 
        Y(0.0),
        ClusterId(-1) {}
        
    Point(double x, double y) : 
        X(x), 
        Y(y),
        ClusterId(-1) {}

    double Distance(Point p) {
        return sqrt((p.X - X) * (p.X - X) + (p.Y - Y) * (p.Y - Y));
    }
};

struct Cluster {
    int Id;
    Point Centroid;
    std::vector<Point> Points;

    Cluster(int clusterId, Point centroid) :
        Id(clusterId),
        Centroid(centroid) {}
};

class CsvProcessor
{
    public:
        CsvProcessor(const std::string& filename, const std::string& columnXName, const std::string& columnYName, uint64_t maxVectorCount = 5000);
        ~CsvProcessor() = default;
        bool GetIsReady() { return _ready; }
        void PerformClusterization(uint32_t K, uint8_t threadCount = 1);
        std::vector<Cluster> GetCluseters() { return _clusters; }

    private:
        void ReadFileAndNormalize(const std::string& filename, std::vector<Point>& points, GraphInfo& info);
        void CutToVectorCount(uint64_t vectorCount);
        void ClampToOne(std::vector<Point>& points, double maxX, double maxY);

        void CalculateDissimalarityAndSimilarity(uint32_t start, uint32_t end, uint32_t K, int pointsCount);
        double CalculateSilhouette(uint32_t K, int pointsCount, uint8_t threadCount);
        
        void CalculateNearestClusterForDots(uint32_t start, uint32_t end);
        int GetNearestClusterId(Point& point);

        void ClearClusterPoints();
        void RecalculateClusterCentroids(uint32_t clusterId, uint32_t K, uint32_t pointsCount);
        
    private:
        // std::atomic_bool _clusterizationDone = true;
        bool _ready = false;
        // threading and time
        std::vector<std::thread> _threads;
        std::chrono::steady_clock::time_point _tsBegin;
        std::chrono::steady_clock::time_point _tsEnd;

        // csv data
        // std::vector
        std::vector<Point> _points;
        std::vector<Cluster> _clusters;
        GraphInfo _graphInfo;

        std::vector<double> a;
        std::vector<double> b;

};