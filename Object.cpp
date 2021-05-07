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

glm::vec3 Object::getAngle(){
    return rotAngle;
}

glm::mat4 Object::getTransform(){
    glm::mat4 transM = glm::translate(pos);
    glm::mat4 scaleM = glm::scale(scale);
    glm::mat4 rotMx = glm::rotate(rotAngle.x, rotAxis);
    glm::mat4 rotMy = glm::rotate(rotAngle.y, rotAxis);
    glm::mat4 rotMz = glm::rotate(rotAngle.z, rotAxis);

    return transM * rotMx * rotMy * rotMz * scaleM;
}

/*int Object::getIndex(){
    return index;
}

int Object::getNumVerts(){
    return numVerts;
}*/

int Object::tex(){
    return texNum;
}