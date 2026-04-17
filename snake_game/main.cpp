#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <deque>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <iostream>
#include <fstream>
#include <string>

const int GRID_SIZE = 20;
const int WIDTH     = 800;
const int HEIGHT    = 600;

enum Direction  { UP, DOWN, LEFT, RIGHT };
enum GameState  { MENU, PLAYING, GAMEOVER };

class ScoreManager {
public:
    static void saveScore(int score) {
        if (score > getHighScore()) {
            std::ofstream file("highscore.txt");
            if (file.is_open()) { file << score; }
        }
    }
    static int getHighScore() {
        int hs = 0;
        std::ifstream file("highscore.txt");
        if (file.is_open()) { file >> hs; }
        return hs;
    }
};

class GameObject {
protected:
    sf::Vector2f      position;
    sf::RectangleShape shape;
public:
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual ~GameObject() = default;
    sf::Vector2f getPosition() const { return position; }
    void setPosition(sf::Vector2f pos) {
        position = pos;
        shape.setPosition(position);
    }
};

class Food : public GameObject {
public:
    Food() {
        shape.setSize(sf::Vector2f(GRID_SIZE, GRID_SIZE));
        shape.setFillColor(sf::Color(220, 50, 50));
        respawn();
    }
    void respawn() {
        int x = (rand() % (WIDTH  / GRID_SIZE)) * GRID_SIZE;
        int y = (rand() % (HEIGHT / GRID_SIZE)) * GRID_SIZE;
        setPosition(sf::Vector2f(static_cast<float>(x), static_cast<float>(y)));
    }
    void draw(sf::RenderWindow& window) override { window.draw(shape); }
};

class Snake : public GameObject {
private:
    std::deque<sf::Vector2f> body;
    Direction currentDir;
public:
    Snake() { reset(); }

    void reset() {
        body.clear();
        shape.setSize(sf::Vector2f(GRID_SIZE, GRID_SIZE));
        shape.setOutlineThickness(-1.f);
        shape.setOutlineColor(sf::Color(30, 30, 30));
        body.push_back(sf::Vector2f(WIDTH / 2.f,              HEIGHT / 2.f));
        body.push_back(sf::Vector2f(WIDTH / 2.f - GRID_SIZE,  HEIGHT / 2.f));
        body.push_back(sf::Vector2f(WIDTH / 2.f - 2*GRID_SIZE, HEIGHT / 2.f));
        position   = body.front();
        currentDir = RIGHT;
    }

    Direction    getDirection() const { return currentDir; }
    void         setDirection(Direction d) { currentDir = d; }
    sf::Vector2f getHead()      const { return body.front(); }

    void move(bool grow) {
        sf::Vector2f newHead = body.front();
        switch (currentDir) {
            case UP:    newHead.y -= GRID_SIZE; break;
            case DOWN:  newHead.y += GRID_SIZE; break;
            case LEFT:  newHead.x -= GRID_SIZE; break;
            case RIGHT: newHead.x += GRID_SIZE; break;
        }
        body.push_front(newHead);
        position = newHead;
        if (!grow) body.pop_back();
    }

    bool checkCollision() const {
        sf::Vector2f head = body.front();
        if (head.x < 0 || head.x >= WIDTH || head.y < 0 || head.y >= HEIGHT)
            return true;
        for (size_t i = 1; i < body.size(); ++i)
            if (head == body[i]) return true;
        return false;
    }

    void draw(sf::RenderWindow& window) override {
        for (size_t i = 0; i < body.size(); ++i) {
            shape.setFillColor(i == 0 ? sf::Color(0, 200, 0) : sf::Color(0, 160, 0));
            shape.setPosition(body[i]);
            window.draw(shape);
        }
    }
};

struct Button {
    sf::RectangleShape box;
    sf::Text           label;

    void init(sf::Font& font, const std::string& text, sf::Color color,
              float x, float y, float w = 220.f, float h = 55.f)
    {
        box.setSize({ w, h });
        box.setFillColor(color);
        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color(255, 255, 255, 80));
        box.setPosition(x, y);

        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(26);
        label.setFillColor(sf::Color::White);
        sf::FloatRect lb = label.getLocalBounds();
        label.setOrigin(lb.width / 2.f, lb.height / 2.f);
        label.setPosition(x + w / 2.f, y + h / 2.f - 4.f);
    }

    bool contains(sf::Vector2i mousePos) const {
        return box.getGlobalBounds().contains(
            static_cast<float>(mousePos.x),
            static_cast<float>(mousePos.y));
    }

    void draw(sf::RenderWindow& w) { w.draw(box); w.draw(label); }
};

