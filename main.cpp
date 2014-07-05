#include <iostream>
#include <hge.h>
#include <hgesprite.h>
#include <hgefont.h>
#include <math.h>       /* cos */
#include <windows.h>
#include "tinyxml/tinyxml.h"
#include "resource.h"

using namespace std;

LPTSTR icon;

HGE* hge = 0;

HTEXTURE    hex_texture;
hgeSprite** hex_sprites    = new hgeSprite*[16];
HTEXTURE    square_texture;
hgeSprite** square_sprites = new hgeSprite*[16];
HTEXTURE    pc_texture;
hgeSprite*  pc_sprite;
hgeSprite*  empty_sprite;
hgeFont*    font_arial12;
HTEXTURE    compass_texture;
hgeSprite*  compass_sprite;

int images = 16;

int zoom           = 3;
float origin_x     = 0.0f;
float origin_y     = 0.0f;
float origin_angle = 0.0f;

float zoom_values[10] = {0.125f, 0.25f, 0.5f, 1.0f, 2.0f};

int map_width  = 100;
int map_height = 100;

int** tile_states  = 0;
int** tile_visible = 0;///0 - не определено, 1 - видно, 2 - не видно
DWORD** tile_color = 0;
int** tile_images  = 0;

int work_mode = 0;
char* mode_names[3] = {"Редактор", "Свободный просмотр", "Исследование"};
char  flag_chars[3] = " +";

//float tile_width       = 16.0f;
//float tile_height      = 13.0f;
float tile_width       = 27.7128f;
float tile_height      = 24.0f;
float half_tile_width  = tile_width * 0.5f;
float half_tile_height = tile_height * 0.5f;
bool hex_mode = true;

int window_width  = 1024;
int window_height = 768;
int half_window_width  = window_width * 0.5;
int half_window_height = window_height * 0.5;

float pc_x = 0.0f;
float pc_y = 0.0f;
float pc_a = 0.0f;
float pc_x_old = 0.0f;
float pc_y_old = 0.0f;
float pc_rotate_speed = 2.0f;
float pc_move_speed   = 60.0f;

float free_move_speed   = 1000.0f;
float free_rotate_speed = 1.0f;

bool show_help           = false;
bool show_compass        = true;
bool keep_pc_orientation = true;

DWORD left_color  = 0xFFAAAAAA;
DWORD right_color = 0xFFFFFFFF;

int left_tile_image  = 0;
int right_tile_image = 0;

int select_image;

float dt  = 0.0f;
float fps = 0.0f;

int loading_state = 0;

int GetLeftmostTile() {
    int x;
    if (origin_angle < M_PI_2) {/// Левый верхний угол
        x = ( cosf(-origin_angle) * origin_x / zoom_values[zoom] - sinf(-origin_angle) * origin_y / zoom_values[zoom] ) / tile_width - 1;
    }
    else if (origin_angle < M_PI) {/// Правый верхний
        x = ( cosf(-origin_angle) * (origin_x + window_width) / zoom_values[zoom] - sinf(-origin_angle) * origin_y / zoom_values[zoom] ) / tile_width - 1;
    }
    else if ( origin_angle < (M_PI + M_PI_2) ) {/// Правый нижний
        x = ( cosf(-origin_angle) * (origin_x + window_width) / zoom_values[zoom] - sinf(-origin_angle) * (window_height + origin_y) / zoom_values[zoom]  ) / tile_width - 1;
    }
    else {/// Левый нижний
        x = ( cosf(-origin_angle) * (origin_x / zoom_values[zoom]) - sinf(-origin_angle) * (window_height + origin_y) / zoom_values[zoom] ) / tile_width - 1;
    }

    if (x < 0) {
        return 0;
    } else if (x >= map_width) {
        return map_width;
    } else {
        return x;
    }
}
int GetRightmostTile() {
    int x;
    if (origin_angle < M_PI_2) {/// Правый нижний угол
        x = ( cosf(-origin_angle) * (origin_x + window_width) / zoom_values[zoom] - sinf(-origin_angle) * (window_height + origin_y) / zoom_values[zoom] ) / tile_width + 1;
    }
    else if (origin_angle < M_PI) {/// Левый нижний
        x = ( cosf(-origin_angle) * (origin_x / zoom_values[zoom]) - sinf(-origin_angle) * (window_height + origin_y) / zoom_values[zoom] ) / tile_width + 1;
    }
    else if ( origin_angle < (M_PI + M_PI_2) ) {/// Левый верхний
        x = ( cosf(-origin_angle) * origin_x / zoom_values[zoom] - sinf(-origin_angle) * origin_y / zoom_values[zoom] ) / tile_width + 1;
    }
    else {/// Правый верхний
        x = ( cosf(-origin_angle) * (origin_x + window_width) / zoom_values[zoom] - sinf(-origin_angle) * origin_y / zoom_values[zoom] ) / tile_width + 1;
    }
    if (x < 0) {
        return 0;
    } else if (x >= map_width) {
        return map_width;
    } else {
        return x;
    }
}
int GetTopmostTile() {
    int y;
    if (origin_angle < M_PI_2) {/// Правый верхний угол
        y = ( sinf(-origin_angle) * (origin_x + window_width) / zoom_values[zoom] + cosf(-origin_angle) * origin_y / zoom_values[zoom] ) / tile_height - 1;
    }
    else if (origin_angle < M_PI) {/// Правый нижний
        y = ( sinf(-origin_angle) * (origin_x + window_width) / zoom_values[zoom] + cosf(-origin_angle) * (window_height + origin_y) / zoom_values[zoom] ) / tile_height - 1;
    }
    else if ( origin_angle < (M_PI + M_PI_2) ) {/// Левый нижний
        y = ( sinf(-origin_angle) * origin_x / zoom_values[zoom] + cosf(-origin_angle) * (window_height + origin_y) / zoom_values[zoom] ) / tile_height - 1;
    }
    else {/// Левый верхний
        y = ( sinf(-origin_angle) * (origin_x / zoom_values[zoom]) + cosf(-origin_angle) * origin_y / zoom_values[zoom] ) / tile_height - 1;
    }
    if (y < 0) {
        return 0;
    } else if (y >= map_height) {
        return map_height;
    } else {
        return y;
    }
}
int GetBottommostTile() {
    int y;
    if (origin_angle < M_PI_2) {/// Левый нижний угол
        y = ( sinf(-origin_angle) * origin_x / zoom_values[zoom] + cosf(-origin_angle) * (window_height + origin_y) / zoom_values[zoom] ) / tile_height + 1;
    }
    else if (origin_angle < M_PI) {/// Левый верхний
        y = ( sinf(-origin_angle) * origin_x / zoom_values[zoom] + cosf(-origin_angle) * origin_y / zoom_values[zoom] ) / tile_height + 1;
    }
    else if ( origin_angle < (M_PI + M_PI_2) ) {///  Правый верхний
        y = ( sinf(-origin_angle) * (origin_x + window_width) / zoom_values[zoom] + cosf(-origin_angle) * origin_y / zoom_values[zoom] ) / tile_height + 1;
    }
    else {/// Правый нижний
        y = ( sinf(-origin_angle) * (origin_x + window_width) / zoom_values[zoom] + cosf(-origin_angle) * (window_height + origin_y) / zoom_values[zoom] ) / tile_height + 1;
    }
    if (y < 0) {
        return 0;
    } else if (y >= map_height) {
        return map_height;
    } else {
        return y;
    }
}

