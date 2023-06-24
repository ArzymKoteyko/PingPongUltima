#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>

#define SCREEN_HEIGHT 30
#define SCREEN_WIDTH  100

struct termios orig_termios;

typedef enum {
    false,
    true,
} bool;

typedef enum {
    HORIZONTAL,
    VERTICAL
} orientation;

struct iVector {
    int x;
    int y;
};

struct dVector {
    double x;
    double y;
};

struct ball {
    struct dVector pos; // char
    struct dVector spd; // char/s
    char symbol;
};

struct wall {
    orientation type;
    struct iVector pos; // char [most up left corner]
    int size; // wall length 
};

void enable_raw_mode();
void disable_raw_mode();

int drawBall(struct ball* ball, char** windowBuffer);
int drawWall(struct wall*, char** windowBuffer);

int moveBall(struct ball*);

int updateScreen(char**);
char** initWindowBuffer();
int clearWindowBuffer(char**);

bool checkBallToWallColision(struct ball*, struct wall*);




int main() {
    bool isPlaying = true;
    char **windowBuffer = initWindowBuffer(); 
    struct ball ball = {{2, 2}, {0.1, 0.1}, 'l'};
    struct wall lvwall = {VERTICAL, {0,0}, SCREEN_HEIGHT};
    struct wall rvwall = {VERTICAL, {SCREEN_WIDTH-1,0}, SCREEN_HEIGHT};
    struct wall thwall = {HORIZONTAL, {0,0}, SCREEN_WIDTH};
    struct wall bhwall = {HORIZONTAL, {0,SCREEN_HEIGHT-1}, SCREEN_WIDTH};
    struct wall p1bar = {VERTICAL, {10,10}, 8};
    struct wall p2bar = {VERTICAL, {SCREEN_WIDTH-11,10}, 8};

    clearWindowBuffer(windowBuffer);
    printf("\e[?25l"); // Dis cursor
    
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(STDOUT_FILENO, F_SETFL, flags);
    enable_raw_mode();
    
    while(isPlaying) {

        char buffer[3];
        
        size_t bytesRead = read(STDIN_FILENO, &buffer, sizeof(buffer));
        if (bytesRead > 0) {
            switch(buffer[0]) {
                case 'q':
                    isPlaying = false;
                    break;
                case 'w':
                    p1bar.pos.y -= 2;
                    break;
                case 's':
                    p1bar.pos.y += 2;
                    break;
                case '\033':
                    switch(buffer[2]) {
                        case 'A':
                            p2bar.pos.y -= 2;
                            break;
                        case 'B':
                            p2bar.pos.y += 2;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break; 
            }
            memset(buffer, 0, sizeof(buffer));
        }
        
        clearWindowBuffer(windowBuffer);

        // Draw ruting
        drawBall(&ball, windowBuffer);
        drawWall(&lvwall, windowBuffer);
        drawWall(&rvwall, windowBuffer);
        drawWall(&thwall, windowBuffer);
        drawWall(&bhwall, windowBuffer);
        drawWall(&p1bar, windowBuffer);
        drawWall(&p2bar, windowBuffer);

        // Phisics rutin
        
        if (checkBallToWallColision(&ball, &p1bar)) {
            ball.spd.x = -ball.spd.x;
        }
        if (checkBallToWallColision(&ball, &p2bar)) {
            ball.spd.x = -ball.spd.x;
        }
        if (checkBallToWallColision(&ball, &lvwall)) {
            ball.spd.x = -ball.spd.x;
        }
        if (checkBallToWallColision(&ball, &rvwall)) {
            ball.spd.x = -ball.spd.x;
        }
        if (checkBallToWallColision(&ball, &thwall)) {
            ball.spd.y = -ball.spd.y;
        }
        if (checkBallToWallColision(&ball, &bhwall)) {
            ball.spd.y = -ball.spd.y;
        }
        
        moveBall(&ball);

        updateScreen(windowBuffer);
        usleep(5000);
    }
    disable_raw_mode();
    printf("\e[?25h");
    return 0;
}




int updateScreen(char **windowBuffer) {
    for (int y=0; y < SCREEN_HEIGHT; y++) {
        printf("\e[A");
    }
    for (int y=0; y<SCREEN_HEIGHT; y++) {
        for (int x=0; x<SCREEN_WIDTH; x++) {
            printf("%c", windowBuffer[x][y]);
        }
        printf("\n");
    }
    return 1; // Updated successfully
};

char** initWindowBuffer() {
    char **windowBuffer = (char**)malloc(SCREEN_WIDTH * sizeof(char*) + SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(char));
    char *ptr = (char*)(windowBuffer + SCREEN_WIDTH);
    for (int i=0; i<SCREEN_WIDTH; i++) {
        windowBuffer[i] = (ptr + SCREEN_HEIGHT * i);
    }
    return windowBuffer;
}

int clearWindowBuffer(char **windowBuffer) {
    for (int y=0; y<SCREEN_HEIGHT; y++) {
        for (int x= 0; x<SCREEN_WIDTH; x++) {
            windowBuffer[x][y] = ' ';
        }
    }
    return 1;
}

int drawBall(struct ball* ball, char** windowBuffer) {
    struct iVector point = {(int)ball->pos.x, (int)ball->pos.y};
    if (point.x >= 0 && point.x < SCREEN_WIDTH && point.y >= 0 && point.y < SCREEN_HEIGHT) {
        windowBuffer[(int)ball->pos.x][(int)ball->pos.y] = ball->symbol;
    }
    return 1; // Drawed successfully
};

int drawWall(struct wall* wall, char** windowBuffer) {
    switch (wall->type) {
        case HORIZONTAL:
            if (wall->pos.y >= 0 && wall->pos.y < SCREEN_HEIGHT) {
                for (int x=wall->pos.x; x<wall->pos.x+wall->size; x++) {
                    if (x >= 0 && x < SCREEN_WIDTH) {
                        windowBuffer[x][wall->pos.y] = '#';
                    }
                }
            }
        case VERTICAL:
            if (wall->pos.x >= 0 && wall->pos.x < SCREEN_WIDTH) {
                for (int y=wall->pos.y; y<wall->pos.y+wall->size; y++) {
                    if (y >= 0 && y < SCREEN_HEIGHT) {
                        windowBuffer[wall->pos.x][y] = '#';
                    }
                }
            }
        default:
            return 0; // Error
    }
    if (wall->pos.x > 0 && wall->pos.x < SCREEN_WIDTH && wall->pos.y > 0 && wall->pos.y < SCREEN_HEIGHT) {

    }
}

int moveBall(struct ball* ball) {
    ball->pos.x += ball->spd.x;
    ball->pos.y += ball->spd.y;
    return 1;
}

bool checkBallToWallColision(struct ball* ball, struct wall* wall) {
    switch (wall->type) {
    case HORIZONTAL:
        if ((int)ball->pos.y == wall->pos.y && ball->pos.x >= wall->pos.x && ball->pos.x <= wall->pos.x+wall->size) {
            return true;
        }
        else {
            return false;
        }
    case VERTICAL:
        if ((int)ball->pos.x == wall->pos.x && ball->pos.y >= wall->pos.y && ball->pos.y <= wall->pos.y+wall->size) {
            return true;
        }
        else {
            return false;
        }
    default:
        return false;
    }
}


void enable_raw_mode() {
    struct termios raw;

    // Get the current terminal attributes
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;

    // Disable canonical mode (line buffering) and echoing
    raw.c_lflag &= ~(ICANON | ECHO);

    // Apply the modified terminal attributes
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode() {
    // Restore the original terminal attributes
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
