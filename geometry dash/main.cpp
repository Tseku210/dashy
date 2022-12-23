#include <GLUT/glut.h>
#include <GLUT/GLUT.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <string>

using namespace std;
void level();

int winHeight, winWidth;
unsigned int basewall, spike;
unsigned int character;
bool started = false;
string map;

int levelWidth, levelHeight;
float cameraX = 0.0f, cameraY = 0.0f;
float playerPosX = 0.0f, playerPosY = 5.0f;
float playerVelX = 2.0f, playerVelY = 0.0f;
bool playerOnGround = false;

unsigned int addBody(const char *filename)
{
    SDL_Surface *img = SDL_LoadBMP(filename);
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->w, img->h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, img->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SDL_FreeSurface(img);
    return id;
}

void reshape(int w, int h)
{
    if(h==0)
        h=1;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(w<=h)
    {
        winHeight=250*h/w;
        winWidth=250;
    }else
    {
        winWidth=250*w/h;
        winHeight=250;
    }
    glOrtho(0,winWidth,winHeight,0,1,-1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

char getObj(int x, int y) {
    if (x >= 0 && x < levelWidth && y >= 0 && y < levelWidth) {
        return map[y*levelWidth+x];
    } else
        return ' ';
}

void fill(unsigned int texture, float a, float b, float c, float d, float red, float green, float blue) {
    if (texture)
        glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
        glVertex3f(a, b, 0);
    glTexCoord2f(0.0f, 1.0f);
        glVertex3f(a, d, 0);
    glTexCoord2f(1.0f, 1.0f);
        glVertex3f(c, d, 0);
    glTexCoord2f(1.0f, 0.0f);
        glVertex3f(c, b, 0);
    glEnd();
}

void drawObstacle(float a, float b, float c, float d) {
    glBindTexture(GL_TEXTURE_2D, basewall);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.1f);
        glVertex3f(a, b, 0);
    glTexCoord2f(0.0f, 0.9f);
        glVertex3f(a, d, 0);
    glTexCoord2f(1.0f, 0.9f);
        glVertex3f(c, d, 0);
    glTexCoord2f(1.0f, 0.1f);
        glVertex3f(c, b, 0);
    glEnd();

}

void drawCharacter(float a, float b, float c, float d) {
    glBindTexture(GL_TEXTURE_2D, character);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
        glVertex3f(a, b, 0);
    glTexCoord2f(0.0, 1.0);
        glVertex3f(a, d, 0);
    glTexCoord2f(1.0, 1.0);
        glVertex3f(c, d, 0);
    glTexCoord2f(1.0, 0.0);
        glVertex3f(c, b, 0);
    glEnd();
}
void processSpecialKeys2(int key, int xx, int yy) {
    switch (key) {
        case 13:
            started = true;
            playerVelY = 0;
            break;
        case 32:
            if (playerOnGround)
                playerVelY = -4.0f;
            break;
        case 27:
            exit(1);
            break;
    }
}

void print(int x, int y, char *string) {
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2i(200, 100);
    glDisable(GL_TEXTURE);
    glDisable(GL_TEXTURE_2D);
    int i;
    for (i = 0; i < strlen(string); i++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)string[i]);
    }
    glEnable(GL_TEXTURE);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
}

void init() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE);
    levelWidth = 100;
    levelHeight = 15;
    level();
 
    basewall = addBody("../images/wall.bmp");
    spike = addBody("../images/spike.bmp");
    character = addBody("../images/dash.bmp");
}

