#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"

class Object{
public:
    glm::vec3 pos, scale, rotAxis, color;
    int modelNum, texNum;
    glm::vec3 rotAngle;
    char type;
    bool isSelected;
    Object(){
        pos = glm::vec3(0,0,0);
        scale = glm::vec3(1,1,1);
        rotAxis = glm::vec3(1,1,1);
        color = glm::vec3(0,0,0);
        rotAngle = glm::vec3(0,0,0);
        modelNum = 0;
        texNum = -1;
        isSelected = false;
    }

    Object(glm::vec3 p, glm::vec3 s, glm::vec3 ax, glm::vec3 a, int m, int tex, glm::vec3 col, char t){
        pos = p;
        scale = s;
        rotAxis = ax;
        rotAngle = a;
        modelNum = m;
        texNum = tex;
        color = col;
        type = t;
        isSelected = false;
    }

    glm::vec3 getPos();
    glm::vec3 getScale();
    glm::vec3 getAxis();
    glm::vec3 getAngle();
    glm::mat4 getTransform();
    //int getIndex();
    //int getNumVerts();
    int tex();
};