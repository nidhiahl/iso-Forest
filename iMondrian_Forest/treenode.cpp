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
    timeStamp = 0.0;
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
    timeStamp = 0.0;
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


double treenode::splitInfoSelection(const data &dataObject){

    std::random_device random_seed_generator;
    std::mt19937_64 RandomEngine(random_seed_generator());

    l.resize(dataObject.getnumAttributes(), 999999.0);
    u.resize(dataObject.getnumAttributes(), -999999.0);

    for(int i=0; i<dataPointIndices.size(); i++)
    {
        for(int j=0; j<dataObject.getnumAttributes(); j++)
        {
            l[j]=min(l[j], dataObject.dataVector[(dataPointIndices[i])]->attributes[j]);
            u[j]=max(u[j], dataObject.dataVector[(dataPointIndices[i])]->attributes[j]);
        }
    }

    double lambda = 0.0;
    for(int j=0; j<dataObject.getnumAttributes(); j++)
    {
        lambda += u[j]-l[j];
    }

    double e = std::exponential_distribution<double> (lambda)(RandomEngine);
    timeStamp += e;

    vector<double> prob_dis(dataObject.getnumAttributes());
    for(int j=0; j<dataObject.getnumAttributes(); j++)
    {
        prob_dis[j] = (u[j]-l[j])/lambda;
    }

    double cdf = 0.0;
    double aux = std::uniform_real_distribution<double> (0.0, 1.0)(RandomEngine);
    for(int j=0; j<dataObject.getnumAttributes(); j++)
    {
        cdf += prob_dis[j];
        if(aux <= cdf)
        {
            splitAttribute = j;
            break;
        }
    }

    return std::uniform_real_distribution<double> (l[splitAttribute], u[splitAttribute])(RandomEngine);
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

	
