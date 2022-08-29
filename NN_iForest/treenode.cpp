#include "treenode.h"
#include <math.h>
#include <random>


treenode::treenode()
{
    nodeId=-1;
    nextId=-1;
    nearestNeighbour=NULL;
    center.resize(0);
    radius=999999.0;
    isLeaf=false;
}

treenode::treenode(int nId): nodeId(nId)
{
    nextId=-1;
    nearestNeighbour=NULL;
    center.resize(0);
    radius=999999.0;
    isLeaf=false;
}

treenode::~treenode()
{
    //dtor
}



	
