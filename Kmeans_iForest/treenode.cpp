#include "treenode.h"
#include <math.h>
#include <random>

treenode::treenode(int height)
{
    splitAttribute = -1;
    isLeaf = bool(0);
    nodeSize = 0;
    pathLengthEst = 0;
    dataPointIndices.resize(0);
    children.resize(0);
	centroid=0.0;
	leftLimit=999999.0;
	rightLimit=-999999.0;
	nodeHeight=height;
}


treenode::~treenode()
{ 
    //dtor
}

void treenode::createChild()
{
    treenode *newNode = new treenode(nodeHeight+1);
    children.push_back(newNode);
}

//get K random samples out of N without replacement.
inline std::vector<int> sample_without_replacement(int k, int N)
{
    std::random_device random_seed_generator;
    std::mt19937_64 RandomEngine(random_seed_generator());

    std::unordered_set<int> samples;

    for(int i=N-k+1; i<N+1; i++)
    {
        int v=std::uniform_int_distribution<>(1, i)(RandomEngine);
        if(!samples.insert(v).second) samples.insert(i);
    }

    std::vector<int> result(samples.begin(), samples.end());
    std::shuffle(result.begin(), result.end(), RandomEngine);

    return result;    
}

double knn(int k, vector<double>& data, vector<vector<double>>& centroids)
{
	
	vector<double> centroid(k, 0.0);
	vector<int> cluster(data.size(), -1);
	vector<double> clusterSize(k, 0.0);

	//get Random centroids
	vector<int> samples=sample_without_replacement(k, data.size());
	for(int i=0;i<k;i++) centroid[i]=data[samples[i]-1];
	sort(centroid.begin(), centroid.end());

	double wcss;
	bool change=true;
	while(change)       //while the process has not converged
	{
		wcss=0.0;
		change=false;
		//assign datapoints to clusters
		for(int i=0;i<data.size();i++)
		{
			double dis=999999.0;
			int clusterid=-1;
			for(int j=0;j<k;j++)
			{
				if(dis>abs(data[i]-centroid[j]))
				{
					dis=abs(data[i]-centroid[j]);
					clusterid=j;
				}
			}

			if(cluster[i]!=clusterid)
			{
				change=true;
				cluster[i]=clusterid;
			}

			wcss+=abs(data[i]-centroid[cluster[i]]);
		}

		//get new centroids
		clusterSize.resize(k, 0.0);
		centroid.resize(k, 0.0);

		for(int i=0;i<data.size();i++)
		{
			clusterSize[cluster[i]]+=1.0;
			centroid[cluster[i]] += (data[i] - centroid[cluster[i]]) / clusterSize[cluster[i]];
		}
	}

	centroids.push_back(centroid);
	return wcss;
}

int elbow(vector<double>& wcss)
{
	int optimalK=1;
	double mx_secDer=0.0;

	for(int i=1;i<wcss.size()-1;i++)
	{
		if(mx_secDer<wcss[i-1]+wcss[i+1]-2*wcss[i])
		{
			mx_secDer=wcss[i-1]+wcss[i+1]-2*wcss[i];
			optimalK=i;
		}
		
	}

	return optimalK+1;
}

void treenode::splitInfoSelection(const data &dataObject){
    std::random_device random_seed_generator;
    std::mt19937_64 RandomEngine(random_seed_generator());
	splitAttribute = std::uniform_int_distribution<>(0, dataObject.getnumAttributes()-1)(RandomEngine);
	
	
	vector<double> data(dataPointIndices.size());
	for(int i=0; i<dataPointIndices.size();i++)
	{
		data[i]=dataObject.dataVector[(dataPointIndices[i])]->attributes[splitAttribute];
		leftLimit=min(leftLimit, data[i]);
		rightLimit=max(rightLimit, data[i]);
	}

	vector<vector<double>> centroids;
	vector<double> wcss;

	int possibleK=10;
	if(data.size()<10) possibleK=data.size();

	for(int i=0;i<possibleK;i++)
	{
		wcss.push_back(knn(i+1, data, centroids));
	}

	int optimalK=elbow(wcss);

	for(int i=0;i<optimalK;i++)
	{
		createChild();
		children[i]->centroid=centroids[optimalK-1][i];
		children[i]->rightLimit=centroids[optimalK-1][i];
		children[i]->leftLimit=centroids[optimalK-1][i];
		
		if(i>0)
		{
			children[i]->leftLimit=(children[i-1]->centroid + children[i]->centroid)/2.0;
			children[i-1]->rightLimit=(children[i-1]->centroid + children[i]->centroid)/2.0;
		}
			
		// cout<<children[i]->leftLimit<<" "<<children[i]->centroid<<" "<<children[i]->rightLimit<<endl;
	}
}




	
