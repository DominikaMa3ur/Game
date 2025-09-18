#include "raylib.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <string>
#include <map>

const std::map<std::string, int> FOOD_VALUE {{"FRUIT",15},{"DECAYING_FRUIT",8}};

class Collidable {
    private:
    public:
        Vector3 bb = {1.0f,1.0f,1.0f};
        Vector3 pos = {0.0f,1.0f,0.0f};
        BoundingBox box() {
            return (BoundingBox){pos, (Vector3){pos.x+bb.x,pos.y+bb.y,pos.z+bb.z}};
        }
        void draw() {}
};

bool isColliding(Collidable c1, Collidable c2)
{
    return CheckCollisionBoxes(c1.box(),c2.box());
}

class Item : public Collidable {
    public:
    explicit Item(Vector3 item_pos) {
        pos = item_pos;
    }
    explicit Item(float x, float y, float z) {
        pos = {x,y,z};
    }
    void draw()
    {
        DrawCubeV(pos, bb, BLUE);
    }
};

class Shrub : public Collidable {
    private:
    float age = 0.0f;
    float last_fruit = 0.0f;
    float FRUIT_TIME = 60.0f;
    float MAX_AGE = 200.0f;
    bool dead = false;
    bool fruit_ready = false;
    public:
    explicit Shrub(Vector3 new_pos) {pos = new_pos; last_fruit = rand()%20;}
    void update(float delta) {
        age += delta;
        last_fruit += delta;
    if (age >= MAX_AGE) dead = true;
    if (last_fruit >= FRUIT_TIME) {fruit_ready = true; last_fruit -= FRUIT_TIME;};
    }
    bool is_dead() {return dead;}
    bool collect_fruit() {if (fruit_ready) {fruit_ready = false; return true;} return false;}
    void draw()
    {
        DrawCubeV(pos, (Vector3){1.2,2.4,1.2}, DARKGREEN);
    }
};

class FoodItem : public Item {
    private:
    int age = 0;
    int decay = 0;
    public:
    const static int max_decay = 5;
    using Item::Item;
    void update(int a = 1) {
        if (a > 0) age += a;
        if (age > 10) decay += a;
    }
    void draw()
    {
        if (decay == 0) DrawCubeV(pos, (Vector3){0.5f, 0.1f, 0.5f}, ORANGE);
        else DrawCubeV(pos, (Vector3){0.5f, 0.1f, 0.5f}, RED);
    }
    bool decaying() {return decay > 0;}
    bool decayed() {return max_decay <= decay;}
};