void GetPointedTile(float x, float y, int* tx, int* ty) {
    float map_x = x;
    float map_y = y;

    *ty = floor(map_y / tile_height);
    if ( (*ty % 2) && hex_mode) map_x -= half_tile_width;
    *tx = map_x / tile_width;
}

void DrawTile(int x, int y) {
    if (work_mode > 0 && tile_visible[x][y] != 1) return;
    float cx = ((x * tile_width)  + half_tile_width) * zoom_values[zoom];
    float cy = ((y * tile_height) + half_tile_height) * zoom_values[zoom];
    if ( (y % 2) && hex_mode) cx += half_tile_width * zoom_values[zoom];
    float c_angle = atan2f(cy, cx);
    float c_l = sqrtf(cx * cx + cy * cy);
    cx = cosf(c_angle + origin_angle) * c_l;
    cy = sinf(c_angle + origin_angle) * c_l;
    cx -= origin_x;
    cy -= origin_y;
    if (cx < -20 || cx > window_width + 20 || cy < -20 || cy > window_height + 20 ) return;

    hgeSprite** sprites = hex_mode ? hex_sprites : square_sprites;

    sprites[ tile_images[x][y] ]->SetColor(tile_color[x][y]);
    sprites[ tile_images[x][y] ]->RenderEx(cx, cy, origin_angle, zoom_values[zoom], zoom_values[zoom]);
}
void DrawMap() {
    for (int y = GetTopmostTile(); y < GetBottommostTile(); y++) {
        for (int x = GetLeftmostTile(); x < GetRightmostTile(); x++) {
            DrawTile(x, y);
        }
    }
}
void DrawPc() {
    float pc_angle = atan2f(pc_y, pc_x);
    float pc_l = sqrtf(pc_x * pc_x + pc_y * pc_y);
    float x = cosf(pc_angle + origin_angle) * pc_l * zoom_values[zoom];
    float y = sinf(pc_angle + origin_angle) * pc_l * zoom_values[zoom];
    pc_sprite->RenderEx(x - origin_x, y - origin_y, pc_a + origin_angle, zoom_values[zoom], zoom_values[zoom]);
}
void DrawHelp() {
    empty_sprite->SetColor(0xAAAAAAAA);
    font_arial12->SetColor(0xFF000000);
    empty_sprite->RenderStretch(0, window_height - 20.0f, window_width, window_height);
    font_arial12->printf(10.0f, window_height - 18.0f, HGETEXT_LEFT, "(TAB): %s", mode_names[work_mode]);
    font_arial12->printf(200.0f, window_height - 18.0f, HGETEXT_LEFT, "C: [%c]", flag_chars[show_compass]);
    font_arial12->printf(230.0f, window_height - 18.0f, HGETEXT_LEFT, "O: [%c]", flag_chars[keep_pc_orientation]);
    font_arial12->printf(260.0f, window_height - 18.0f, HGETEXT_LEFT, "W%.*f%%", zoom > 0 ? 0 : 1, 100.0f * zoom_values[zoom]);
    font_arial12->printf(310.0f, window_height - 18.0f, HGETEXT_LEFT, "X");
    font_arial12->printf(340.0f, window_height - 18.0f, HGETEXT_LEFT, "Y");

    hgeSprite** sprites = hex_mode ? hex_sprites : square_sprites;
    sprites[ left_tile_image ]->SetColor(left_color);
    sprites[ left_tile_image ]->RenderEx(330.0f, window_height - 10.0f, 0, 0.5f);
    sprites[ right_tile_image ]->SetColor(right_color);
    sprites[ right_tile_image ]->RenderEx(359.0f, window_height - 10.0f, 0, 0.5f);
    font_arial12->SetColor(0xFF000000);
    if (show_help) {
        empty_sprite->SetColor(0xFFDDDDDD);
        font_arial12->SetColor(0xFF000000);
        empty_sprite->RenderStretch(window_width - 200.0f, 0.0f, window_width, window_height);
        if (work_mode == 0) {
            font_arial12->Render(window_width - 100.0f, 10.0f, HGETEXT_CENTER, "Редактор");
            font_arial12->Render(window_width - 190.0f, 30.0f, HGETEXT_LEFT,
                "Кнопки мыши: редактирование\nСтрелки: перемещение\nQ, E: Вращение(R: Сброс)\nКолесо: Масштаб\nS: сохранить, L: загрузить\nM: Смена режима hex/square\nЦвет и текстура выбираются\nкликом по соответствующим\nпиктограммам нижнего меню");
        }
        else if (work_mode == 1) {
            font_arial12->Render(window_width - 100.0f, 10.0f, HGETEXT_CENTER, "Свободный просмотр");
            font_arial12->Render(window_width - 190.0f, 30.0f, HGETEXT_LEFT, "Стрелки: перемещение\nQ, E: Вращение(R: Сброс)\nКолесо: Масштаб");
        }
        else if (work_mode == 2) {
            font_arial12->Render(window_width - 100.0f, 10.0f, HGETEXT_CENTER, "Режим исследования");
            font_arial12->Render(window_width - 190.0f, 30.0f, HGETEXT_LEFT, "O: сменить режим камеры");
            if (keep_pc_orientation) {
                font_arial12->Render(window_width - 190.0f, 45.0f, HGETEXT_LEFT, "Стрелки: перемещение\nи вращение");
            }
            else {
                font_arial12->Render(window_width - 190.0f, 45.0f, HGETEXT_LEFT, "Стрелки: перемещение\nQ, E: вращение(R: Сброс)");
            }
            font_arial12->Render(window_width - 190.0f, 75.0f, HGETEXT_LEFT, "Колесо: Масштаб");
        }
        font_arial12->printf(window_width - 100.0f, window_height - 40.0f, HGETEXT_CENTER, "FPS: %.4f", fps);
        font_arial12->printf(window_width - 100.0f, window_height - 60.0f, HGETEXT_CENTER, "ox: %.0f oy: %.0f", origin_x, origin_y);
        font_arial12->printf(window_width - 100.0f, window_height - 80.0f, HGETEXT_CENTER, "F1: Инфо");
    }
    else {

    }
    font_arial12->Render(window_width - 100.0f, window_height - 18.0f, HGETEXT_CENTER, "H: Помощь");

    if (select_image) {
        int offset = 300;
        DWORD color = left_color;
        int image = left_tile_image;
        if (select_image == 2) {
            offset = 330;
            color = right_color;
            image = right_tile_image;
        }
        empty_sprite->SetColor(0xFF000000);
        empty_sprite->RenderStretch(offset, window_height - 60 - 32, offset + 8 + (32 + 2) * images, window_height - 25);
        empty_sprite->SetColor(0xFFDDDDDD);
        empty_sprite->RenderStretch(offset + 1, window_height - 59 - 32, offset + 7 + (32 + 2) * images, window_height - 26);
        empty_sprite->SetColor(0xFF000000);
        empty_sprite->RenderStretch(offset + 5, window_height - 50, offset + (32 + 2) * images + 3, window_height - 30);
        empty_sprite->RenderStretch(offset + 5 + (32 + 2) * image, window_height - 87, offset + 37 + (32 + 2) * image, window_height - 55);
        empty_sprite->SetColor(color);
        empty_sprite->RenderStretch(offset + 6, window_height - 49, offset + (32 + 2) * images + 2, window_height - 31);
        empty_sprite->SetColor(0xFFAAFFAA);
        empty_sprite->RenderStretch(offset + 6 + (32 + 2) * image, window_height - 86, offset + 36 + (32 + 2) * image, window_height - 56);

        for (int i = 0; i < images; i++) {
            sprites[i]->SetColor(color);
            sprites[i]->Render(offset + 5 + 16 + (32 + 2) * i, window_height - 55 - 16);
        }
    }
}
void DrawCompass() {
    compass_sprite->RenderEx(window_width - 40.0f, window_height - 60.0f, origin_angle, 1, 1);
}

