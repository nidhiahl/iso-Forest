#include "itree.h"
#include <math.h>
#include <queue>


itree::itree(const data & dataObject): _dataObject(dataObject){}

itree::itree(const data & dataObject, int sampleSize, int maxTreeHeight, int maxNumOfNodes, int avgPLEstimation): _dataObject(dataObject), _sampleSize(sampleSize), _maxTreeHeight(maxTreeHeight), _maxNumOfNodes(maxNumOfNodes){}

itree::~itree(){}
//*************************************************STATIC iTree creation*******************************************************************//
void itree::constructiTree(){
    rootNode = new treenode(1);
	rootNode->dataPointIndices = _dataObject.getSample(_sampleSize);
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
        		currNode->dataPointIndices.clear();
        		currNode->dataPointIndices.resize(0);
    		}
    		else{
    			currNode->splitInfoSelection(_dataObject);
   
				for(int i=0; i<currNode->nodeSize; i++){     

					for(int j=0;j<currNode->children.size();j++)
					{
						if(currNode->children[j]->leftLimit <= _dataObject.dataVector[currNode->dataPointIndices[i]]->attributes[currNode->splitAttribute] 
						&& _dataObject.dataVector[currNode->dataPointIndices[i]]->attributes[currNode->splitAttribute] <= currNode->children[j]->rightLimit)
						{
							currNode->children[j]->dataPointIndices.push_back(currNode->dataPointIndices[i]);
						}
					}					
        		}
        		
        		currNode->dataPointIndices.clear();
        		currNode->dataPointIndices.resize(0);
        		
				for(int i=0;i<currNode->children.size();i++)
				{
					BFTforNodes.push(currNode->children[i]);
				}
    		}
    	}
	}
    _avgPLCompOfBST = 	this->avgPathLengthComputationOfBST();
}


//*******************************************************Compute path length**************************************************************//
long double itree::computeAnomalyScore(int pointX, const data & testDataObject){
	long double anomalyScore = 0.0;
	long double numOfNodes = 0.0;
	treenode * node = rootNode;

	while(node!=NULL){
		// cout<<anomalyScore<<endl;
		// cout<<node->splitAttribute<<endl;

		numOfNodes+=1.0;
		if(testDataObject.dataVector[pointX]->attributes[node->splitAttribute]<node->leftLimit || testDataObject.dataVector[pointX]->attributes[node->splitAttribute]>node->rightLimit || node->rightLimit==node->leftLimit)
		{
			anomalyScore+=1.0;
			break;
		}

		anomalyScore += (abs(testDataObject.dataVector[pointX]->attributes[node->splitAttribute]-node->centroid)) / (node->rightLimit - node->leftLimit);

		int index=-1;
		double dis=999999.0;
		for(int i=0;i<node->children.size();i++)
		{
			if(dis>abs(testDataObject.dataVector[pointX]->attributes[node->splitAttribute]-node->centroid))
			{
				dis=abs(testDataObject.dataVector[pointX]->attributes[node->splitAttribute]-node->centroid);
				index=i;
			}
		}

		node=node->children[index];
	
	}
	
	return anomalyScore/numOfNodes;
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
			for(int i=0;i<node->children.size();i++)
			{
				BFTqueue.push(node->children[i]);
			}
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


























