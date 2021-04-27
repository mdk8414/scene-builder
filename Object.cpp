#include "Object.h"

glm::vec3 Object::getPos(){
    return pos;
}

glm::vec3 Object::getScale(){
    return scale;
}

glm::vec3 Object::getAxis(){
    return rotAxis;
}

float Object::getAngle(){
    return rotAngle;
}

glm::mat4 Object::getTransform(){
    glm::mat4 transM = glm::translate(pos);
    glm::mat4 scaleM = glm::scale(scale);
    glm::mat4 rotM = glm::rotate(rotAngle, rotAxis);

    return transM * rotM * scaleM;
}

int Object::getIndex(){
    return index;
}

int Object::getNumVerts(){
    return numVerts;
}

int Object::tex(){
    return texNum;
}