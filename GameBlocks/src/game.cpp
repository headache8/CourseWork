#define _CRT_SECURE_NO_WARNINGS
#include<SDL.h>
#include"game.h"
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<stdio.h>

// Функция отрисовки титульного экрана с информацией о программе.
// Принимает renderer, два шрифта (обычный и большой).
// Побочное действие: рисует на экране чёрный фон, текст и кнопку
void drawTitle(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* fontBig)
{
	// Чёрный фон
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_Rect bg = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	SDL_RenderFillRect(renderer, &bg);

	// Заголовок
	drawTextCentered(renderer, font, "ИНФОРМАЦИЯ О ПРОГРАММЕ", 80, 255, 255, 255);

	// Информация о программе и авторе
	drawTextCentered(renderer, font, "Игровая программа «Game Blocks»", 130, 255, 255, 255);
	drawTextCentered(renderer, font, "Автор: Студент гр.25-ИВТ-1-ПО-Б Трепутин И.А.", 170, 255, 255, 255);

	// Разделитель между блоками информации
	SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	SDL_RenderDrawLine(renderer, 50, 215, WINDOW_WIDTH - 50, 215);

	// Основной функционал программы
	drawTextCentered(renderer, font, "Функционал:", 230, 200, 200, 200);
	drawTextCentered(renderer, font, "Игровое поле 8x8, три случайные фигуры", 265, 200, 200, 200);
	drawTextCentered(renderer, font, "Перетаскивание мышью, очистка линий", 295, 200, 200, 200);
	drawTextCentered(renderer, font, "Два режима: Классический и На время", 325, 200, 200, 200);

	// Разделитель между блоками
	SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	SDL_RenderDrawLine(renderer, 50, 365, WINDOW_WIDTH - 50, 365);

	// Дополнительный функционал
	drawTextCentered(renderer, font, "Доп. функционал:", 380, 200, 200, 200);
	drawTextCentered(renderer, font, "Таблица рекордов, сохранение игры", 415, 200, 200, 200);
	drawTextCentered(renderer, font, "Анимация очистки линий, смена темы поля", 445, 200, 200, 200);
	drawTextCentered(renderer, font, "Комбо система, бонусное время", 475, 200, 200, 200);

	// Кнопка для перехода в главное меню
	SDL_Rect btnRect = { 150, 580, 300, 80 };
	SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
	SDL_RenderFillRect(renderer, &btnRect);
	SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(renderer, &btnRect);

	// Подсказка внутри кнопки
	drawTextCentered(renderer, font, "Нажмите ПРОБЕЛ", 600, 255, 255, 255);
	drawTextCentered(renderer, font, "для продолжения", 630, 255, 255, 255);
}

// Функция добавляет новый рекорд в связный список рекордов.
// Принимает указатель на GameState и новый счёт newScore.
// Побочное действие: вставляет узел в список, сохраняя сортировку
// по убыванию, удаляет узлы сверх топ-5
void addToScoreList(GameState* game, int newScore)
{
	if (newScore <= 0) return;

	ScoreNode* newNode = new ScoreNode();
	newNode->score = newScore;
	newNode->next = NULL;

	
	if (game->scoreList == NULL || newScore > game->scoreList->score)
	{
		newNode->next = game->scoreList;
		game->scoreList = newNode;
	}
	else
	{
		
		ScoreNode* curr = game->scoreList;
		while (curr->next != NULL && curr->next->score >= newScore)
		{
			curr = curr->next;
		}
		newNode->next = curr->next;
		curr->next = newNode;
	}

	// Удаляем узлы после 5-го чтобы список не рос бесконечно
	ScoreNode* curr = game->scoreList;
	int count = 1;
	while (curr->next != NULL)
	{
		if (count >= 5)
		{
			ScoreNode* Delete = curr->next;
			curr->next = NULL;
			while (Delete != NULL)
			{
				ScoreNode* next = Delete->next;
				delete Delete;
				Delete = next;
			}
			break;
		}
		count++;
		curr = curr->next;
	}
}

// Функция освобождает всю динамически выделенную память списка рекордов.
// Принимает указатель на GameState.
// Побочное действие: удаляет все узлы списка, устанавливает scoreList = NULL
void freeScoreList(GameState* game)
{
	ScoreNode* curr = game->scoreList;
	while (curr != NULL)
	{
		ScoreNode* next = curr->next;
		delete curr;
		curr = next;
	}
	game->scoreList = NULL;
}

