#include <iostream>
#include <string>
# include "data.h"
# include "iforest.h"
#include <string>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp> 
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

using namespace std;

int parseLine(char *line){
    int i = strlen(line);
    const char *p = line;
    while(*p <'0' || *p >'9')
	p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValue(int par){
    FILE *file = fopen("/proc/self/status","r");
    int result = -1;
    char line[128];

    while(fgets(line,128,file) != NULL){
	if(par == 1){               //Ram used
   	    if(strncmp(line, "VmRSS:", 6) == 0){
		result = parseLine(line);
		break;
	    }
	}
	if(par == 2){               //virtual memory used
   	    if(strncmp(line, "VmSize:", 7) == 0){
		result = parseLine(line);
		break;
	    }
	}
    }
    fclose(file);
    return result;
}


int main(int argc, char* argv[])
{
    //i) dataFile: original data file name having complete dataset
	//ii) numOfTrees: count of itreees in the iforest.
	//iii) smaplingFactor: Fraction of total points in dataset that will act as the sampleSize for iTree creation. 0 < samplingFactor < 1
	//iv) minSampleSize: an interger value, useful when samplingFactor fails to give sufficient samples to create an iTree, when datset is small*/
    //v) exLevel: an integer value, denoting extension level.

    srand(time(0));
	const string &dataFile = argv[1];
	const int &numOfTrees = atoi(argv[2]);
	const double &samplingFactor = atof(argv[3]);
	const int &minSampleSize = atoi(argv[4]);
    int exLevel = atoi(argv[5]);
	
    /************************************************dataPreparation******************************************************************/

    struct timespec start_dP,end_dP;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dP);
    data *dataObject = new data();
    const data &refDataObject = *dataObject;
    dataObject->createDataVector(dataFile);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_dP);
    double dPTime =  (((end_dP.tv_sec - start_dP.tv_sec) * 1e9)+(end_dP.tv_nsec - start_dP.tv_nsec))*1e-9;
    //cout<<"Time taken in dP: "<< fixed<<dPTime<<"sec"<<endl;


    /************************************************iForest creation***************************************************/
    int sampleSize;
    int countOfCurrentPoints = dataObject->getnumInstances();
	sampleSize = countOfCurrentPoints * samplingFactor < minSampleSize ? minSampleSize :countOfCurrentPoints * samplingFactor;
    sampleSize = countOfCurrentPoints < sampleSize ? countOfCurrentPoints : sampleSize;
	int iForestRamUsed = getValue(1);
	iforest *iForestObject = new iforest(refDataObject, numOfTrees, sampleSize, exLevel);
    struct timespec start_iF,end_iF;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_iF);

    iForestObject->constructiForest();
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_iF);
	iForestRamUsed = getValue(1) - iForestRamUsed;
	double iFTime =  (((end_iF.tv_sec - start_iF.tv_sec) * 1e9)+(end_iF.tv_nsec - start_iF.tv_nsec))*1e-9;

    //cout<<"RAM="<<iForestRamUsed<<endl;
    //cout << "Time taken to create iforest: " << fixed << iFTime<<"sec"<<endl;
	
	 

	/*****************************Anomaly detection(AD): Path length computation*********************************************************/
	
	struct timespec start_AD,end_AD;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_AD);
    
	for(int pointi =0; pointi < refDataObject.getnumInstances();pointi++){
    	iForestObject->computeAnomalyScore(pointi);
   	}
   	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_AD);
	iForestRamUsed = getValue(1) - iForestRamUsed;
	double ADTime =  (((end_AD.tv_sec - start_AD.tv_sec) * 1e9)+(end_AD.tv_nsec - start_AD.tv_nsec))*1e-9;
    
    vector<double> AnomalyScore = iForestObject->anomalyScore;

	delete iForestObject;
	//cout<<"RAM="<<iForestRamUsed<<endl;
    
	cout<<"dPTime: "<<dPTime<<" iFTime: "<<iFTime<<" ADTime: "<<ADTime<<" iFMemUsed:  "<<iForestRamUsed<<endl;


	/****************************************Anomaly Score writing to file**************************************************************/

	ofstream outAnomalyScore(dataFile+"iForestAnomalyScore.csv", ios::out|ios::binary);
    outAnomalyScore<<"pointId "<<"Ascore "<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < refDataObject.getnumInstances(); pointi++){
    	outAnomalyScore<<pointi<<" "<<AnomalyScore[pointi]<<" "<<refDataObject.dataVector[pointi]->label<<endl;
    }



    return 0;
}
