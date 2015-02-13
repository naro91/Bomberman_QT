#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>
//#include <time.h>
#include <SFML/Graphics.hpp>

using namespace sf;

#define amountWidth 23
#define amountHeight 13
#define Width 1265
#define Heigth 715
#define numberBot 6

class Element {
public:
    float currentFrame;
    Sprite sprite;
    std::string *TileMap;
    Element (Texture & texture) {
        TileMap = NULL;
        sprite.setTexture(texture);
        currentFrame = 0;
    }
    Element (Texture & texture, std::string *Map) {
        TileMap = Map;
        sprite.setTexture(texture);
        currentFrame = 0;
    }
};

class KeyAndDoor {
public:
    std::string *TileMap;
    int iKey;
    int jKey;
    int iDoor;
    int jDoor;
    void randomSetPosition();
    KeyAndDoor(std::string *Map) {
        TileMap = Map;
        iKey = jKey = iDoor = jDoor = 0;
    }
};

class Bot : public Element {
public:
    float dx[numberBot], dy[numberBot];
    float currentFrame[numberBot];
    FloatRect rect[numberBot];
    int i[numberBot];
    int j[numberBot];
    int dir[numberBot];
    bool status[numberBot];
    bool changeDir[numberBot];
    inline void randomSetPosition();
    inline void moveLeft(float &time, int k);
    inline void moveRight(float &time, int k);
    inline void moveUp(float &time, int k);
    inline void moveDown(float &time, int k);
    inline void discretization(float &time, int k);
    inline void collisionDetection(int k);
    inline void moveBot(RenderWindow &window, float &time);
    Bot (Texture& texture, std::string *Map) : Element(texture, Map) {
        for (int k = 0; k < numberBot; k++) {
            dx[k] = dy[k] = 0;
            rect[k] = FloatRect(0, 0, 40, 50);
            i[k] = 0;
            j[k] = 0;
            dir[k] = 1;
            currentFrame[k] = 0;
            changeDir[k] = false;
            status[k] = true;
        }
    }
};

class Bomb : public Element {
public:
    int i;
    int j;
    bool status;
    FloatRect rect;
    Bot *bot;
    inline void bombAction (RenderWindow &window, FloatRect &rectPlayer, bool &statusPlayer, float &time);
    inline void setBomb (int i, int j);
    inline void destructionByExplosion (RenderWindow &window, FloatRect &rectPlayer, bool &statusPlayer);
    inline void changeStatus(FloatRect &rectPlayer);
    Bomb (Texture & texture, Bot *b,  std::string *Map) : Element(texture, Map) {
        i = 0;
        j = 0;
        status = false;
        rect = FloatRect(0,0,0,0);
        bot = b;
    }
};

class Player : public Element {
public:
    float dx, dy; // скорость движения игрока
    FloatRect rect;
    bool status;
    Bomb *bomb;
    Bot *bot;
    inline void moveLeft(float &time);
    inline void moveRight(float &time);
    inline void moveUp(float &time);
    inline void moveDown(float &time);
    inline void discretization(float &time);
    inline void collisionDetection();
    inline void botCollisionDetection();
    inline void movePlayer(float &time); // отслеживает нажатие клавиш и отрисовывает игрока с анимацией движения
    Player (Texture &texture, Bomb *b, Bot * bt, std::string *Map) : Element(texture, Map) {
        dx = dy = 0;
        bomb = b;
        bot = bt;
        status = true;
        rect = FloatRect(110, 495, 40, 50);
        sprite.setTextureRect(IntRect(500,0,40,50));
        sprite.setPosition(110, 495);
    }
};

class Arena : public Element {
public:
    Player *player;
    Bomb *bomb;
    Bot *bot;
    KeyAndDoor *key;
    std::ifstream map;
    int i;
    inline void drawArena (RenderWindow &window);
    Arena (Texture &texture) : Element(texture){
        TileMap = new std::string[18];
        bot = new Bot(texture,this->TileMap);
        bomb = new Bomb(texture, bot, this->TileMap);
        key = new KeyAndDoor(TileMap);
        map.open("arena.txt");
        i = 0;
        if(map.is_open()) {
            while(map) {
                std::getline(map, TileMap[i]);
                i++;
            }
        }
        map.close();
        key->randomSetPosition();
        bot->randomSetPosition();
        player = new Player(texture, bomb, bot, this->TileMap);
    }
    ~Arena() {
        delete player;
        delete bomb;
        delete bot;
        delete[] TileMap;
    }
};



