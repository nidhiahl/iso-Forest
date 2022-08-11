#include "itree.h"
#include <math.h> 
#include <queue>


//inner product of two vectors
inline double inner_product(double* curr, std::vector<double>& normal_vector)
{
    double result=0.0;
    for(int i=0;i<normal_vector.size();i++) result+=((double) curr[i])*normal_vector[i];
    return result;
}

//*****************************************************************************************************************************************//

itree::itree(const data & dataObject): _dataObject(dataObject){}

itree::itree(const data & dataObject, int sampleSize, int maxTreeHeight, int maxNumOfNodes, int avgPLEstimation, int numOfAttributes, int numOfTrialsForHyperplane): _dataObject(dataObject), _sampleSize(sampleSize), _maxTreeHeight(maxTreeHeight), _maxNumOfNodes(maxNumOfNodes), _numOfAttributes(numOfAttributes), _numOfTrialsForHyperplane(numOfTrialsForHyperplane){}

itree::~itree(){}

//*************************************************STATIC iTree creation*******************************************************************//
void itree::constructiTree(int random_seed){
    rootNode = new treenode(0);
    rootNode->dataPointIndices = _dataObject.getSample(_sampleSize);
	std::random_device random_seed_generator;
  
    queue<treenode*> BFTforNodes;
    BFTforNodes.push(rootNode);
	
    while(!BFTforNodes.empty()){
    	treenode *currNode = BFTforNodes.front();
		BFTforNodes.pop();
		if(currNode){
			currNode->nodeSize = currNode->dataPointIndices.size();
			currNode->pathLengthEst = pathLengthEstimationForUnbuiltTree(currNode->nodeSize);
			if(currNode->nodeSize <=2 || currNode->nodeHeight ==_maxTreeHeight){
    			currNode->pathLengthEst = pathLengthEstimationForUnbuiltTree(currNode->nodeSize);
        		currNode->isLeaf = bool(1);
        		currNode->dataPointIndices.clear();
        		currNode->dataPointIndices.resize(0);
    		}
    		else{

    			currNode->splitInfoSelection(_dataObject, _numOfAttributes, _numOfTrialsForHyperplane, random_seed_generator()+random_seed);
				currNode->createLeftChild();
				currNode->createRightChild();
			
				for(int i=0; i<currNode->nodeSize; i++)
				{
					double currdotn = inner_product(_dataObject.dataVector[currNode->dataPointIndices[i]]->attributes, currNode->normal_vector);
					if(currdotn < currNode->splitValue){
						currNode->lChildAdd->dataPointIndices.push_back(currNode->dataPointIndices[i]);
					}
					else{
						currNode->rChildAdd->dataPointIndices.push_back(currNode->dataPointIndices[i]);
					}
				}
        		
        		currNode->dataPointIndices.clear();
        		currNode->dataPointIndices.resize(0);
        		
        		BFTforNodes.push(currNode->lChildAdd);
        		BFTforNodes.push(currNode->rChildAdd);
    		}
    	}
    }
    _avgPLCompOfBST = 	this->avgPathLengthComputationOfBST();
}


//*******************************************************Compute path length**************************************************************//
long double itree::computePathLength(int pointX, const data & testDataObject){
	long double pathLength = 0;
	treenode * node = rootNode;
	while(!node->isLeaf){
		double pointxdotn = inner_product(testDataObject.dataVector[pointX]->attributes, node->normal_vector);

		if(pointxdotn-node->splitValue < node->upperLimit && pointxdotn-node->splitValue > node->lowerLimit) 
		{
			pathLength+=1;
		}

		if(pointxdotn < node->splitValue){
			node = node->lChildAdd;
		}else{
			node = node->rChildAdd;
		}
	}
	return pathLength;
}

long double itree::avgPathLengthComputationOfBST(){
	long double avgActualPathLengthOfBST = 0;
	queue<treenode *> BFTqueue;
	BFTqueue.push(rootNode);
	while(!BFTqueue.empty()){
		treenode *node = BFTqueue.front();
    	BFTqueue.pop();
		
		if(node->isLeaf){
			avgActualPathLengthOfBST += node->nodeSize*(node->nodeHeight-1+ node->pathLengthEst);
		}else{
			BFTqueue.push(node->lChildAdd);
			BFTqueue.push(node->rChildAdd);
		}
	}
	avgActualPathLengthOfBST /= _sampleSize;
	return avgActualPathLengthOfBST;
}


long double itree::pathLengthEstimationForUnbuiltTree(int nodeSize){
	long double es = 0;
	if(nodeSize == 3 || nodeSize == 4){es=1;}
	else if(nodeSize > 4){es = (2 * (log(nodeSize-4) + 0.5772156649)) - (2 * (nodeSize - 4) / nodeSize);}	
	return (es);
}


























