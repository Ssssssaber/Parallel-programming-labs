#pragma once

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <cmath>

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
        CsvProcessor(const std::string& filename, const std::string& columnX, const std::string& columnY);
        ~CsvProcessor() = default;
        bool GetIsReady() { return _ready; }
        void PerformClusterization(uint32_t K);
        std::vector<Cluster> GetCluseters() { return _clusters; }

    private:
        void ReadFileAndNormalize(const std::string& filename, std::vector<Point>& points, GraphInfo& info);
        void ClampToOne(std::vector<Point>& points, double maxX, double maxY);
        
        void PerformClusterization(uint32_t count, std::vector<Point>& points, std::vector<Cluster>& clusters);
        int GetNearestClusterId(Point& point, std::vector<Cluster>& clusters);
        void ClearClusterPoints(std::vector<Cluster>& clusters);
        
    private:
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

};