int main() {
    Clock clock;
    float time;
    RenderWindow window(VideoMode(Width,Heigth), "BOMBERMAN");

    Texture texture;
    texture.loadFromFile("sprite.png");

    Arena arena(texture);
    while (window.isOpen()) {
        time = clock.getElapsedTime().asMicroseconds();
        clock.restart();
        time /= 800;
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event:: Closed) {
                window.close();
            }
        }

        arena.player->movePlayer(time);

        window.clear();
        arena.drawArena(window);
        arena.bomb->bombAction(window,arena.player->rect,arena.player->status, time);
        arena.bot->moveBot(window,time);
        window.draw(arena.player->sprite);
        window.display();
    }
    return 0;
}

void KeyAndDoor::randomSetPosition() {
    srand(time(NULL));
    while (true) {
        iKey = rand()%13;
        jKey = rand()%23;
        if (TileMap[iKey][jKey] == ' ' && ((iKey != 8 && iKey != 9) || (jKey != 1 && jKey != 2 && jKey != 3) )) {
            TileMap[iKey][jKey] = 'c';
            break;
        }
    }
    while (true) {
        iDoor = rand()%13;
        jDoor = rand()%23;
        if (TileMap[iDoor][jDoor] == ' ' && ((iDoor != 8 && iDoor != 9) || (jDoor != 1 && jDoor != 2 && jDoor != 3) )) {
            TileMap[iDoor][jDoor] = 'v';
            break;
        }
    }
}

inline void Arena :: drawArena(RenderWindow &window) {
    for (int i = 0; i < amountHeight; i++) {
        for (int j = 0; j < amountWidth; j++) {
            if (TileMap[i][j] == 'P') sprite.setTextureRect(IntRect(0, 110, 55, 55));
            if (TileMap[i][j] == 'K' || TileMap[i][j] == 'c' || TileMap[i][j] == 'v') sprite.setTextureRect(IntRect(58, 110, 55, 55));
            if (TileMap[i][j] == 'C') sprite.setTextureRect(IntRect(58, 170, 55, 55));
            if (TileMap[i][j] == 'V') sprite.setTextureRect(IntRect(0, 170, 55, 55));
            if (TileMap[i][j] == ' ' || TileMap[i][j] == 'A' || TileMap[i][j] == 'b' || TileMap[i][j] == 'B'
                    || TileMap[i][j] == 'D') {
                sprite.setTextureRect(IntRect(400, 200, 55, 55));
            }

            sprite.setPosition(j*55, i*55);
            window.draw(sprite);
        }
    }
}

inline void Bomb::setBomb(int i, int j) {
    if (!status) {
        this->i = i;
        this->j = j;
        TileMap[i][j] = 'b';
        status = true;
        rect.left = j * 55 + 7.5;
        rect.top = i * 55 + 2.5;
    }
}

inline void Bomb::changeStatus(FloatRect &rectPlayer) {
    if (status) {
        if (TileMap[i][j] != 'A' && (fabs(rect.left - rectPlayer.left) > 50
                                     || fabs(rect.top - rectPlayer.top) > 53)) {
            TileMap[i][j] = 'B';
        }
    }
}

