#include "itree.h"
#include <math.h>
#include <queue>


long double euc_dis(vector<double>& node1, vector<double>& node2)
{
	long double squared_dist = 0.0;

	for(int j=0; j<node1.size(); j++)
	{
		squared_dist += (node1[j] - node2[j])*(node1[j] - node2[j]);
	}

	return sqrt(squared_dist);
}


itree::itree(const data & dataObject): _dataObject(dataObject){}

itree::itree(const data & dataObject, int sampleSize, int maxTreeHeight, int maxNumOfNodes, int avgPLEstimation): _dataObject(dataObject), _sampleSize(sampleSize), _maxTreeHeight(maxTreeHeight), _maxNumOfNodes(maxNumOfNodes){}

itree::~itree(){}
//*************************************************STATIC iTree creation*******************************************************************//
void itree::constructiTree(){

	vector<int> indices = _dataObject.getSample(_sampleSize);

	for(int i=0;i<indices.size();i++)
	{
		treenode* hypersphere=new treenode(i);
		for(int j=0;j<_dataObject.getnumAttributes();j++)
		{
			hypersphere->center.push_back(_dataObject.dataVector[indices[i]]->attributes[j]);
		}
		hyperspheres.push_back(hypersphere);
	}

	for(int i=0;i<indices.size();i++)
	{
		for(int j=i+1;j<indices.size();j++)
		{
			long double dis=euc_dis(hyperspheres[i]->center, hyperspheres[j]->center);
			if(dis<hyperspheres[i]->radius)
			{
				hyperspheres[i]->radius=dis;
				hyperspheres[i]->nextId=hyperspheres[j]->nodeId;
				hyperspheres[i]->nearestNeighbour=hyperspheres[j];
			}
			if(dis<hyperspheres[j]->radius)
			{
				hyperspheres[j]->radius=dis;
				hyperspheres[j]->nextId=hyperspheres[i]->nodeId;
				hyperspheres[j]->nearestNeighbour=hyperspheres[i];
			}
		}
	}

}


//*******************************************************Compute path length**************************************************************//
long double itree::computeAnomalyScore(int pointX, const data & testDataObject){

	long double anomalyScore=1.0;

	vector<double> testNode;
	for(int j=0;j<_dataObject.getnumAttributes();j++)
	{
		testNode.push_back(testDataObject.dataVector[pointX]->attributes[j]);
	}

	int smallestHypersphereIndex=-1;
	long double smallestHypersphereRadius=999999.0;

	for(int i=0;i<hyperspheres.size();i++)
	{
		long double dis=euc_dis(hyperspheres[i]->center, testNode);

		if(dis<=hyperspheres[i]->radius)
		{
			if(hyperspheres[i]->radius < smallestHypersphereRadius)
			{
				smallestHypersphereIndex=i;
				smallestHypersphereRadius=dis;
			}
		}
		
	}

	if(smallestHypersphereIndex>=0)
	{
		if(hyperspheres[smallestHypersphereIndex]->radius==0.0)
		{
			anomalyScore-=1.0;
		}
		else
		{
			anomalyScore -= hyperspheres[smallestHypersphereIndex]->nearestNeighbour->radius/hyperspheres[smallestHypersphereIndex]->radius;
		}
	}

	return anomalyScore;
}





