// Функция инициализирует все поля GameState начальными значениями.
// Принимает указатель на GameState.
// Побочное действие: обнуляет поле, сбрасывает счёт, комбо и все флаги
void initGame(GameState* game)
{
	// Обнуляем игровое поле
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++)
			game->board[i][j] = 0;

	// Сброс флагов сообщений
	game->showSaveMsg = 0;
	game->saveMsgType = 0;

	// Сброс игровых показателей
	game->score = 0;
	game->combo = 0;
	game->screen = SCREEN_TITLE;
	game->mode = MODE_CLASSIC;
	game->dragging = 0;
	game->timeLeft = 45.0;
	game->colorTheme = 0;

	// Сброс анимации
	game->animating = 0;
	game->animFrame = 0;
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		game->clearedRows[i] = 0;
		game->clearedCols[i] = 0;
	}

	// Деактивируем все фигуры
	for (int i = 0; i < NUM_PIECES; i++)
		game->pieces[i].active = 0;
}

// Функция отрисовывает игровое поле 8x8.
// Принимает renderer и указатель на GameState.
// Побочное действие: рисует все клетки поля с учётом темы и анимации
void drawBoard(SDL_Renderer* renderer, GameState* game)
{
	int offsetX = (WINDOW_WIDTH - BOARD_SIZE * CELL_SIZE) / 2;
	int offsetY = 90;

	// Определяем цвет пустой клетки в зависимости от выбранной темы
	int cellR, cellG, cellB;
	switch (game->colorTheme)
	{
	case 0: cellR = 60;  cellG = 60;  cellB = 60;  break; // Серый
	case 1: cellR = 20;  cellG = 40;  cellB = 80;  break; // Океан
	case 2: cellR = 20;  cellG = 60;  cellB = 20;  break; // Лес
	case 3: cellR = 60;  cellG = 20;  cellB = 80;  break; // Закат
	default:cellR = 60;  cellG = 60;  cellB = 60;  break;
	}

	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			SDL_Rect cell;
			cell.x = offsetX + j * CELL_SIZE;
			cell.y = offsetY + i * CELL_SIZE;
			cell.w = CELL_SIZE - 2;
			cell.h = CELL_SIZE - 2;

			// Проверяем входит ли клетка в анимируемую линию
			bool isCleared = game->clearedRows[i] || game->clearedCols[j];

			if (isCleared && game->animating)
			{
				// Плавное затухание: смешиваем цвет клетки с белым
				// ratio убывает от 1.0 до 0.0 по мере анимации
				float ratio = game->animFrame / 20.0f;
				int r, g, b;
				if (game->board[i][j] != 0)
					getColor(game->board[i][j], &r, &g, &b);
				else { r = cellR; g = cellG; b = cellB; }

				int fr = (int)(r * ratio + 255 * (1.0f - ratio));
				int fg = (int)(g * ratio + 255 * (1.0f - ratio));
				int fb = (int)(b * ratio + 255 * (1.0f - ratio));
				SDL_SetRenderDrawColor(renderer, fr, fg, fb, 255);
			}
			else if (game->board[i][j] == 0)
			{
				// Пустая клетка — цвет зависит от темы
				SDL_SetRenderDrawColor(renderer, cellR, cellG, cellB, 255);
			}
			else
			{
				// Занятая клетка — цвет фигуры
				int r, g, b;
				getColor(game->board[i][j], &r, &g, &b);
				SDL_SetRenderDrawColor(renderer, r, g, b, 255);
			}

			SDL_RenderFillRect(renderer, &cell);
		}
	}
}

// Шаблоны фигур: массив ячеек, ширина и высота
struct PieceTemplate
{
	int cells[MAX_PIECE_SIZE][MAX_PIECE_SIZE];
	int width, height;
};

// Массив всех возможных шаблонов фигур
PieceTemplate templates[] =
{
	{ { {1},{1},{1},{1} } , 1, 4},  // вертикальная линия 1x4
	{ { {1,1},{1,1} }, 2, 2},       // квадрат 2x2
	{ { {1,1,1},{0,1,0} } , 3, 2},  // T-фигура
	{ { {0,1},{1,1},{1,0} }, 2, 3 },// Z вертикальная
	{ { {1,0},{1,1},{0,1} }, 2, 3 },// S вертикальная
	{ { {0,1,1},{1,1,0} }, 3, 2},   // Z горизонтальная
	{ { {1,1,0},{0,1,1} }, 3, 2},   // S горизонтальная
	{ { {1,0},{1,0},{1,1} }, 2, 3}, // L-фигура
	{ { {1,1},{1,0},{1,0} }, 2, 3}, // L-фигура отражённая
	{ { {1,1},{0,1},{0,1} }, 2, 3}, // J-фигура
	{ { {0,1},{0,1},{1,1} }, 2, 3}, // J-фигура отражённая
	{ { {1,1} }, 2, 1 },            // горизонтальная линия 2x1
	{ { {1,1,1} }, 3, 1 },          // горизонтальная линия 3x1
	{ { {1,1,1},{1,1,1} }, 3, 2},// блок 3x2
	{ { {1} },1,1},//.
	{ { {1,1,1},{1,1,1},{1,1,1} }, 3, 3}, // блок 3x3
	{ { { 1,0,0},{1,0,0},{1,1,1} }, 3, 3 }, // L большая
	{ { { 0,0,1 },{0,0,1},{1,1,1} }, 3, 3 },// J большая
	{ { { 1,1,1},{1,0,0},{1,0,0} }, 3, 3 }, // L большая отражённая
	{ { { 1,1,1},{0,0,1},{0,0,1} }, 3, 3 }, // J большая отражённая
	{ { {1,0},{1,1} }, 2, 2 }, // малая L
	{ { {0,1},{1,1} }, 2, 2 }, // малая J
	{ { {1,1},{1,0} }, 2, 2 }, // малая L отражённая
	{ { {1,1},{0,1} }, 2, 2 }, // малая J отражённая
};