inline void Bomb::destructionByExplosion(RenderWindow &window, FloatRect &rectPlayer, bool &statusPlayer) {
    for (int k = this->i; k < this->i+2; k++) {
        if (k < amountHeight) {
            if (TileMap[k][j] == 'P') break;
            if (TileMap[k][j] == 'c') TileMap[k][j] = 'C';
            if (TileMap[k][j] == 'v') TileMap[k][j] = 'V';
            if (TileMap[k][j] == 'K' || TileMap[k][j] == 'D' || TileMap[k][j] == ' ') {
                TileMap[k][j] = ' ';
                sprite.setTextureRect(IntRect(56*int(currentFrame),280,55,55));
                sprite.setPosition(j*55 + 10, k*55);
                window.draw(sprite);
            }
            for (int l = 0; l < numberBot; l++) {
                if (bot->status[l]) {
                    if (fabs(j*55 - bot->rect[l].left) < 27 && fabs(k*55 - bot->rect[l].top) < 27) {
                        bot->status[l] = false;
                        break;
                    }
                }
            }
            if (statusPlayer) {
                if (fabs(j*55 - rectPlayer.left) < 27 && fabs(k*55 - rectPlayer.top) < 27) {
                    statusPlayer = false;
                }
            }
        }
    }
    for (int k = this->i-1; k > this->i-2; k--) {
        if (k >= 0) {
            if (TileMap[k][j] == 'P') break;
            if (TileMap[k][j] == 'c') TileMap[k][j] = 'C';
            if (TileMap[k][j] == 'v') TileMap[k][j] = 'V';
            if (TileMap[k][j] == 'K' || TileMap[k][j] == 'D' || TileMap[k][j] == ' ') {
                TileMap[k][j] = ' ';
                sprite.setTextureRect(IntRect(56*int(currentFrame),280,55,55));
                sprite.setPosition(j*55 + 10, k*55);
                window.draw(sprite);
            }
            for (int l = 0; l < numberBot; l++) {
                if (bot->status[l]) {
                    if (fabs(j*55 - bot->rect[l].left) < 27 && fabs(k*55 - bot->rect[l].top) < 27) {
                        bot->status[l] = false;
                        break;
                    }
                }
            }
            if (statusPlayer) {
                if (fabs(j*55 - rectPlayer.left) < 27 && fabs(k*55 - rectPlayer.top) < 27) {
                    statusPlayer = false;
                }
            }
        }
    }
    for (int j = this->j; j < this->j+2; j++) {
        if (j < amountWidth) {
            if (TileMap[i][j] == 'P') break;
            if (TileMap[i][j] == 'c') TileMap[i][j] = 'C';
            if (TileMap[i][j] == 'v') TileMap[i][j] = 'V';
            if (TileMap[i][j] == 'K' || TileMap[i][j] == 'D' || TileMap[i][j] == ' ') {
                TileMap[i][j] = ' ';
                sprite.setTextureRect(IntRect(56*int(currentFrame),280,55,55));
                sprite.setPosition(j*55 + 10, i*55);
                window.draw(sprite);
            }
            for (int l = 0; l < numberBot; l++) {
                if (bot->status[l]) {
                    if (fabs(j*55 - bot->rect[l].left) < 27 && fabs(i*55 - bot->rect[l].top) < 27) {
                        bot->status[l] = false;
                        break;
                    }
                }
            }
            if (statusPlayer) {
                if (fabs(j*55 - rectPlayer.left) < 27 && fabs(i*55 - rectPlayer.top) < 27) {
                    statusPlayer = false;
                }
            }
        }
    }
    for (int j = this->j-1; j > this->j-2; j--) {
        if (j >= 0) {
            if (TileMap[i][j] == 'P') break;
            if (TileMap[i][j] == 'c') TileMap[i][j] = 'C';
            if (TileMap[i][j] == 'v') TileMap[i][j] = 'V';
            if (TileMap[i][j] == 'K' || TileMap[i][j] == 'D' || TileMap[i][j] == ' ') {
                TileMap[i][j] = ' ';
                sprite.setTextureRect(IntRect(56*int(currentFrame),280,55,55));
                sprite.setPosition(j*55 + 10, i*55);
                window.draw(sprite);
            }
            for (int l = 0; l < numberBot; l++) {
                if (bot->status[l]) {
                    if (fabs(j*55 - bot->rect[l].left) < 27 && fabs(i*55 - bot->rect[l].top) < 27) {
                        bot->status[l] = false;
                        break;
                    }
                }
            }
            if (statusPlayer) {
                if (fabs(j*55 - rectPlayer.left) < 27 && fabs(i*55 - rectPlayer.top) < 27) {
                    statusPlayer = false;
                }
            }
        }
    }
}

inline void Bomb::bombAction(RenderWindow &window, FloatRect &rectPlayer, bool &statusPlayer,  float &time) {
    if (TileMap[i][j] == 'b' || TileMap[i][j] == 'B') {
        currentFrame += 0.001*time;
        if (currentFrame > 7) {
            currentFrame = 0;
            TileMap[i][j] = 'A';
        }
        sprite.setTextureRect(IntRect(56*int(currentFrame),58,55,55));
        sprite.setPosition(j*55 + 10, i*55);
        window.draw(sprite);
    }
    if (TileMap[i][j] == 'A' && status) {
        currentFrame += 0.01*time;
        if (currentFrame > 3) {
            currentFrame = 0;
            TileMap[i][j] = ' ';
            status = false;
        }
        sprite.setTextureRect(IntRect(56*int(currentFrame),280,55,55));
        sprite.setPosition(j*55 + 10, i*55);
        window.draw(sprite);
        destructionByExplosion(window,rectPlayer, statusPlayer);
    }
}


