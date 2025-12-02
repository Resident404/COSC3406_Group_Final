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
	class Player : public SceneNode {	//Should Entity inherit from SceneNode or 
		//  should the player and enemies inherit from SceneNode AND Entity???

	public:
		Player(const std::string name, const Resource* geometry, const Resource* material);
		~Player();

		glm::vec3 GetForward(void);   //May not need these...
		glm::vec3 GetSide(void) const;
		glm::vec3 GetUp(void) const;

		bool GetCameraViewMode();
		void ToggleCameraViewMode();

		int GetScore();
		void SetScore(int addScore);
		float GetHealth();
		void SetHealth();
		void Attack();	//Should be used to throw bomb power up.

		void SetPowerUpMode(std::string powerUp);//***Needs to be passed an argument to decide which powerUp effect to apply.
												 //   A string should do just fine.

		void SetxMax(float xMaxIn);
		float GetxMax();
		void SetxMin(float xMinIn);
		float GetxMin();

		void SetyMax(float yMaxIn);
		float GetyMax();
		void SetyMin(float yMinIn);
		float GetyMin();
		//Add additional member functions here as needed to increase functionality.

		void Update(double deltaTime); //Signature may need to be changed depending on how movement is implemented.

	private:
		bool cameraViewMode_ = true; //false for 1st person|true for 3rd person //Do we even need this???

		//The Player should only ever move in the x y plane.
		const float accelerationFactor_ = 0.1;			//Acceleration and velocity implementation need to be checked.
		glm::vec3 velocity_ = glm::vec3(0.0, 0.0, 0.0);	//  Unsure if this is the best way to store the variables.

		glm::vec3 forward_;	//Initial forward vector
		glm::vec3 side_;	//Initial side vector

		//AABB info
		float xMax_;
		float xMin_;
		float yMax_;
		float yMin_;

		float health_;	//Number of 'hits' before being caught.
		int score_ = 0;	//Culmulative point total.

	public:
		// Player movement state
		int currentLane_ = 1;  // 0 = left, 1 = center, 2 = right
		float targetX_ = 0.0f;  // Target X position for smooth lane switching
		bool isJumping_ = false;
		float jumpStartTime_ = 0.0f;
		float jumpHeight_ = 1.5f;
		float jumpDuration_ = 0.6f;
	};
}