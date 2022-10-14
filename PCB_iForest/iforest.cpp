#include "iforest.h"
#include <queue>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <math.h>
using namespace std;


double binSearch(double val, vector<double>& arr)
{
	int l=0;
	int h=arr.size()-1;

	while(l<=h)
	{
		int m=l+(h-l)/2;

		if(arr[m]<=val)
		{
			l=m+1;
		}
		else
		{
			h=m-1;
		}
	}

	return double(l);
}

//iforest::iforest(){}

iforest::iforest(const data & dataObject): _dataObject(dataObject){}

iforest::iforest(const data & dataObject, int numiTrees, int sampleSize, int windowSize, double anomalyThreshold):_dataObject(dataObject), _numiTrees(numiTrees), _sampleSize(sampleSize), _windowSize(windowSize), _anomalyThreshold(anomalyThreshold)
{
	_maxTreeHeight = (int)log2(_sampleSize);
	_maxNumOfNodes = (int)pow(2.0,_maxTreeHeight+1)-1;
	_avgPLEstimationOfBST = avgPathLengthEstimationOfBST(_sampleSize);
	_iTrees.resize(_numiTrees);
	startWindow = 0;
	endWindow = -1;
}

iforest::~iforest(){}

//***************************************************STATIC iForest creation***************************************************************//
void iforest::constructiForest(){
    for(int treeId = 0; treeId < _numiTrees; treeId++){
		//cout<<"treeId===="<<treeId<<endl;
		_iTrees[treeId] = new itree(_dataObject, _sampleSize, _maxTreeHeight, _maxNumOfNodes, _avgPLEstimationOfBST);
		_iTrees[treeId]->constructiTree(0, _windowSize-1);
		pc.push_back(0);
		
	}
	//_avgPLComputationOfBST = avgPathLengthComputationOfBST();
}

//***************************************************DYNAMIC iForest Updation*************************************************************//
long double iforest::computeAnomalyScore(int pointX, const data & testDataObject){
	long double avgPathLength = computeAvgPathLength(pointX, testDataObject);
	//long double AscoreComputed = pow(2,-(avgPathLength/_avgPLComputationOfBST));   
	//cout<<"iForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<avgPathLength<<endl;
	//cout<<"iForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<_avgPLEstimationOfBST<<endl;
	
	// long double AscoreEstimated = avgPathLength;
	//long double AscoreEstimated = pow(2,-(avgPathLength/_avgPLEstimationOfBST));
	long double AscoreEstimated = computeRelativeMass(pointX, testDataObject);

	//cout<<"iForestObject->computeAnomalyScore(pointi,refTestDataObject)="<<AscoreEstimated<<endl;
	//anomalyScore.push_back(AscoreEstimated);

	
	return AscoreEstimated;
}

long double iforest::addElement(int point)
{
	vector<long double> ascores(_numiTrees); 
	long double avgAscore=0.0;

	for(int treeId = 0; treeId < _numiTrees; treeId++){
		long double PathLength = _iTrees[treeId]->computePathLength(point, _dataObject);
		ascores[treeId] = pow(2,-(PathLength/_avgPLEstimationOfBST));
		avgAscore += (ascores[treeId]-avgAscore)/(treeId+1);
	}

	
	for(int treeId = 0; treeId < _numiTrees; treeId++){
		if(avgAscore>_anomalyThreshold)
		{
			if(ascores[treeId]>_anomalyThreshold)
			{
				pc[treeId]++;
			}
			else
			{
				pc[treeId]--;
			}
		}
		else
		{
			if(avgAscore<_anomalyThreshold)
			{
				pc[treeId]++;
			}
			else
			{
				pc[treeId]--;
			}
		}
	}

	endWindow++;
	if(endWindow-startWindow+1>_windowSize)
	{
		startWindow++;
	}
	else
	{
		return avgAscore;
	}

	if(isDrift())
	{
		for(int treeId = 0; treeId < _numiTrees; treeId++){
			// cout<<pc[treeId]<<endl;
			if(pc[treeId]<0)
			{
				// cout<<treeId<<endl;
				_iTrees[treeId]->deleteSubTree(_iTrees[treeId]->rootNode);
				_iTrees[treeId]->constructiTree(startWindow, endWindow);
			}
			pc[treeId]=0;
		}
		startWindow += ((_windowSize)*2)/3;
	}

	return avgAscore;
}

