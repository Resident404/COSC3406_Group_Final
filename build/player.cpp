#include "player.h"
#include <iostream>

namespace game {

	Player::Player(const std::string name, const Resource* geometry, const Resource* material) : SceneNode(name, geometry, material) {
		//May not even need to set up forward_ and side_
		//Set up initial forward and side vectors.
		forward_ = glm::vec3(0, 0, -1) * glm::angleAxis(glm::pi<float>() / 2.0f, glm::vec3(-1.0, 0.0, 0.0));
		forward_ = -glm::normalize(forward_);
		side_ = glm::cross(glm::vec3(0, 1, 0), forward_);
		side_ = glm::normalize(side_);
	}
	Player::~Player() {};

	bool Player::GetCameraViewMode() {
		return cameraViewMode_;
	}

	void Player::ToggleCameraViewMode() {
		cameraViewMode_ = (cameraViewMode_ == true) ? false : true; //false for 1st person|true for 3rd person //Do we even need this???
		//std::cout << "\n\nattempted to toggle cameraViewMode_\n\n";
		ToggleShouldDraw();
	}


	//Following three Get______ functions are taken from existing camera functions. //May not even need these depending on how movement is implemented.
	glm::vec3 Player::GetForward(void) {

		glm::vec3 current_forward = this->GetOrientation() * forward_;
		return -current_forward; // Return -forward since the camera coordinate system points in the opposite direction
	}

	glm::vec3 Player::GetSide(void) const {

		glm::vec3 current_side = this->GetOrientation() * side_;
		return current_side;
	}

	glm::vec3 Player::GetUp(void) const {

		glm::vec3 current_forward = this->GetOrientation() * forward_;
		glm::vec3 current_side = this->GetOrientation() * side_;
		glm::vec3 current_up = glm::cross(current_forward, current_side);
		current_up = glm::normalize(current_up);

		return current_up;
	}


	void Player::Update(double deltaTime) {
		// Lane positions: Left = -0.9, Center = 0.0, Right = 0.9
		float lanePositions[3] = {-0.9f, 0.0f, 0.9f};
		targetX_ = lanePositions[currentLane_];

		// Get current position
		glm::vec3 currentPos = GetPosition();

		// Smooth lane switching (interpolate X position)
		float newX = currentPos.x + (targetX_ - currentPos.x) * 8.0f * deltaTime;

		// AUTOMATIC FORWARD MOVEMENT - Player runs forward FAST!
		float forwardSpeed = 15.0f;  // Running speed (increased from 5 to 15!)
		float newZ = currentPos.z - forwardSpeed * deltaTime;  // Move in negative Z direction

		// Jump physics
		float newY = 0.5f;  // Default standing height (raised from 0.25 to 0.5)
		if (isJumping_) {
			double currentTime = glfwGetTime();
			float timeSinceJump = currentTime - jumpStartTime_;

			if (timeSinceJump < jumpDuration_) {
				// Parabolic jump arc using sine wave
				float t = timeSinceJump / jumpDuration_;
				newY = 0.5f + jumpHeight_ * sin(t * glm::pi<float>());
			} else {
				// Landing
				isJumping_ = false;
				newY = 0.5f;
			}
		}

		// Update player position
		SetPosition(glm::vec3(newX, newY, newZ));
	}
}