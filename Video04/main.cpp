#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <format>
#include <iostream>
#include <memory>
#include <random>

constexpr SDL_InitFlags SDL_FLAGS = SDL_INIT_VIDEO;

constexpr const char *WINDOW_TITLE = "Changing Colors";
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

class Game {
    public:
        Game()
            : is_running{true},
              event{},
              gen{},
              rand_color{0, 255},
              window{nullptr, SDL_DestroyWindow},
              renderer{nullptr, SDL_DestroyRenderer},
              background{nullptr, SDL_DestroyTexture} {}

        ~Game();

        void init();
        void run();

    private:
        void initSdl();
        void loadMedia();
        void renderColor();
        void events();
        void draw() const;

        bool is_running;
        SDL_Event event;
        std::mt19937 gen;
        std::uniform_int_distribution<Uint8> rand_color;

        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer;
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> background;
};

Game::~Game() {
    this->background.reset();
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

void Game::loadMedia() {
    this->background.reset(
        IMG_LoadTexture(this->renderer.get(), "images/background.png"));
    if (!this->background) {
        auto error = std::format("Error loading Texture: {}", SDL_GetError());
        throw std::runtime_error(error);
    }
}

void Game::init() {
    this->initSdl();

    this->loadMedia();

    this->gen.seed(std::random_device()());
}

void Game::renderColor() {
    SDL_SetRenderDrawColor(this->renderer.get(), this->rand_color(this->gen),
                           this->rand_color(this->gen),
                           this->rand_color(this->gen), 255);
}

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
            case SDL_SCANCODE_SPACE:
                this->renderColor();
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

    SDL_RenderTexture(this->renderer.get(), this->background.get(), nullptr,
                      nullptr);

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
