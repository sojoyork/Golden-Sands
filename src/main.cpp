#include <iostream>
#include <cmath>
#include <ncurses.h>
#include <chrono>
#include <thread>
#include <vector>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define MAP_WIDTH 16
#define MAP_HEIGHT 16

// Player position and angle
float playerX = 8.0f;
float playerY = 8.0f;
float playerAngle = 0.0f;
int health = 100;  // Player health

// Map (walls represented as 1, empty space as 0)
int worldMap[MAP_WIDTH][MAP_HEIGHT] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1},
    {1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1},
    {1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// Function to initialize ncurses
void initNcurses() {
    initscr(); // Initialize the screen
    raw();     // Disable line buffering
    keypad(stdscr, TRUE); // Enable special keys (like arrow keys)
    noecho();  // Don't echo pressed keys
    curs_set(0); // Hide the cursor
    start_color(); // Start color functionality

    // Define colors (pair 1: wall color, pair 2: player, pair 3: background)
    init_pair(1, COLOR_RED, COLOR_BLACK);    // Wall color
    init_pair(2, COLOR_GREEN, COLOR_BLACK);  // Player color
    init_pair(3, COLOR_WHITE, COLOR_BLACK);  // Background color
}

// Function to handle player movement
void handleMovement() {
    int ch = getch();  // Get the user input

    float moveSpeed = 0.1f;  // Player movement speed
    float rotationSpeed = 0.05f; // Player turning speed

    switch (ch) {
        case KEY_UP:    // Move forward
        case 'w':       // Move forward with 'W'
            playerX += cos(playerAngle) * moveSpeed;
            playerY += sin(playerAngle) * moveSpeed;
            break;
        case KEY_DOWN:  // Move backward
        case 's':       // Move backward with 'S'
            playerX -= cos(playerAngle) * moveSpeed;
            playerY -= sin(playerAngle) * moveSpeed;
            break;
        case KEY_LEFT:  // Turn left
        case 'a':       // Turn left with 'A'
            playerAngle -= rotationSpeed;
            break;
        case KEY_RIGHT: // Turn right
        case 'd':       // Turn right with 'D'
            playerAngle += rotationSpeed;
            break;
        case 'q': // Quit game with 'q'
            endwin();
            exit(0);
            break;
    }

    // Keep the player inside the map boundaries
    if (playerX < 0) playerX = 0;
    if (playerY < 0) playerY = 0;
    if (playerX >= MAP_WIDTH) playerX = MAP_WIDTH - 1;
    if (playerY >= MAP_HEIGHT) playerY = MAP_HEIGHT - 1;
}

// Function to render the world using raycasting (simulating 3D)
void renderWorld() {
    // Field of view and number of rays
    const float FOV = M_PI / 3.0f; // 60 degree field of view
    const int NUM_RAYS = SCREEN_WIDTH; // Number of rays corresponds to screen width
    const float MAX_DIST = 16.0f;  // Max distance to cast the rays

    // Loop through each ray (column)
    for (int x = 0; x < NUM_RAYS; x++) {
        // Calculate the ray angle based on the player's angle and FOV
        float rayAngle = playerAngle - FOV / 2.0f + (FOV * x / NUM_RAYS);
        float distanceToWall = 0.0f;
        bool hitWall = false;

        float eyeX = cos(rayAngle);
        float eyeY = sin(rayAngle);

        // Cast the ray until we hit a wall or exceed the max distance
        while (!hitWall && distanceToWall < MAX_DIST) {
            distanceToWall += 0.1f;

            int testX = int(playerX + eyeX * distanceToWall);
            int testY = int(playerY + eyeY * distanceToWall);

            // Check if the ray hits a wall (map value 1 means a wall)
            if (testX < 0 || testX >= MAP_WIDTH || testY < 0 || testY >= MAP_HEIGHT || worldMap[testX][testY] == 1) {
                hitWall = true;
            }
        }

        // Calculate the height of the wall based on distance (simple perspective)
        int wallHeight = int(SCREEN_HEIGHT / (distanceToWall));

        // Draw the wall (this is a simplified 2D representation)
        for (int y = SCREEN_HEIGHT / 2 - wallHeight / 2; y < SCREEN_HEIGHT / 2 + wallHeight / 2; y++) {
            if (y >= 0 && y < SCREEN_HEIGHT) {
                // Draw wall with shading (brighter walls are closer)
                if (distanceToWall < MAX_DIST / 2) {
                    attron(COLOR_PAIR(1)); // Bright wall color
                } else {
                    attron(COLOR_PAIR(2)); // Darker wall color
                }
                mvaddch(y, x, '#');
                attroff(COLOR_PAIR(1));
                attroff(COLOR_PAIR(2));
            }
        }
    }
}

// Main game loop
void gameLoop() {
    while (true) {
        clear();

        // Render the world in 3D-like perspective
        renderWorld();

        // Handle player movement and input
        handleMovement();

        // Display health
        attron(COLOR_PAIR(2));
        mvprintw(1, 1, "Health: %d", health);
        attroff(COLOR_PAIR(2));

        // Delay for frame rate control
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// Main entry point
int main() {
    initNcurses();  // Initialize ncurses
    gameLoop();     // Start the game loop

    // End ncurses mode
    endwin();
    return 0;
}
