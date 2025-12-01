#include <iostream>
#include <time.h>
#include <sstream>
#include <cmath>

#include "game.h"
#include "path_config.h"

namespace game {

// Some configuration constants
// They are written here as global variables, but ideally they should be loaded from a configuration file

// Main window settings
const std::string window_title_g = "Subway Surfers Style Runner";
const unsigned int window_width_g = 1200;
const unsigned int window_height_g = 1400;
const bool window_full_screen_g = false;

// Viewport and camera settings
float camera_near_clip_distance_g = 0.01;
float camera_far_clip_distance_g = 1000.0;
float camera_fov_g = 45.0; // Increased FOV for better view like Subway Surfers
const glm::vec3 viewport_background_color_g(0.53, 0.81, 0.92); // Sky blue background
glm::vec3 camera_position_g(0.0, 2.5, 5.0); // Behind and above player
glm::vec3 camera_look_at_g(0.0, 1.0, -2.0); // Looking forward at player
glm::vec3 camera_up_g(0.0, 1.0, 0.0);

// Materials 
const std::string material_directory_g = MATERIAL_DIRECTORY;


Game::Game(void){
    // Initialize game state variables
    current_lane_ = 1;  // Start in middle lane
    target_x_position_ = 0.0f;
    player_z_position_ = 0.0f;
    player_y_position_ = 0.6f;  // Standing height
    player_velocity_y_ = 0.0f;
    is_jumping_ = false;
    is_sliding_ = false;
    jump_start_time_ = 0.0f;
    slide_start_time_ = 0.0f;
    run_animation_time_ = 0.0f;
    
    // Game constants
    lane_width_ = 1.8f;  // Distance between lanes
    forward_speed_ = 8.0f;  // Running speed
    jump_height_ = 2.0f;  // Max jump height
    jump_duration_ = 0.6f;  // How long jump lasts
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

    // Set up camera - Subway Surfers style (behind and above player)
    camera_.SetView(camera_position_g, camera_look_at_g, camera_up_g);
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
    // Create geometric primitives for the robot player and environment
    resman_.CreateCube("CubeMesh");
    resman_.CreateSphere("SphereMesh");
    resman_.CreateCylindricalGeometry("CylinderMesh");

    // Load material
    std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/shiny_blue");
    resman_.LoadResource(Material, "ObjectMaterial", filename.c_str());
}


void Game::SetupRobotPlayer(void){
    // Create hierarchical robot player character
    // This creates a simple robot with body, head, arms, and legs
    
    // Player root node (for easy movement of entire character)
    player_root_ = CreateInstance("PlayerRoot", "", "");
    player_root_->SetPosition(glm::vec3(0.0f, player_y_position_, 0.0f));
    
    // Body (torso) - main cube
    player_body_ = CreateInstance("PlayerBody", "CubeMesh", "ObjectMaterial");
    player_body_->SetScale(glm::vec3(0.4f, 0.6f, 0.3f));  // Width, Height, Depth
    player_body_->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Head - sphere on top of body
    player_head_ = CreateInstance("PlayerHead", "SphereMesh", "ObjectMaterial");
    player_head_->SetScale(glm::vec3(0.25f, 0.25f, 0.25f));
    player_head_->SetPosition(glm::vec3(0.0f, 0.5f, 0.0f));  // On top of body
    
    // Left Arm (upper arm) - attached to body
    player_left_arm_ = CreateInstance("PlayerLeftArm", "CubeMesh", "ObjectMaterial");
    player_left_arm_->SetScale(glm::vec3(0.12f, 0.35f, 0.12f));
    player_left_arm_->SetPosition(glm::vec3(-0.3f, 0.15f, 0.0f));  // Left side of body
    
    // Right Arm (upper arm) - attached to body  
    player_right_arm_ = CreateInstance("PlayerRightArm", "CubeMesh", "ObjectMaterial");
    player_right_arm_->SetScale(glm::vec3(0.12f, 0.35f, 0.12f));
    player_right_arm_->SetPosition(glm::vec3(0.3f, 0.15f, 0.0f));  // Right side of body
    
    // Left Forearm - child of left arm
    player_left_forearm_ = CreateInstance("PlayerLeftForearm", "CubeMesh", "ObjectMaterial");
    player_left_forearm_->SetScale(glm::vec3(0.1f, 0.3f, 0.1f));
    player_left_forearm_->SetPosition(glm::vec3(0.0f, -0.35f, 0.0f));  // Below upper arm
    
    // Right Forearm - child of right arm
    player_right_forearm_ = CreateInstance("PlayerRightForearm", "CubeMesh", "ObjectMaterial");
    player_right_forearm_->SetScale(glm::vec3(0.1f, 0.3f, 0.1f));
    player_right_forearm_->SetPosition(glm::vec3(0.0f, -0.35f, 0.0f));  // Below upper arm
    
    // Left Leg (upper leg) - attached to body
    player_left_leg_ = CreateInstance("PlayerLeftLeg", "CubeMesh", "ObjectMaterial");
    player_left_leg_->SetScale(glm::vec3(0.15f, 0.4f, 0.15f));
    player_left_leg_->SetPosition(glm::vec3(-0.15f, -0.5f, 0.0f));  // Below body, left side
    
    // Right Leg (upper leg) - attached to body
    player_right_leg_ = CreateInstance("PlayerRightLeg", "CubeMesh", "ObjectMaterial");
    player_right_leg_->SetScale(glm::vec3(0.15f, 0.4f, 0.15f));
    player_right_leg_->SetPosition(glm::vec3(0.15f, -0.5f, 0.0f));  // Below body, right side
    
    // Left Foot - child of left leg
    player_left_foot_ = CreateInstance("PlayerLeftFoot", "CubeMesh", "ObjectMaterial");
    player_left_foot_->SetScale(glm::vec3(0.14f, 0.35f, 0.14f));
    player_left_foot_->SetPosition(glm::vec3(0.0f, -0.4f, 0.0f));
    
    // Right Foot - child of right leg
    player_right_foot_ = CreateInstance("PlayerRightFoot", "CubeMesh", "ObjectMaterial");
    player_right_foot_->SetScale(glm::vec3(0.14f, 0.35f, 0.14f));
    player_right_foot_->SetPosition(glm::vec3(0.0f, -0.4f, 0.0f));
    
    // Build hierarchy (parent-child relationships)
    // This is CRITICAL for hierarchical movement!
    player_root_->AddChild(player_body_);
    player_body_->AddChild(player_head_);
    player_body_->AddChild(player_left_arm_);
    player_body_->AddChild(player_right_arm_);
    player_body_->AddChild(player_left_leg_);
    player_body_->AddChild(player_right_leg_);
    
    player_left_arm_->AddChild(player_left_forearm_);
    player_right_arm_->AddChild(player_right_forearm_);
    player_left_leg_->AddChild(player_left_foot_);
    player_right_leg_->AddChild(player_right_foot_);
}


void Game::SetupScene(void){
    // Set background color for the scene
    scene_.SetBackgroundColor(viewport_background_color_g);

    root_ = CreateInstance("root", "", "");

    // Setup the hierarchical robot player
    SetupRobotPlayer();
    root_->AddChild(player_root_);

    // Create three-lane track system
    lane_width_ = 1.8f;
    
    // Ground plane - long runway
    ground_plane_ = CreateInstance("GroundPlane", "CubeMesh", "ObjectMaterial");
    ground_plane_->SetPosition(glm::vec3(0.0f, -0.1f, -20.0f));
    ground_plane_->SetScale(glm::vec3(6.0f, 0.1f, 50.0f));  // Wide, thin, long
    
    // Left lane divider
    lane_divider_1_ = CreateInstance("LaneDivider1", "CubeMesh", "ObjectMaterial");
    lane_divider_1_->SetPosition(glm::vec3(-lane_width_, 0.05f, -20.0f));
    lane_divider_1_->SetScale(glm::vec3(0.1f, 0.02f, 50.0f));
    
    // Right lane divider
    lane_divider_2_ = CreateInstance("LaneDivider2", "CubeMesh", "ObjectMaterial");
    lane_divider_2_->SetPosition(glm::vec3(lane_width_, 0.05f, -20.0f));
    lane_divider_2_->SetScale(glm::vec3(0.1f, 0.02f, 50.0f));

    // Create obstacles (3 types as specified)
    
    // 1. BIG obstacle (existing one - full height, player must dodge)
    testObstacle_ = new Obstacle("BigObstacle", resman_.GetResource("CubeMesh"), resman_.GetResource("ObjectMaterial"));
    testObstacle_->SetPosition(glm::vec3(0.0f, 0.75f, -8.0f));
    testObstacle_->SetScale(glm::vec3(0.8f, 1.5f, 0.8f));  // This is your biggest obstacle size
    
    // 2. MEDIUM obstacle (half height of big one)
    obstacle_medium_ = CreateInstance("MediumObstacle", "CubeMesh", "ObjectMaterial");
    obstacle_medium_->SetPosition(glm::vec3(lane_width_, 0.375f, -15.0f));  // Right lane
    obstacle_medium_->SetScale(glm::vec3(0.8f, 0.75f, 0.8f));  // Half height of big obstacle
    
    // 3. OVERHEAD obstacle (player must slide/glide under it - like Subway Surfer pole)
    obstacle_overhead_ = CreateInstance("OverheadObstacle", "CubeMesh", "ObjectMaterial");
    obstacle_overhead_->SetPosition(glm::vec3(-lane_width_, 1.2f, -12.0f));  // Left lane, high up
    obstacle_overhead_->SetScale(glm::vec3(1.5f, 0.3f, 0.8f));  // Wide horizontal bar

    // Create environment objects (trees, buildings, tunnels)
    
    // Trees - simple cylinders
    tree1_ = CreateInstance("Tree1", "CylinderMesh", "ObjectMaterial");
    tree1_->SetPosition(glm::vec3(-4.0f, 0.5f, -10.0f));  // Left side of track
    tree1_->SetScale(glm::vec3(0.3f, 1.0f, 0.3f));
    
    tree2_ = CreateInstance("Tree2", "CylinderMesh", "ObjectMaterial");
    tree2_->SetPosition(glm::vec3(4.0f, 0.5f, -18.0f));  // Right side of track
    tree2_->SetScale(glm::vec3(0.3f, 1.0f, 0.3f));
    
    // Buildings - tall cubes on sides of track
    building1_ = CreateInstance("Building1", "CubeMesh", "ObjectMaterial");
    building1_->SetPosition(glm::vec3(-5.5f, 2.0f, -5.0f));
    building1_->SetScale(glm::vec3(1.5f, 4.0f, 1.5f));
    
    building2_ = CreateInstance("Building2", "CubeMesh", "ObjectMaterial");
    building2_->SetPosition(glm::vec3(5.5f, 2.0f, -25.0f));
    building2_->SetScale(glm::vec3(1.5f, 4.0f, 1.5f));
    
    // Tunnel - large hollow cube player runs through
    tunnel_ = CreateInstance("Tunnel", "CubeMesh", "ObjectMaterial");
    tunnel_->SetPosition(glm::vec3(0.0f, 2.0f, -35.0f));
    tunnel_->SetScale(glm::vec3(6.0f, 3.0f, 5.0f));

    // Add everything to the scene hierarchy
    root_->AddChild(ground_plane_);
    root_->AddChild(lane_divider_1_);
    root_->AddChild(lane_divider_2_);
    root_->AddChild(testObstacle_);
    root_->AddChild(obstacle_medium_);
    root_->AddChild(obstacle_overhead_);
    root_->AddChild(tree1_);
    root_->AddChild(tree2_);
    root_->AddChild(building1_);
    root_->AddChild(building2_);
    root_->AddChild(tunnel_);

    // Keep old capsule parts for reference (but don't add to scene)
    bottomSphere_ = CreateInstance("bottomSphere", "SphereMesh", "ObjectMaterial");
    bottomSphere_->SetPosition(glm::vec3(10.0f, 0.25f, 0.0f));  // Moved far away
    bottomSphere_->SetScale(glm::vec3(0.415f, 0.415f, 0.415f));
    
    middleCylinder_ = CreateInstance("middleCylinder", "CylinderMesh", "ObjectMaterial");
    middleCylinder_->SetPosition(glm::vec3(0.0f, 0.25f, 0.0f));

    topSphere_ = CreateInstance("topSphere", "SphereMesh", "ObjectMaterial");
    topSphere_->SetPosition(glm::vec3(0.0f, 0.25f, 0.0f));
    topSphere_->SetScale(glm::vec3(0.415f, 0.415f, 0.415f));
    
    bottomSphere_->AddChild(middleCylinder_);
    middleCylinder_->AddChild(topSphere_);

    // Set root of the hierarchy
    scene_.SetRoot(root_);
}


void Game::UpdatePlayer(float delta_time){
    // Update player position based on current lane
    float lane_positions[3] = {-lane_width_, 0.0f, lane_width_};  // Left, Middle, Right
    target_x_position_ = lane_positions[current_lane_];
    
    // Smooth lane switching (interpolate to target position)
    glm::vec3 current_pos = player_root_->GetPosition();
    float current_x = current_pos.x;
    float new_x = current_x + (target_x_position_ - current_x) * 10.0f * delta_time;  // Smooth transition
    
    // Update jump physics
    if (is_jumping_){
        float time_since_jump = glfwGetTime() - jump_start_time_;
        
        if (time_since_jump < jump_duration_){
            // Parabolic jump arc using sine wave
            float t = time_since_jump / jump_duration_;
            player_y_position_ = 0.6f + jump_height_ * sin(t * glm::pi<float>());
        } else {
            // Land
            is_jumping_ = false;
            player_y_position_ = 0.6f;
        }
    }
    
    // Update slide state
    if (is_sliding_){
        float time_since_slide = glfwGetTime() - slide_start_time_;
        
        if (time_since_slide < 0.5f){  // Slide duration 0.5 seconds
            player_y_position_ = 0.3f;  // Lower position when sliding
            
            // Rotate body forward slightly when sliding (tilt forward)
            player_body_->SetOrientation(glm::angleAxis(glm::pi<float>() / 6.0f, glm::vec3(1.0f, 0.0f, 0.0f)));
        } else {
            // Stand back up
            is_sliding_ = false;
            player_y_position_ = 0.6f;
            player_body_->SetOrientation(glm::quat());  // Reset rotation to upright
        }
    }
    
    // Update player root position
    player_root_->SetPosition(glm::vec3(new_x, player_y_position_, player_z_position_));
    
    // Running animation - swing arms and legs like Subway Surfers
    run_animation_time_ += delta_time * 8.0f;  // Animation speed
    
    if (!is_jumping_ && !is_sliding_){
        // Normal running animation
        float arm_swing = sin(run_animation_time_) * 0.4f;  // Swing amplitude
        float leg_swing = sin(run_animation_time_) * 0.5f;
        
        // Swing arms (opposite to legs for natural running motion)
        player_left_arm_->SetOrientation(glm::angleAxis(arm_swing, glm::vec3(1.0f, 0.0f, 0.0f)));
        player_right_arm_->SetOrientation(glm::angleAxis(-arm_swing, glm::vec3(1.0f, 0.0f, 0.0f)));
        
        // Swing legs
        player_left_leg_->SetOrientation(glm::angleAxis(-leg_swing, glm::vec3(1.0f, 0.0f, 0.0f)));
        player_right_leg_->SetOrientation(glm::angleAxis(leg_swing, glm::vec3(1.0f, 0.0f, 0.0f)));
    } else if (is_jumping_){
        // Arms up when jumping
        player_left_arm_->SetOrientation(glm::angleAxis(-glm::pi<float>() / 3.0f, glm::vec3(1.0f, 0.0f, 0.0f)));
        player_right_arm_->SetOrientation(glm::angleAxis(-glm::pi<float>() / 3.0f, glm::vec3(1.0f, 0.0f, 0.0f)));
        
        // Legs together when jumping
        player_left_leg_->SetOrientation(glm::quat());
        player_right_leg_->SetOrientation(glm::quat());
    }
    
    // Move obstacles towards player (simulate forward movement)
    float obstacle_speed = forward_speed_ * delta_time;
    
    testObstacle_->Translate(glm::vec3(0.0f, 0.0f, obstacle_speed));
    obstacle_medium_->Translate(glm::vec3(0.0f, 0.0f, obstacle_speed));
    obstacle_overhead_->Translate(glm::vec3(0.0f, 0.0f, obstacle_speed));
    
    // Reset obstacles when they pass the player (endless runner mechanic)
    if (testObstacle_->GetPosition().z > 5.0f){
        testObstacle_->SetPosition(glm::vec3(lane_positions[rand() % 3], 0.75f, -30.0f));
    }
    if (obstacle_medium_->GetPosition().z > 5.0f){
        obstacle_medium_->SetPosition(glm::vec3(lane_positions[rand() % 3], 0.375f, -35.0f));
    }
    if (obstacle_overhead_->GetPosition().z > 5.0f){
        obstacle_overhead_->SetPosition(glm::vec3(lane_positions[rand() % 3], 1.2f, -40.0f));
    }
    
    // Move environment objects towards player
    tree1_->Translate(glm::vec3(0.0f, 0.0f, obstacle_speed));
    tree2_->Translate(glm::vec3(0.0f, 0.0f, obstacle_speed));
    building1_->Translate(glm::vec3(0.0f, 0.0f, obstacle_speed));
    building2_->Translate(glm::vec3(0.0f, 0.0f, obstacle_speed));
    tunnel_->Translate(glm::vec3(0.0f, 0.0f, obstacle_speed));
    
    // Reset environment objects when they pass the player
    if (tree1_->GetPosition().z > 10.0f){
        tree1_->SetPosition(glm::vec3(-4.0f, 0.5f, -40.0f));
    }
    if (tree2_->GetPosition().z > 10.0f){
        tree2_->SetPosition(glm::vec3(4.0f, 0.5f, -45.0f));
    }
    if (building1_->GetPosition().z > 10.0f){
        building1_->SetPosition(glm::vec3(-5.5f, 2.0f, -50.0f));
    }
    if (building2_->GetPosition().z > 10.0f){
        building2_->SetPosition(glm::vec3(5.5f, 2.0f, -55.0f));
    }
    if (tunnel_->GetPosition().z > 10.0f){
        tunnel_->SetPosition(glm::vec3(0.0f, 2.0f, -60.0f));
    }
}


void Game::UpdateCamera(void){
    // Subway Surfers style camera - follows player from behind and above
    glm::vec3 player_pos = player_root_->GetPosition();
    
    // Camera position: behind and above the player
    glm::vec3 camera_offset(0.0f, 2.5f, 5.0f);  // X, Y, Z offset from player
    glm::vec3 new_camera_pos = player_pos + camera_offset;
    
    // Look at position: slightly ahead of player
    glm::vec3 look_at_offset(0.0f, 1.0f, -3.0f);
    glm::vec3 new_look_at = player_pos + look_at_offset;
    
    // Update camera view to follow player
    camera_.SetView(new_camera_pos, new_look_at, glm::vec3(0.0f, 1.0f, 0.0f));
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
    double last_time = glfwGetTime();
    
    while (!glfwWindowShouldClose(window_)){
        // Calculate delta time for smooth frame-rate independent movement
        double current_time = glfwGetTime();
        float delta_time = static_cast<float>(current_time - last_time);
        last_time = current_time;
        
        // Limit delta time to prevent large jumps when debugging/pausing
        if (delta_time > 0.1f){
            delta_time = 0.1f;
        }
        
        // Animate the scene
        if (animating_){
            // Update player movement and animation
            UpdatePlayer(delta_time);
            
            // Update camera to follow player (Subway Surfers style)
            UpdateCamera();
            
            // Update scene
            scene_.Update();
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

    // Pause/Resume animation if space bar is pressed
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        game->animating_ = (game->animating_ == true) ? false : true;
    }

    // ===== SUBWAY SURFERS STYLE CONTROLS =====
    
    // LEFT ARROW - Move to left lane
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
        if (game->current_lane_ > 0){
            game->current_lane_--;
        }
    }
    
    // RIGHT ARROW - Move to right lane
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
        if (game->current_lane_ < 2){
            game->current_lane_++;
        }
    }
    
