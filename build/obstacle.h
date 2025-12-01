#pragma once

#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>

#include "../resource.h"
#include "../scene_node.h"

namespace game {
	class Obstacle : public SceneNode {	//Should Entity inherit from SceneNode or 
		//  should the player and enemies inherit from SceneNode AND Entity???

	public:
		Obstacle(const std::string name, const Resource* geometry, const Resource* material);
		~Obstacle();

		float GetHealth();
		void SetHealth(int hp);

		void SetStartPoint(glm::vec3 startPoint);
		glm::vec3 GetStartPoint(glm::vec3 startPoint);

		void SetEndPoint(glm::vec3 endPoint);
		glm::vec3 GetEndPoint(glm::vec3 endPoint);

//Add additional member functions here as needed to increase functionality.

		void Update(float deltaTime); //Signature may need to be changed depending on how movement is implemented.

	private:
		//The Player should only ever move in the x y plane.
		const float accelerationFactor_ = 0.1;			//Acceleration and velocity implementation need to be checked.
		glm::vec3 velocity_ = glm::vec3(0.0, 0.0, 0.0);	//  Unsure if this is the best way to store the variables.

		//Points to define the path the obstacle will follow. Linearly interpolate from start to end.
		glm::vec3 startPoint_ = glm::vec3(0.,0.,0.);
		glm::vec3 endPoint_;		//Set end point far enough back that the obstacle is offscreen before moving it for reuse.
		glm::vec3 currentPosition_; //The cureent linearly interpolated position of the obstacle.

		float lifeTime_ = 0.0;

		//AABB info
		float xMax;
		float xMin;
		float yMax;
		float yMin;

		//Distance from z=0
		float collisionCheckDistance;

		int health_ = 1;
	};
}