// Количество шаблонов в массиве
int numTemplate = sizeof(templates) / sizeof(templates[0]);

// Функция генерирует три случайные фигуры для панели выбора.
// Принимает указатель на GameState.
// Побочное действие: заполняет game->pieces случайными фигурами из templates
void generatorPieces(GameState* game)
{
	for (int i = 0; i < NUM_PIECES; i++)
	{
		// Выбираем случайный шаблон
		int idx = rand() % numTemplate;

		// Обнуляем ячейки фигуры перед копированием
		memset(game->pieces[i].cells, 0, sizeof(game->pieces[i].cells));

		// Копируем ячейки из шаблона
		for (int r = 0; r < templates[idx].height; r++)
			for (int c = 0; c < templates[idx].width; c++)
				game->pieces[i].cells[r][c] = templates[idx].cells[r][c];

		game->pieces[i].width = templates[idx].width;
		game->pieces[i].height = templates[idx].height;
		game->pieces[i].color = (rand() % 7) + 1; // случайный цвет 1-7
		game->pieces[i].active = 1;
	}
}

// Функция возвращает RGB цвет по индексу colorIndex (1-7).
// Принимает индекс цвета и указатели на r, g, b для записи результата.
// Побочное действие: записывает значения RGB в переданные указатели
void getColor(int colorIndex, int* r, int* g, int* b)
{
	switch (colorIndex) {
	case 1: *r = 231; *g = 76;  *b = 60;  break; // красный
	case 2: *r = 46;  *g = 204; *b = 113; break; // зелёный
	case 3: *r = 52;  *g = 152; *b = 219; break; // синий
	case 4: *r = 241; *g = 196; *b = 15;  break; // жёлтый
	case 5: *r = 155; *g = 89;  *b = 182; break; // фиолетовый
	case 6: *r = 230; *g = 126; *b = 34;  break; // оранжевый
	case 7: *r = 26;  *g = 188; *b = 156; break; // бирюзовый
	default: *r = 255; *g = 255; *b = 255; break; // белый (по умолчанию)
	}
}

// Функция проверяет заполненные строки и столбцы поля,
// помечает их для анимации в clearedRows и clearedCols.
// Принимает указатель на GameState.
// Возвращает количество помеченных линий (строк + столбцов)
int clearLines(GameState* game)
{
	int cleared = 0;

	// Сбрасываем предыдущие метки анимации
	for (int i = 0; i < BOARD_SIZE; i++) {
		game->clearedRows[i] = 0;
		game->clearedCols[i] = 0;
	}

	// Проверяем каждую строку на заполненность
	for (int i = 0; i < BOARD_SIZE; i++) 
	{
		bool full = true;
		for (int j = 0; j < BOARD_SIZE; j++) 
		{
			if (game->board[i][j] == 0) { full = false; break; }
		}
		if (full) 
		{
			game->clearedRows[i] = 1; // помечаем строку
			cleared++;
		}
	}

	// Проверяем каждый столбец на заполненность
	for (int j = 0; j < BOARD_SIZE; j++) 
	{
		bool full = true;
		for (int i = 0; i < BOARD_SIZE; i++) 
		{
			if (game->board[i][j] == 0) { full = false; break; }
		}
		if (full) 
		{
			game->clearedCols[j] = 1; // помечаем столбец
			cleared++;
		}
	}

	return cleared;
}

