#include "./Bfs.h"
#include "../../src/util/ReadFile.hpp"

using namespace DFSimTest;

const uint BfsTest::initialNode = 38;
const uint BfsTest::nodeNum = 256;
const uint BfsTest::edgeNum = 4096;
vector<int> BfsTest::memData;
vector<Edge_t> BfsTest::edges;
vector<Node_t> BfsTest::nodes;
vector<int> BfsTest::level;

void BfsTest::generateData()
{
    memData.resize(nodeNum * 2 + edgeNum);  // Node: begin + end; Edge: dst;
    edges.resize(edgeNum);
    nodes.resize(nodeNum);
    level.resize(nodeNum);

    std::ifstream filePath("./resource/Bfs/input.data");
    ReadFile::readFile2UnifiedVector(memData, filePath);

    for (size_t i = 0; i < nodeNum; ++i)
    {
        nodes[i].begin = memData[i*2];
        nodes[i].end = memData[i * 2 + 1];
    }

    uint initAddr = nodeNum * 2;
    for (size_t i = 0; i < edgeNum; ++i)
    {
        edges[i].dst = memData[initAddr + i];
    }
}