inline void Bot::randomSetPosition() {
    int k = 0;
    srand(time(NULL));
    while (k < numberBot) {
        i[k] = rand()%13;
        j[k] = rand()%23;
        if (TileMap[i[k]][j[k]] == ' ' && ((i[k] != 8 && i[k] != 9) || (j[k] != 1 && j[k] != 2 && j[k] != 3) )) {
            TileMap[i[k]][j[k]] = 'D';
            rect[k].left = j[k]*55;
            rect[k].top = i[k]*55;
            ++k;
        }
    }
}

inline void Bot::collisionDetection(int k) {
    for (int i = rect[k].top/55; i < (rect[k].top + rect[k].height)/55; i++) {
        for (int j = rect[k].left/55; j < (rect[k].left + rect[k].width)/55; j++) {
            if (i < amountHeight && j < amountWidth) {
                if(TileMap[i][j] == 'P' || TileMap[i][j] == 'K' || TileMap[i][j] == 'B'
                        || TileMap[i][j] == 'c' || TileMap[i][j] == 'v') {
                    if (dx[k] > 0) {
                        rect[k].left = j * 55 - rect[k].width;
                        dir[k] = rand()%4;
                        dx[k] = 0;
                    }
                    if (dx[k] < 0) {
                        rect[k].left = j * 55 + 55;
                        dir[k] = rand()%4;
                        dx[k] = 0;
                    }
                    if (dy[k] > 0) {
                        rect[k].top = i * 55 - rect[k].height;
                        dir[k] = rand()%4;
                        dy[k] = 0;
                    }
                    if (dy[k] < 0) {
                        rect[k].top = i * 55 + 55;
                        dir[k] = rand()%4;
                        dy[k] = 0;
                    }
                }
            }

        }
    }
}

inline void Bot::discretization(float &time, int k) {
    if (dx[k] < 0) {
        if (fabs( fabs(rect[k].left - (int(rect[k].left/55))*55) - 7.5 ) < 2 ) {
            dx[k] = 0;
            rect[k].left = int(rect[k].left/55)*55 + 7.5;
            sprite.setPosition(rect[k].left, rect[k].top);
            sprite.setTextureRect(IntRect(0,225,55,55));
            changeDir[k] = false;
        } else {
            moveLeft(time, k);
        }
    } else {
        if (dx[k] > 0) {
            if (fabs( fabs(rect[k].left - (int(rect[k].left/55))*55) - 7.5 ) < 2 ) {
                dx[k] = 0;
                rect[k].left = int(rect[k].left/55)*55 + 7.5;
                sprite.setPosition(rect[k].left, rect[k].top);
                sprite.setTextureRect(IntRect(0,225,55,55));
                changeDir[k] = false;
            } else {
                moveRight(time, k);
            }
        } else {
            if (dy[k] < 0) {
                if (fabs( fabs(rect[k].top - (int(rect[k].top/55))*55) - 2.5 ) < 2 ) {
                    dy[k] = 0;
                    rect[k].top = int(rect[k].top/55)*55 + 2.5;
                    sprite.setPosition(rect[k].left, rect[k].top);
                    sprite.setTextureRect(IntRect(0,225,55,55));
                    changeDir[k] = false;
                } else {
                    moveUp(time, k);
                }
            } else {
                if (dy[k] > 0) {
                    if (fabs( fabs(rect[k].top - (int(rect[k].top/55))*55) - 7.5 ) < 2 ) {
                        dy[k] = 0;
                        rect[k].top = int(rect[k].top/55)*55 + 2.5;
                        sprite.setPosition(rect[k].left, rect[k].top);
                        sprite.setTextureRect(IntRect(0,225,55,55));
                        changeDir[k] = false;
                    } else {
                        moveDown(time, k);
                    }
                } else changeDir[k] = 0;
            }
        }
    }
}

inline void Bot::moveLeft(float &time, int k) {
    dx[k] = -0.1;
    dy[k] = 0;
    rect[k].left += dx[k]*time;
    if (rect[k].left < 0) {
        rect[k].left = 0;
        dx[k] = 0;
        dir[k] = rand()%4;
    }
    currentFrame[k] += 0.005*time;
    if (rand()%201 == 200) {
        dir[k] = rand()%4;
        changeDir[k] = true;
    }
    if (currentFrame[k] > 2) currentFrame[k] -= 2;
    sprite.setTextureRect(IntRect(55*int(currentFrame[k]),225,55,55));
    sprite.setPosition(rect[k].left, rect[k].top);
}

