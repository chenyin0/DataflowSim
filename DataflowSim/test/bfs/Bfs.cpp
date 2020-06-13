#include "./Bfs.h"
#include "../../src/util/ReadFile.hpp"

using namespace DFSimTest;

const uint BfsTest::initialNode = 38;
const uint BfsTest::nodeNum = 256;
const uint BfsTest::edgeNum = 4096;

// Address map: 
// 0x00 -> node -> level -> edge
const uint BfsTest::nodeBaseAddr = 0;
const uint BfsTest::levelBaseAddr = nodeNum;
const uint BfsTest::edgeBaseAddr = nodeNum * 2;

vector<int> BfsTest::memData;
vector<Edge_t> BfsTest::edges;
vector<Node_t> BfsTest::nodes;
vector<int> BfsTest::level;

void BfsTest::generateData()
{
    memData.resize(nodeNum * 2 + edgeNum);  // Node: begin + end; Edge: dst;
    edges.resize(edgeNum + 1);  // Index start at 1
    nodes.resize(nodeNum + 1);  // Index start at 1
    level.resize(nodeNum + 1);  // Index start at 1

    std::ifstream filePath("./resource/Bfs/input.data");
    ReadFile::readFile2UnifiedVector(memData, filePath);

    for (size_t i = 0; i < nodeNum; ++i)
    {
        nodes[i + 1].begin = memData[i * 2];  // Index start at 1
        nodes[i + 1].end = memData[i * 2 + 1];  // Index start at 1
    }

    uint initAddr = nodeNum * 2;
    for (size_t i = 0; i < edgeNum; ++i)
    {
        edges[i + 1].dst = memData[initAddr + i];  // Index start at 1
    }

    for (auto& i : level)
    {
        i = -1;  // Initial = -1, signify has not been visited
    }

    // Start from initialNode, set this node's level to 0
    level[initialNode + 1] = 0;  // Index start at 1
}
