#include "treenode.h"
#include <math.h>
#include <random>


treenode::treenode()
{
    splitAttribute = -1;
    isLeaf = bool(0);
    nodeId = -1;
    parentId = -1;
    lChildId = -1;
    rChildId = -1;
    distance = 0.0;
    nodeSize = 0;
    pathLengthEst = 0;
    dataPointIndices.resize(0);
    parentAdd = nullptr;
    lChildAdd = nullptr;
    rChildAdd = nullptr;

}

treenode::treenode(int nId): nodeId(nId)
{
    splitAttribute = -1;
    isLeaf = bool(0);
    parentId = -1;
    lChildId = -1;
    rChildId = -1;
	distance = 0.0;
    nodeSize = 0;
    pathLengthEst = 0;
    dataPointIndices.resize(0);
    parentAdd = nullptr;
    lChildAdd = nullptr;
    rChildAdd = nullptr;
}

treenode::~treenode()
{
    //dtor
}




