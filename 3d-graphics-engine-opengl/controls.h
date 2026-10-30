#ifndef CONTROLS_HPP
#define CONTROLS_HPP

void computeMatricesFromInputs(float aspect, float near, float far);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::vec3 getCameraPosition();
glm::vec3 getCameraFront();
glm::vec3 getCameraUp();
glm::vec3 getCameraRight();
float getFOV();

#endif