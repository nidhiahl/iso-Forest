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
    srand(time(0));
	const string &dataFile = argv[1];
	const int &numOfTrees = atoi(argv[2]);
	const double &samplingFactor = atof(argv[3]);
	const int &minSampleSize = atoi(argv[4]);
	const int &windowSize = atoi(argv[5]);

    //This is for testing on different data.
    const string &dataFile2 = argv[6];

    /************************************************dataPreparation******************************************************************/

    struct timespec start_dP,end_dP;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dP);
    data *dataObject = new data();
    const data &trainDataObject = *dataObject;
    dataObject->createDataVector(dataFile);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_dP);
    double dPTime =  (((end_dP.tv_sec - start_dP.tv_sec) * 1e9)+(end_dP.tv_nsec - start_dP.tv_nsec))*1e-9;
    //cout<<"Time taken in dP: "<< fixed<<dPTime<<"sec"<<endl;

    //This is for testing on different data
    data *dataObject2 = new data();
    const data &testDataObject = *dataObject2;
    dataObject2->createDataVector(dataFile2);

    /************************************************iForest creation***************************************************/
    int sampleSize;
    int countOfCurrentPoints = dataObject->getnumInstances();
	sampleSize = countOfCurrentPoints * samplingFactor < minSampleSize ? minSampleSize :countOfCurrentPoints * samplingFactor;
    sampleSize = windowSize < sampleSize ? windowSize : sampleSize;
	int iForestRamUsed = getValue(1);
	iforest *iForestObject = new iforest(trainDataObject, numOfTrees, sampleSize);
    struct timespec start_iF,end_iF;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_iF);

    //Sliding iForest Logic****
    int windowStartIndex=0;
    double anomaly_rate=0.05;

    iForestObject->constructiForest(windowStartIndex, windowSize);
    vector<pair<double, int>> AnomalyScores;

    for(int windowStartIndex=0; windowStartIndex<trainDataObject.getnumInstances(); windowStartIndex+=windowSize)
    {
        int actualAnomalies=0;
        int truePositives=0;
        iForestObject->anomalyScore.clear();
        AnomalyScores.clear();

        for(int pointi=windowStartIndex; pointi<windowStartIndex+windowSize; pointi++)
        {
            iForestObject->computeAnomalyScore(pointi, trainDataObject);
            AnomalyScores.push_back({iForestObject->anomalyScore[pointi-windowStartIndex], pointi});
        }

        sort(AnomalyScores.begin(), AnomalyScores.end());
        reverse(AnomalyScores.begin(), AnomalyScores.end());

        for(pair<double, int> tmp: AnomalyScores)
        {
            if(trainDataObject.dataVector[tmp.second]->label == 1) actualAnomalies++;
        }

        for(int i=0; i<actualAnomalies; i++)
        {
            if(trainDataObject.dataVector[AnomalyScores[i].second]->label == 1) truePositives++;
        }

        if(truePositives > anomaly_rate*windowSize)
        {
            iForestObject->constructiForest(windowStartIndex, windowSize);
        }
        
    }
    

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_iF);
	iForestRamUsed = getValue(1) - iForestRamUsed;
	double iFTime =  (((end_iF.tv_sec - start_iF.tv_sec) * 1e9)+(end_iF.tv_nsec - start_iF.tv_nsec))*1e-9;

    //cout<<"RAM="<<iForestRamUsed<<endl;
    //cout << "Time taken to create iforest: " << fixed << iFTime<<"sec"<<endl;
	
	 

	/*****************************Anomaly detection(AD): Path length computation*********************************************************/
	
	struct timespec start_AD,end_AD;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_AD);
    
	for(int pointi =0; pointi < testDataObject.getnumInstances();pointi++){
    	iForestObject->computeAnomalyScore(pointi, testDataObject);
   	}
   	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_AD);
	iForestRamUsed = getValue(1) - iForestRamUsed;
	double ADTime =  (((end_AD.tv_sec - start_AD.tv_sec) * 1e9)+(end_AD.tv_nsec - start_AD.tv_nsec))*1e-9;
    
    vector<double> AnomalyScore = iForestObject->anomalyScore;

	delete iForestObject;
	//cout<<"RAM="<<iForestRamUsed<<endl;
    
	cout<<"dPTime: "<<dPTime<<" iFTime: "<<iFTime<<" ADTime: "<<ADTime<<" iFMemUsed:  "<<iForestRamUsed<<endl;


	/****************************************Anomaly Score writing to file**************************************************************/

    string outputFileName="anomalyScores/"+dataFile2.substr(10,dataFile2.length()-14)+"_tested_over_"+dataFile.substr(10);
    // cout<<outputFileName<<endl;
	ofstream outAnomalyScore(outputFileName, ios::out|ios::binary);
    outAnomalyScore<<"pointId "<<"Ascore "<<"actuallabel"<<endl;
    for(int pointi = 0; pointi < testDataObject.getnumInstances(); pointi++){
    	outAnomalyScore<<pointi<<" "<<AnomalyScore[pointi]<<" "<<testDataObject.dataVector[pointi]->label<<endl;
    }



    return 0;
}
