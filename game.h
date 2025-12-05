#ifndef GAME_H_
#define GAME_H_

#include <exception>
#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "scene_graph.h"
#include "resource_manager.h"
#include "camera.h"
#include "asteroid.h"
#include "build/player.h"
#include "build/obstacle.h"

namespace game {

    // Exception type for the game
    class GameException: public std::exception
    {
        private:
            std::string message_;
        public:
            GameException(std::string message) : message_(message) {};
            virtual const char* what() const throw() { return message_.c_str(); };
            virtual ~GameException() throw() {};
    };

    // Game application
    class Game {

        public:
            // Constructor and destructor
            Game(void);
            ~Game();
            // Call Init() before calling any other method
            void Init(void); 
            // Set up resources for the game
            void SetupResources(void);
            // Set up initial scene
            void SetupScene(void);
            // Run the game: keep the application active
            void MainLoop(void); 

        private:
            // GLFW window
            GLFWwindow* window_;

            // Scene graph containing all nodes to render
            SceneGraph scene_;

            // Resources available to the game
            ResourceManager resman_;

            // Camera abstraction
            Camera camera_;

            // Root of scene hierarchy
            SceneNode* root_;

            // Flag to turn animation on/off
            bool animating_;

            // Player - Blue Robot
            Player *player_root_;
            SceneNode *player_body_, *player_head_;
            SceneNode *player_left_arm_, *player_right_arm_;
            SceneNode *player_left_leg_, *player_right_leg_;

            // Track/Lane markers
            SceneNode *ground_plane_;
            SceneNode *lane_divider_1_, *lane_divider_2_;

            Obstacle* obstacle1_, *obstacle2_, *obstacle3_, *obstacle4_, *obstacle5_, *obstacle6_, *obstacle7_, *obstacle8_, *obstacle9_, *obstacle10_;

            Obstacle *coin1_, *coin2_, *coin3_, *coin4_, *coin5_;

            Obstacle *treeTrunk1_, *treeTrunk2_, *treeTrunk3_, *treeTrunk4_, *treeTrunk5_, *treeTrunk6_, *treeTrunk7_, *treeTrunk8_, *treeTrunk9_, *treeTrunk10_;
            SceneNode *treeTop1_, *treeTop2_, *treeTop3_, *treeTop4_, *treeTop5_, *treeTop6_, *treeTop7_, *treeTop8_, *treeTop9_, *treeTop10_;

            // Mechanical arm
            SceneNode *arm1_, *arm2_, *claw1_, *claw2_, *orbit_arm2_, *orbit_claw1_, *orbit_claw2_;

            // Methods to initialize the game
            void InitWindow(void);
            void InitView(void);
            void InitEventHandlers(void);
 
            // Methods to handle events
            static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void ResizeCallback(GLFWwindow* window, int width, int height);

            // Asteroid field
            // Create instance of one asteroid
            Asteroid *CreateAsteroidInstance(std::string entity_name, std::string object_name, std::string material_name);
            // Create entire random asteroid field
            void CreateAsteroidField(int num_asteroids = 1500);

            // Create an instance of an object
            SceneNode *CreateInstance(std::string entity_name, std::string object_name, std::string material_name);

            bool AABBcheck(Player* player, Obstacle* obstacle);

    }; // class Game

} // namespace game

#endif // GAME_H_