class Player : public Collidable {
    private:
        const double CAMRADIUS = 8.0;
        const float speed = 15.0f;
        double angle = 0.0f;
        Vector3 lastpos = {0.0f,1.0f,0.0f};
        int food = 100;
        int temperature = 0;
        float hunger_time = 0.0;
        const float max_hunger_time = 1.25;
        const int max_food = 150;
        struct player_state {
            unsigned int starving: 1;
            unsigned int sick: 1;
            unsigned int too_cold: 1;
            unsigned int too_hot: 1;
            unsigned int thirsty: 1;
        };
        player_state PS;
    public:
    Player() {
        PS.starving = false;
        PS.sick = false;
        PS.too_hot = false;
        PS.too_cold = false;
        PS.thirsty = false;
    }
    float calculateSpeed() {
        float v = speed;
        if (PS.starving) v *= 0.6;
        if (PS.sick) v *= 0.5;
        if (PS.too_cold) v *= 0.75;
        if (PS.too_hot) v *= 0.8;
        return v;
    }
    void update(Camera &cam)
    {
        lastpos = pos;
        float v = calculateSpeed();
        float delta = GetFrameTime();
        if (IsKeyDown(KEY_W)) {
            cam.target.x -= v*sin(angle)*delta;
            cam.target.z -= v*cos(angle)*delta;
        }
        else if (IsKeyDown(KEY_S)) {
            cam.target.x += 0.25*v*sin(angle)*delta;
            cam.target.z += 0.25*v*cos(angle)*delta;
        }
        pos = cam.target;
        if (IsKeyDown(KEY_A)) {
            angle += 2.0f*delta;
        }
        if (IsKeyDown(KEY_D)) {
            angle -= 2.0f*delta;
        }
        while (angle > 2*M_PI) {angle -= 2*M_PI;}
        cam.position = (Vector3){float(cam.target.x + sin(angle)*CAMRADIUS),
        cam.position.y, float(cam.target.z + cos(angle)*CAMRADIUS)};
        updateHunger(delta);
    }
    void updateHunger(float delta) {
        hunger_time += delta;
        while (hunger_time > max_hunger_time) {
            food -= 1;
            hunger_time -= max_hunger_time;
            if (food < 25) {PS.starving = true;}
            else if (food > 25) {PS.starving = false;}
        }
        DrawText(std::to_string(food).c_str(), 4, 4, 24, BLUE);
        displayIcons();
    }
    void draw(Camera &cam)
    {
        DrawCube(cam.target, 1.0f, 1.0f, 1.0f, ORANGE);
        
    }
    void addFood(int f) {
            food += f;
            if (food > 25) {PS.starving = false;}
            if (food > max_food) {PS.sick = true;}
        }
    void last(Camera &cam, Collidable &c) {
        float delta = GetFrameTime();
        pos = lastpos;
        if (sin(angle) > cos(angle)) {
            pos.x -= speed*sin(angle)*delta;
            if (isColliding(*this, c)) pos.x = lastpos.x;
            pos.z -= speed*cos(angle)*delta;
            if (isColliding(*this, c)) pos.z = lastpos.z;
            cam.target = pos;
        }
        else {
            pos.z -= speed*cos(angle)*delta;
            if (isColliding(*this, c)) pos.z = lastpos.z;
            pos.x -= speed*sin(angle)*delta;
            if (isColliding(*this, c)) pos.x = lastpos.x;
            cam.target = pos;
        }
    }
    void displayIcons() {
        Vector2 position = {16,16};
        Vector2 icon = {64,64};
        if (PS.sick) {DrawRectangleV(position, icon, GREEN); position.x += 16 + icon.x;}
        if (PS.starving) {DrawRectangleV(position, icon, RED); position.x += 16 + icon.x;}
    }
    void badFoodEaten() {if (rand()%8==0) PS.sick = true;}
};

class FoodGroup {
    private:
    std::vector<FoodItem> food;
    float time = 0.0;
    const float max_time = 2.0;
    public:
    std::vector<Vector3> seeds;
    void add(FoodItem f) {food.push_back(f);}
    void draw()
    {for (int f = 0; f < food.size(); f++) {food[f].draw();}}
    void update(Player &player)
    {
        float delta = GetFrameTime();
        time += delta;        
        for (int i = 0; i < food.size(); i++) {
            if (food[i].decayed()) {seeds.push_back(food[i].pos);food.erase(food.begin()+i);}
            else if (isColliding(player,food[i])) {
                if (not food[i].decaying()) {player.addFood(FOOD_VALUE.at("FRUIT"));}
                else {player.addFood(FOOD_VALUE.at("DECAYING_FRUIT"));player.badFoodEaten();}
                food.erase(food.begin()+i);
            }
        }
        while (time > max_time) {
            time -= max_time;
            for (int i = 0; i < food.size(); i++) {food[i].update();}
            }
    }
    void create_food(Vector3 pos)
    {FoodItem f = FoodItem(pos); f.pos.y = 0.1f; add(f);}
};