void newRenderScene(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    
    int tileWidth = 16, tileHeight = 16;
    int visibleTileX = winWidth / tileWidth;
    int visibleTileY = winHeight / tileHeight;
    
    float offsetX = cameraX - visibleTileX / 2.0f;
    float offsetY = cameraY - visibleTileY / 2.0f;
    
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;
    if (offsetX > levelWidth - visibleTileX) offsetX = levelWidth - visibleTileX;
    if (offsetY > levelHeight - visibleTileY) offsetY = levelHeight - visibleTileY;
    
    float tileOffsetX = (offsetX - (int)offsetX)*tileWidth;
    float tileOffsetY = (offsetY - (int)offsetY)*tileHeight;
    
    if (started) {
        playerVelY += 1.0f * 0.2;
        
        float newPlayerPosX = playerPosX + playerVelX * 0.1;
        float newPlayerPosY = playerPosY + playerVelY * 0.1;
        
        if (playerVelX > 1.0f) {
            playerVelX = 1.0f;
        }
        if (playerVelX < -1.0f) {
            playerVelX = -1.0f;
        }
        if (playerVelY > 100.0f) {
            playerVelY = 100.0f;
        }
        if (playerVelY < -3.0f) {
            playerVelY = -3.0f;
        }
        
        if (playerVelX <= 0) {
            if (getObj(newPlayerPosX + 0.0f, playerPosY + 0.0f) == '|' || getObj(newPlayerPosX + 0.0f, playerPosY + 0.9f) == '|') {
                playerPosX = playerPosY = cameraX = cameraY = newPlayerPosX = newPlayerPosY = playerVelY = 0;
                started = false;
                newPlayerPosY = 5.0f;
            }
        }else {
            if (getObj(newPlayerPosX + 1.0f, playerPosY + 0.0f) == '|' || getObj(newPlayerPosX + 1.0f, playerPosY + 0.9f) == '|' ||
                getObj(newPlayerPosX + 1.0f, playerPosY + 0.0f) == '^' || getObj(newPlayerPosX + 1.0f, playerPosY + 0.9f) == '^' ||
                getObj(newPlayerPosX + 1.0f, playerPosY + 0.0f) == '*' || getObj(newPlayerPosX + 1.0f, playerPosY + 0.9f) == '*') {
                playerPosX = playerPosY = cameraX = cameraY = newPlayerPosX = newPlayerPosY = playerVelY = 0;
                started = false;
                newPlayerPosY = 5.0f;
            }
        }
        playerOnGround = false;
        if (playerVelY <= 0) {
            if (getObj(newPlayerPosX + 0.0f, newPlayerPosY) != '.' || getObj(newPlayerPosX + 0.9f, newPlayerPosY + 0.0f) != '.') {
                if (getObj(newPlayerPosX + 0.0f, newPlayerPosY) == '|' || getObj(newPlayerPosX + 0.9f, newPlayerPosY + 0.0f) == '|') {
                    
                }
                playerPosX = playerPosY = cameraX = cameraY = newPlayerPosX = newPlayerPosY = playerVelY = 0;
                started = false;
                newPlayerPosY = 5.0f;
            }
        } else {
            if (getObj(newPlayerPosX + 0.0f, newPlayerPosY + 1.0f) != '.' || getObj(newPlayerPosX + 0.9f, newPlayerPosY + 1.0f) != '.') {
                if (getObj(newPlayerPosX + 0.0f, newPlayerPosY + 1.0f) == '|' || getObj(newPlayerPosX + 0.9f, newPlayerPosY + 1.0f) == '|' ||
                    getObj(newPlayerPosX + 0.0f, newPlayerPosY + 1.0f) == '*' || getObj(newPlayerPosX + 0.9f, newPlayerPosY + 1.0f) == '*') {
                    playerPosX = playerPosY = cameraX = cameraY = newPlayerPosX = newPlayerPosY = playerVelY = 0;
                    started = false;
                    newPlayerPosY = 5.0f;
                } else {
                    newPlayerPosY = (int)newPlayerPosY;
                    playerVelY = 0;
                    playerOnGround = true;
                }
            }
        }
        
        playerPosX = newPlayerPosX;
        playerPosY = newPlayerPosY;
        
        if (getObj(playerPosX + 0.0f, playerPosY + 1.0f) == '$' || getObj(playerPosX + 1.0f, playerPosY + 1.0f) == '$' || getObj(playerPosX + 1.0f, playerPosY + 0.0f) == '$' || getObj(playerPosX + 0.0f, playerPosY + 0.0f) == '$'){
            playerPosX = playerPosY = cameraX = cameraY = newPlayerPosX = newPlayerPosY = 0;
            playerPosX = playerPosY = cameraX = cameraY = newPlayerPosX = newPlayerPosY = playerVelY = 0;
            started = false;
            newPlayerPosY = 5.0f;
        }
    } else {
        print(visibleTileX, visibleTileY, "Press Enter");
    }
    
    for (int x = -1; x < visibleTileX+1+1; x++) {  
        for (int y = -1; y < visibleTileY+1+1; y++) {
            char tileId = getObj(x+offsetX, y+offsetY);
            switch(tileId) {
            case '.':
                break;
            case '@':
                    fill(basewall, x * tileWidth - tileOffsetX, y * tileHeight - tileOffsetY, (x+1)*tileWidth - tileOffsetX, (y+1)*tileHeight - tileOffsetY, 0.8f, 0.0f, 0.0f);
                break;
            case '|':
                    drawObstacle(x * tileWidth - tileOffsetX, y * tileHeight - tileOffsetY, (x+1)*tileWidth - tileOffsetX, (y+1)*tileHeight - tileOffsetY);
                    break;
            case '*':
                    fill(spike, x * tileWidth - tileOffsetX, y * tileHeight - tileOffsetY, (x+1)*tileWidth - tileOffsetX, (y+1)*tileHeight - tileOffsetY, 0.8f, 0.0f, 0.0f);
                    break;
            case '^':
                    drawObstacle(x * tileWidth - tileOffsetX, y * tileHeight - tileOffsetY, (x+1)*tileWidth - tileOffsetX, (y+1)*tileHeight - tileOffsetY);
                    break;
            default:
                break;
            }
        }
    }
    drawCharacter((playerPosX- offsetX)*tileWidth, (playerPosY - offsetY)*tileHeight, (playerPosX - offsetX+1)*tileWidth, (playerPosY-offsetY+1)*tileHeight);
    
    cameraX = playerPosX;
    cameraY = playerPosY;
    glutSwapBuffers();
}
//endtest
void level() {
    map = "";
    map += "...................................................................................................$";
    map += "...................................................................................................$";
    map += "...................................................................................................$";
    map += "...................................................................................................$";
    map += "................................................................................................^..$";
    map += ".............................................................................................^..|..$";
    map += ".....................................................................................^....^..|..|..$";
    map += ".....................................................................................|.^..|..|..|..$";
    map += "......................^...........................................................^..|.|..|..|..|..$";
    map += "...................^..|..^..............^......^...............................^..|..|.|..|..|..|..$";
    map += "............*......|..|..|....*......^..|.^....|....^.....*....*...*...*....^..|..|..|.|..|..|..|..$";
    map += "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
    map += "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
    map += "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
    map += "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
    map += "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
    map += "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
    
    
}

void timerFunc(int state) {
    glutPostRedisplay();
    glutTimerFunc(1000/60, timerFunc, 0);
}
int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,200);
    glutInitWindowSize(1000,1000);
    glutCreateWindow("Dash2");
    glutDisplayFunc(newRenderScene);
    glutTimerFunc(1000/60, timerFunc, 0);
    glutSpecialFunc(processSpecialKeys2);
    glutReshapeFunc(reshape);
    glutIgnoreKeyRepeat(1);
    init();
    glutMainLoop();
}
