#include <iostream>
#include <string>
# include "data.h"
# include "iforest.h"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

using namespace std;


int main(int argc, char* argv[])
{
	srand(time(0));
	const string &Di = argv[1];
	const int &numOfTrees = atoi(argv[2]);
	const double &samplingFactor = atof(argv[3]);
	const int &minSampleSize = atoi(argv[4]);
	const int &windowSize = atoi(argv[5]);
	const double &anomalyThreshold = atof(argv[6]);
	

/********************************Create data vector***********************/
	struct timespec start_dP,end_dP;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dP);

	data *dataObject = new data();
    const data &refDataObject = *dataObject;
	dataObject->createDataVector(Di);
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_dP);
    double dPTime =  (((end_dP.tv_sec - start_dP.tv_sec) * 1e9)+(end_dP.tv_nsec - start_dP.tv_nsec))*1e-9;


/********************************training iforest***************************/
	struct timespec start_train,end_train;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dP);

	int sampleSize;
	int totalPoints = dataObject->getnumInstances();
	sampleSize = totalPoints * samplingFactor < minSampleSize ? minSampleSize : totalPoints * samplingFactor;
    sampleSize = totalPoints < sampleSize ? totalPoints : sampleSize;
    iforest *iForestObject = new iforest(refDataObject, numOfTrees, sampleSize, windowSize, anomalyThreshold);
    iForestObject->constructiForest();


/***************************Evaluate the created iforest***************************************/	
	ofstream outAnomalyScore("anomalyScores/_AnomalyScore.csv", ios::out|ios::binary);
    outAnomalyScore<<"pointId "<<"anomalyscore"<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < refDataObject.getnumInstances(); pointi++){

    	outAnomalyScore<<pointi<<" "<<iForestObject->addElement(pointi)<<" "<<refDataObject.dataVector[pointi]->label<<endl;	
	}

	return 0;
}

















