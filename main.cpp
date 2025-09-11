#include "raylib.h"
#include <vector>
#include <cmath>
#include <cstdlib>

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

class Player : public Collidable {
    private:
        const double CAMRADIUS = 8.0;
        const float speed = 7.0f;
        double angle = 0.0f;
        Vector3 lastpos = {0.0f,1.0f,0.0f};
        int food = 100;
        int temperature = 0;
    public:
    void update(Camera &cam)
    {
        lastpos = pos;
        float delta = GetFrameTime();
        if (IsKeyDown(KEY_W)) {
            cam.target.x -= speed*sin(angle)*delta;
            cam.target.z -= speed*cos(angle)*delta;
        }
        else if (IsKeyDown(KEY_S)) {
            cam.target.x += 0.25*speed*sin(angle)*delta;
            cam.target.z += 0.25*speed*cos(angle)*delta;
        }
        pos = cam.target;
        if (IsKeyDown(KEY_A)) {
            angle += 2.0f*delta;
        }
        if (IsKeyDown(KEY_D)) {
            angle -= 2.0f*delta;
        }
        while (angle > 2*M_PI) {angle -= 2*M_PI;}
        cam.position = (Vector3){cam.target.x + sin(angle)*CAMRADIUS,
        cam.position.y, cam.target.z + cos(angle)*CAMRADIUS};
    }
    void draw(Camera &cam)
    {
        DrawCube(cam.target, 1.0f, 1.0f, 1.0f, ORANGE);
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
};

class FoodItem : public Item {
    public:
    using Item::Item;
    void draw()
    {
        DrawCubeV(pos, (Vector3){0.5f, 0.1f, 0.5f}, ORANGE);
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
    Vector3 v = {(rx-512)*0.1 + center.x,16.0,(rz-512)*0.1 + center.z};
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
    std::vector<Item> items;
    std::vector<FoodItem> food;
    for (int i = 1; i < 16; i++) {Item item = Item(4.0f*i, 1.0f, 4.0f*i); items.push_back(item);}
    for (int i = 1; i < 16; i++) {FoodItem f = FoodItem(6.0f*i, 0.1f, 3.0f*i); food.push_back(f);}
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
        for (int i = 0; i < food.size(); i++) food[i].draw();
        for (int i = 0; i < items.size(); i++) if (isColliding(player,items[i])) player.last(camera,items[i]);
        for (int i = 0; i < food.size(); i++) if (isColliding(player,food[i])) food.erase(food.begin()+i);
        player.draw(camera);
        drawRain(camera.target);
        DrawCube((Vector3){0.0f,-0.05f,0.0f}, 1024.0f, 0.1f, 128.0f, GREEN);
        EndMode3D();
        for (int i = 0; i < int(menu.size()); i++) {menu[i].draw();}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