class PlantGroup {
    private:
    std::vector<Shrub> plants = {};
    FoodGroup* food;
    Vector3 fruitPos(Vector3 pos0, float radius = 0.044, float minradius = 1.2)
    {
        float x = (rand() % 100)*radius+minradius;
        float z = (rand() % 100)*radius+minradius;
        if (rand()%2 == 0) x *= -1;
        if (rand()%2 == 0) z *= -1;
        Vector3 fruit_pos = pos0;
        fruit_pos.x += x; fruit_pos.z += z;
        return fruit_pos;
    }
    public:
    PlantGroup (FoodGroup* fg) {
        food = fg;
        for (int x = 0; x < 12; x++)
        {
            for (int z = 0; z < 12; z++)
            {
                if (rand()%3 == 0) continue;                
                plants.push_back(Shrub((Vector3){32.0f*x-128.0f+float(rand()%8-4), 1.0, 32.0f*z-128.0f+float(rand()%8-4)}));
            }
        }
    }
    void update() {
        for (int i = 0; i < plants.size(); i++) {
            float delta = GetFrameTime();
            plants[i].update(delta);
            if (plants[i].collect_fruit()) {
                for (int j = 0; j < 5; j++)food->create_food(fruitPos(plants[i].pos));
            }
            if (plants[i].is_dead()) {plants.erase(plants.begin()+i);}
            while (!food->seeds.empty())
            {
                if (rand() % 15 == 0) createAt(food->seeds.back());
                food->seeds.pop_back();
            }
        }
    }
    void draw() {
        for (int i = 0; i < plants.size(); i++) plants[i].draw();
    }
    void createAt(Vector3 pos)
    {
        plants.push_back(Shrub(pos));
    }
};

class GameButton {
    private:
        Rectangle rect;
        Rectangle rect_shadow;
        const char* text;
        bool isSelected = false;

        Color color_main = Color{64,64,196,255};
        Color color_shadow = Color{32,32,128,255};
        Color color3 = Color{24,24,96,255};
        Rectangle pressedRect (Rectangle r)
        {
            r.x -= 6; r.y += 6;
            r.width += 12;
            r.height -= 12;            
            return r;
        }
        void drawButtonText()
        {
            DrawText(text, rect.x + rect.width/2 - MeasureText(text, 20)/2, rect.y+rect.height/2+3, 24, color_shadow);
            DrawText(text, rect.x + rect.width/2 - MeasureText(text, 20)/2, rect.y+rect.height/2, 24, WHITE);
        }
    public:
        GameButton(int x, int y, int w, int h, const char* GameButtontext = "OK")
            {rect.x = x; rect.y = y; rect.width = w; rect.height = h; set_rect(rect); text = GameButtontext;}
        Rectangle get_rect() {return rect;}
        void center() {rect.x = (GetScreenWidth()-rect.width)/2; set_rect(rect);}
        void set_rect(Rectangle new_rect) {
            rect = new_rect; rect_shadow = rect; rect_shadow.y += 4;
        }
        bool isInside() {return CheckCollisionPointRec(GetMousePosition(), rect);}
        bool isPressed() {return (isInside() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));}
        bool isReleased() {return (isInside() && IsMouseButtonReleased(MOUSE_BUTTON_LEFT));}
        void select(bool selected) {isSelected = selected;}
        void draw() {
            if ((isInside() && IsMouseButtonDown(MOUSE_BUTTON_LEFT)))
                {
                    Rectangle rect4 = pressedRect(rect_shadow);
                    DrawRectangleRounded(rect4,0.1,4,color3);
                    Rectangle rect3 = pressedRect(rect);
                    DrawRectangleRounded(rect3,0.1,4,color_shadow);
                    drawButtonText();
                }
            else if (isSelected)
                {
                    //
                }
            else if (isInside())
                {
                    DrawRectangleRounded(rect_shadow,0.1,4,color_shadow);
                    Rectangle rect3 = rect;
                    rect3.y += 2;
                    DrawRectangleRounded(rect3,0.1,4,color_main);
                    drawButtonText();
                }
            else {
                    DrawRectangleRounded(rect_shadow,0.1,4,color_shadow);
                    DrawRectangleRounded(rect,0.1,4,color_main);
                    drawButtonText();
                }
        }
};

std::vector<GameButton> centerButtons(std::vector<GameButton> buttons)
{
    for (int i = 0; i < buttons.size(); i++)
    {
        buttons[i].center();
    }
    return buttons;
}

std::vector<GameButton> buttonsMenu()
{
    std::vector<GameButton> menu = {
        GameButton(16,16,320,80, "New game"), GameButton(16,128,320,80, "Exit")
    };
    menu = centerButtons(menu);
    return menu;
}

std::vector<GameButton> buttonsPause()
{
    std::vector<GameButton> menu = {GameButton(16,16,320,80, "Continue"), GameButton(16,128,320,80, "Menu")};
    menu = centerButtons(menu);
    return menu;
}