// Функция реально очищает помеченные строки и столбцы поля.
// Вызывается после завершения анимации исчезновения.
// Принимает указатель на GameState.
// Побочное действие: обнуляет ячейки помеченных линий и сбрасывает метки
void eraseClearedLines(GameState* game)
{
	// Очищаем помеченные строки
	for (int i = 0; i < BOARD_SIZE; i++) {
		if (game->clearedRows[i]) {
			for (int j = 0; j < BOARD_SIZE; j++)
				game->board[i][j] = 0;
			game->clearedRows[i] = 0;
		}
	}

	// Очищаем помеченные столбцы
	for (int j = 0; j < BOARD_SIZE; j++) {
		if (game->clearedCols[j]) {
			for (int i = 0; i < BOARD_SIZE; i++)
				game->board[i][j] = 0;
			game->clearedCols[j] = 0;
		}
	}
}

// Функция начисляет очки за очищенные линии с учётом комбо и бонусов.
// Принимает указатель на GameState и количество очищенных линий.
// Побочное действие: увеличивает game->score и game->combo,
// добавляет бонусное время в режиме на время
void addScore(GameState* game, int linesCleared)
{
	if (linesCleared > 0) {
		// Базовые очки: 10 за клетку * 8 клеток * количество линий
		int baseScore = linesCleared * BOARD_SIZE * 10;

		// Комбо бонус растёт с каждым ходом где есть очистка
		game->combo++;
		int comboBonus = (game->combo - 1) * 50;

		// Дополнительный бонус за несколько линий одновременно
		int multiBonus = 0;
		if (linesCleared == 2) multiBonus = 100;
		if (linesCleared == 3) multiBonus = 300;
		if (linesCleared >= 4) multiBonus = 600;

		game->score += baseScore + comboBonus + multiBonus;

		// В режиме на время добавляем бонусное время за линии
		if (game->mode == MODE_TIME)
		{
			game->timeLeft += linesCleared * 5.0f;
			if (game->timeLeft > 45.0f)
				game->timeLeft = 45.0f; // не больше максимума
		}
	}
	else {
		game->combo = 0; // нет очистки — сброс комбо
	}
}

// Функция отрисовывает три фигуры в панели выбора внизу экрана.
// Принимает renderer и указатель на GameState.
// Побочное действие: рисует активные фигуры, пропускает перетаскиваемую
void drawPieces(SDL_Renderer* renderer, GameState* game)
{
	int startY = 600; 
	for (int i = 0; i < NUM_PIECES; i++)
	{
		if (!game->pieces[i].active) continue;
		
		if (game->dragging && (game->dragIndex == i)) continue;

		int startX = 95 + i * 160; 
		int pieceCell = 35;         
		int r, g, b;
		getColor(game->pieces[i].color, &r, &g, &b);

		// Рисуем каждую заполненную ячейку фигуры
		for (int row = 0; row < game->pieces[i].height; row++)
			for (int col = 0; col < game->pieces[i].width; col++)
				if (game->pieces[i].cells[row][col])
				{
					SDL_Rect rect;
					rect.x = startX + col * pieceCell;
					rect.y = startY + row * pieceCell;
					rect.w = pieceCell - 2;
					rect.h = pieceCell - 2;
					SDL_SetRenderDrawColor(renderer, r, g, b, 255);
					SDL_RenderFillRect(renderer, &rect);
				}
	}
}

// Функция определяет по координатам мыши на какую фигуру кликнул игрок.
// Принимает указатель на GameState и координаты мыши mouseX, mouseY.
// Возвращает индекс фигуры (0-2) или -1 если клик мимо всех фигур
int getPieceAt(GameState* game, int mouseX, int mouseY)
{
	int startY = 600;
	int pieceCell = 35;

	for (int i = 0; i < NUM_PIECES; i++)
	{
		if (!game->pieces[i].active) continue;

		int startX = 110 + i * 150;

		// Прямоугольник области фигуры
		int endX = startX + game->pieces[i].width * pieceCell;
		int endY = startY + game->pieces[i].height * pieceCell;

		// Проверяем попал ли клик в область фигуры
		if (mouseX >= startX && mouseX < endX &&
			mouseY >= startY && mouseY < endY)
			return i;
	}
	return -1;
}

// Функция отрисовывает фигуру которую игрок перетаскивает мышью.
// Рисует фигуру в полный размер (CELL_SIZE) центрированно под курсором.
// Принимает renderer и указатель на GameState.
// Побочное действие: рисует только если game->dragging == 1
void drawDragging(SDL_Renderer* renderer, GameState* game)
{
	if (!game->dragging) return;

	int idx = game->dragIndex;
	Piece* p = &game->pieces[idx];

	int r, g, b;
	getColor(p->color, &r, &g, &b);

	// Центрируем фигуру относительно курсора мыши
	int offsetX = game->dragX - (p->width * CELL_SIZE) / 2;
	int offsetY = game->dragY - (p->height * CELL_SIZE) / 2;

	// Рисуем каждую заполненную ячейку фигуры
	for (int row = 0; row < p->height; row++)
		for (int col = 0; col < p->width; col++)
			if (p->cells[row][col])
			{
				SDL_Rect rect;
				rect.x = offsetX + col * CELL_SIZE;
				rect.y = offsetY + row * CELL_SIZE;
				rect.w = CELL_SIZE - 2;
				rect.h = CELL_SIZE - 2;
				SDL_SetRenderDrawColor(renderer, r, g, b, 255);
				SDL_RenderFillRect(renderer, &rect);
			}
}