class Game {
private:
    sf::RenderWindow window;
    Snake snake;
    Food  food;
    GameState state;

    sf::Font font;
    int score;

    std::thread logicThread;
    std::mutex  mtx;
    bool        isRunning;
    Direction   nextDir;

    sf::Text  txtTitle, txtMenuHint;
    Button    btnMenuPlay, btnMenuExit;

    sf::Text  txtHUD;

    sf::RectangleShape overlay;
    sf::Text  txtGameOver, txtFinalScore, txtHighScore;
    Button    btnPlayAgain, btnBackMenu, btnExitGO;

    void initUI() {
        if (!font.loadFromFile("arial.ttf")) {
            std::cerr << "Khong tim thay arial.ttf – dat file vao cung thu muc exe\n";
        }

        txtTitle.setFont(font);
        txtTitle.setString("SNAKE");
        txtTitle.setCharacterSize(80);
        txtTitle.setStyle(sf::Text::Bold);
        txtTitle.setFillColor(sf::Color(0, 220, 0));
        txtTitle.setOutlineColor(sf::Color::Black);
        txtTitle.setOutlineThickness(3.f);
        {
            sf::FloatRect r = txtTitle.getLocalBounds();
            txtTitle.setOrigin(r.width / 2.f, r.height / 2.f);
            txtTitle.setPosition(WIDTH / 2.f, 140.f);
        }

        txtMenuHint.setFont(font);
        txtMenuHint.setString("Dung phim mui ten de di chuyen");
        txtMenuHint.setCharacterSize(18);
        txtMenuHint.setFillColor(sf::Color(180, 180, 180));
        {
            sf::FloatRect r = txtMenuHint.getLocalBounds();
            txtMenuHint.setOrigin(r.width / 2.f, 0);
            txtMenuHint.setPosition(WIDTH / 2.f, 460.f);
        }

        btnMenuPlay.init(font, "Choi Ngay", sf::Color(50, 180, 80),  WIDTH / 2.f - 110.f, 260.f);
        btnMenuExit.init(font, "Thoat",     sf::Color(180, 60, 60),  WIDTH / 2.f - 110.f, 340.f);

        txtHUD.setFont(font);
        txtHUD.setCharacterSize(22);
        txtHUD.setFillColor(sf::Color::White);
        txtHUD.setPosition(10.f, 8.f);

        overlay.setSize({ static_cast<float>(WIDTH), static_cast<float>(HEIGHT) });
        overlay.setFillColor(sf::Color(0, 0, 0, 170));
        overlay.setPosition(0, 0);

        txtGameOver.setFont(font);
        txtGameOver.setString("GAME OVER");
        txtGameOver.setCharacterSize(60);
        txtGameOver.setStyle(sf::Text::Bold);
        txtGameOver.setFillColor(sf::Color(220, 50, 50));
        txtGameOver.setOutlineColor(sf::Color::Black);
        txtGameOver.setOutlineThickness(3.f);
        {
            sf::FloatRect r = txtGameOver.getLocalBounds();
            txtGameOver.setOrigin(r.width / 2.f, r.height / 2.f);
            txtGameOver.setPosition(WIDTH / 2.f, 130.f);
        }

        txtFinalScore.setFont(font);
        txtFinalScore.setCharacterSize(28);
        txtFinalScore.setFillColor(sf::Color(255, 220, 0));

        txtHighScore.setFont(font);
        txtHighScore.setCharacterSize(22);
        txtHighScore.setFillColor(sf::Color(180, 255, 180));

        btnPlayAgain.init(font, "Choi Lai",   sf::Color(50, 180, 80),  WIDTH / 2.f - 110.f, 300.f);
        btnBackMenu.init(font,  "Menu Chinh", sf::Color(70, 130, 200), WIDTH / 2.f - 110.f, 375.f);
        btnExitGO.init(font,    "Thoat",      sf::Color(180, 60, 60),  WIDTH / 2.f - 110.f, 450.f);
    }

    void updateScoreTexts() {
        txtFinalScore.setString("Diem cua ban:  " + std::to_string(score));
        {
            sf::FloatRect r = txtFinalScore.getLocalBounds();
            txtFinalScore.setOrigin(r.width / 2.f, 0);
            txtFinalScore.setPosition(WIDTH / 2.f, 210.f);
        }
        txtHighScore.setString("Ky luc:  " + std::to_string(ScoreManager::getHighScore()));
        {
            sf::FloatRect r = txtHighScore.getLocalBounds();
            txtHighScore.setOrigin(r.width / 2.f, 0);
            txtHighScore.setPosition(WIDTH / 2.f, 252.f);
        }
    }