bool iforest::isDrift()
{
	for(int attr=0; attr<_dataObject.getnumAttributes(); attr++)
	{
		vector<int> index1 = _dataObject.getSample(endWindow-(_windowSize/3)+1, endWindow, _windowSize/3);
		vector<int> index2 = _dataObject.getSample(startWindow, endWindow-(_windowSize/3), _windowSize/3);

		vector<double> arr1;
		vector<double> arr2;

		for(int i=0; i<_windowSize/3; i++)
		{
			arr1.push_back(_dataObject.dataVector[index1[i]]->attributes[attr]);
			arr2.push_back(_dataObject.dataVector[index2[i]]->attributes[attr]);
		}

		sort(arr1.begin(), arr1.end());
		sort(arr2.begin(), arr2.end());

		double prob;
		double st;
		ks_2samp(arr1, arr2, prob, st);
		// cout<<prob<<" "<<st<<endl;
		if(prob<=0.01 && st>0.1)
		{
			return true;
		}

	}
	return false;
}

void iforest::ks_2samp(vector<double> &arr1, vector<double> &arr2, double &prob, double &st)
{
	vector<double> arr;
	for(int i=0; i<arr1.size(); i++)
	{
		arr.push_back(arr1[i]);
	}
	for(int i=0; i<arr2.size(); i++)
	{
		arr.push_back(arr2[i]);
	}

	vector<double> cdf1;
	vector<double> cdf2;
	vector<double> cddiffs;
	double minS = 0.0;
	double maxS = 0.0;
	for(int i=0; i<arr.size(); i++)
	{
		double val1 = binSearch(arr[i], arr1)/(double) arr1.size();
		cdf1.push_back(val1);
		double val2 = binSearch(arr[i], arr2)/(double) arr2.size();
		cdf2.push_back(val2);
		cddiffs.push_back(cdf1[i]-cdf2[i]);

		minS = min(minS, cddiffs[i]);
		maxS = max(maxS, cddiffs[i]);
	}

	double d = max(maxS, -1*minS);

	_attempt_exact_2kssamp(arr1.size(), d, prob);
	st = d;
}

void iforest::_attempt_exact_2kssamp(int n, double & d, double & prob)
{
	int h = round(d*n);
	d = double(h) / double(n);

	if(h==0)
	{
		prob=1.0;
		return ;
	}

	prob = 0.0;
	int k = n/h;
	while(k>=0)
	{
		double prob1 = 1.0;

		for(int j=0; j<h; j++)
		{
			prob1 = ((n-k*h-j)*prob1)/(n+k*h+j+1);
		}
		prob = prob1*(1.0 - prob);
		k--;
	}

	prob*=2.0;
}


long double iforest::avgPathLengthEstimationOfBST(int size){
	long double avgPathLengthEstimationOfBST = 0;
	if(size == 2){avgPathLengthEstimationOfBST=1;}
	else if(size > 2){avgPathLengthEstimationOfBST = (2 * (log(size-1) + 0.5772156649)) - (2 * (size - 1) / size);}	
	return (avgPathLengthEstimationOfBST);
}



long double iforest::computeAvgPathLength(int pointX, const data & testDataObject){
	long double avgPathLength = 0;
	for(int treeId = 0; treeId < _numiTrees; treeId++){
		avgPathLength += _iTrees[treeId]->computePathLength(pointX, testDataObject);
	}
	avgPathLength /=_numiTrees;	
	return avgPathLength;
}

long double iforest::computeRelativeMass(int pointX, const data & testDataObject)
{
	long double avgRelativeMass = 0.0;
	for(int treeId = 0; treeId < _numiTrees; treeId++){
		avgRelativeMass += _iTrees[treeId]->computeRelativeMass(pointX, testDataObject);
	}
	avgRelativeMass /=_numiTrees;
	return avgRelativeMass;
}

