void SetTileState(int x, int y, int state, int image, DWORD color) {
    if (x < 0 || x >= map_width || y < 0 || y >= map_width) return;
    tile_states[x][y] = state;
    tile_images[x][y] = image;
    tile_color[x][y]  = color;
}

void ResetVisibility() {
    for (int y = GetTopmostTile(); y < GetBottommostTile(); y++) {
        for (int x = GetLeftmostTile(); x < GetRightmostTile(); x++) {
            tile_visible[x][y] = false;
        }
    }
}
void TestVisibility(float cx, float cy, int r) {
    int tile_x, tile_y;
    GetPointedTile(cx, cy, &tile_x, &tile_y);
    if (tile_x < 0 || tile_x >= map_width || tile_y < 0 || tile_y > map_height) return;
    float pixel_r    = r * tile_width;
    float step       = half_tile_height - 1.0f;
    float angle_step = 0.5f * step / pixel_r;

    ResetVisibility();
    tile_visible[tile_x][tile_y] = 1;

//    int cx = (tile_x * tile_width)  + half_tile_width;
//    int cy = (tile_y * tile_height) + half_tile_height;

    for (float angle = 0.0f; angle < 2 * M_PI; angle += angle_step) {
        float x = cx;
        float y = cy;
        float display_x = x;
        float length = 0.0f;
        float step_x = step * cosf(angle);
        float step_y = step * sinf(angle);
        while (length < pixel_r) {
            x += step_x;
            y += step_y;
            length += step;

            int tile_y = y / tile_height;
            if ( (tile_y % 2) && hex_mode) display_x = x - half_tile_width; else display_x = x;
            int tile_x = display_x / tile_width;
            if (tile_x < 0 || tile_x >= map_width || tile_y < 0 || tile_y > map_height) break;
            tile_visible[tile_x][tile_y] = 1;
            if (tile_states[tile_x][tile_y] == 1) break;
//            if (tile_visible[tile_x][tile_y] == 0) {
//                tile_visible[tile_x][tile_y] = 1;
//                if (tile_states[tile_x][tile_y] == 1) break;
//            } else if (tile_visible[tile_x][tile_y] == 2) {
//                break;
//            }
        }
    }
}

