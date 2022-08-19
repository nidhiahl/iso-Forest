#define AnomalyDetection false
#include "iforest.h"
#include <queue>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <math.h>
using namespace std;

iforest::iforest(const data & dataObject): _dataObject(dataObject){}

iforest::iforest(const data & dataObject, int numiTrees, int sampleSize):_dataObject(dataObject), _numiTrees(numiTrees), _sampleSize(sampleSize){
	_maxTreeHeight = (int)log2(_sampleSize);
	_maxNumOfNodes = (int)pow(2.0,_maxTreeHeight+1)-1;
	_iTrees.resize(_numiTrees);	
}

iforest::~iforest(){}

//***************************************************Training: iForest creation*********************************************************//
void iforest::constructiForest(){
    for(int treeId = 0; treeId < _numiTrees; treeId++){
		_iTrees[treeId] = new itree(_dataObject, _sampleSize, _maxTreeHeight, _maxNumOfNodes, _avgPLEstimationOfBST);
		_iTrees[treeId]->constructiTree();
	}
	//_avgPLComputationOfBST = this->avgPathLengthComputationOfBST();
	_avgPLEstimationOfBST = this->avgPathLengthEstimationOfBST(_sampleSize);
}



//***************************************Evaluation: Anoamly detection AD: anomaly score computation *************************************//

void iforest::computeAnomalyScore(int pointX, const data & testDataObject){
	long double AscoreComputed = 0;
	for(int treeId = 0; treeId < _numiTrees; treeId++){
		AscoreComputed += _iTrees[treeId]->computeRelativeMass(pointX, testDataObject, _sampleSize);
	}
	// cout<<AscoreComputed<<endl;
	AscoreComputed /=_numiTrees;
	anomalyScore.push_back(AscoreComputed);
}

long double iforest::avgPathLengthEstimationOfBST(int size){
	long double avgPathLengthEstimationOfBST = 0;
	if(size == 2){avgPathLengthEstimationOfBST=1;}
	else if(size > 2){avgPathLengthEstimationOfBST = (2 * (log(size-1) + 0.5772156649)) - (2 * (size - 1) / size);}	
	return (avgPathLengthEstimationOfBST);
}

long double iforest::avgPathLengthComputationOfBST(){
	long double avgPathLengthComputationOfBST = 0;
	for(int treeId = 0; treeId < _numiTrees; treeId++){
		avgPathLengthComputationOfBST += _iTrees[treeId]->avgPathLengthComputationOfBST();
	}
	avgPathLengthComputationOfBST /=_numiTrees;
	return avgPathLengthComputationOfBST;
}



//***************************************************Write iforest to file*************************************************************//
void iforest::writeFOREST(const string & FORESTFile){
    //cout<<"/**************SAVE forest******************///////"<<endl;
    string treenodeFile = FORESTFile + "_TREENODES";
    ofstream storeNodes(treenodeFile.c_str(),ios::out|ios::binary);
    if(!storeNodes){
        cout<<"Cannot open the output file "<<FORESTFile<<endl;
        exit(0);
    }
    boost::archive::binary_oarchive storeNodesArchive(storeNodes);
    for(int treeId = 0; treeId < _numiTrees; treeId++){
        queue<treenode*> BFTforNodes;
		BFTforNodes.push(_iTrees[treeId]->rootNode);
		while(!BFTforNodes.empty()){
    		treenode *currNode = BFTforNodes.front();
    		BFTforNodes.pop();
	        if(currNode->lChildAdd != nullptr){
				BFTforNodes.push(currNode->lChildAdd);
			}
			if(currNode->rChildAdd != nullptr){
				BFTforNodes.push(currNode->rChildAdd);
			}
            storeNodesArchive << *(currNode);
		}        
    }
    storeNodes.close();
    string iForestFile = FORESTFile + "_iFOREST";
    ofstream storeForest(iForestFile.c_str(),ios::out|ios::binary);
    if(!storeForest){
            cout<<"Cannot open the output file "<<FORESTFile<<endl;
            exit(0);
        }
    boost::archive::binary_oarchive storeForestArchive(storeForest);
    storeForestArchive << *(this);
    storeForest.close();
}


//***************************************************read iforest from file**************************************************************//
void iforest::readFOREST(const string & FORESTFile){
    //cout<<"/**********************read forest**************/"<<endl;
    string iTreesFile = FORESTFile + "_iFOREST";
    ifstream readForest(iTreesFile.c_str(),ios::in|ios::binary);
    if(!readForest){
            cout<<"Cannot open the input file "<<FORESTFile<<endl;
            exit(0);
        }
    boost::archive::binary_iarchive readForestArchive(readForest);
    readForestArchive >> *(this);
    readForest.close();
    string treenodeFile = FORESTFile + "_TREENODES";
    ifstream readNodes(treenodeFile.c_str(),ios::in|ios::binary);
    if(!readNodes){
            cout<<"Cannot open the input file "<<FORESTFile<<endl;
            exit(0);
        }
    boost::archive::binary_iarchive readNodesArchive(readNodes);
    _iTrees.resize(_numiTrees);
    for(int treeId = 0; treeId < _numiTrees; treeId++){
    	_iTrees[treeId] = new itree(_dataObject,_sampleSize,_maxTreeHeight,_maxNumOfNodes,_avgPLEstimationOfBST);
    	
    	treenode *rootNode = new treenode();
    	readNodesArchive >> *(rootNode);
    	_iTrees[treeId]->rootNode = rootNode;
    	queue<treenode*> BFTforNodes;
		BFTforNodes.push(rootNode);
    	while(!BFTforNodes.empty()){
    		treenode *currNode = BFTforNodes.front();
    		BFTforNodes.pop();
	    	if(currNode->lChildId > 0){
	    		treenode *lChild = new treenode();
    			readNodesArchive >> *(lChild);
    			currNode->lChildAdd = lChild;
    			lChild->parentAdd = currNode;
    			BFTforNodes.push(lChild);    	
	    	}
	    	if(currNode->rChildId > 0){
	    		treenode *rChild = new treenode();
    			readNodesArchive >> *(rChild);
    			currNode->rChildAdd = rChild;
    			rChild->parentAdd = currNode;
    			BFTforNodes.push(rChild);    	
	    	}
	    }
    }
    readNodes.close();
}









