#include <iostream>
#include <time.h>
#include <sstream>

#include "game.h"
#include "build/path_config.h"

namespace game {

// Some configuration constants
// They are written here as global variables, but ideally they should be loaded from a configuration file

// Main window settings
const std::string window_title_g = "Demo";
const unsigned int window_width_g = 1200;
const unsigned int window_height_g = 1400;
const bool window_full_screen_g = false;

// Viewport and camera settings
float camera_near_clip_distance_g = 0.01;
float camera_far_clip_distance_g = 1000.0;
float camera_fov_g = 30.0; // Field-of-view of camera (Default = 20)
const glm::vec3 viewport_background_color_g(0.23, 0.38, 0.47); //Default = (0.0, 0.0, 0.0)
glm::vec3 camera_position_g(0.0, 3.0, 7.0); //Default = (0.0, 0.0, 50.0)
glm::vec3 camera_look_at_g(0.0, 0.0, -3.5);
glm::vec3 camera_up_g(0.0, 1.0, 0.0);

// Materials 
const std::string material_directory_g = MATERIAL_DIRECTORY;


Game::Game(void){

    // Don't do work in the constructor, leave it for the Init() function
}


void Game::Init(void){

    // Run all initialization steps
    InitWindow();
    InitView();
    InitEventHandlers();

    // Set variables
    animating_ = true;
}

       
void Game::InitWindow(void){

    // Initialize the window management library (GLFW)
    if (!glfwInit()){
        throw(GameException(std::string("Could not initialize the GLFW library")));
    }

    // Create a window and its OpenGL context
    if (window_full_screen_g){
        window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), glfwGetPrimaryMonitor(), NULL);
    } else {
        window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), NULL, NULL);
    }
    if (!window_){
        glfwTerminate();
        throw(GameException(std::string("Could not create window")));
    }

    // Make the window's context the current one
    glfwMakeContextCurrent(window_);

    // Initialize the GLEW library to access OpenGL extensions
    // Need to do it after initializing an OpenGL context
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK){
        throw(GameException(std::string("Could not initialize the GLEW library: ")+std::string((const char *) glewGetErrorString(err))));
    }
}


void Game::InitView(void){

    // Set up z-buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Set viewport
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    glViewport(0, 0, width, height);

    // Set up camera
    // Set current view
    camera_.SetView(camera_position_g, camera_look_at_g, camera_up_g);
    // Set projection
    camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
}


void Game::InitEventHandlers(void){

    // Set event callbacks
    glfwSetKeyCallback(window_, KeyCallback);
    glfwSetFramebufferSizeCallback(window_, ResizeCallback);

    // Set pointer to game object, so that callbacks can access it
    glfwSetWindowUserPointer(window_, (void *) this);
}


void Game::SetupResources(void){
    //Player Model
    //Enemy Models 1, 2, and 3
    //Level Models

    // Create a cube to represent the parts of the mechanical arm
    resman_.CreateCube("CubeMesh");

    // Create parts to use for capsule shaped model.
    resman_.CreateSphere("SphereMesh");
    resman_.CreateCylindricalGeometry("CylinderMesh");

    // Load material to be applied to mechanical arm
    std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/shiny_blue");
    resman_.LoadResource(Material, "ObjectMaterial", filename.c_str());

    filename = std::string(MATERIAL_DIRECTORY) + std::string("/red_material");
    resman_.LoadResource(Material, "RedMaterial", filename.c_str());
}


