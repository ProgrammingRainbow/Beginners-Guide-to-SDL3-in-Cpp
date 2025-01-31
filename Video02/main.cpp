#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <format>
#include <iostream>
#include <memory>

constexpr SDL_InitFlags SDL_FLAGS = SDL_INIT_VIDEO;

constexpr const char *WINDOW_TITLE = "Close Window";
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

class Game {
    public:
        Game()
            : is_running{true},
              event{},
              window{nullptr, SDL_DestroyWindow},
              renderer{nullptr, SDL_DestroyRenderer} {}

        ~Game();

        void init();
        void run();

    private:
        void initSdl();
        void events();
        void draw() const;

        bool is_running;
        SDL_Event event;

        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer;
};

Game::~Game() {
    this->renderer.reset();
    this->window.reset();

    SDL_Quit();
}

void Game::initSdl() {
    if (!SDL_Init(SDL_FLAGS)) {
        auto error = std::format("Error initialize SDL2: {}", SDL_GetError());
        throw std::runtime_error(error);
    }

    this->window.reset(
        SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0));
    if (!this->window) {
        auto error = std::format("Error creating Window: {}", SDL_GetError());
        throw std::runtime_error(error);
    }

    this->renderer.reset(SDL_CreateRenderer(this->window.get(), nullptr));
    if (!this->renderer) {
        auto error = std::format("Error creating Renderer: {}", SDL_GetError());
        throw std::runtime_error(error);
    }
}

void Game::init() { this->initSdl(); }

void Game::events() {
    while (SDL_PollEvent(&this->event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            this->is_running = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            switch (event.key.scancode) {
            case SDL_SCANCODE_ESCAPE:
                this->is_running = false;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

void Game::draw() const {
    SDL_RenderClear(this->renderer.get());

    SDL_RenderPresent(this->renderer.get());
}

void Game::run() {
    while (this->is_running) {
        this->events();

        this->draw();

        SDL_Delay(16);
    }
}

int main() {
    int exit_val = EXIT_SUCCESS;

    try {
        Game game;
        game.init();
        game.run();
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        exit_val = EXIT_FAILURE;
    }

    return exit_val;
}