// Функция проверяет можно ли разместить фигуру на поле.
// Принимает GameState, индекс фигуры, строку и столбец верхнего левого угла.
// Возвращает true если размещение возможно, false если выходит за границы
// или перекрывает уже занятые клетки
bool canPlacePiece(GameState* game, int pieceIdx, int boardRow, int boardCol)
{
	Piece* p = &game->pieces[pieceIdx];

	for (int r = 0; r < p->height; r++)
		for (int c = 0; c < p->width; c++)
			if (p->cells[r][c])
			{
				int targetRow = boardRow + r;
				int targetCol = boardCol + c;

				// Проверка выхода за границы поля
				if (targetRow < 0 || targetRow >= BOARD_SIZE ||
					targetCol < 0 || targetCol >= BOARD_SIZE)
					return false;

				// Проверка занятости клетки
				if (game->board[targetRow][targetCol] != 0)
					return false;
			}
	return true;
}

// Функция проверяет наступил ли конец игры.
// Конец игры — ни одну из активных фигур нельзя разместить на поле.
// Принимает указатель на GameState.
// Возвращает true если нет доступных ходов, false если есть хотя бы один
bool GameOver(GameState* game)
{
	for (int i = 0; i < NUM_PIECES; i++)
	{
		if (!game->pieces[i].active) continue;
		// Перебираем все позиции поля
		for (int row = 0; row < BOARD_SIZE; row++)
			for (int col = 0; col < BOARD_SIZE; col++)
				if (canPlacePiece(game, i, row, col))
					return false; // нашли хоть одну позицию — игра продолжается
	}
	return true;
}

// Функция размещает фигуру на поле и запускает анимацию если есть линии.
// Принимает GameState, индекс фигуры, строку и столбец размещения.
// Побочное действие: записывает цвет в board[][], деактивирует фигуру,
// запускает анимацию, начисляет очки, генерирует новые фигуры если нужно.
// Возвращает количество помеченных линий
int placePiece(GameState* game, int pieceIdx, int boardRow, int boardCol)
{
	Piece* p = &game->pieces[pieceIdx];

	// Записываем цвет фигуры в ячейки поля
	for (int r = 0; r < p->height; r++)
		for (int c = 0; c < p->width; c++)
			if (p->cells[r][c])
				game->board[boardRow + r][boardCol + c] = p->color;

	p->active = 0; // фигура использована

	// Проверяем заполненные линии и помечаем их
	int cleared = clearLines(game);

	if (cleared > 0)
	{
		// Запускаем анимацию исчезновения на 20 кадров
		game->animating = 1;
		game->animFrame = 20;
		addScore(game, cleared);
	}
	else
	{
		addScore(game, 0); 
	}

	// Генерация новых фигур и проверка Game Over 
	if (!game->animating)
	{
		bool anyActive = false;
		for (int i = 0; i < NUM_PIECES; i++)
			if (game->pieces[i].active) { anyActive = true; break; }
		if (!anyActive)
			generatorPieces(game);

		if (GameOver(game))
		{
			addToScoreList(game, game->score);
			game->screen = SCREEN_GAMEOVER;
		}
	}

	return cleared;
}

// Функция рисует текст в заданной позиции (x, y) заданным цветом.
// Принимает renderer, шрифт, строку текста, координаты и RGB цвет.
// Побочное действие: создаёт и уничтожает Surface и Texture каждый вызов
void drawText(SDL_Renderer* renderer, TTF_Font* font,
	const char* text, int x, int y, int r, int g, int b)
{
	SDL_Color color = { (Uint8)r, (Uint8)g, (Uint8)b, 255 };

	// Создаём поверхность с UTF-8 текстом
	SDL_Surface* surface = TTF_RenderUTF8_Solid(font, text, color);
	if (!surface) return;

	// Конвертируем поверхность в текстуру для рендерера
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	if (!texture) return;

	// Получаем размер текста и рисуем
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);
	SDL_Rect dst = { x, y, w, h };
	SDL_RenderCopy(renderer, texture, NULL, &dst);
	SDL_DestroyTexture(texture);
}