void ControlRotation() {
    if ( hge->Input_GetKeyState(HGEK_Q) ) {
        float cx = origin_x + half_window_width;
        float cy = origin_y + half_window_height;
        float l = sqrtf(cx * cx + cy * cy);
        float a = atan2f(cy, cx) - origin_angle;
        origin_angle += free_rotate_speed * dt;
        origin_x = cosf(origin_angle + a) * l - half_window_width;
        origin_y = sinf(origin_angle + a) * l - half_window_height;
    }
    if ( hge->Input_GetKeyState(HGEK_E) ) {
        float cx = origin_x + half_window_width;
        float cy = origin_y + half_window_height;
        float l = sqrtf(cx * cx + cy * cy);
        float a = atan2f(cy, cx) - origin_angle;
        origin_angle -= free_rotate_speed * dt;
        origin_x = l * cosf(origin_angle + a) - half_window_width;
        origin_y = l * sinf(origin_angle + a) - half_window_height;
    }

    if (origin_angle < 0)        origin_angle += 2 * M_PI;
    if (origin_angle > 2 * M_PI) origin_angle -= 2 * M_PI;
}

void SetMode(bool _hex_mode) {
    hex_mode = _hex_mode;

    if (hex_mode) {
        tile_width       = 27.7128f;
        tile_height      = 24.0f;
    }
    else {
        tile_width       = 24.0f;
        tile_height      = 24.0f;
    }
    half_tile_width  = tile_width * 0.5f;
    half_tile_height = tile_height * 0.5f;
}