inline void Bot::moveRight(float &time, int k) {
    dx[k] = 0.1;
    dy[k] = 0;
    rect[k].left += dx[k]*time;
    if (rect[k].left > Width - 40) {
        rect[k].left = Width - 40;
        dx[k] = 0;
        dir[k] = rand()%4;
    }
    currentFrame[k] += 0.005*time;
    if (rand()%201 == 200) {
        dir[k] = rand()%4;
        changeDir[k] = true;
    }
    if (currentFrame[k] > 2) currentFrame[k] -= 2;
    sprite.setTextureRect(IntRect(55*int(currentFrame[k]),225,55,55));
    sprite.setPosition(rect[k].left, rect[k].top);
}

inline void Bot::moveUp(float &time, int k) {
    dy[k] = -0.1;
    dx[k] = 0;
    rect[k].top += dy[k]*time;
    if (rect[k].top < 0) {
        rect[k].top = 0;
        dy[k] = 0;
        dir[k] = rand()%4;
    }
    currentFrame[k] += 0.005*time;
    if (rand()%201 == 200) {
        dir[k] = rand()%4;
        changeDir[k] = true;
    }
    if (currentFrame[k] > 2) currentFrame[k] -= 2;
    sprite.setTextureRect(IntRect(55*int(currentFrame[k]),225,55,55));
    sprite.setPosition(rect[k].left, rect[k].top);
}

inline void Bot::moveDown(float &time, int k) {
    dy[k] = 0.1;
    dx[k] = 0;
    rect[k].top += dy[k]*time;
    if (rect[k].top > Heigth - 50) {
        rect[k].top = Heigth - 50;
        dy[k] = 0;
        dir[k] = rand()%4;
    }
    currentFrame[k] += 0.005*time;
    if (rand()%201 == 200) {
        dir[k] = rand()%4;
        changeDir[k] = true;
    }
    if (currentFrame[k] > 2) currentFrame[k] -= 2;
    sprite.setTextureRect(IntRect(55*int(currentFrame[k]),225,55,55));
    sprite.setPosition(rect[k].left, rect[k].top);
}

inline void Bot::moveBot(RenderWindow &window, float &time) {
    int k = 0;
    while (k < numberBot) {
        if (status[k]) {
            if (dir[k] == 0 && !changeDir[k]) {
                moveLeft(time, k);
                ++k;
            } else {
                if (dir[k] == 1 && !changeDir[k]) {
                    moveRight(time, k);
                    ++k;
                } else {
                    if (dir[k] == 2 && !changeDir[k]) {
                        moveUp(time, k);
                        ++k;
                    } else {
                        if (dir[k] == 3 && !changeDir[k]) {
                            moveDown(time, k);
                            ++k;
                        } else {
                            if (changeDir[k]) {
                                discretization(time, k);
                                ++k;
                            }
                        }
                    }
                }
            }
            collisionDetection(k-1);
            window.draw(sprite);
        }else {
            ++k;
        }
    }
}


inline void Player::moveLeft(float &time) {
    dx = -0.2;
    dy = 0;
    rect.left += dx*time;
    if (rect.left < 0) {
        rect.left = 0;
        dx = 0;
    }
    currentFrame += 0.01*time;
    if (currentFrame > 2) currentFrame -= 2;
    sprite.setTextureRect(IntRect(60*int(currentFrame)+40,0,-40,50));
    sprite.setPosition(rect.left, rect.top);
    bomb->changeStatus(rect);
}

inline void Player::moveRight(float &time) {
    dx = 0.2;
    dy = 0;
    rect.left += dx*time;
    if (rect.left > Width - 40) {
        rect.left = Width - 40;
        dx = 0;
    }
    currentFrame += 0.01*time;
    if (currentFrame > 2) currentFrame -= 2;
    sprite.setTextureRect(IntRect(60*int(currentFrame),0,40,50));
    sprite.setPosition(rect.left, rect.top);
    bomb->changeStatus(rect);
}

inline void Player::moveUp(float &time) {
    dy = -0.2;
    dx = 0;
    rect.top += dy*time;
    if (rect.top < 0) {
        rect.top = 0;
        dy = 0;
    }
    currentFrame += 0.01*time;
    if (currentFrame > 2) currentFrame -= 2;
    sprite.setTextureRect(IntRect(60*int(currentFrame)+152,0,-40,50));
    sprite.setPosition(rect.left, rect.top);
    bomb->changeStatus(rect);
}

