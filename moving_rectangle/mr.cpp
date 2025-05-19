#include <iostream>
#include <SDL3/SDL.h>

// Window dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Square dimensions
const int SQUARE_SIZE = 50;

// Movement speed (pixels per second)
const float MOVE_SPEED = 300.0f; // Increased for better visibility of smoothness

int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("SDL3 Smooth Movement", SCREEN_WIDTH, SCREEN_HEIGHT, 0); // Removed ALWAYS_ON_TOP for simplicity
	if (window == nullptr) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	// --- Renderer Creation and VSync ---
	// Option 1: Let SDL choose the best driver and try to enable VSync via flags
	// SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr, SDL_RENDERER_ACCELERATED | SDL_RENDERER_VSYNC);

	// Option 2: Specify a driver (like you did) and then try to set VSync
	SDL_Renderer* renderer = SDL_CreateRenderer(window, "vulkan"); // Create with specific driver, no flags initially
	if (renderer == nullptr) {
		std::cerr << "SDL_CreateRenderer Error (trying Vulkan): " << SDL_GetError() << std::endl;
		std::cerr << "Falling back to default renderer..." << std::endl;
		// Fallback to default renderer without specific name, but request acceleration
		renderer = SDL_CreateRenderer(window, "opengl");
		if (renderer == nullptr) {
			std::cerr << "SDL_CreateRenderer Error (fallback): " << SDL_GetError() << std::endl;
			SDL_DestroyWindow(window);
			SDL_Quit();
			return 1;
		}
	}

	// Attempt to set VSync after renderer creation
	if (SDL_SetRenderVSync(renderer, 1)) {
		std::cout << "VSync was enabled successfully." << std::endl;
	}
	else {
		std::cout << "Warning: Could not enable VSync. SDL_Error: " << SDL_GetError() << std::endl;
		std::cout << "Smoothness might be affected by screen tearing or uncapped frame rates." << std::endl;
	}

	std::cout << "Available Render Drivers:" << std::endl;
	for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
		std::cout << "- " << SDL_GetRenderDriver(i) << std::endl;
	}

	SDL_FRect squareRect = { (float)(SCREEN_WIDTH - SQUARE_SIZE) / 2, (float)(SCREEN_HEIGHT - SQUARE_SIZE) / 2, (float)SQUARE_SIZE, (float)SQUARE_SIZE };

	float velocityX = 0.0f;
	float velocityY = 0.0f;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	bool quit = false;
	SDL_Event event;

	Uint64 lastCounter = SDL_GetPerformanceCounter();

	while (!quit) {
		Uint64 currentCounter = SDL_GetPerformanceCounter();
		double deltaTime = static_cast<double>(currentCounter - lastCounter) / static_cast<double>(SDL_GetPerformanceFrequency());
		lastCounter = currentCounter;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				quit = true;
			}
		}

		const bool* currentKeyStates = SDL_GetKeyboardState(nullptr);
		velocityX = 0.0f;
		velocityY = 0.0f;

		if (currentKeyStates[SDL_SCANCODE_UP] || currentKeyStates[SDL_SCANCODE_W]) {
			velocityY = -1.0f;
		}
		if (currentKeyStates[SDL_SCANCODE_DOWN] || currentKeyStates[SDL_SCANCODE_S]) {
			velocityY = 1.0f;
		}
		if (currentKeyStates[SDL_SCANCODE_LEFT] || currentKeyStates[SDL_SCANCODE_A]) {
			velocityX = -1.0f;
		}
		if (currentKeyStates[SDL_SCANCODE_RIGHT] || currentKeyStates[SDL_SCANCODE_D]) {
			velocityX = 1.0f;
		}

		if (velocityX != 0.0f && velocityY != 0.0f) {
			float length = SDL_sqrtf(velocityX * velocityX + velocityY * velocityY);
			// Avoid division by zero if somehow length is zero
			if (length > 0.0001f) {
				velocityX = (velocityX / length);
				velocityY = (velocityY / length);
			}
		}

		squareRect.x += velocityX * MOVE_SPEED * static_cast<float>(deltaTime);
		squareRect.y += velocityY * MOVE_SPEED * static_cast<float>(deltaTime);

		if (squareRect.x < 0) squareRect.x = 0;
		if (squareRect.y < 0) squareRect.y = 0;
		if (squareRect.x > SCREEN_WIDTH - squareRect.w) squareRect.x = (float)SCREEN_WIDTH - squareRect.w;
		if (squareRect.y > SCREEN_HEIGHT - squareRect.h) squareRect.y = (float)SCREEN_HEIGHT - squareRect.h;

		SDL_SetRenderDrawColor(renderer, 0, 200, 150, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderFillRect(renderer, &squareRect);

		SDL_RenderPresent(renderer);
	}

	// --- Cleanup ---
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit(); // Quit SDL

	return 0;
}