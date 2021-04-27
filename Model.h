#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class Model{
    int numVerts;
    int start;
    //float* model;
public:

    Model(const std::string &fileName){
        ifstream modelFile;
        modelFile.open(fileName);
        int numLines = 0;
        modelFile >> numLines;
        float* model = new float[numLines];
        for (int i = 0; i < numLines; i++){
            modelFile >> model[i];
        }
        printf("%d\n",numLines);
        numVerts = numLines/8;
        modelFile.close();
    }

};