Vector3 newRainPos(Vector3 center) {
    int rx = rand() % 1024;
    int rz = rand() % 1024;
    Vector3 v = {float((rx-512)*0.1 + center.x),16.0,float((rz-512)*0.1 + center.z)};
    return v;
}

void drawRain(Vector3 center)
{
    const int RAIN = 1024;
    static bool position_set = false;
    static Vector3 rainPos[RAIN];
    if (not position_set) {
        for (int i = 0; i < RAIN; i++) {
            rainPos[i] = newRainPos(center);
            rainPos[i].y -= (rand()%100)*0.2;
            position_set = true;
        }
    }
    float delta = GetFrameTime();
    for (int i = 0; i < RAIN; i++) {
        rainPos[i].y -= delta*2.5*(18.0 - rainPos[i].y);
        if (rainPos[i].y < 0.0) {rainPos[i] = newRainPos(center);}
    }
    for (int i = 0; i < RAIN; i++) {
        DrawCubeV(rainPos[i], (Vector3){0.01,0.4,0.01}, BLUE);
    }
}

void drawSnow(Vector3 center)
{
    const int RAIN = 1024;
    static bool position_set = false;
    static Vector3 rainPos[RAIN];
    if (not position_set) {
        for (int i = 0; i < RAIN; i++) {
            rainPos[i] = newRainPos(center);
            rainPos[i].y -= (rand()%100)*0.2;
            position_set = true;
        }
    }
    float delta = GetFrameTime();
    for (int i = 0; i < RAIN; i++) {
        rainPos[i].y -= delta*3.0*((18.0 - rainPos[i].y)/8.0);
        if (rainPos[i].y < 0.0) {rainPos[i] = newRainPos(center);}
    }
    for (int i = 0; i < RAIN; i++) {
        DrawCubeV(rainPos[i], (Vector3){0.05,0.05,0.05}, WHITE);
    }
}

int main ()
{
    InitWindow(1280, 800, "3D");
    SetExitKey(KEY_NULL);

    enum WEATHER {CLEAR, RAIN, SNOW};

	Player player;
    Camera3D camera = { 0 };
    camera.position = (Vector3){ -8.0f, 2.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    enum GAME_STATE {PAUSED, MENU, RUNNING};
    GAME_STATE G = MENU;
    FoodGroup foodg;
    PlantGroup plantg(&foodg);
    std::vector<Item> items;
    for (int i = 1; i < 1; i++) {Item item = Item(4.0f*i, 1.0f, 4.0f*i); items.push_back(item);}
    Color CLEAR_COLOR = BLACK;
    std::vector<GameButton> menu = buttonsMenu();
    bool exited = false;


	while (!exited)
	{
        if (G == RUNNING) {
            player.update(camera);
            if (WindowShouldClose()) {exited = true;}
            if (IsKeyPressed(KEY_ESCAPE))
            {
                menu = buttonsPause();
                G = PAUSED;
            }
        }
        if (G == MENU) {
            if (menu[0].isReleased()) {
                G = RUNNING;
                menu = {};
            }
            if (menu[1].isReleased() || WindowShouldClose()) {exited = true;}
        }
        if (G == PAUSED) {
            if (menu[0].isReleased()) {
                G = RUNNING;
                menu = {};
            }
            if (menu[1].isReleased()) {
                G = MENU;
                menu = buttonsMenu();
            }
            if (WindowShouldClose()) {exited = true;}
        }

		BeginDrawing();

		ClearBackground(CLEAR_COLOR);
        BeginMode3D(camera);
        for (int i = 0; i < items.size(); i++) items[i].draw();
        foodg.draw();
        for (int i = 0; i < items.size(); i++) if (isColliding(player,items[i])) player.last(camera,items[i]);
        foodg.update(player);
        plantg.update();
        player.draw(camera);
        plantg.draw();
        drawSnow(camera.target);
        DrawCube((Vector3){0.0f,-0.05f,0.0f}, 1024.0f, 0.1f, 128.0f, GREEN);
        EndMode3D();
        for (int i = 0; i < int(menu.size()); i++) {menu[i].draw();}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
