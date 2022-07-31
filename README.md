# iso-Forest
A c++ implementation of isolation forest.

iForest/data.h and iForest/data.cpp are the data preparation files, reads from a data file and create the input ready for iForest construction.

iForest/iforest.h and iForest/iforest.cpp, iForest/itree.h and iForest/itree.cpp, iForest/treenode.h and iForest/treenode.cpp consists of components for isoaltion forest creation and anomaly detection application.

iForest/main.cpp is the driver cpp file that run the end to end algorithm from dataPreparation to Anomaly score computation and writing anomaly score to file.
