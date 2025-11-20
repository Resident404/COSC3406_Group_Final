#include <iostream>
#include <time.h>
#include <sstream>

#include "game.h"
#include "bin/path_config.h"

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
glm::vec3 camera_position_g(0.0, 3.5, 7.0); //Default = (0.0, 0.0, 50.0)
glm::vec3 camera_look_at_g(0.0, 0.0, -2.0);
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
}


void Game::SetupScene(void){

    // Set background color for the scene
    scene_.SetBackgroundColor(viewport_background_color_g);

    root_ = CreateInstance("root", "", "");

    // Setup the player capsule bits
    bottomSphere_ = CreateInstance("bottomSphere", "SphereMesh", "ObjectMaterial");
    bottomSphere_->Translate(glm::vec3(0.0, 0.25, 0.0));;
    bottomSphere_->Scale(glm::vec3(0.415, 0.415, 0.415));
    
    middleCylinder_ = CreateInstance("middleCylinder", "CylinderMesh", "ObjectMaterial");
    middleCylinder_->Translate(glm::vec3(0.0,0.25,0.0));

    topSphere_ = CreateInstance("topSphere", "SphereMesh", "ObjectMaterial");
    topSphere_->Translate(glm::vec3(0.0,0.25,0.0));
    topSphere_->Scale(glm::vec3(0.415, 0.415, 0.415));

    SceneNode* laneGuide = CreateInstance("laneGuide", "CubeMesh", "ObjectMaterial");
    laneGuide->Translate(glm::vec3(0.0,0.0,-5.0));
    laneGuide->Scale(glm::vec3(0.9,0.01,10.0));

    SceneNode* laneGuide2 = CreateInstance("laneGuide2", "CubeMesh", "ObjectMaterial");
    laneGuide2->Translate(glm::vec3(0.9, 0.0, -5.0));
    laneGuide2->Scale(glm::vec3(0.9, 0.01, 10.0));

    testObstacle_ = new Obstacle("testObstacle", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    testObstacle_->Translate(glm::vec3(0.0, 0.75, -1.5));
    testObstacle_->Scale(glm::vec3(0.8, 1.2, 3.0));

//    // Create mechanical arm parts
//    arm1_ = CreateInstance("Arm1", "CubeMesh", "ObjectMaterial");
//    arm1_->SetPosition(glm::vec3(2.0, 0.0, 0.0));
//    arm1_->SetScale(glm::vec3(4.0, 1.2, 1.2));
//
//    orbit_arm2_ = CreateInstance("OrbitArm2", "", "");
//    orbit_arm2_->SetOrientation(glm::angleAxis(glm::pi<float>()/4.0f, glm::vec3(0.0, 0.0, 1.0)));
//    orbit_arm2_->SetPosition(glm::vec3(-2.0 + 4.0, 0.0, 0.0));
//
//    arm2_ = CreateInstance("Arm2", "CubeMesh", "ObjectMaterial");
//    arm2_->SetPosition(glm::vec3(2.0, 0.0, 0.0));
//    arm2_->SetScale(glm::vec3(4.0, 1.1, 1.1));
//
//    orbit_claw1_ = CreateInstance("OrbitClaw1", "", "");
//    orbit_claw1_->SetPosition(glm::vec3(-1.5 + 3.0, 0.0, 0.0));
//    orbit_claw1_->SetOrientation(glm::angleAxis(glm::pi<float>()/10.0f, glm::vec3(0.0, 0.0, 1.0)));
//
//    claw1_ = CreateInstance("Claw1", "CubeMesh", "ObjectMaterial");
//    claw1_->SetPosition(glm::vec3(1.5, 0.0, 0.0));
//    claw1_->SetScale(glm::vec3(3.0, 1.0, 1.0));
//
//    orbit_claw2_ = CreateInstance("OrbitClaw2", "", "");
//    orbit_claw2_->SetPosition(glm::vec3(-1.5 + 3.0, 0.0, 0.0));
//    orbit_claw2_->SetOrientation(glm::angleAxis(-glm::pi<float>()/10.0f, glm::vec3(0.0, 0.0, 1.0)));
//
//    claw2_ = CreateInstance("Claw2", "CubeMesh", "ObjectMaterial");
//    claw2_->SetPosition(glm::vec3(1.5, 0.0, 0.0));
//    claw2_->SetScale(glm::vec3(3.0, 1.0, 1.0));

    // Create hierarchy of scene nodes for the mechanical arm parts
    root_->AddChild(laneGuide);
    root_->AddChild(laneGuide2);
    root_->AddChild(testObstacle_);
    root_->AddChild(bottomSphere_);
    bottomSphere_->AddChild(middleCylinder_);
    middleCylinder_->AddChild(topSphere_);
//    arm1_->AddChild(orbit_arm2_);
//    orbit_arm2_->AddChild(arm2_);
//    arm2_->AddChild(orbit_claw1_);
//    arm2_->AddChild(orbit_claw2_);
//    orbit_claw1_->AddChild(claw1_);
//    orbit_claw2_->AddChild(claw2_);

    // Set root of the hierarchy
    scene_.SetRoot(root_);
//    scene_.SetRoot(arm1_); 
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
            if ((current_time - last_time) > 0.1){
                scene_.Update();

//                // Animate mechanical arm
//
//                // Rotate arm1 around x and z axes
//                float ang1 = map_angle(current_time, 4.0, glm::pi<float>()/4.0);
//                float ang2 = map_angle(current_time, 4.0, glm::pi<float>()/4.0);
//                glm::quat rotation1 = glm::angleAxis(ang1, glm::vec3(1.0, 0.0, 0.0));
//                glm::quat rotation2 = glm::angleAxis(ang2, glm::vec3(0.0, 0.0, 1.0));
//                arm1_->SetOrientation(rotation1 * rotation2);
//
//                // Now rotate arm2 by rotating the orbit_arm2 node
//                ang1 = map_angle(current_time, 4.0, glm::pi<float>()/2.0);
//                rotation1 = glm::angleAxis(ang1, glm::vec3(0.0, 0.0, 1.0));
//                orbit_arm2_->SetOrientation(rotation1);
//
//                // Rotate the first claw
//                ang1 = map_angle(current_time, 2.0, glm::pi<float>()/3.0);
//                rotation1 = glm::angleAxis(ang1, glm::vec3(0.0, 0.0, 1.0));
//                orbit_claw1_->SetOrientation(rotation1);
//
//                // Rotate the second claw in the opposite direction
//                rotation2 = glm::angleAxis(-ang1, glm::vec3(0.0, 0.0, 1.0));
//                orbit_claw2_->SetOrientation(rotation2);

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

    // View control
    float rot_factor(glm::pi<float>() / 180);
    float trans_factor = 0.2; //Default = 1.0
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Pitch(rot_factor);
    }
    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Pitch(-rot_factor);
    }
    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Yaw(rot_factor);
    }
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Yaw(-rot_factor);
    }
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Roll(-rot_factor);
    }
    if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Roll(rot_factor);
    }
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Translate(game->camera_.GetForward()*trans_factor);
    }
    if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Translate(-game->camera_.GetForward()*trans_factor);
    }
    if (key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Translate(-game->camera_.GetSide()*trans_factor);
    }
    if (key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Translate(game->camera_.GetSide()*trans_factor);
    }
    if (key == GLFW_KEY_I && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Translate(game->camera_.GetUp()*trans_factor);
    }
    if (key == GLFW_KEY_K && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Translate(-game->camera_.GetUp()*trans_factor);
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

} // namespace game