inline void Player::moveDown(float &time) {
    dy = 0.2;
    dx = 0;
    rect.top += dy*time;
    if (rect.top > Heigth - 50) {
        rect.top = Heigth - 50;
        dy = 0;
    }
    currentFrame += 0.01*time;
    if (currentFrame > 3) currentFrame -= 3;
    sprite.setTextureRect(IntRect(60*int(currentFrame)+263,0,-40,50));
    sprite.setPosition(rect.left, rect.top);
    bomb->changeStatus(rect);
}

inline void Player::discretization(float &time) {
    if (dx < 0) {
        if (fabs( fabs(rect.left - (int(rect.left/55))*55) - 7.5 ) < 2 ) {
            dx = 0;
            rect.left = int(rect.left/55)*55 + 7.5;
            sprite.setPosition(rect.left, rect.top);
            sprite.setTextureRect(IntRect(60+205,0,-40,50));
        } else {
            moveLeft(time);
        }
    } else {
        if (dx > 0) {
            if (fabs( fabs(rect.left - (int(rect.left/55))*55) - 7.5 ) < 2 ) {
                dx = 0;
                rect.left = int(rect.left/55)*55 + 7.5;
                sprite.setPosition(rect.left, rect.top);
                sprite.setTextureRect(IntRect(60+205,0,-40,50));
            } else {
                moveRight(time);
            }
        } else {
            if (dy < 0) {
                if (fabs( fabs(rect.top - (int(rect.top/55))*55) - 2.5 ) < 2 ) {
                    dy = 0;
                    rect.top = int(rect.top/55)*55 + 2.5;
                    sprite.setPosition(rect.left, rect.top);
                    sprite.setTextureRect(IntRect(60+205,0,-40,50));
                } else {
                    moveUp(time);
                }
            } else {
                if (dy > 0) {
                    if (fabs( fabs(rect.top - (int(rect.top/55))*55) - 7.5 ) < 2 ) {
                        dy = 0;
                        rect.top = int(rect.top/55)*55 + 2.5;
                        sprite.setPosition(rect.left, rect.top);
                        sprite.setTextureRect(IntRect(60+205,0,-40,50));
                    } else {
                        moveDown(time);
                    }
                } else {
                    sprite.setTextureRect(IntRect(60+205,0,-40,50));
                }
            }
        }
    }




}

inline void Player::collisionDetection() {
    for (int i = rect.top/55; i < (rect.top + rect.height)/55; i++) {
        for (int j = rect.left/55; j < (rect.left + rect.width)/55; j++) {
            if (i < amountHeight && j < amountWidth) {
                if(TileMap[i][j] == 'P' || TileMap[i][j] == 'K' || TileMap[i][j] == 'B'
                        || TileMap[i][j] == 'c' || TileMap[i][j] == 'v') {
                    if (dx > 0) {
                        rect.left = j * 55 - rect.width;
                        dx = 0;
                    }
                    if (dx < 0) {
                        rect.left = j * 55 + 55;
                        dx = 0;
                    }
                    if (dy > 0) {
                        rect.top = i * 55 - rect.height;
                        dy = 0;
                    }
                    if (dy < 0) {
                        rect.top = i * 55 + 55;
                        dy = 0;
                    }
                }
            }

        }
    }
}

inline void Player::botCollisionDetection() {
    for (int i = 0; i < numberBot; i++) {
        if (bot->status[i]) {
            if (fabs(rect.left - bot->rect[i].left) < 27 && fabs(rect.top - bot->rect[i].top) < 27) {
                status = false;
                break;
            }
        }
    }
}

inline void Player::movePlayer(float &time){
    if(status) {
        if (Keyboard::isKeyPressed(Keyboard::Left)) {
            moveLeft(time);
        } else {
            if (Keyboard::isKeyPressed(Keyboard::Right)) {
                moveRight(time);
            } else {
                if (Keyboard::isKeyPressed(Keyboard::Up)) {
                    moveUp(time);
                } else {
                    if (Keyboard::isKeyPressed(Keyboard::Down)) {
                        moveDown(time);
                    } else {
                        discretization(time);
                        if (Keyboard::isKeyPressed(Keyboard::Space)) {
                            bomb->setBomb(rect.top/55, rect.left/55);
                        }
                        if (Keyboard::isKeyPressed(Keyboard::Return)) {
                            if (bomb->status) {
                                TileMap[bomb->i][bomb->j] = 'A';
                                bomb->currentFrame = 0.0;
                            }
                        }
                    }
                }
            }
        }
        collisionDetection();
        botCollisionDetection();
    } else {
        sprite.setTextureRect(IntRect(400,0, 55,55));
        sprite.setPosition(rect.left, rect.top);
    }
}