void SaveMap(char* filename) {
    printf("saving map %s ... \n", filename);
    TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
	doc.LinkEndChild( decl );
	TiXmlElement* root = new TiXmlElement( "map" );
	root->SetAttribute("width", map_width);
	root->SetAttribute("height", map_height);
	root->SetAttribute("mode", hex_mode);

	for (int y = 0; y < map_height; y++) {
        for (int x = 0; x < map_width; x++) {
            TiXmlElement* element = new TiXmlElement( "tile" );
            root->LinkEndChild( element );
            element->SetAttribute("x", x);
            element->SetAttribute("y", y);
            element->SetAttribute("state", tile_states[x][y]);
            element->SetAttribute("color", tile_color[x][y]);
            element->SetAttribute("image", tile_images[x][y]);
        }
    }

	doc.LinkEndChild( root );
	doc.SaveFile(filename);
	printf("done\n");
}
void LoadMap(char* filename) {
    delete tile_states;
    delete tile_visible;
    delete tile_color;
    delete tile_images;

	printf("loading map %s ... \n", filename);
    TiXmlDocument doc(filename);
    bool loadOkay = doc.LoadFile();
    if (loadOkay) {
    	TiXmlElement* root = doc.FirstChildElement("map");

        if (root->Attribute("width")) {
			map_width = atof(root->Attribute("width"));
		}
    	if (root->Attribute("height")) {
			map_height = atof(root->Attribute("height"));
		}
		if (root->Attribute("mode")) {
			SetMode(atoi(root->Attribute("mode")));
		}
		else {
            SetMode(true);
		}
		tile_states  = new int*[map_width];
        tile_visible = new int*[map_width];
        tile_color   = new DWORD*[map_width];
        tile_images  = new int*[map_width];
        for (int i = 0; i < map_width; i++) {
            tile_states[i]  = new int[map_height];
            tile_visible[i] = new int[map_height];
            tile_color[i]   = new DWORD[map_height];
            tile_images[i]  = new int[map_height];
        }

        for (int y = 0; y < map_height; y++) {
            for (int x = 0; x < map_width; x++) {
                tile_states[x][y] = 0;
            }
        }

        TiXmlElement* element = root->FirstChildElement("tile");
        int i = 0;
        while (element) {
        	int x = atoi(element->Attribute("x"));
			int y = atoi(element->Attribute("y"));
			tile_states[x][y] = atoi(element->Attribute("state"));
			tile_color[x][y]  = element->Attribute("color") ? atoi(element->Attribute("color")) : tile_states[x][y] ? 0xFFAAAAAA : 0xFFFFFFFF;
			tile_images[x][y]  = element->Attribute("image") ? atoi(element->Attribute("image")) : 0;
            element = element->NextSiblingElement("tile");
        }
        printf("done\n");
    } else {
        printf("failed\n");
    }
}

void Preload() {
    hex_texture = hge->Texture_Load("hex.png");
    hex_sprites[0]  = new hgeSprite(hex_texture, 0.0f, 0.0f, 32.0f, 32.0f);
    hex_sprites[1]  = new hgeSprite(hex_texture, 32.0f, 0.0f, 32.0f, 32.0f);
    hex_sprites[2]  = new hgeSprite(hex_texture, 64.0f, 0.0f, 32.0f, 32.0f);
    hex_sprites[3]  = new hgeSprite(hex_texture, 96.0f, 0.0f, 32.0f, 32.0f);
    hex_sprites[4]  = new hgeSprite(hex_texture, 0.0f, 32.0f, 32.0f, 32.0f);
    hex_sprites[5]  = new hgeSprite(hex_texture, 32.0f, 32.0f, 32.0f, 32.0f);
    hex_sprites[6]  = new hgeSprite(hex_texture, 64.0f, 32.0f, 32.0f, 32.0f);
    hex_sprites[7]  = new hgeSprite(hex_texture, 96.0f, 32.0f, 32.0f, 32.0f);
    hex_sprites[8]  = new hgeSprite(hex_texture, 0.0f, 64.0f, 32.0f, 32.0f);
    hex_sprites[9]  = new hgeSprite(hex_texture, 32.0f, 64.0f, 32.0f, 32.0f);
    hex_sprites[10] = new hgeSprite(hex_texture, 64.0f, 64.0f, 32.0f, 32.0f);
    hex_sprites[11] = new hgeSprite(hex_texture, 96.0f, 64.0f, 32.0f, 32.0f);
    hex_sprites[12] = new hgeSprite(hex_texture, 0.0f, 96.0f, 32.0f, 32.0f);
    hex_sprites[13] = new hgeSprite(hex_texture, 32.0f, 96.0f, 32.0f, 32.0f);
    hex_sprites[14] = new hgeSprite(hex_texture, 64.0f, 96.0f, 32.0f, 32.0f);
    hex_sprites[15] = new hgeSprite(hex_texture, 96.0f, 96.0f, 32.0f, 32.0f);

    square_texture = hge->Texture_Load("square.png");
    square_sprites[0]  = new hgeSprite(square_texture, 0.0f, 0.0f, 32.0f, 32.0f);
    square_sprites[1]  = new hgeSprite(square_texture, 32.0f, 0.0f, 32.0f, 32.0f);
    square_sprites[2]  = new hgeSprite(square_texture, 64.0f, 0.0f, 32.0f, 32.0f);
    square_sprites[3]  = new hgeSprite(square_texture, 96.0f, 0.0f, 32.0f, 32.0f);
    square_sprites[4]  = new hgeSprite(square_texture, 0.0f, 32.0f, 32.0f, 32.0f);
    square_sprites[5]  = new hgeSprite(square_texture, 32.0f, 32.0f, 32.0f, 32.0f);
    square_sprites[6]  = new hgeSprite(square_texture, 64.0f, 32.0f, 32.0f, 32.0f);
    square_sprites[7]  = new hgeSprite(square_texture, 96.0f, 32.0f, 32.0f, 32.0f);
    square_sprites[8]  = new hgeSprite(square_texture, 0.0f, 64.0f, 32.0f, 32.0f);
    square_sprites[9]  = new hgeSprite(square_texture, 32.0f, 64.0f, 32.0f, 32.0f);
    square_sprites[10] = new hgeSprite(square_texture, 64.0f, 64.0f, 32.0f, 32.0f);
    square_sprites[11] = new hgeSprite(square_texture, 96.0f, 64.0f, 32.0f, 32.0f);
    square_sprites[12] = new hgeSprite(square_texture, 0.0f, 96.0f, 32.0f, 32.0f);
    square_sprites[13] = new hgeSprite(square_texture, 32.0f, 96.0f, 32.0f, 32.0f);
    square_sprites[14] = new hgeSprite(square_texture, 64.0f, 96.0f, 32.0f, 32.0f);
    square_sprites[15] = new hgeSprite(square_texture, 96.0f, 96.0f, 32.0f, 32.0f);
    for (int i = 0; i < images; i++) {
        hex_sprites[i]->SetHotSpot(16.0f, 16.0f);
        square_sprites[i]->SetHotSpot(16.0f, 16.0f);
    }

    pc_texture = hge->Texture_Load("pc.png");
    pc_sprite = new hgeSprite(pc_texture, 0.0f, 0.0f, 16.0f, 16.0f);
    pc_sprite->SetHotSpot(8.0f, 8.0f);

    compass_texture = hge->Texture_Load("compass.png");
    compass_sprite = new hgeSprite(compass_texture, 0.0f, 0.0f, 64.0f, 64.0f);
    compass_sprite->SetHotSpot(32.0f, 32.0f);

    empty_sprite = new hgeSprite(0, 0.0f, 0.0f, 1.0f, 1.0f);

    font_arial12 = new hgeFont("arial12.fnt");

    tile_states  = new int*[map_width];
    tile_visible = new int*[map_width];
    tile_color   = new DWORD*[map_width];
    tile_images  = new int*[map_width];
    for (int i = 0; i < map_width; i++) {
        tile_states[i]  = new int[map_height];
        tile_visible[i] = new int[map_height];
        tile_color[i]   = new DWORD[map_height];
        tile_images[i]  = new int[map_height];
    }

    for (int y = 0; y < map_height; y++) {
        for (int x = 0; x < map_width; x++) {
            tile_states[x][y] = 0;
            tile_color[x][y]  = 0xFFFFFFFF;
            tile_images[x][y] = 0;
        }
    }
}

