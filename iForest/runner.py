import sys, os
from unittest import result
import pandas as pd
import numpy as np
import csv
# import matplotlib.pyplot as plt
# from sklearn import metrics

runs = 10
dataset = str(sys.argv[1])
numTrees = 100
samplingFactor = 0.01
minSampleSize = 256

avg_dpTime=0.0
avg_ifTime=0.0
avg_adTime=0.0
avg_iFMemUsed=0.0

os.system('g++ ./main.cpp ./data.cpp  ./iforest.cpp ./itree.cpp ./treenode.cpp -lboost_serialization')
os.system("rm -f "+dataset[10:-4]+'_result.csv')

# file_results = open('SingleBlob_result.csv', 'w')
# file_results.write("dptime dptime_val iftime iftime_val adttime adttime_val ifmemused ifmemused_val\n")

for run in range(1, runs+1):    
    os.system('./a.out '+dataset+' '+str(numTrees)+' '+str(samplingFactor)+' '+str(minSampleSize)+' >> '+dataset[10:-4]+'_result.csv')

    data = pd.read_csv('anomalyScores/'+dataset[10:-4]+'.csv', delimiter=' ')
    point_ids = data['pointId'].to_numpy()
    ascores = data['Ascore'].to_numpy()
    actual_labels = data['actuallabel'].to_numpy()


# results = pd.read_csv(dataset[10:-4]+'_result.csv', delimiter=' ')
# print(results)

run=0
with open(dataset[10:-4]+'_result.csv', "r", encoding='ascii') as file:
    read = csv.reader(file)
    for row in read :
        run+=1
        avg_dpTime = avg_dpTime + (float(row[0].split(' ')[1]) - avg_dpTime)/run
        avg_ifTime = avg_ifTime + (float(row[0].split(' ')[3]) - avg_ifTime)/run
        avg_adTime = avg_adTime + (float(row[0].split(' ')[5]) - avg_adTime)/run
        avg_iFMemUsed = avg_iFMemUsed + (float(row[0].split(' ')[7]) - avg_iFMemUsed)/run


print("dpTime : " + str(avg_dpTime) + "\tifTime : " + str(avg_ifTime) + "\tadTime : " + str(avg_adTime) + "\tiFMemUsed : " + str(avg_iFMemUsed))





