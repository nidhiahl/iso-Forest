#ifndef IFOREST_H
#define IFOREST_H
#include "data.h"
#include "itree.h"
#include <random>

class iforest
{
    public:
	//iforest();
    iforest(const data &);                              //used for: create an empty forest load the iforest from file and insert new points in data object
	iforest(const data &, int, int, int, int);          //used for: create a forest using data object and initialize _sampleSize and _numiTrees
    virtual ~iforest();
    void constructiForest();    						//used for creating an iForest (static vesion) 
    void writeFOREST(const string &);					//writes iforest in file.
    void readFOREST(const string &);        			//reads iforest from file.
	void computeAnomalyScore(int, const data &);		        		//compute anomaly score of a point passed as an int argument
	
	private:    
	long double computeAvgPathLength(int, const data &);              //compute average path length of point passed as an argumnet i.e Averaged across all the iTrees
    long double avgPathLengthEstimationOfBST(int);      //compute the average path length estimation of a 'n' sized BST, 'n' is passed as an argument
    long double avgPathLengthComputationOfBST();        //compute the actual average path length of all the created iTrees from randomly choosen sample points,

	public:
    vector<double> anomalyScore;						//contains the computed anomaly score of all the points, evaluated using the iForest in hand 
    
    private:
    int _numiTrees;										//number of iTrees in the iForest.
	int _sampleSize;									//sample size representing the iForest.
    int _maxTreeHeight;									//max Height of each iTree in iForest.
  	int _maxNumOfNodes;									//max number of node possible in each iTree.
	int _avgPLEstimationOfBST;							//average path length estimation of BST having '_sampleSize' number of nodes. 
	int _avgPLComputationOfBST;
	int _exLevel;                                       //number of attributes to be considered while choosing hyperplane.
	int _numOfTrialsForHyperplane;                      //number of trials to be taken before choosing the best hyperplane.
	const data & _dataObject;							//reference of the input dataObject, only a container not responsible for deletion of the object.
	vector<itree*> _iTrees;								//list of pointers to the iTrees in the forest.
	    
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
		//ar & BOOST_SERIALIZATION_NVP(anomalyScore);
        ar & _sampleSize;
        ar & _maxTreeHeight;
  		ar & _maxNumOfNodes;
		ar & _numiTrees;
		ar & _avgPLEstimationOfBST;
		ar & _avgPLComputationOfBST;
	}
};

#endif // IFOREST_H