// Функция рисует текст горизонтально по центру окна на высоте y.
// Принимает renderer, шрифт, строку текста, Y-координату и RGB цвет.
void drawTextCentered(SDL_Renderer* renderer, TTF_Font* font,const char* text, int y, int r, int g, int b)
{
	SDL_Color color = { (Uint8)r, (Uint8)g, (Uint8)b, 255 };
	SDL_Surface* surface = TTF_RenderUTF8_Solid(font, text, color);
	if (!surface) return;
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	if (!texture) return;

	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);

	// Вычисляем X для центрирования
	int x = (WINDOW_WIDTH - w) / 2;
	SDL_Rect dst = { x, y, w, h };
	SDL_RenderCopy(renderer, texture, NULL, &dst);
	SDL_DestroyTexture(texture);
}

// Функция отрисовывает экран конца игры с затемнением и итоговым счётом.
// Принимает renderer, GameState, обычный и большой шрифты.
// Побочное действие: рисует полупрозрачный оверлей поверх игрового поля
void drawGameOver(SDL_Renderer* renderer, GameState* game,TTF_Font* font, TTF_Font* fontBig)
{
	// Полупрозрачное затемнение поверх поля
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_Rect overlay = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
	SDL_RenderFillRect(renderer, &overlay);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

	// Заголовок Game Over большим шрифтом
	drawTextCentered(renderer, fontBig, "GAME OVER", 200, 231, 76, 60);

	// Итоговый счёт текущей партии
	char scoreText[64];
	sprintf_s(scoreText, sizeof(scoreText), "Счёт: %d", game->score);
	drawTextCentered(renderer, font, scoreText, 290, 255, 255, 255);

	// Лучший результат из таблицы рекордов
	char bestText[64];
	sprintf_s(bestText, sizeof(bestText), "Рекорд: %d", getTopScore(game));
	drawTextCentered(renderer, font, bestText, 330, 241, 196, 15);

	// Подсказки по управлению
	drawTextCentered(renderer, font, "R — играть снова", 390, 180, 180, 180);
	drawTextCentered(renderer, font, "ESC — главное меню", 425, 180, 180, 180);
}

// Функция отрисовывает верхнюю панель с очками, комбо и режимом игры.
// Принимает renderer, GameState и шрифт.
// Побочное действие: рисует тёмную панель высотой 90px с текстом
void drawScore(SDL_Renderer* renderer, GameState* game, TTF_Font* font)
{
	// Фон панели счёта
	SDL_Rect panel = { 0, 0, WINDOW_WIDTH, 90 };
	SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
	SDL_RenderFillRect(renderer, &panel);
	SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
	SDL_RenderDrawRect(renderer, &panel);

	char scoreText[64];
	char comboText[64];
	sprintf_s(scoreText, "Счёт: %d", game->score);
	sprintf_s(comboText, "Комбо: x%d", game->combo);

	// Счёт — белый, комбо — золотой
	drawText(renderer, font, scoreText, 20, 10, 255, 255, 255);
	drawText(renderer, font, comboText, 20, 40, 255, 215, 0);

	// Название режима в правом углу
	if (game->mode == MODE_TIME)
		drawText(renderer, font, "ВРЕМЯ", 480, 10, 231, 76, 60);
	else
		drawText(renderer, font, "КЛАССИКА", 450, 10, 46, 204, 113);

	// Подсказки по клавишам
	drawText(renderer, font, "S - сохранить", 200, 10, 120, 120, 120);

	// Текущая тема поля
	const char* themes[] = { "Серый", "Океан", "Лес", "Закат" };
	drawText(renderer, font, "T-тема:", 200, 40, 120, 120, 120);
	drawText(renderer, font, themes[game->colorTheme], 300, 40, 150, 150, 150);
}

// Функция проверяет попал ли клик мыши в область кнопки.
// Принимает указатель на Button и координаты мыши.
// Возвращает true если точка (mouseX, mouseY) внутри прямоугольника кнопки
bool isButtonClicked(Button* btn, int mouseX, int mouseY)
{
	return mouseX >= btn->rect.x &&
		mouseX <= btn->rect.x + btn->rect.w &&
		mouseY >= btn->rect.y &&
		mouseY <= btn->rect.y + btn->rect.h;
}

// Функция отрисовывает одну кнопку с текстом по центру.
// Принимает renderer, шрифт и указатель на Button.
// Побочное действие: меняет цвет фона при наведении мыши (btn->hovered)
void drawButton(SDL_Renderer* renderer, TTF_Font* font, Button* btn)
{
	// Подсветка при наведении мыши
	if (btn->hovered)
		SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
	else
		SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

	SDL_RenderFillRect(renderer, &btn->rect);

	// Рамка кнопки
	SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
	SDL_RenderDrawRect(renderer, &btn->rect);

	// Текст по центру кнопки
	SDL_Color color = { 255, 255, 255, 255 };
	SDL_Surface* surface = TTF_RenderUTF8_Solid(font, btn->text, color);
	if (!surface) return;
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	if (!texture) return;

	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);

	// Вычисляем позицию для центрирования текста внутри кнопки
	SDL_Rect dst = {
		btn->rect.x + (btn->rect.w - w) / 2,
		btn->rect.y + (btn->rect.h - h) / 2,
		w, h
	};
	SDL_RenderCopy(renderer, texture, NULL, &dst);
	SDL_DestroyTexture(texture);
}

