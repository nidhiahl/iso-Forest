#include "treenode.h"
#include <math.h>
treenode::treenode()
{
    splitAttribute = -1;
    isLeaf = bool(0);
    isNew = bool(0);
    nodeId = -1;
    parentId = -1;
    lChildId = -1;
    rChildId = -1;
    nodeMass = 0;
    nodeSize = 0;
    pathLengthEst = 0;
    dataPointIndices.resize(0);
    parentAdd = nullptr;
    lChildAdd = nullptr;
    rChildAdd = nullptr;

	updateIndices.resize(0);
}

treenode::treenode(int nId): nodeId(nId)
{
    splitAttribute = -1;
    isLeaf = bool(0);
    isNew = bool(0);
    parentId = nodeId == 0 ? 0 : (nodeId-1)/2;
    lChildId = -1;
    rChildId = -1;
	nodeHeight = (int)log2(nodeId+1)+1;
    nodeMass = 0;
    nodeSize = 0;
    pathLengthEst = 0;
    dataPointIndices.resize(0);
    parentAdd = nullptr;
    lChildAdd = nullptr;
    rChildAdd = nullptr;

	updateIndices.resize(0);

	
}

treenode::~treenode()
{
    //dtor
}


double treenode::splitInfoSelection(const data &dataObject){
	int maxVal = -999999;
	int minVal = 999999;
	int attempts = 0;
	while(attempts < 10){
		splitAttribute = rand()%dataObject.getnumAttributes();
		for( int i = 0; i < dataPointIndices.size(); i++){
        		if(maxVal < dataObject.dataVector[(dataPointIndices[i])]->attributes[splitAttribute]){
				maxVal = dataObject.dataVector[(dataPointIndices[i])]->attributes[splitAttribute];
			}
        		if(minVal > dataObject.dataVector[(dataPointIndices[i])]->attributes[splitAttribute]){
        			minVal = dataObject.dataVector[(dataPointIndices[i])]->attributes[splitAttribute];
        		}
        }
        attempts = attempts + 1;
        double dataDiff = maxVal - minVal;
		if(dataDiff >= 0.0000000000000001){
			attempts = 10;
		}
	}
	maximumVal = maxVal;
	minimumVal = minVal;
	return (minVal + ((double)rand()/RAND_MAX)*(maxVal-minVal));
}

double treenode::splitInfoSelectionForUpdates(const data &dataObject){
	int maxVal = -999999;
	int minVal = 999999;
	int attempts = 0;
	while(attempts < 10){
		splitAttribute = rand()%dataObject.getnumAttributes();
		//cout<<"after attribute selection"<<endl;
		for( int i = 0; i < updateIndices.size(); i++){
        		if(maxVal < dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute]){
				maxVal = dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute];
			}
        		if(minVal > dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute]){
        			minVal = dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute];
        		}
        }
        attempts = attempts + 1;
        double dataDiff = maxVal - minVal;
		if(dataDiff >= 0.0000000000000001){
			attempts = 10;
		}
	}
	maximumVal = maxVal;
	minimumVal = minVal;
	return (minVal + ((double)rand()/RAND_MAX)*(maxVal-minVal));
}


void treenode::newMinMaxSelection(const data &dataObject, double *min, double *max){
	int maxVal = maximumVal;
	int minVal = minimumVal;
	for( int i = 0; i < updateIndices.size(); i++){
				if(maxVal < dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute]){
				maxVal = dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute];
			}
        		if(minVal > dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute]){
        			minVal = dataObject.dataVector[(updateIndices[i])]->attributes[splitAttribute];
        		}
        }
        *min = minVal;
        *max = maxVal;
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

	
