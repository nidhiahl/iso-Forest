#include "itree.h"
#include <math.h>
#include <queue>

//*****************************************************************************************************************************************//

itree::itree(const data & dataObject): _dataObject(dataObject){}

itree::itree(const data & dataObject, int sampleSize, int maxTreeHeight, int maxNumOfNodes, int avgPLEstimation): _dataObject(dataObject), _sampleSize(sampleSize), _maxTreeHeight(maxTreeHeight), _maxNumOfNodes(maxNumOfNodes){}

itree::~itree(){}
//*****************************************************************************************************************************************//

//Standard Deviation Calculator
class StdDevCalc{
    private:

        double count;
        double meanPrev, meanCurr;
        double sPrev, sCurr;
        double varianceCurr;

    public:

        StdDevCalc() {
            count = 0;
        }

        void append(double d) {
            count++;

            if (count == 1) {
                // Set the very first values.
                meanCurr     = d;
                sCurr        = 0;
                varianceCurr = sCurr;
            }
            else {
                // Save the previous values.
                meanPrev     = meanCurr;
                sPrev        = sCurr;

                // Update the current values.
                meanCurr     = meanPrev + (d - meanPrev) / count;
                sCurr        = sPrev    + (d - meanPrev) * (d - meanCurr);
                varianceCurr = sCurr / (count - 1);
            }
        }

    double get_std_dev() {
        return sqrt(varianceCurr);
    }
};


void generalise(double* oldData, vector<long double>& mean, vector<long double>& standard_dev, vector<double>& newData)
{
	for(int j=0;j<mean.size();j++)
	{
		newData.push_back((oldData[j]-mean[j])/(standard_dev[j]));
	}
}


void DataConvertor(const data & dataObject, vector<int> &indices, vector<long double> &mean, vector<long double> &standard_dev, vector<vector<double>>& trainingData)
{
	for(int j=0;j<dataObject.getnumAttributes();j++)
	{
		StdDevCalc stddev;
		mean.push_back(0.0);
		for(int i=0;i<indices.size();i++)
		{
			mean[j] += (dataObject.dataVector[indices[i]]->attributes[j] - mean[j]) / (double)(i+1.0);
			stddev.append(dataObject.dataVector[indices[i]]->attributes[j]);
		}
		standard_dev.push_back(stddev.get_std_dev());
	}

	for(int i=0;i<indices.size();i++)
	{
		vector<double> newData;
		generalise(dataObject.dataVector[indices[i]]->attributes, mean, standard_dev, newData);

		trainingData.push_back(newData);
	}
}


double euclidean_dis(vector<double>& node1, vector<double>& node2)
{
	long double squared_dist=0.0;

	for(int j=0; j<node1.size(); j++)
	{
		squared_dist+=(node1[j] - node2[j])*(node1[j] - node2[j]);
	}

	return sqrt(squared_dist);
}


treenode* findParentNode(int x, vector<treenode *>& baseTrees)
{
	treenode* node = baseTrees[x];
	while(node->parentAdd!=NULL)
	{
		node=node->parentAdd;
	}

	return node;
}

void mergeNodes(treenode* y, treenode* z, double distance, int nodeIndex)
{
	treenode* newNode = new treenode(nodeIndex);
	newNode->lChildAdd = y;
	newNode->rChildAdd = z;
	newNode->parentAdd = NULL;
	newNode->distance = distance;
	newNode->nodeSize = y->nodeSize+z->nodeSize;
	y->parentAdd = newNode;
	z->parentAdd = newNode;
}

void itree::constructiTree(){

    vector<int> indices = _dataObject.getSample(_sampleSize);

	vector<vector<double>> trainingData;
	DataConvertor(_dataObject, indices, mean, standard_dev, trainingData);
	
	// for(int i=0;i<_dataObject.getnumAttributes();i++)
	// {
	// 	cout<<mean[i]<<" "<<standard_dev[i]<<endl;
	// }
	vector<pair<double, pair<int, int>>> edgeList;
	
	for(int i=0; i<trainingData.size(); i++)
	{
		for(int j=i+1; j<trainingData.size(); j++)
		{
			edgeList.push_back({euclidean_dis(trainingData[i], trainingData[j]), {i, j}});
		}
	}

	sort(edgeList.begin(), edgeList.end());

	baseTrees.resize(trainingData.size());
	for(int i=0; i<trainingData.size(); i++)
	{
		baseTrees[i] = new treenode(i);
		baseTrees[i]->parentAdd = NULL;
		baseTrees[i]->lChildAdd = NULL;
		baseTrees[i]->rChildAdd = NULL;
		baseTrees[i]->dataPoint = trainingData[i];
		baseTrees[i]->nodeSize = 1;
	}

	int edgesMSTCounter = 0;
	int edgesIndex = 0;
	int nodeIndex = trainingData.size();

	while(edgesMSTCounter<trainingData.size()-1)
	{
		pair<double, pair<int, int>> edge=edgeList[edgesIndex];
		edgesIndex++;

		treenode * y = findParentNode(edge.second.first, baseTrees);
		treenode * z = findParentNode(edge.second.second, baseTrees);

		if(y!=z)
		{
			edgesMSTCounter++;
			mergeNodes(y, z, edge.first, nodeIndex);
			nodeIndex++;
		}
	}
	
}


//*******************************************************Compute path length**************************************************************//
void itree::computePathLengthAndNearestPointDistance(int pointX, const data & testDataObject, long double & pathLength, long double & nearestPointDistance)
{
	nearestPointDistance = 0.0;

	vector<double> testData;
	generalise(testDataObject.dataVector[pointX]->attributes, mean, standard_dev, testData);

	int nearestPointindex = 0;
	nearestPointDistance = euclidean_dis(testData, baseTrees[0]->dataPoint);

	for(int i=1; i<baseTrees.size(); i++)
	{
		if(nearestPointDistance > euclidean_dis(testData, baseTrees[i]->dataPoint))
		{
			nearestPointDistance = euclidean_dis(testData, baseTrees[i]->dataPoint);
			nearestPointindex = i;
		}
	}

	pathLength = 0.0;
	treenode* node = baseTrees[nearestPointindex];
	while(node->parentAdd != NULL)
	{
		node = node->parentAdd;
		pathLength += 1.0;
	}

}

long double itree::avgPathLengthComputationOfBST(){
	long double avgActualPathLengthOfBST = 0;
	queue<treenode *> BFTqueue;
	int height = 0;
	BFTqueue.push(rootNode);
	while(!BFTqueue.empty()){

		int siz=BFTqueue.size();
		while(siz--)
		{
			treenode *node = BFTqueue.front();
    		BFTqueue.pop();
		
			if(node->isLeaf){
				avgActualPathLengthOfBST += node->nodeSize*(height);
			}else{
				BFTqueue.push(node->lChildAdd);
				BFTqueue.push(node->rChildAdd);
			}
		}
		height++;
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


