bool FrameFunc() {
	// By returning "true" we tell HGE
	// to stop running the application.
	if ( hge->Input_GetKeyState(HGEK_ESCAPE) ) return true;
	if ( hge->Input_KeyDown(HGEK_H) )  show_help = !show_help;
	if ( hge->Input_KeyDown(HGEK_C) )  show_compass = !show_compass;
	if ( hge->Input_KeyDown(HGEK_O) )  keep_pc_orientation = !keep_pc_orientation;
	if ( hge->Input_KeyDown(HGEK_M) && work_mode == 0 )  SetMode(!hex_mode);
	if ( hge->Input_KeyDown(HGEK_R) ) {
        float cx = origin_x + half_window_width;
        float cy = origin_y + half_window_height;
        float l = sqrtf(cx * cx + cy * cy);
        float a = atan2f(cy, cx) - origin_angle;
        origin_angle = 0.0f;
        origin_x = cosf(origin_angle + a) * l - half_window_width;
        origin_y = sinf(origin_angle + a) * l - half_window_height;
	}
	if (hge->Input_GetMouseWheel() > 0 && zoom < 4) {
        zoom++;
        origin_x *= zoom_values[zoom] / zoom_values[zoom - 1];
        origin_y *= zoom_values[zoom] / zoom_values[zoom - 1];
        origin_x += half_window_width;
        origin_y += half_window_height;
	}
	else if (hge->Input_GetMouseWheel() < 0 && zoom > 0) {
        zoom--;
        origin_x *= zoom_values[zoom] / zoom_values[zoom + 1];
        origin_y *= zoom_values[zoom] / zoom_values[zoom + 1];
        origin_x -= half_window_width * (zoom_values[zoom] / zoom_values[zoom + 1]);
        origin_y -= half_window_height * (zoom_values[zoom] / zoom_values[zoom + 1]);
	}

	if ( hge->Input_GetKeyState(HGEK_F1) ) {
        MessageBox(NULL,
           "Dungeon Wanderer v0.2.\n© Владимир Затолока 2014.\nPowered by HGE (hge.relishgames.com)\nОтзывы и предложения слать по адресу: me@spidamoo.ru",
           "Info",
           MB_OK | MB_ICONINFORMATION | MB_APPLMODAL
       );
	}

	fps = hge->Timer_GetFPS();
	dt = hge->Timer_GetDelta();

	if (work_mode == 0 || work_mode == 1) {
        if ( hge->Input_GetKeyState(HGEK_LEFT) )  origin_x     -= free_move_speed * dt;
        if ( hge->Input_GetKeyState(HGEK_RIGHT) ) origin_x     += free_move_speed * dt;
        if ( hge->Input_GetKeyState(HGEK_UP) )    origin_y     -= free_move_speed * dt;
        if ( hge->Input_GetKeyState(HGEK_DOWN) )  origin_y     += free_move_speed * dt;

        ControlRotation();

        float mx, my;
        float mouse_x, mouse_y;
        hge->Input_GetMousePos(&mouse_x, &mouse_y);
        mx = mouse_x + origin_x;
        my = mouse_y + origin_y;

        mx /= zoom_values[zoom];
        my /= zoom_values[zoom];

        float m_angle = atan2f(my, mx);
        float m_l = sqrtf(mx * mx + my * my);
        float x = cosf(m_angle - origin_angle) * m_l;
        float y = sinf(m_angle - origin_angle) * m_l;

        if (work_mode == 0) {
            if (mouse_y < window_height - 20 && (!show_help || mouse_x < window_width - 200) && select_image == 0 ) {
                if ( hge->Input_GetKeyState(HGEK_LBUTTON) ) {
                    int tile_x, tile_y;
                    GetPointedTile(x, y, &tile_x, &tile_y);
                    SetTileState(tile_x, tile_y, 1, left_tile_image, left_color);
                }
                if ( hge->Input_GetKeyState(HGEK_RBUTTON) ) {
                    int tile_x, tile_y;
                    GetPointedTile(x, y, &tile_x, &tile_y);
                    SetTileState(tile_x, tile_y, 0, right_tile_image, right_color);
                }
            }
            if (mouse_y > window_height - 20) {
                if (mouse_x > 310 && mouse_x < 340) {
                    if ( hge->Input_KeyDown(HGEK_LBUTTON) ) {
                        if (select_image == 1) {
                            select_image = 0;
                        }
                        else {
                            select_image = 1;
                        }
                    }
                }
                else if (mouse_x > 340 && mouse_x < 370) {
                    if ( hge->Input_KeyDown(HGEK_LBUTTON) ) {
                        if (select_image == 2) {
                            select_image = 0;
                        }
                        else {
                            select_image = 2;
                        }
                    }
                }
            }
            if (select_image) {
                int offset = 300;
                DWORD* color = &left_color;
                int* image = &left_tile_image;
                if (select_image == 2) {
                    offset = 330;
                    color = &right_color;
                    image = &right_tile_image;
                }
                if (hge->Input_KeyDown(HGEK_LBUTTON) && mouse_x > offset + 5 && mouse_y > window_height - 50 && mouse_x < offset + (32 + 2) * images + 3 && mouse_y < window_height - 30) {
                    CHOOSECOLOR cc;                 // common dialog box structure
                    static COLORREF acrCustClr[16]; // array of custom colors

                    DWORD red   = (*color & 0xFF0000) >> 16;
                    DWORD green = (*color & 0xFF00)   >> 8;
                    DWORD blue  = (*color & 0xFF);

                    // Initialize CHOOSECOLOR
                    ZeroMemory(&cc, sizeof(cc));
                    cc.lStructSize = sizeof(cc);
                    cc.hwndOwner = hge->System_GetState(HGE_HWND);
                    cc.lpCustColors = (LPDWORD) acrCustClr;

                    cc.rgbResult = red + (green << 8) + (blue << 16);
                    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

                    if ( ChooseColor(&cc) ) {
                        red   =  cc.rgbResult & 0xFF;
                        green = (cc.rgbResult & 0xFF00)   >> 8;
                        blue  = (cc.rgbResult & 0xFF0000) >> 16;

                        *color = 0xFF000000 + (red << 16) + (green << 8) + blue;
                    }
                }
                for (int i = 0; i < images; i++) {
                    if (hge->Input_KeyDown(HGEK_LBUTTON) && mouse_x > offset + 5 + (32 + 2) * i && mouse_y > window_height - 87
                        && mouse_x < offset + 37 + (32 + 2) * i && mouse_y < window_height - 55
                    ) {
                        *image = i;
                    }
                }
            }

            if ( hge->Input_KeyDown(HGEK_S) ) {
                loading_state = 1;
            }
            if (loading_state == 2) {
                OPENFILENAME ofn;
                char szFile[512]="\0";
                char szTemp[512];

                ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hge->System_GetState(HGE_HWND);
                ofn.lpstrFilter = "XML file\0*.xml\0All Files\0*.*\0\0";
                ofn.lpstrFile= szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.Flags = OFN_OVERWRITEPROMPT; // | OFN_NOCHANGEDIR
                ofn.lpstrDefExt="xml";
                if ( GetSaveFileName(&ofn) ) {
                    SaveMap(szFile);
                }
                loading_state = 0;
            }
            if ( hge->Input_KeyDown(HGEK_L) ) {
                loading_state = 3;
            }
            if (loading_state == 4) {
                OPENFILENAME ofn;
                char szFile[512]="\0";
                char szTemp[512];

                ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hge->System_GetState(HGE_HWND);
                ofn.lpstrFilter = "XML file\0*.xml\0All Files\0*.*\0\0";
                ofn.lpstrFile= szFile;
                ofn.nMaxFile = sizeof(szFile);
                //ofn.Flags = OFN_OVERWRITEPROMPT; // | OFN_NOCHANGEDIR
                ofn.lpstrDefExt="xml";
                if ( GetOpenFileName(&ofn) ) {
                    LoadMap(szFile);
                }
                loading_state = 0;
            }

            if ( hge->Input_KeyDown(HGEK_TAB) ) work_mode = 1;
        }
        else if (work_mode == 1) {
            TestVisibility(x, y, 20);

            if ( hge->Input_KeyDown(HGEK_TAB) ) {
                work_mode = 2;
                pc_x = x;
                pc_y = y;
            }
        }
    }
    else if (work_mode == 2) {
        pc_x_old = pc_x;
        pc_y_old = pc_y;
        if ( hge->Input_GetKeyState(HGEK_LEFT) )  pc_a -= pc_rotate_speed * dt;
        if ( hge->Input_GetKeyState(HGEK_RIGHT) ) pc_a += pc_rotate_speed * dt;
        if ( hge->Input_GetKeyState(HGEK_UP) ) {
            pc_x += pc_move_speed * cosf(pc_a) * dt;
            pc_y += pc_move_speed * sinf(pc_a) * dt;
        }
        if ( hge->Input_GetKeyState(HGEK_DOWN) ) {
            pc_x -= pc_move_speed * cosf(pc_a) * dt;
            pc_y -= pc_move_speed * sinf(pc_a) * dt;
        }
        if (pc_a > M_PI + M_PI_2) pc_a -= 2 * M_PI;
        if (pc_a < -M_PI_2)       pc_a += 2 * M_PI;
        if (keep_pc_orientation) {
            origin_angle = M_PI + M_PI_2 - pc_a;
        }
        else {
            ControlRotation();
        }
        int tile_x, tile_y;
        GetPointedTile(pc_x, pc_y, &tile_x, &tile_y);
        if (tile_x < 0 || tile_x >= map_width || tile_y < 0 || tile_y >= map_height || tile_states[tile_x][tile_y] == 1 ) {
            pc_x = pc_x_old;
            pc_y = pc_y_old;
        }
        float pc_angle = atan2f(pc_y, pc_x);
        float pc_l = sqrtf(pc_x * pc_x + pc_y * pc_y);
        float x = cosf(pc_angle + origin_angle) * pc_l * zoom_values[zoom];
        float y = sinf(pc_angle + origin_angle) * pc_l * zoom_values[zoom];
        origin_x = x - half_window_width;
        origin_y = y - half_window_height;

        TestVisibility(pc_x, pc_y, 20);

        if ( hge->Input_KeyDown(HGEK_TAB) ) work_mode = 0;
    }

	// Continue execution
	return false;
}
bool RenderFunc() {
	hge->Gfx_BeginScene();
	hge->Gfx_Clear(0);
	if (loading_state == 0) {
        DrawMap();
        DrawPc();
        if (show_compass) DrawCompass();
        DrawHelp();
	}
	if (loading_state == 1) {
        font_arial12->SetColor(0xFFFFFFFF);
        font_arial12->Render(
            half_window_width,
            half_window_height - 6.0f,
            HGETEXT_CENTER,
            "Карта сохраняется..."
        );
        loading_state = 2;
	}
	if (loading_state == 3) {
	    font_arial12->SetColor(0xFFFFFFFF);
        font_arial12->Render(
            half_window_width,
            half_window_height - 6.0f,
            HGETEXT_CENTER,
            "Карта загружается..."
        );
        loading_state = 4;
	}
	hge->Gfx_EndScene();

	return false;
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR, int) {
    icon = MAKEINTRESOURCE(THETA_ICON);

	hge = hgeCreate(HGE_VERSION);

	hge->System_SetState(HGE_INIFILE, "DungeonWanderer.ini");

	window_width  = hge->Ini_GetInt("HGE", "window_width", 800);
	window_height = hge->Ini_GetInt("HGE", "window_height", 600);
	map_width  = hge->Ini_GetInt("HGE", "map_width", 100);
	map_height = hge->Ini_GetInt("HGE", "map_height", 100);
	half_window_width  = window_width * 0.5;
    half_window_height = window_height * 0.5;

	hge->System_SetState(HGE_FRAMEFUNC,    FrameFunc);
	hge->System_SetState(HGE_RENDERFUNC,   RenderFunc);
	hge->System_SetState(HGE_TITLE,        "Dungeon Wanderer v0.4");
	hge->System_SetState(HGE_ICON,         icon);
	hge->System_SetState(HGE_WINDOWED,     true);
	hge->System_SetState(HGE_SCREENWIDTH,  window_width);
	hge->System_SetState(HGE_SCREENHEIGHT, window_height);
	hge->System_SetState(HGE_USESOUND,     false);
	hge->System_SetState(HGE_HIDEMOUSE,    false);
	hge->System_SetState(HGE_FPS,          60);

	if ( hge->System_Initiate() ) {
        Preload();
        SetMode(false);
		hge->System_Start();
	}
	else {
		MessageBox(NULL, hge->System_GetErrorMessage(), "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
	}

	hge->System_Shutdown();
	hge->Release();

	return 0;
}
