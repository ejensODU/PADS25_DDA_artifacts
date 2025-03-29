#pragma once

#include "Vertex.h"
#include "OoO_SimExec.h"
#include "OoO_SV.h"

class TriangularDist;
class UniformIntDist;

class OoO_SimModel {
public:
    OoO_SimModel(double maxSimTime, size_t numThreads, size_t distSeed, int numSerialOoO_Execs, std::string traceFolderName);
    
    // Set/get the number of vertices in the model
    void setNumVertices(size_t numVertices);
    size_t getNumVertices();
    
    // Run the simulation 
    void SimulateModel(std::string execOrderFilename);
    
    // Abstract methods to be implemented by derived classes
    virtual void PrintSVs() const = 0;
    virtual void PrintNumVertexExecs() const = 0;
    
    // Initialize the out-of-order simulation
    void Init_OoO(std::string tableFilename);
    
protected:
    std::vector<std::vector<size_t>> _Is;       // Input state variables indices for each vertex
    std::vector<std::vector<size_t>> _Os;       // Output state variables indices for each vertex
    std::vector<std::vector<Edge>> _edges;      // Edges connecting vertices
    std::vector<OoO_Event*> _initEvents;        // Initial events to start the simulation
    const size_t _distSeed;                     // Seed for random distributions
    const int _numSerialOoO_Execs;              // Controls OoO execution behavior
    const std::string _traceFolderName;         // Folder name for trace outputs
    
private:
    // Floyd-Warshall algorithm to compute shortest paths
    std::vector<std::vector<float>> FloydWarshall();
    
    // Generate Independence Time Limit (ITL) table
    std::vector<std::vector<float>> MakeITL(std::string tableFilename);
    
    // Read and write ITL tables to/from CSV
    std::vector<std::vector<float>> ReadITLTableFromCSV(const std::string& tableFilename) const;
    void WriteITLTableToCSV(std::vector<std::vector<float>>& ITL, std::string tableFilename) const;
    
    std::unique_ptr<OoO_SimExec> _simExec;      // Simulation executor
    size_t _numVertices;                        // Number of vertices in the model
    const double _maxSimTime;                   // Maximum simulation time
    const size_t _numThreads;                   // Number of threads for execution
};