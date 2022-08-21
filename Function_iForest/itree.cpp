#include "itree.h"
#include <math.h> 
#include <queue>


inline std::vector<double> derivate (double* X1, double* time, int dim)
{
    /* return the derivative of the function X1 whose have been measured at times time.*/

    std::vector<double> derivate(dim-1, 0.0);
    for(int i=1; i<dim; i++)
    {
        derivate[i-1] = (X1[i] - X1[i-1]) / (time[i] - time[i-1]);
    }
    return derivate;
}

inline double inner_product(double* X1, double* X2, double* time, double alpha, int dim)
{
    /* Return the innerproduct between X1 and X2 as a convex combination
	 *between L2 innerproduct and the L2 innerproduct of derivatives. 
	 *
	 * 'alpha=1' corresponds to L2 innerproduct
	 * 'alpha=0.5' corresponds to the Sobolev innerproduct
	 * 'alpha=0' corresponds to the derivative innerproduct. 
	*/

    double result = 0.0;

    if(alpha == 1)
    {
        std::vector<double> prod(dim, 0.0);
        
        prod[0] = X1[0] * X2[0];
        for(int i=1; i<dim; i++)
        {
            prod[i] = X1[i] * X2[i];
            result += (time[i] - time[i-1]) * (prod[i] + prod[i-1]) / 2.0; 
        }
    }
    else if(alpha == 0)
    {
        std::vector<double> prod_derivate(dim-1, 0.0);
        std::vector<double> X1_derivate(dim-1, 0.0);
        std::vector<double> X2_derivate(dim-1, 0.0);

        X1_derivate = derivate(X1, time, dim);
		X2_derivate = derivate(X2, time, dim);		
		
        prod_derivate[0] = X1_derivate[0] * X2_derivate[0];
        for(int i=1; i<dim-1; i++)
        {
            prod_derivate[i] = X1_derivate[i] * X2_derivate[i];
            result += (time[i] - time[i-1]) * (prod_derivate[i] + prod_derivate[i-1]) / 2.0;
        }
    }
    else
    {
        std::vector<double> prod_derivate(dim-1, 0.0);
        std::vector<double> X1_derivate(dim-1, 0.0);
        std::vector<double> X2_derivate(dim-1, 0.0);
        std::vector<double> prod(dim, 0.0);
        std::vector<double> step_time (dim-1, 0.0);
        double inner = 0.0;
        double inner_derivate = 0.0;
        double norm_X1 = 0.0;
		double norm_X2 = 0.0;
		double norm_X1_derivate = 0.0;
		double norm_X2_derivate = 0.0;

        prod[0] = X1[0] * X2[0];
        for(int i=1; i<dim; i++)
        {
            prod[i] = X1[i] * X2[i];
            step_time[i-1] = time[i] - time[i-1];
            inner += step_time[i-1] * (prod[i] + prod[i-1]) / 2.0;
            norm_X1 += step_time[i-1] * (std::pow (X1[i], 2.0) + std::pow (X1[i-1], 2.0)) / 2.0;
			norm_X2 += step_time[i-1] * (std::pow (X2[i], 2.0) + std::pow (X2[i-1], 2.0)) / 2.0;
        }

        X1_derivate = derivate(X1, time, dim);
		X2_derivate = derivate(X2, time, dim);

		prod_derivate[0] = X1_derivate[0] * X2_derivate[0];
        for(int i=1; i<dim-1; i++)
        {
            prod_derivate[i] = X1_derivate[i] * X2_derivate[i];
            inner_derivate += step_time[i-1]  * (prod_derivate[i] + prod_derivate[i-1]) / 2.0;
			norm_X1_derivate += step_time[i-1]  * (std::pow (X1_derivate[i], 2.0) + std::pow (X1_derivate[i-1], 2.0)) / 2.0;
			norm_X2_derivate += step_time[i-1]  * (std::pow (X2_derivate[i], 2.0) + std::pow (X2_derivate[i-1], 2.0)) / 2.0;
        }

        result = alpha * inner / (std::sqrt (norm_X1) * std::sqrt (norm_X2)) + (1 - alpha) * inner_derivate / (std::sqrt (norm_X1_derivate) * std::sqrt (norm_X2_derivate));
    }

    return result;
}

//*****************************************************************************************************************************************//

itree::itree(const data & dataObject, const data & timeDataObject): _dataObject(dataObject), _timeDataObject(timeDataObject){}

itree::itree(const data & dataObject, int sampleSize, int maxTreeHeight, int maxNumOfNodes, int avgPLEstimation, double alpha, int dic_number, const data & timeDataObject): _dataObject(dataObject), _sampleSize(sampleSize), _maxTreeHeight(maxTreeHeight), _maxNumOfNodes(maxNumOfNodes), _alpha(alpha), _dic_number(dic_number), _timeDataObject(timeDataObject){}

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
			if(currNode->nodeSize <=1 || currNode->nodeHeight ==_maxTreeHeight){
    			currNode->pathLengthEst = pathLengthEstimationForUnbuiltTree(currNode->nodeSize);
        		currNode->isLeaf = bool(1);
        		currNode->dataPointIndices.clear();
        		currNode->dataPointIndices.resize(0);
    		}
    		else{

    			currNode->splitValue = currNode->splitInfoSelection(_dataObject, _alpha, _dic_number, _timeDataObject, random_seed_generator()+random_seed);
				currNode->createLeftChild();
				currNode->createRightChild();

				for(int i=0; i<currNode->nodeSize; i++)
				{
					if(currNode->innerprod[i] < currNode->splitValue){
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
		double pointxdotn = inner_product(testDataObject.dataVector[pointX]->attributes, &node->dic_vector[0], _timeDataObject.dataVector[pointX]->attributes, _alpha, testDataObject.getnumAttributes());
		if(pointxdotn < node->splitValue){
			node = node->lChildAdd;
		}else{
			node = node->rChildAdd;
		}
	}
	pathLength = node->nodeHeight - 1 + node->pathLengthEst;
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
	if(nodeSize == 2){es=1;}
	else if(nodeSize > 2){es = (2 * (log(nodeSize-1) + 0.5772156649)) - (2 * (nodeSize - 1) / nodeSize);}	
	return (es);
}


























