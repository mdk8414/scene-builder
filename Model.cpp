#include "Model.h"
#include <fstream>

Model::Model(const std::string &fileName){
    //Load Model 1
	std::ifstream modelFile;
	modelFile.open(fileName);
	int numLines = 0;
	modelFile >> numLines;
	model = new float[numLines];
	for (int i = 0; i < numLines; i++){
		modelFile >> model[i];
	}
	printf("%d\n",numLines);
	int numVertsTeapot = numLines/8;
	modelFile.close();
}