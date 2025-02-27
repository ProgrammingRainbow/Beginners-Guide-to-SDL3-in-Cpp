#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <format>
#include <iostream>
#include <memory>
#include <random>

constexpr SDL_InitFlags SDL_FLAGS = SDL_INIT_VIDEO;

constexpr const char *WINDOW_TITLE = "Player Sprite";
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr float TEXT_SIZE = 80;
constexpr SDL_Color TEXT_COLOR = {255, 255, 255, 255};
constexpr const char *TEXT_STR = "SDL";
constexpr float TEXT_VEL = 3;

constexpr float SPRITE_VEL = 5;

class Game {
    public:
        Game()
            : is_running{true},
              event{},
              gen{},
              rand_color{0, 255},
              text_rect{},
              text_xvel{TEXT_VEL},
              text_yvel{TEXT_VEL},
              sprite_rect{},
              keystate{SDL_GetKeyboardState(nullptr)},
              window{nullptr, SDL_DestroyWindow},
              renderer{nullptr, SDL_DestroyRenderer},
              background{nullptr, SDL_DestroyTexture},
              text_image{nullptr, SDL_DestroyTexture},
              icon_surf{nullptr, SDL_DestroySurface},
              sprite_image{nullptr, SDL_DestroyTexture} {}

        ~Game();

        void init();
        void run();

    private:
        void initSdl();
        void loadMedia();
        void renderColor();
        void updateText();
        void updateSprite();
        void events();
        void update();
        void draw() const;

        bool is_running;
        SDL_Event event;
        std::mt19937 gen;
        std::uniform_int_distribution<Uint8> rand_color;
        SDL_FRect text_rect;
        float text_xvel;
        float text_yvel;
        SDL_FRect sprite_rect;

        const bool *keystate;

        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer;
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> background;
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> text_image;
        std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> icon_surf;
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>
            sprite_image;
};

Game::~Game() {
    this->sprite_image.reset();
    this->icon_surf.reset();
    this->text_image.reset();
    this->background.reset();
    this->renderer.reset();
    this->window.reset();

    TTF_Quit();
    SDL_Quit();
}

void Game::initSdl() {
    if (!SDL_Init(SDL_FLAGS)) {
        auto error = std::format("Error initialize SDL2: {}", SDL_GetError());
        throw std::runtime_error(error);
    }

    if (!TTF_Init()) {
        auto error =
            std::format("Error initialize SDL_ttf: {}", SDL_GetError());
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

    this->icon_surf.reset(IMG_Load("images/Cpp-logo.png"));
    if (!this->icon_surf) {
        auto error = std::format("Error loading Surface: {}", SDL_GetError());
        throw std::runtime_error(error);
    }
    SDL_SetWindowIcon(this->window.get(), this->icon_surf.get());
}

void Game::loadMedia() {
    this->background.reset(
        IMG_LoadTexture(this->renderer.get(), "images/background.png"));
    if (!this->background) {
        auto error = std::format("Error loading Texture: {}", SDL_GetError());
        throw std::runtime_error(error);
    }

    std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)> font{
        TTF_OpenFont("fonts/freesansbold.ttf", TEXT_SIZE), TTF_CloseFont};
    if (!font) {
        auto error = std::format("Error creating Font: {}", SDL_GetError());
        throw std::runtime_error(error);
    }

    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> text_surf{
        TTF_RenderText_Blended(font.get(), TEXT_STR, 0, TEXT_COLOR),
        SDL_DestroySurface};
    if (!text_surf) {
        auto error =
            std::format("Error loading text Surface: {}", SDL_GetError());
        throw std::runtime_error(error);
    }

    this->text_rect.w = static_cast<float>(text_surf->w);
    this->text_rect.h = static_cast<float>(text_surf->h);

    this->text_image.reset(
        SDL_CreateTextureFromSurface(this->renderer.get(), text_surf.get()));
    if (!this->text_image) {
        auto error = std::format("Error creating Texture from Surface: {}",
                                 SDL_GetError());
        throw std::runtime_error(error);
    }

    this->sprite_image.reset(SDL_CreateTextureFromSurface(
        this->renderer.get(), this->icon_surf.get()));
    if (!this->sprite_image) {
        auto error = std::format("Error creating Texture from Surface: {}",
                                 SDL_GetError());
        throw std::runtime_error(error);
    }

    if (!SDL_GetTextureSize(this->sprite_image.get(), &this->sprite_rect.w,
                            &this->sprite_rect.h)) {
        auto error =
            std::format("Error getting Texture size: {}", SDL_GetError());
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

void Game::updateText() {
    this->text_rect.x += this->text_xvel;
    this->text_rect.y += this->text_yvel;

    if (this->text_rect.x < 0) {
        this->text_xvel = TEXT_VEL;
    } else if (this->text_rect.x + this->text_rect.w > WINDOW_WIDTH) {
        this->text_xvel = -TEXT_VEL;
    }
    if (this->text_rect.y < 0) {
        this->text_yvel = TEXT_VEL;
    } else if (this->text_rect.y + this->text_rect.h > WINDOW_HEIGHT) {
        this->text_yvel = -TEXT_VEL;
    }
}

void Game::updateSprite() {
    if (this->keystate[SDL_SCANCODE_LEFT] || this->keystate[SDL_SCANCODE_A]) {
        this->sprite_rect.x -= SPRITE_VEL;
    }
    if (this->keystate[SDL_SCANCODE_RIGHT] || this->keystate[SDL_SCANCODE_D]) {
        this->sprite_rect.x += SPRITE_VEL;
    }
    if (this->keystate[SDL_SCANCODE_UP] || this->keystate[SDL_SCANCODE_W]) {
        this->sprite_rect.y -= SPRITE_VEL;
    }
    if (this->keystate[SDL_SCANCODE_DOWN] || this->keystate[SDL_SCANCODE_S]) {
        this->sprite_rect.y += SPRITE_VEL;
    }
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

void Game::update() {
    this->updateText();
    this->updateSprite();
}

void Game::draw() const {
    SDL_RenderClear(this->renderer.get());

    SDL_RenderTexture(this->renderer.get(), this->background.get(), nullptr,
                      nullptr);
    SDL_RenderTexture(this->renderer.get(), this->text_image.get(), nullptr,
                      &this->text_rect);
    SDL_RenderTexture(this->renderer.get(), this->sprite_image.get(), nullptr,
                      &this->sprite_rect);

    SDL_RenderPresent(this->renderer.get());
}

void Game::run() {
    while (this->is_running) {
        this->events();

        this->update();

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