// Функция отрисовывает главное меню с кнопками и таблицей рекордов.
// Принимает renderer, GameState, обычный и большой шрифты.
// Побочное действие: рисует фон, название, кнопки, таблицу рекордов
void drawMenu(SDL_Renderer* renderer, GameState* game,
	TTF_Font* font, TTF_Font* fontBig)
{
	// Тёмный фон меню
	SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
	SDL_Rect bg = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	SDL_RenderFillRect(renderer, &bg);

	// Название игры большим шрифтом по центру
	drawTextCentered(renderer, fontBig, "GAME BLOCKS", 60, 52, 152, 219);

	// Кнопки выбора режима и загрузки
	Button btnClassic = { {150, 200, 300, 50}, "Классический", false };
	Button btnTimed = { {150, 270, 300, 50}, "На время (45с)", false };
	Button btnContinue = { {150, 340, 300, 50}, "Загрузить сохранение", false };

	// Обновляем состояние наведения мыши
	int mx, my;
	SDL_GetMouseState(&mx, &my);
	btnClassic.hovered = isButtonClicked(&btnClassic, mx, my);
	btnTimed.hovered = isButtonClicked(&btnTimed, mx, my);
	btnContinue.hovered = isButtonClicked(&btnContinue, mx, my);

	drawButton(renderer, font, &btnClassic);
	drawButton(renderer, font, &btnTimed);

	// Кнопка загрузки: серая если нет файла сохранения, активная если есть
	if (!hasSaveFile())
	{
		SDL_SetRenderDrawColor(renderer, 35, 35, 35, 255);
		SDL_RenderFillRect(renderer, &btnContinue.rect);
		SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
		SDL_RenderDrawRect(renderer, &btnContinue.rect);
		SDL_Color grey = { 100, 100, 100, 255 };
		SDL_Surface* surf = TTF_RenderUTF8_Solid(font, "Загрузить сохранение", grey);
		if (surf) {
			SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
			SDL_FreeSurface(surf);
			if (tex) {
				int w, h;
				SDL_QueryTexture(tex, NULL, NULL, &w, &h);
				SDL_Rect dst = {
					btnContinue.rect.x + (btnContinue.rect.w - w) / 2,
					btnContinue.rect.y + (btnContinue.rect.h - h) / 2,
					w, h
				};
				SDL_RenderCopy(renderer, tex, NULL, &dst);
				SDL_DestroyTexture(tex);
			}
		}
	}
	else
	{
		drawButton(renderer, font, &btnContinue);
	}

	// Горизонтальный разделитель перед таблицей рекордов
	SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
	SDL_RenderDrawLine(renderer, 50, 410, WINDOW_WIDTH - 50, 410);

	// Заголовок таблицы рекордов
	drawTextCentered(renderer, font, "Таблица рекордов:", 425, 200, 200, 200);

	// Обходим связный список рекордов и выводим топ-5
	ScoreNode* curr = game->scoreList;
	int rank = 1;
	int y = 455;
	while (curr != NULL && rank <= 5)
	{
		char text[64];
		sprintf_s(text, sizeof(text), "%d.  %d", rank, curr->score);

		// Первое место выделяем золотым цветом
		if (rank == 1)
			drawTextCentered(renderer, font, text, y, 241, 196, 15);
		else
			drawTextCentered(renderer, font, text, y, 200, 200, 200);
		y += 28;
		rank++;
		curr = curr->next;
	}

	// Если список пуст — показываем сообщение
	if (game->scoreList == NULL)
		drawTextCentered(renderer, font, "Нет рекордов", 455, 120, 120, 120);

	// Подсказка по выходу из программы
	drawTextCentered(renderer, font, "ESC - выход", 700, 120, 120, 120);
}

