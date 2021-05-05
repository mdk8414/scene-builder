#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"

class Object{
public:
    glm::vec3 pos, scale, rotAxis, color;
    int index, numVerts, texNum;
    glm::vec3 rotAngle;
    char type;
    bool isSelected = false;
    Object(){
        pos = glm::vec3(0,0,0);
        scale = glm::vec3(1,1,1);
        rotAxis = glm::vec3(1,1,1);
        color = glm::vec3(0,0,0);
        rotAngle = glm::vec3(0,0,0);
        index = -1;
        numVerts = 0;
        texNum = -1;
    }

    Object(glm::vec3 p, glm::vec3 s, glm::vec3 ax, glm::vec3 a, int m, int nv, int tex, glm::vec3 col, char t){
        pos = p;
        scale = s;
        rotAxis = ax;
        rotAngle = a;
        index = m;
        numVerts = nv;
        texNum = tex;
        color = col;
        type = t;
    }

    glm::vec3 getPos();
    glm::vec3 getScale();
    glm::vec3 getAxis();
    glm::vec3 getAngle();
    glm::mat4 getTransform();
    int getIndex();
    int getNumVerts();
    int tex();
};