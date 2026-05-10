#include<SDL.h>
#include<SDL_ttf.h>
#include"game.h"
#include<stdio.h>
#include<locale.h>
#include<stdlib.h>
#include<time.h>


int main(int argc,char*argv[])
{
	setlocale(LC_ALL ,"ru");
	srand(time(NULL));
	GameState game;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Error:%s", SDL_GetError());
		return 1;
	}
	if (TTF_Init() < 0)
	{
		printf("TTF Error: %s\n", TTF_GetError());
		return 1;
	}
	SDL_Window* window = SDL_CreateWindow("Game Blocks",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN);
	if (!window)
	{
		printf("Error:%s", SDL_GetError());
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
	{
		printf("Error:%s", SDL_GetError());
		return 1;
	}
	TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
	TTF_Font* fontbig = TTF_OpenFont("arial.ttf", 52);
	if (!font || !fontbig) 
	{
		printf("Font Error: %s\n", TTF_GetError());
		return 1;
	}

	int running = 1;
	SDL_Event event;

	initGame(&game);
	generatorPieces(&game);
    initScoreList(&game);

    

	
	
	Uint32 gameStartTime = SDL_GetTicks();  // время старта
	float timeBonus = 0.0f;

    while (running)
    {
        
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) running = 0;

            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_SPACE)
                {
                    if (game.screen == SCREEN_TITLE)
                        game.screen = SCREEN_MENU;
                }
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    if (game.screen == SCREEN_GAME)
                    {
                        
                        game.screen = SCREEN_MENU;
                    }
                    else if (game.screen == SCREEN_MENU)
                        running = 0;
                    else if (game.screen == SCREEN_GAMEOVER)
                    {
                        game.screen = SCREEN_MENU;
                    }
                }
                if (event.key.keysym.sym == SDLK_r)
                {
                    if (game.screen == SCREEN_GAMEOVER)
                    {
                        initGame(&game);
                        generatorPieces(&game);
                        game.screen = SCREEN_GAME;
                    }
                }
                if (event.key.keysym.sym == SDLK_s)
                {
                    if (game.screen == SCREEN_GAME)
                    {
                        saveGame(&game);
                    }
                }
                if (event.key.keysym.sym == SDLK_t)
                {
                    if (game.screen == SCREEN_GAME)
                    {
                        game.colorTheme = (game.colorTheme + 1) % 4;
                    }
                }
            }

            if (event.type == SDL_MOUSEBUTTONDOWN &&event.button.button == SDL_BUTTON_LEFT)
            {
                int mx = event.button.x;
                int my = event.button.y;

                if (game.screen == SCREEN_MENU)
                {
                    Button btnClassic = { {150,200,300,50}, "Классический", false };
                    Button btnTimed = { {150,270,300,50}, "На время (45с)", false };
                    Button btnContinue = { {150,340,300,50}, "Продолжить", false };

                    if (isButtonClicked(&btnClassic, mx, my))
                    {
                        initGame(&game);
                        generatorPieces(&game);
                        game.mode = MODE_CLASSIC;
                        game.screen = SCREEN_GAME;
                    }
                    if (isButtonClicked(&btnTimed, mx, my))
                    {
                        initGame(&game);
                        generatorPieces(&game);
                        game.mode = MODE_TIME;
                        game.timeLeft = 45.0f;
                        game.screen = SCREEN_GAME;
                        gameStartTime = SDL_GetTicks();
                        timeBonus = 0.0f;
                    }
                    if (isButtonClicked(&btnContinue, mx, my))
                    {
                        if (!hasSaveFile())
                        {
                            
                            game.showSaveMsg = 180;
                            game.saveMsgType = 1;  
                        }
                        else
                        {
                            

                            float savedTimeLeft = 0.0f;
                            loadGame(&game);
                            savedTimeLeft = game.timeLeft;

                            

                            game.screen = SCREEN_GAME;
                            game.showSaveMsg = 180;
                            game.saveMsgType = 2;  

                            if (game.mode == MODE_TIME)
                            {
                                gameStartTime = SDL_GetTicks();
                                timeBonus = savedTimeLeft - 45.0f;
                            }
                        }
                    }
                }

                if (game.screen == SCREEN_GAME)
                {
                    int idx = getPieceAt(&game, mx, my);
                    if (idx >= 0)
                    {
                        game.dragging = 1;
                        game.dragIndex = idx;
                        game.dragX = mx;
                        game.dragY = my;
                    }
                }
            }

            if (event.type == SDL_MOUSEMOTION && game.dragging)
            {
                game.dragX = event.motion.x;
                game.dragY = event.motion.y;
            }

            if (event.type == SDL_MOUSEBUTTONUP &&
                event.button.button == SDL_BUTTON_LEFT)
            {
                if (game.dragging && game.screen == SCREEN_GAME)
                {
                    int idx = game.dragIndex;
                    Piece* p = &game.pieces[idx];

                    int offsetX = (WINDOW_WIDTH - BOARD_SIZE * CELL_SIZE) / 2;
                    int offsetY = 80;
                    int startX = game.dragX - (p->width * CELL_SIZE) / 2;
                    int startY = game.dragY - (p->height * CELL_SIZE) / 2;
                    int boardCol = (startX - offsetX + CELL_SIZE / 2) / CELL_SIZE;
                    int boardRow = (startY - offsetY + CELL_SIZE / 2) / CELL_SIZE;

                    if (canPlacePiece(&game, idx, boardRow, boardCol))
                    {
                        int cleared = placePiece(&game, idx, boardRow, boardCol);
                        if (cleared > 0 && game.mode == MODE_TIME)
                        {
                            timeBonus += cleared * 5.0f;
                        }
                    }
                    game.dragging = 0;
                }

            }
        } 

        
        if (game.screen == SCREEN_GAME && game.mode == MODE_TIME)
        {
            float elapsed = (SDL_GetTicks() - gameStartTime) / 1000.0f;
            game.timeLeft = 45.0f + timeBonus - elapsed;
            if (game.timeLeft <= 0)
            {
                addToScoreList(&game, game.score);
                game.timeLeft = 0;
                game.screen = SCREEN_GAMEOVER;
            }
        }

        
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderClear(renderer);

        if (game.screen == SCREEN_TITLE)
        {
            drawTitle(renderer, font, fontbig);
        }
        else if (game.screen == SCREEN_MENU)
        {
            drawMenu(renderer, &game, font, fontbig);
            if (game.showSaveMsg > 0)
            {
                if (game.saveMsgType == 1)
                {
                    drawTextCentered(renderer, font, "Нет сохранения!",390, 231, 76, 60);  
                }
                game.showSaveMsg--;
            }
        }
        else if (game.screen == SCREEN_GAME)
        {
            drawScore(renderer, &game, font);
            drawBoard(renderer, &game);
            drawPieces(renderer, &game);
            drawDragging(renderer, &game);
            drawTimer(renderer, &game, font);

            if (game.showSaveMsg > 0)
            {
                if (game.saveMsgType == 0)
                    drawTextCentered(renderer, font, "Игра сохранена!",350, 46, 204, 113);   
                else if (game.saveMsgType == 2)
                    drawTextCentered(renderer, font, "Игра загружена!",350, 52, 152, 219);   
                game.showSaveMsg--;
            }
        }
        else if (game.screen == SCREEN_GAMEOVER)
        {
            drawScore(renderer, &game, font);
            drawBoard(renderer, &game);
            drawPieces(renderer, &game);
            drawGameOver(renderer, &game, font, fontbig);
        }
        // Анимация линий
        if (game.screen == SCREEN_GAME && game.animating)
        {
            game.animFrame--;

            if (game.animFrame <= 0)
            {
                game.animating = 0;
                eraseClearedLines(&game);

                // Генерируем новые фигуры если нужно
                bool anyActive = false;
                for (int i = 0; i < NUM_PIECES; i++)
                    if (game.pieces[i].active) { anyActive = true; break; }

                if (!anyActive)
                    generatorPieces(&game);

                // Game Over
                if (GameOver(&game))
                {
                    addToScoreList(&game, game.score);
                    game.screen = SCREEN_GAMEOVER;
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);

    }
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(font);
	TTF_CloseFont(fontbig);
    freeScoreList(&game);
	SDL_Quit();
	return 0;
	
}
