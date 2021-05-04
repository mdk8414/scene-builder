#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"

class Mouse{
    glm::vec3 curRay;
    glm::mat4 projMatrix;
    glm::mat4 viewMatrix;
    glm::vec3 eye;
public:
    Mouse(glm::vec3 e, glm::mat4 proj){
        eye = e;
        projMatrix = proj;
        viewMatrix = glm::lookAt(eye, glm::vec3(0,0,-1), glm::vec3(0,1,0));
    }

    glm::vec3 getCurRay(){
        return curRay;
    }

    void update(){
        
    }

    glm::vec3 calcRay(){

    }

}