    void startNewGame() {
        std::lock_guard<std::mutex> lock(mtx);
        snake.reset();
        food.respawn();
        score   = 0;
        nextDir = RIGHT;
        state   = PLAYING;
    }

    void logicUpdate() {
        while (isRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::lock_guard<std::mutex> lock(mtx);

            if (state != PLAYING) continue;

            snake.setDirection(nextDir);

            sf::Vector2f nextHead = snake.getHead();
            switch (nextDir) {
                case UP:    nextHead.y -= GRID_SIZE; break;
                case DOWN:  nextHead.y += GRID_SIZE; break;
                case LEFT:  nextHead.x -= GRID_SIZE; break;
                case RIGHT: nextHead.x += GRID_SIZE; break;
            }
            bool grow = (nextHead == food.getPosition());
            if (grow) { score++; food.respawn(); }

            snake.move(grow);

            if (snake.checkCollision()) {
                state = GAMEOVER;
                ScoreManager::saveScore(score);
            }
        }
    }

public:
    Game() : window(sf::VideoMode({ WIDTH, HEIGHT }), "Snake Game"),
             state(MENU), score(0), isRunning(true), nextDir(RIGHT)
    {
        window.setFramerateLimit(60);
        srand(static_cast<unsigned>(time(nullptr)));
        initUI();
        logicThread = std::thread(&Game::logicUpdate, this);
    }

    ~Game() {
        isRunning = false;
        if (logicThread.joinable()) logicThread.join();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            render();
        }
    }

private:
    void handleEvents() {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                isRunning = false;
                window.close();
                return;
            }

            if (state == MENU) {
                if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                    sf::Vector2i mp = sf::Mouse::getPosition(window);
                    if (btnMenuPlay.contains(mp)) startNewGame();
                    if (btnMenuExit.contains(mp)) { isRunning = false; window.close(); }
                }
            }
            else if (state == PLAYING) {
                if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                    std::lock_guard<std::mutex> lock(mtx);
                    Direction cur = snake.getDirection();
                    if      (kp->code == sf::Keyboard::Key::Up    && cur != DOWN)  nextDir = UP;
                    else if (kp->code == sf::Keyboard::Key::Down  && cur != UP)    nextDir = DOWN;
                    else if (kp->code == sf::Keyboard::Key::Left  && cur != RIGHT) nextDir = LEFT;
                    else if (kp->code == sf::Keyboard::Key::Right && cur != LEFT)  nextDir = RIGHT;
                }
            }
            else if (state == GAMEOVER) {
                if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                    sf::Vector2i mp = sf::Mouse::getPosition(window);
                    if (btnPlayAgain.contains(mp)) startNewGame();
                    if (btnBackMenu.contains(mp))  { std::lock_guard<std::mutex> lock(mtx); state = MENU; }
                    if (btnExitGO.contains(mp))    { isRunning = false; window.close(); }
                }
            }
        }
    }

    void render() {
        window.clear(sf::Color(30, 30, 30));
        std::lock_guard<std::mutex> lock(mtx);

        if (state == MENU) {
            drawGrid();
            window.draw(txtTitle);
            window.draw(txtMenuHint);
            btnMenuPlay.draw(window);
            btnMenuExit.draw(window);
        }
        else if (state == PLAYING) {
            drawGrid();
            GameObject* objects[] = { &food, &snake };
            for (auto* obj : objects) obj->draw(window);
            txtHUD.setString("Diem: " + std::to_string(score)
                           + "   |   Ky luc: " + std::to_string(ScoreManager::getHighScore()));
            window.draw(txtHUD);
        }
        else if (state == GAMEOVER) {
            drawGrid();
            GameObject* objects[] = { &food, &snake };
            for (auto* obj : objects) obj->draw(window);
            window.draw(overlay);
            updateScoreTexts();
            window.draw(txtGameOver);
            window.draw(txtFinalScore);
            window.draw(txtHighScore);
            btnPlayAgain.draw(window);
            btnBackMenu.draw(window);
            btnExitGO.draw(window);
        }

        window.display();
    }

    void drawGrid() {
        sf::RectangleShape line;
        line.setFillColor(sf::Color(255, 255, 255, 12));
        line.setSize({ static_cast<float>(WIDTH), 1.f });
        for (int y = 0; y < HEIGHT; y += GRID_SIZE) {
            line.setPosition(0.f, static_cast<float>(y));
            window.draw(line);
        }
        line.setSize({ 1.f, static_cast<float>(HEIGHT) });
        for (int x = 0; x < WIDTH; x += GRID_SIZE) {
            line.setPosition(static_cast<float>(x), 0.f);
            window.draw(line);
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