    // UP ARROW or W - Jump
    if ((key == GLFW_KEY_UP || key == GLFW_KEY_W) && action == GLFW_PRESS){
        if (!game->is_jumping_ && !game->is_sliding_){
            game->is_jumping_ = true;
            game->jump_start_time_ = glfwGetTime();
        }
    }
    
    // DOWN ARROW or S - Slide/Glide (to go under overhead obstacles)
    if ((key == GLFW_KEY_DOWN || key == GLFW_KEY_S) && action == GLFW_PRESS){
        if (!game->is_jumping_ && !game->is_sliding_){
            game->is_sliding_ = true;
            game->slide_start_time_ = glfwGetTime();
        }
    }

    // Camera manual control (for debugging/testing - kept from original demo style)
    float rot_factor(glm::pi<float>() / 180);
    float trans_factor = 0.2;
    
    if (key == GLFW_KEY_I && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Pitch(rot_factor);
    }
    if (key == GLFW_KEY_K && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Pitch(-rot_factor);
    }
    if (key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Yaw(rot_factor);
    }
    if (key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        game->camera_.Yaw(-rot_factor);
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
    return ast;
}


void Game::CreateAsteroidField(int num_asteroids){
    // Create a number of asteroid instances
    for (int i = 0; i < num_asteroids; i++){
        std::stringstream ss;
        ss << i;
        std::string index = ss.str();
        std::string name = "AsteroidInstance" + index;

        Asteroid *ast = CreateAsteroidInstance(name, "SimpleSphereMesh", "ObjectMaterial");

        ast->SetPosition(glm::vec3(-300.0 + 600.0*((float) rand() / RAND_MAX), -300.0 + 600.0*((float) rand() / RAND_MAX), 600.0*((float) rand() / RAND_MAX)));
        ast->SetOrientation(glm::normalize(glm::angleAxis(glm::pi<float>()*((float) rand() / RAND_MAX), glm::vec3(((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX)))));
        ast->SetAngM(glm::normalize(glm::angleAxis(0.05f*glm::pi<float>()*((float) rand() / RAND_MAX), glm::vec3(((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX)))));
    }
}


SceneNode *Game::CreateInstance(std::string entity_name, std::string object_name, std::string material_name){
    // Get resources
    Resource *geom;
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
