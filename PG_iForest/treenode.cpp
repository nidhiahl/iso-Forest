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
    parentId = nodeId == 0 ? 0 : (nodeId-1)/2;
    lChildId = -1;
    rChildId = -1;
	nodeHeight = (int)log2(nodeId+1)+1;
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

double invertedCumulativeProbabilityFunction(double target)
{
    double x = 0.0;
    double y = 0.5 - target;

    while(abs(y) > 0.000001)
    {

    }

    return x;
}

double treenode::splitInfoSelection(const data &dataObject, int powerForSegment){

    std::random_device random_seed_generator;
    std::mt19937_64 RandomEngine(random_seed_generator());

    splitAttribute = std::uniform_int_distribution<>(0, dataObject.getnumAttributes()-1)(RandomEngine);

    vector<double> values;
    for(int i=0; i<dataPointIndices.size(); i++)
    {
        values.push_back(dataObject.dataVector[(dataPointIndices[i])]->attributes[splitAttribute]);
    }

    sort(values.begin(), values.end());

    vector<double> diff;
    double sum=0.0;

    for(int i=1; i<dataPointIndices.size(); i++)
    {
        diff.push_back(values[i]-values[i-1]);
        diff[i-1]=pow(diff[i-1], powerForSegment+1);

        sum+=diff[i-1];
    }

    vector<double> cumulativeProbability;
    for(int i=0; i<diff.size(); i++)
    {
        cumulativeProbability.push_back(diff[i]/sum);
    }

    double c = std::uniform_real_distribution<double> (0.0, 1.0)(RandomEngine);

    int i=0;
    while(cumulativeProbability[i]<c)
    {
        c-=cumulativeProbability[i];
        i++;
    }

    double y=(2*c)/cumulativeProbability[i]-1;

    return y*(values[i+1]-values[i])/2+(values[i]+values[i+1])/2;
}



void treenode::createLeftChild(){
	lChildAdd = new treenode(2*nodeId+1);
	lChildAdd->parentAdd = this;
	lChildId = lChildAdd->nodeId;
}

void treenode::createRightChild(){
	rChildAdd = new treenode(2*nodeId+2);
	rChildAdd->parentAdd = this;
	rChildId = rChildAdd->nodeId;
}

	
