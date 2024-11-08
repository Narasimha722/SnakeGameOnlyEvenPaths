#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

static bool allowMove = false;

Color lightBlue = {173, 216, 230, 255}; 
Color red = {255, 0, 0, 255};
Color green = {173, 204, 96, 255}; 
Color darkGreen = {0, 100, 0, 255}; 
Color darkRed = {139, 0, 0, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool EventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake
{
public:
    deque<Vector2> body = {Vector2{8, 8}, Vector2{7, 8}, Vector2{6, 8}}; // Starting positions
    Vector2 direction = {1, 0}; // Initial movement direction
    bool addSegment = false;

    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction)); // Move the snake head
        if (addSegment)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back(); 
        }
    }

    void Reset()
    {
        body = {Vector2{8, 8}, Vector2{7, 8}, Vector2{6, 8}};
        direction = {1, 0};
    }
};

class Food
{
public:
    Vector2 position;
    Texture2D texture;

    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("Food/frog.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }

    ~Food()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, (cellCount /2))* 2;
        float y = GetRandomValue(0, (cellCount / 2)) * 2;
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Game
{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    Sound eatSound;
    Sound wallSound;

    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void Draw()
    {
        DrawGrid();
        food.Draw();
        snake.Draw();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x >= cellCount || snake.body[0].x < 0 ||
            snake.body[0].y >= cellCount || snake.body[0].y < 0)
        {
            GameOver();
        }
    }

    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
        PlaySound(wallSound);
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }

    void DrawGrid()
    {
        for (int i = 0; i <= cellCount; i++)
        {
            // Vertical lines
            DrawLine(offset + i * cellSize, offset, offset + i * cellSize, offset + cellCount * cellSize, darkGreen);
            // Horizontal lines
            DrawLine(offset, offset + i * cellSize, offset + cellCount * cellSize, offset + i * cellSize, darkGreen);
        }
    }
};

int main()
{
    cout << "Starting the game..." << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "IndianCobra");
    SetTargetFPS(60);

    Game game = Game();

    while (!WindowShouldClose())
    {
        BeginDrawing();

        if (EventTriggered(0.2))
        {
            allowMove = true;
            game.Update();
        }

        // Allow changing direction only on even cells
        Vector2 head = game.snake.body[0];
        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1 && allowMove && (int)head.x % 2 == 0 && (int)head.y % 2 == 0)
        {
            game.snake.direction = {0, -1};
            game.running = true;
            allowMove = false;
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1 && allowMove && (int)head.x % 2 == 0 && (int)head.y % 2 == 0)
        {
            game.snake.direction = {0, 1};
            game.running = true;
            allowMove = false;
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1 && allowMove && (int)head.x % 2 == 0 && (int)head.y % 2 == 0)
        {
            game.snake.direction = {-1, 0};
            game.running = true;
            allowMove = false;
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1 && allowMove && (int)head.x % 2 == 0 && (int)head.y % 2 == 0)
        {
            game.snake.direction = {1, 0};
            game.running = true;
            allowMove = false;
        }

        // Drawing
        ClearBackground(lightBlue);
        DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, red);
        DrawText("IndianCobra,Can Move Only Even Paths", offset - 5, 20, 40, darkGreen);
        DrawText(TextFormat("Score: %i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkRed);
        game.Draw();

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
