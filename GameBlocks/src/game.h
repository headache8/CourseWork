#ifndef GAME_H
#define GAME_H
#include<SDL.h>
#include<SDL_ttf.h>

#define BOARD_SIZE 8
#define MAX_PIECE_SIZE 4
#define NUM_PIECES 3
#define CELL_SIZE 60

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 780
enum GameMode { MODE_CLASSIC,MODE_TIME };

enum GameScreen { SCREEN_MENU, SCREEN_GAME, SCREEN_GAMEOVER,SCREEN_TITLE };



struct Button 
{
	SDL_Rect rect;      
	const char* text;   
	bool hovered;       
};
struct ScoreNode 
{
	int score;
	ScoreNode* next;
};

struct Piece{
	int cells[MAX_PIECE_SIZE][MAX_PIECE_SIZE];
	int width, height;
	int color;
	int active;
};
struct GameState {
	int board[BOARD_SIZE][BOARD_SIZE];
	Piece pieces[NUM_PIECES];
	int score;
	int combo;
	ScoreNode* scoreList;
	GameMode mode;
	GameScreen screen;
	float timeLeft;
	int showSaveMsg;
	int saveMsgType;
	int dragging;
	int dragIndex;
	int dragX, dragY;
	int colorTheme;
	int animating;
	int animFrame;
	int clearedRows[BOARD_SIZE];
	int clearedCols[BOARD_SIZE];
};

struct SaveData 
{
	int board[BOARD_SIZE][BOARD_SIZE];
	int score;
	int combo;
	GameMode mode;
	float timeLeft;
	Piece pieces[NUM_PIECES];
	int highScores[5];
};
void drawTitle(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* fontBig);
void saveGame(GameState* game);
bool loadGame(GameState* game);
bool hasSaveFile();
void initGame(GameState* game);
void drawBoard(SDL_Renderer* renderer, GameState* game);
void generatorPieces(GameState* game);
void drawPieces(SDL_Renderer* renderer, GameState* game);
void getColor(int colorIndex, int* r, int* g, int* b);
int getPieceAt(GameState* game, int mouseX, int mouseY);
void drawDragging(SDL_Renderer* renderer, GameState* game);
bool canPlacePiece(GameState* game, int pieceIdx, int boardRow, int boardCol);
int placePiece(GameState* game, int pieceIdx, int boardRow, int boardCol);
int clearLines(GameState* game);
void addScore(GameState* game, int linesCleared);
void drawTextCentered(SDL_Renderer* renderer, TTF_Font* font,const char* text, int y, int r, int g, int b);
void drawScore(SDL_Renderer* renderer, GameState* game, TTF_Font* font);
bool GameOver(GameState* game);
void drawGameOver(SDL_Renderer* renderer, GameState* game, TTF_Font* font,TTF_Font* fontbig);
void drawMenu(SDL_Renderer* renderer, GameState* game,TTF_Font* font, TTF_Font* fontBig);
bool isButtonClicked(Button* btn, int mouseX, int mouseY);
void drawButton(SDL_Renderer* renderer, TTF_Font* font, Button* btn);
void drawText(SDL_Renderer* renderer, TTF_Font* font,const char* text, int x, int y, int r, int g, int b);
void drawTimer(SDL_Renderer* renderer, GameState* game, TTF_Font* font);

void initScoreList(GameState* game);
void addToScoreList(GameState* game, int newScore);
void freeScoreList(GameState* game);
int getTopScore(GameState* game);
void eraseClearedLines(GameState* game);


#endif
