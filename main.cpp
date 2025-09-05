#include "raylib.h"
#include <vector>
#include <cmath>

class Collidable {
    private:
    public:
        Vector3 bb = {1.0f,1.0f,1.0f};
        Vector3 pos = {0.0f,0.0f,0.0f};
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
    Item(Vector3 item_pos) {
        pos = item_pos;
    }
    Item(float x, float y, float z) {
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
        const float speed = 6.5f;
        double angle = 0.0f;
        Vector3 lastpos = {0.0f,0.0f,0.0f};;
    public:
    void update(Camera &cam)
    {
        lastpos = pos;
        float delta = GetFrameTime();
        if (IsKeyDown(KEY_W)) {
            cam.target.x -= speed*sin(angle)*delta;
            cam.target.z -= speed*cos(angle)*delta;
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
        pos.x -= speed*sin(angle)*delta;
        if (isColliding(*this, c)) pos.x = lastpos.x;
        pos.z -= speed*cos(angle)*delta;
        if (isColliding(*this, c)) pos.z = lastpos.z;
        cam.target = pos;
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

int main ()
{
    InitWindow(1280, 800, "3D");
    SetExitKey(KEY_NULL);

	Player player;
    Camera3D camera = { 0 };
    camera.position = (Vector3){ -8.0f, 1.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    enum GAME_STATE {PAUSED, MENU, RUNNING};
    GAME_STATE G = MENU;
    std::vector<Item> items;
    for (int i = 1; i < 16; i++) {Item item = Item(4.0f*i, 0.0f, 4.0f*i); items.push_back(item);}
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
        CLEAR_COLOR = BLACK;
        for (int i = 0; i < items.size(); i++) if (isColliding(player,items[i])) player.last(camera,items[i]);//items.erase(items.begin()+i);
        player.draw(camera);
        EndMode3D();
        for (int i = 0; i < int(menu.size()); i++) {menu[i].draw();}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
