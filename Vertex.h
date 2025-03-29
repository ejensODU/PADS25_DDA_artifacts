#pragma once

#include <vector>
#include <list>
#include <memory>
#include <string>
#include <fstream>

class OoO_Event;
class Edge;
class Entity;
class ExpoDist;

class Vertex {
public:
    Vertex(std::string vertexName, int extraWork, int distSeed, std::string traceFolderName, std::string traceFileHeading);
    virtual void IO_SVs(std::vector<std::vector<size_t>>& Is, std::vector<std::vector<size_t>>& Os) = 0;
    virtual void Run(std::list<OoO_Event*>& newEvents, double simTime, std::shared_ptr<Entity> entity) = 0;
    int ExtraWork();
    static int getNumVertices()  { return _numVertices; }
    int getVertexIndex() const  { return _vertexIndex; }
    //std::vector<Edge> const getEdges();
    std::string getVertexName() const  { return _vertexName; }
    int getNumExecs() const  { return _numExecutions; }
    void WriteToTrace(std::string traceSnapshot);
protected:
    static int _numVertices;
    const int _vertexIndex;
    const std::string _vertexName;
    const int _extraWork;
    const int _distSeed;
    const std::string _traceFolderName;
    std::string _traceFilePath;
    int _numExecutions;
private:
    std::unique_ptr<ExpoDist> _workDist;
    static int _runID;
    double _expoRV;
    std::ofstream _traceFile;
};


class Edge {
public:
    Edge(size_t origVertexIndex, size_t termVertexIndex, const float& minDist);
    size_t getOrigVertexIndex() const  { return _origVertexIndex; }
    size_t getTermVertexIndex() const  { return _termVertexIndex; }
    float getMinDist() const  { return _minDist; }
private:
    const size_t _origVertexIndex;
    const size_t _termVertexIndex;
    const float _minDist;
};