// Функция отрисовывает полосу таймера только в режиме на время.
// Полоса меняет цвет от зелёного к красному по мере убывания времени.
// Принимает renderer, GameState и шрифт для текста времени.
// Побочное действие: рисует полосу и числовое значение времени
void drawTimer(SDL_Renderer* renderer, GameState* game, TTF_Font* font)
{
	if (game->mode != MODE_TIME) return;

	int barWidth = WINDOW_WIDTH - 40;
	int barHeight = 15;
	int barX = 20;
	int barY = 68;

	// Серый фон полосы
	SDL_Rect bgBar = { barX, barY, barWidth, barHeight };
	SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
	SDL_RenderFillRect(renderer, &bgBar);

	// ratio: 1.0 = полная, 0.0 = пустая
	float ratio = game->timeLeft / 45.0f;
	if (ratio < 0) ratio = 0;
	if (ratio > 1) ratio = 1;

	int fillWidth = (int)(barWidth * ratio);

	// Цвет плавно меняется: зелёный → жёлтый → красный
	int r, g, b;
	if (ratio > 0.5f) {
		r = (int)(255 * (1.0f - ratio) * 2);
		g = 200;
		b = 0;
	}
	else {
		r = 255;
		g = (int)(200 * ratio * 2);
		b = 0;
	}

	SDL_Rect fillBar = { barX, barY, fillWidth, barHeight };
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
	SDL_RenderFillRect(renderer, &fillBar);

	// Рамка полосы
	SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
	SDL_RenderDrawRect(renderer, &bgBar);

	// Числовое значение оставшегося времени
	char timeText[32];
	sprintf_s(timeText, sizeof(timeText), "%.1f", game->timeLeft);
	drawText(renderer, font, timeText, barX + barWidth - 50, barY - 25, 255, 255, 255);
}

// Функция сохраняет текущее состояние игры в бинарный файл save.dat.
// Принимает указатель на GameState.
// Побочное действие: создаёт/перезаписывает файл save.dat,
// устанавливает флаг showSaveMsg для показа сообщения
void saveGame(GameState* game)
{
	FILE* file = fopen("save.dat", "wb"); // открываем для записи в бинарном режиме
	if (!file) return;

	SaveData data;

	// Копируем игровое поле
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++)
			data.board[i][j] = game->board[i][j];

	// Копируем игровые показатели
	data.score = game->score;
	data.combo = game->combo;
	data.mode = game->mode;
	data.timeLeft = game->timeLeft;

	// Копируем три текущие фигуры
	for (int i = 0; i < NUM_PIECES; i++)
		data.pieces[i] = game->pieces[i];

	// Конвертируем связный список рекордов в массив для записи
	ScoreNode* curr = game->scoreList;
	for (int i = 0; i < 5; i++)
	{
		if (curr != NULL) {
			data.highScores[i] = curr->score;
			curr = curr->next;
		}
		else {
			data.highScores[i] = 0;
		}
	}

	fwrite(&data, sizeof(SaveData), 1, file);
	fclose(file);

	// Показываем сообщение "Игра сохранена!" на 180 кадров
	game->showSaveMsg = 180;
	game->saveMsgType = 0;
}

// Функция загружает состояние игры из бинарного файла save.dat.
// Принимает указатель на GameState.
// Побочное действие: перезаписывает поля GameState данными из файла,
// восстанавливает связный список рекордов.
// Возвращает true если загрузка успешна, false если файл не найден
bool loadGame(GameState* game)
{
	FILE* file = fopen("save.dat", "rb"); // открываем для чтения в бинарном режиме
	if (!file) return false;

	SaveData data;

	// Читаем одну структуру SaveData из файла
	if (fread(&data, sizeof(SaveData), 1, file) != 1)
	{
		fclose(file);
		return false;
	}
	fclose(file);

	// Восстанавливаем игровое поле
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++)
			game->board[i][j] = data.board[i][j];

	// Восстанавливаем игровые показатели
	game->score = data.score;
	game->combo = data.combo;
	game->mode = data.mode;
	game->timeLeft = data.timeLeft;

	// Восстанавливаем три фигуры
	for (int i = 0; i < NUM_PIECES; i++)
		game->pieces[i] = data.pieces[i];

	// Конвертируем массив рекордов обратно в связный список
	freeScoreList(game);
	for (int i = 4; i >= 0; i--)
		if (data.highScores[i] > 0)
			addToScoreList(game, data.highScores[i]);

	game->dragging = 0;
	game->screen = SCREEN_GAME;
	return true;
}

// Функция проверяет существует ли файл сохранения save.dat.
// Не принимает параметров.
// Возвращает true если файл существует и открывается, false иначе
bool hasSaveFile()
{
	FILE* file = fopen("save.dat", "rb");
	if (!file) return false;
	fclose(file);
	return true;
}

// Функция инициализирует пустой связный список рекордов.
// Принимает указатель на GameState.
// Побочное действие: устанавливает game->scoreList = NULL
void initScoreList(GameState* game)
{
	game->scoreList = NULL;
}

// Функция возвращает лучший результат из таблицы рекордов.
// Принимает указатель на GameState.
// Возвращает score первого узла списка или 0 если список пуст
int getTopScore(GameState* game)
{
	if (game->scoreList == NULL) return 0;
	return game->scoreList->score;
}