void Game::SetupScene(void){
    scene_.SetBackgroundColor(viewport_background_color_g);
    root_ = CreateInstance("root", "", "");

    std::cout << "Creating Subway Surfer game scene..." << std::endl;

    // === 1. CREATE INFINITE GROUND AND LANE DIVIDERS ===
    // Make ground VERY LONG so it appears infinite
    ground_plane_ = CreateInstance("Ground", "CubeMesh", "ObjectMaterial");
    ground_plane_->SetPosition(glm::vec3(0.0, -0.5, -250.0));  // Far ahead
    ground_plane_->SetScale(glm::vec3(3.0, 0.1, 500.0));  // 500 units long!

    // BRIGHT WHITE lane dividers - THICKER and more visible!
    lane_divider_1_ = CreateInstance("LeftDivider", "CubeMesh", "ObjectMaterial");
    lane_divider_1_->SetPosition(glm::vec3(-0.9, -0.4, -250.0));
    lane_divider_1_->SetScale(glm::vec3(0.15, 0.2, 500.0));  // Much thicker and taller!

    lane_divider_2_ = CreateInstance("RightDivider", "CubeMesh", "ObjectMaterial");
    lane_divider_2_->SetPosition(glm::vec3(0.9, -0.4, -250.0));
    lane_divider_2_->SetScale(glm::vec3(0.15, 0.2, 500.0));  // Much thicker and taller!

    SceneNode* playerAABB = CreateInstance("playerAABB", "CubeMesh", "ObjectMaterial");
    playerAABB->SetPosition(glm::vec3(0.0, -0.4, 0.0));
    playerAABB->SetScale(glm::vec3(0.7, 1., 0.3));

    // === 2. CREATE BLUE ROBOT PLAYER ===
    player_root_ = new Player("PlayerRoot", resman_.GetResource("SphereMesh"), resman_.GetResource("ObjectMaterial"));
    player_root_->SetPosition(glm::vec3(0.0, 0.5, 0.0));  // Start in center lane
    player_root_->SetScale(glm::vec3(0.3, 0.3, 0.3));  // Head

    player_root_->SetxMax( 0.35);
    player_root_->SetxMin(-0.35);
    player_root_->SetyMax( 0.18);
    player_root_->SetyMin(-0.50);

    // Body (cylinder)
    player_body_ = CreateInstance("PlayerBody", "CylinderMesh", "ObjectMaterial");
    player_body_->SetPosition(glm::vec3(0.0, -0.4, 0.0));
    player_body_->SetScale(glm::vec3(0.25, 0.6, 0.25));

    // Arms
    player_left_arm_ = CreateInstance("LeftArm", "CylinderMesh", "ObjectMaterial");
    player_left_arm_->SetPosition(glm::vec3(-0.35, -0.2, 0.0));
    player_left_arm_->SetScale(glm::vec3(0.1, 0.4, 0.1));

    player_right_arm_ = CreateInstance("RightArm", "CylinderMesh", "ObjectMaterial");
    player_right_arm_->SetPosition(glm::vec3(0.35, -0.2, 0.0));
    player_right_arm_->SetScale(glm::vec3(0.1, 0.4, 0.1));

    // Legs
    player_left_leg_ = CreateInstance("LeftLeg", "CylinderMesh", "ObjectMaterial");
    player_left_leg_->SetPosition(glm::vec3(-0.15, -0.9, 0.0));
    player_left_leg_->SetScale(glm::vec3(0.12, 0.5, 0.12));

    player_right_leg_ = CreateInstance("RightLeg", "CylinderMesh", "ObjectMaterial");
    player_right_leg_->SetPosition(glm::vec3(0.15, -0.9, 0.0));
    player_right_leg_->SetScale(glm::vec3(0.12, 0.5, 0.12));

    // === 3. CREATE OBSTACLES IN DIFFERENT LANES ===
    // Obstacles start FAR ahead and move TOWARD player (loop infinitely)

    // Obstacle 1: Left lane, full height
    obstacle1_ = new Obstacle("Obstacle1", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    obstacle1_->SetPosition(glm::vec3(-0.9, 0.6, -50.0));
    obstacle1_->SetScale(glm::vec3(0.6, 1.2, 0.6));
    obstacle1_->SetStartPoint(glm::vec3(-0.9, 0.6, -50.0));   // Start far ahead
    obstacle1_->SetEndPoint(glm::vec3(-0.9, 0.6, 50.0));      // End behind player (loops)

    obstacle1_->SetxMax( 0.3);
    obstacle1_->SetxMin(-0.3);
    obstacle1_->SetyMax( 0.6);
    obstacle1_->SetyMin(-0.6);


    // Obstacle 2: Center lane, full height
    obstacle2_ = new Obstacle("Obstacle2", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    obstacle2_->SetPosition(glm::vec3(0.0, 0.6, -80.0));
    obstacle2_->SetScale(glm::vec3(0.6, 1.2, 0.6));
    obstacle2_->SetStartPoint(glm::vec3(0.0, 0.6, -80.0));
    obstacle2_->SetEndPoint(glm::vec3(0.0, 0.6, 50.0));

    obstacle2_->SetxMax( 0.3);
    obstacle2_->SetxMin(-0.3);
    obstacle2_->SetyMax( 0.6);
    obstacle2_->SetyMin(-0.6);


    // Obstacle 3: Right lane, full height
    obstacle3_ = new Obstacle("Obstacle3", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    obstacle3_->SetPosition(glm::vec3(0.9, 0.6, -110.0));
    obstacle3_->SetScale(glm::vec3(0.6, 1.2, 0.6));
    obstacle3_->SetStartPoint(glm::vec3(0.9, 0.6, -110.0));
    obstacle3_->SetEndPoint(glm::vec3(0.9, 0.6, 50.0));

    obstacle3_->SetxMax( 0.3);
    obstacle3_->SetxMin(-0.3);
    obstacle3_->SetyMax( 0.6);
    obstacle3_->SetyMin(-0.6);


    // Obstacle 4: Center lane, HALF height - low jump
    obstacle4_ = new Obstacle("Obstacle4", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    obstacle4_->SetPosition(glm::vec3(0.0, 0.3, -140.0));
    obstacle4_->SetScale(glm::vec3(0.6, 0.6, 0.6));  // Half size!
    obstacle4_->SetStartPoint(glm::vec3(0.0, 0.3, -140.0));
    obstacle4_->SetEndPoint(glm::vec3(0.0, 0.3, 50.0));

    obstacle4_->SetxMax( 0.3);
    obstacle4_->SetxMin(-0.3);
    obstacle4_->SetyMax( 0.3);
    obstacle4_->SetyMin(-0.3);


    // Obstacle 5: Left lane, HALF height - low jump
    obstacle5_ = new Obstacle("Obstacle5", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    obstacle5_->SetPosition(glm::vec3(-0.9, 0.3, -170.0));
    obstacle5_->SetScale(glm::vec3(0.6, 0.6, 0.6));  // Half size!
    obstacle5_->SetStartPoint(glm::vec3(-0.9, 0.3, -170.0));
    obstacle5_->SetEndPoint(glm::vec3(-0.9, 0.3, 50.0));

    obstacle5_->SetxMax( 0.3);
    obstacle5_->SetxMin(-0.3);
    obstacle5_->SetyMax( 0.3);
    obstacle5_->SetyMin(-0.3);


    // Obstacle 6: Right lane, HALF height - low jump
    obstacle6_ = new Obstacle("Obstacle6", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    obstacle6_->SetPosition(glm::vec3(0.9, 0.3, -200.0));
    obstacle6_->SetScale(glm::vec3(0.6, 0.6, 0.6));  // Half size!
    obstacle6_->SetStartPoint(glm::vec3(0.9, 0.3, -200.0));
    obstacle6_->SetEndPoint(glm::vec3(0.9, 0.3, 50.0));

    obstacle6_->SetxMax( 0.3);
    obstacle6_->SetxMin(-0.3);
    obstacle6_->SetyMax( 0.3);
    obstacle6_->SetyMin(-0.3);


    // Obstacle 7: Left lane, full height
    obstacle7_ = new Obstacle("Obstacle7", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    obstacle7_->SetPosition(glm::vec3(-0.9, 0.6, -230.0));
    obstacle7_->SetScale(glm::vec3(0.6, 1.2, 0.6));
    obstacle7_->SetStartPoint(glm::vec3(-0.9, 0.6, -230.0));
    obstacle7_->SetEndPoint(glm::vec3(-0.9, 0.6, 50.0));

    obstacle7_->SetxMax( 0.3);
    obstacle7_->SetxMin(-0.3);
    obstacle7_->SetyMax( 0.6);
    obstacle7_->SetyMin(-0.6);


    // Obstacle 8: Center lane, full height
    obstacle8_ = new Obstacle("Obstacle8", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    obstacle8_->SetPosition(glm::vec3(0.0, 0.6, -260.0));
    obstacle8_->SetScale(glm::vec3(0.6, 1.2, 0.6));
    obstacle8_->SetStartPoint(glm::vec3(0.0, 0.6, -260.0));
    obstacle8_->SetEndPoint(glm::vec3(0.0, 0.6, 50.0));

    obstacle8_->SetxMax( 0.3);
    obstacle8_->SetxMin(-0.3);
    obstacle8_->SetyMax( 0.6);
    obstacle8_->SetyMin(-0.6);


    // Obstacle 9: Right lane, full height
    obstacle9_ = new Obstacle("Obstacle9", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    obstacle9_->SetPosition(glm::vec3(0.9, 0.6, -290.0));
    obstacle9_->SetScale(glm::vec3(0.6, 1.2, 0.6));
    obstacle9_->SetStartPoint(glm::vec3(0.9, 0.6, -290.0));
    obstacle9_->SetEndPoint(glm::vec3(0.9, 0.6, 50.0));

    obstacle9_->SetxMax( 0.3);
    obstacle9_->SetxMin(-0.3);
    obstacle9_->SetyMax( 0.6);
    obstacle9_->SetyMin(-0.6);


    // Obstacle 10: Center lane, HALF height - low jump
    obstacle10_ = new Obstacle("Obstacle10", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    obstacle10_->SetPosition(glm::vec3(0.0, 0.3, -320.0));
    obstacle10_->SetScale(glm::vec3(0.6, 0.6, 0.6));  // Half size!
    obstacle10_->SetStartPoint(glm::vec3(0.0, 0.3, -320.0));
    obstacle10_->SetEndPoint(glm::vec3(0.0, 0.3, 50.0));

    obstacle10_->SetxMax( 0.3);
    obstacle10_->SetxMin(-0.3);
    obstacle10_->SetyMax( 0.3);
    obstacle10_->SetyMin(-0.3);


    // === 4. BUILD SCENE HIERARCHY ===
    root_->AddChild(ground_plane_);
    root_->AddChild(lane_divider_1_);
    root_->AddChild(lane_divider_2_);

    root_->AddChild(player_root_);
    //player_root_->AddChild(playerAABB);
    player_root_->AddChild(player_body_);
    player_root_->AddChild(player_left_arm_);
    player_root_->AddChild(player_right_arm_);
    player_root_->AddChild(player_left_leg_);
    player_root_->AddChild(player_right_leg_);

    root_->AddChild(obstacle1_);
    root_->AddChild(obstacle2_);
    root_->AddChild(obstacle3_);
    root_->AddChild(obstacle4_);
    root_->AddChild(obstacle5_);
    root_->AddChild(obstacle6_);
    root_->AddChild(obstacle7_);
    root_->AddChild(obstacle8_);
    root_->AddChild(obstacle9_);
    root_->AddChild(obstacle10_);

    scene_.SetRoot(root_);

    std::cout << "Scene created: Blue robot player, 3 lanes with dividers, 10 obstacles - CONTINUOUS FLOW!" << std::endl;
}


// Auxiliary function that maps a time value in a given cycle to an angle
float map_angle(float current_time, float cycle_length, float max_angle){

    float rem = current_time - cycle_length*floor(current_time / cycle_length);
    float ang = (rem/cycle_length)*glm::pi<float>();
    ang = sin(ang)*max_angle;
    return ang;
}


void Game::MainLoop(void){

    // Loop while the user did not close the window
    while (!glfwWindowShouldClose(window_)){
        // Animate the scene
        if (animating_){
            static double last_time = 0;
            double current_time = glfwGetTime();
            float deltaTime = current_time - last_time;
            if (deltaTime > 0.01){
                // Update player movement and jumping
                if (player_root_) {
                    player_root_->Update(deltaTime);

                    // CAMERA FOLLOWS PLAYER - Subway Surfer style!
                    glm::vec3 playerPos = player_root_->GetPosition();
                    glm::vec3 cameraPos = playerPos + glm::vec3(0.0, 3.0, 7.0);  // Behind and above player
                    glm::vec3 cameraLookAt = playerPos + glm::vec3(0.0, 0.0, -3.5);  // Look slightly ahead
                    camera_.SetView(cameraPos, cameraLookAt, camera_up_g);

                    // INFINITE GROUND - Ground and lane dividers follow player!
                    // Keep ground centered on player's Z position
                    float playerZ = playerPos.z;
                    ground_plane_->SetPosition(glm::vec3(0.0, -0.5, playerZ - 250.0));
                    lane_divider_1_->SetPosition(glm::vec3(-0.9, -0.4, playerZ - 250.0));
                    lane_divider_2_->SetPosition(glm::vec3(0.9, -0.4, playerZ - 250.0));
                }

                // Update scene (obstacles moving)
                scene_.Update();

                // INFINITE OBSTACLES - Respawn obstacles ahead when they go behind player!
                if (player_root_) {
                    float playerZ = player_root_->GetPosition().z;
                    float respawnDistance = 100.0f;  // Respawn 100 units ahead
                    float despawnThreshold = 20.0f;  // Despawn when 20 units behind

                    // Check each obstacle and respawn if needed
                    Obstacle* obstacles[] = {obstacle1_, obstacle2_, obstacle3_, obstacle4_, obstacle5_,
                                            obstacle6_, obstacle7_, obstacle8_, obstacle9_, obstacle10_};
                    float lanePositions[] = {-0.9f, 0.0f, 0.9f};  // Left, Center, Right

                    for (int i = 0; i < 10; i++) {
                        if (obstacles[i]) {
                            float obstacleZ = obstacles[i]->GetPosition().z;

                            // if (obstacleZ - playerZ <= 0) or somthin
                            //    aabb collision check
                            //std::cout << "playerZ = " << playerZ << std::endl;
                            //std::cout << "obstacleZ = " << obstacleZ << std::endl;
                            //std::cout << "obstacleZ - playerZ = " << obstacleZ - playerZ << std::endl;

                            if (playerZ > obstacleZ && obstacleZ > playerZ - 0.5) {
                                std::cout << "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\nTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\nTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n";
                                if (AABBcheck(player_root_, obstacles[i])) {
                                    player_root_->SetGeometry(resman_.GetResource("SphereMesh"));
                                    player_root_->SetShader(resman_.GetResource("RedMaterial"));
                                    animating_ = false;
                                    std::cout << "GAME OVER\nYour final score is: " << player_root_->GetScore() << std::endl;
                                    //std::cout << "bonk - bonk - bonk - bonk - bonk - bonk - bonk - bonk - bonk - bonk\nbonk - bonk - bonk - bonk - bonk - bonk - bonk - bonk - bonk - bonk\nbonk - bonk - bonk - bonk - bonk - bonk - bonk - bonk - bonk - bonk\n";
                                }
                            }

                            // If obstacle has gone behind player, respawn it ahead
                            if (obstacleZ > playerZ + despawnThreshold) {
                                if (obstacles[i] == obstacles[0]) {
                                    //std::cout << "obstacle1_ just despawned! obstacle1_ just despawned! obstacle1_ just despawned! obstacle1_ just despawned!\n";
                                }

                                // Randomly assign to a lane
                                int randomLane = rand() % 3;
                                float x = lanePositions[randomLane];

                                // Randomly choose full height or half height
                                bool fullHeight = (rand() % 2 == 0);
                                float y = fullHeight ? 0.6f : 0.3f;
                                float scaleY = fullHeight ? 1.2f : 0.6f;

                                // Respawn ahead of player
                                float newZ = playerZ - respawnDistance - (rand() % 50);
                                obstacles[i]->SetPosition(glm::vec3(x, obstacles[i]->GetPosition()[1], newZ));
                                //obstacles[i]->SetScale(glm::vec3(0.6f, scaleY, 0.6f)); //Suspecting this will cause frustration with setting up AABBs
                                obstacles[i]->SetStartPoint(glm::vec3(x, obstacles[i]->GetPosition()[1], newZ));
                                obstacles[i]->SetEndPoint(glm::vec3(x, obstacles[i]->GetPosition()[1], playerZ + 50.0f));
                            }
                        }
                    }
                }

                // Update timer
                last_time = current_time;
            }
        }

        // Draw the scene
        scene_.Draw(&camera_);

        // Push buffer drawn in the background onto the display
        glfwSwapBuffers(window_);

        // Update other events like input handling
        glfwPollEvents();
    }
}


void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){

    // Get user data with a pointer to the game class
    void* ptr = glfwGetWindowUserPointer(window);
    Game *game = (Game *) ptr;

    // Quit game if 'q' is pressed
    if (key == GLFW_KEY_Q && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    // Stop animation if space bar is pressed
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        game->animating_ = (game->animating_ == true) ? false : true;
    }

    // === PLAYER CONTROLS ===
    if (game->player_root_) {
        // LEFT ARROW or A - Move to left lane
        if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_A) && action == GLFW_PRESS){
            if (game->player_root_->currentLane_ > 0){
                game->player_root_->currentLane_--;
                std::cout << "Moving to LEFT lane " << game->player_root_->currentLane_ << std::endl;
            }
        }

        // RIGHT ARROW or D - Move to right lane
        if ((key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) && action == GLFW_PRESS){
            if (game->player_root_->currentLane_ < 2){
                game->player_root_->currentLane_++;
                std::cout << "Moving to RIGHT lane " << game->player_root_->currentLane_ << std::endl;
            }
        }

        // UP ARROW or W - Jump
        if ((key == GLFW_KEY_UP || key == GLFW_KEY_W) && action == GLFW_PRESS){
            if (!game->player_root_->isJumping_){
                game->player_root_->isJumping_ = true;
                game->player_root_->jumpStartTime_ = glfwGetTime();
                std::cout << "JUMP!" << std::endl;
            }
        }
    }
}


void Game::ResizeCallback(GLFWwindow* window, int width, int height){

    // Set up viewport and camera projection based on new window size
    glViewport(0, 0, width, height);
    void* ptr = glfwGetWindowUserPointer(window);
    Game *game = (Game *) ptr;
    game->camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
}


Game::~Game(){
    
    glfwTerminate();
}


Asteroid *Game::CreateAsteroidInstance(std::string entity_name, std::string object_name, std::string material_name){

    // Get resources
    Resource *geom = resman_.GetResource(object_name);
    if (!geom){
        throw(GameException(std::string("Could not find resource \"")+object_name+std::string("\"")));
    }

    Resource *mat = resman_.GetResource(material_name);
    if (!mat){
        throw(GameException(std::string("Could not find resource \"")+material_name+std::string("\"")));
    }

    // Create asteroid instance
    Asteroid *ast = new Asteroid(entity_name, geom, mat);
    //scene_.AddNode(ast);
    return ast;
}


void Game::CreateAsteroidField(int num_asteroids){

    // Create a number of asteroid instances
    for (int i = 0; i < num_asteroids; i++){
        // Create instance name
        std::stringstream ss;
        ss << i;
        std::string index = ss.str();
        std::string name = "AsteroidInstance" + index;

        // Create asteroid instance
        Asteroid *ast = CreateAsteroidInstance(name, "SimpleSphereMesh", "ObjectMaterial");

        // Set attributes of asteroid: random position, orientation, and
        // angular momentum
        ast->SetPosition(glm::vec3(-300.0 + 600.0*((float) rand() / RAND_MAX), -300.0 + 600.0*((float) rand() / RAND_MAX), 600.0*((float) rand() / RAND_MAX)));
        ast->SetOrientation(glm::normalize(glm::angleAxis(glm::pi<float>()*((float) rand() / RAND_MAX), glm::vec3(((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX)))));
        ast->SetAngM(glm::normalize(glm::angleAxis(0.05f*glm::pi<float>()*((float) rand() / RAND_MAX), glm::vec3(((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX)))));
    }
}


SceneNode *Game::CreateInstance(std::string entity_name, std::string object_name, std::string material_name){

    // Get resources
    Resource *geom ;
    if (object_name != std::string("")){
        geom = resman_.GetResource(object_name);
        if (!geom){
            throw(GameException(std::string("Could not find resource \"")+object_name+std::string("\"")));
        }
    } else {
        geom = NULL;
    }

    Resource *mat;
    if (material_name != std::string("")){
        mat = resman_.GetResource(material_name);
        if (!mat){
            throw(GameException(std::string("Could not find resource \"")+material_name+std::string("\"")));
        }
    } else {
        mat = NULL;
    }

    // Create instance
    SceneNode *node = new SceneNode(entity_name, geom, mat);
    return node;
}

bool Game::AABBcheck(Player* player, Obstacle* obstacle) {
    //std::cout << "Player xMax = " << player->GetPosition()[0] + player->GetxMax() << std::endl;
    //std::cout << "Player xMin = " << player->GetPosition()[0] + player->GetxMin() << std::endl;
    //std::cout << "Player yMax = " << player->GetPosition()[1] + player->GetyMax() << std::endl;
    //std::cout << "Player yMin = " << player->GetPosition()[1] + player->GetyMin() << std::endl;
    //
    //std::cout << "Obstacle xMax = " << obstacle->GetPosition()[0] + obstacle->GetxMax() << std::endl;
    //std::cout << "Obstacle xMin = " << obstacle->GetPosition()[0] + obstacle->GetxMin() << std::endl;
    //std::cout << "Obstacle yMax = " << obstacle->GetPosition()[1] + obstacle->GetyMax() << std::endl;
    //std::cout << "Obstacle yMin = " << obstacle->GetPosition()[1] + obstacle->GetyMin() << std::endl;
    //
    //std::cout << "------------------------" << std::endl;
    //std::cout << "Obstacle xPos = " << obstacle->GetPosition()[0] << std::endl;
    //std::cout << "Obstacle yPos = " << obstacle->GetPosition()[1] << std::endl;
    //std::cout << "------------------------" << std::endl;

    return player->GetPosition().x + player->GetxMax() > obstacle->GetPosition().x + obstacle->GetxMin() &&
           player->GetPosition().x + player->GetxMin() < obstacle->GetPosition().x + obstacle->GetxMax() &&
           player->GetPosition().y + player->GetyMax() > obstacle->GetPosition().y + obstacle->GetyMin() &&
           player->GetPosition().y + player->GetyMin() < obstacle->GetPosition().y + obstacle->GetyMax();

        /*
        a.max.x > b.min.x&&
        a.min.x < b.max.x&&
        a.max.y > b.min.y&&
        a.min.y < b.max.y&&
        a.max.z > b.min.z&&
        a.min.z < b.max.z
        */
}

} // namespace game
