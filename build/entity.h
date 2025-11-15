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
	class Entity : public SceneNode {	//Should Entity inherit from SceneNode or 
										//  should the player and enemies inherit from SceneNode AND Entity???

	public:
		Entity(); //How do parent class constructors get called?
		~Entity();

		glm::vec3 GetForward(void);   //Could probably take these from my assignment 2.
		glm::vec3 GetSide(void) const;
		glm::vec3 GetUp(void) const;

		void GetScore();
		int SetScore();
		void GetHealth();
		float SetHealth();
		//Attack(); //Maybe since each entity will have its own attack(s) this should be an overloaded virtual funtion.

		virtual void Update(double deltaTime); //I think this is a good place to use a virtual function.
											   //  Just need to be reminded how that keyword works.

	private:
		bool cameraViewMode_ = true; //false for 1st person|true for 3rd person //Do we even need this???
		
		const float accelerationFactor_ = 0.1;			//Acceleration and velocity implementation need to be checked.
		glm::vec3 velocity_ = glm::vec3(0.0, 0.0, 0.0);	//  Unsure if this is the best way to store the variables.
		
		glm::vec3 forward_;	//Initial forward vector
		glm::vec3 side_;	//Initial side vector

		float health_;
		int score_;
	};
}