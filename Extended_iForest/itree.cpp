#include "itree.h"
#include <math.h> 
#include <queue>


inline double inner_product(double* curr, std::vector<double>& normal_vector)
{
    double result=0.0;
    for(int i=0;i<normal_vector.size();i++) result+=((double) curr[i])*normal_vector[i];
    return result;
}

//*****************************************************************************************************************************************//

itree::itree(const data & dataObject): _dataObject(dataObject){}

itree::itree(const data & dataObject, int sampleSize, int maxTreeHeight, int maxNumOfNodes, int avgPLEstimation, int exLevel): _dataObject(dataObject), _sampleSize(sampleSize), _maxTreeHeight(maxTreeHeight), _maxNumOfNodes(maxNumOfNodes), _exLevel(exLevel){}

itree::~itree(){}

//*************************************************STATIC iTree creation*******************************************************************//
void itree::constructiTree(int random_seed){
    rootNode = new treenode(0);
    rootNode->dataPointIndices = _dataObject.getSample(_sampleSize);
	std::random_device random_seed_generator;
    //cout<<"smapleSize"<<rootNode->dataPointIndices.size()<<endl;
    queue<treenode*> BFTforNodes;
    BFTforNodes.push(rootNode);
	
    while(!BFTforNodes.empty()){
    	treenode *currNode = BFTforNodes.front();
		BFTforNodes.pop();
		if(currNode){
			currNode->nodeSize = currNode->dataPointIndices.size();
			currNode->pathLengthEst = pathLengthEstimationForUnbuiltTree(currNode->nodeSize);
			if(currNode->nodeSize <=1 || currNode->nodeHeight ==_maxTreeHeight){
    			currNode->pathLengthEst = pathLengthEstimationForUnbuiltTree(currNode->nodeSize);
        		currNode->isLeaf = bool(1);
        		//treeNode[nodeId]->isActive = bool(1);
        		currNode->dataPointIndices.clear();
        		currNode->dataPointIndices.resize(0);
    		}
    		else{

    			currNode->splitValue = currNode->splitInfoSelection(_dataObject, _exLevel, random_seed_generator()+random_seed);
				currNode->createLeftChild();
				currNode->createRightChild();
				//treenode *left = new treenode(2*nodeId+1);
        		//treenode *right = new treenode(2*nodeId+2);
				// for(int i=0; i<currNode->nodeSize; i++){     
            	// 	if(_dataObject.dataVector[currNode->dataPointIndices[i]]->attributes[currNode->splitAttribute]<currNode->splitValue){
                // 		currNode->lChildAdd->dataPointIndices.push_back(currNode->dataPointIndices[i]);
            	// 	}
            	// 	else{
                // 		currNode->rChildAdd->dataPointIndices.push_back(currNode->dataPointIndices[i]);
            	// 	}

        		// }

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
        		//left->parentAdd = currNode;
        		//currNode->lChildAdd = left;
        		//currNode->lChildId = left->nodeId;
        		
        		//right->parentAdd = currNode;
        		//currNode->rChildAdd = right;
        		//currNode->rChildId = right->nodeId;
        		
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
long double itree::computePathLength(int pointX){
	long double pathLength = 0;
	treenode * node = rootNode;
	while(!node->isLeaf){
		//pathLength++;
		double pointxdotn = inner_product(_dataObject.dataVector[pointX]->attributes, node->normal_vector);
		if(pointxdotn < node->splitValue){
			node = node->lChildAdd;
		}else{
			node = node->rChildAdd;
		}
	}
	//cout<<"PathLength="<<pathLength;
	//pathLength +=  node->pathLengthEst;
	//cout<<" final PL="<<pathLength;
	
	//cout<<"----------nodeHeight="<<node->nodeHeight;
	pathLength = node->nodeHeight - 1 + node->pathLengthEst;
	//cout<<" final PL="<<pathLength<<endl;
	return pathLength;
}

long double itree::avgPathLengthComputationOfBST(){
	long double avgActualPathLengthOfBST = 0;
	queue<treenode *> BFTqueue;
	BFTqueue.push(rootNode);
	while(!BFTqueue.empty()){
		//pathLength++;
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
	if(nodeSize == 2){es=1;}
	else if(nodeSize > 2){es = (2 * (log(nodeSize-1) + 0.5772156649)) - (2 * (nodeSize - 1) / nodeSize);}	
	return (es);
}


























