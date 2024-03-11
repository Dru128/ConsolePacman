#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <windows.h>

#define ANSI_RESET_ALL     "\x1b[0m"
#define COLOR_RED          "\x1b[31m"
#define COLOR_GREEN        "\x1b[32m"

#define WIDTH 60 // стенка два символа
#define HEIGHT 29

#define EMPTY_TAG '1'
#define WALL_TAG '2'
#define PLAYER_TAG '3'
#define START_TAG '4'
#define FINISH_TAG '5'
#define ENEMY_TAG '6'

/*
 длина консоли 120, ширина 30
 Лабиринт с монстрами
 Ведется стаистика прохождения времени и сохраняется в rate.txt
 Уровень хранится в level_1.txt
*/
typedef struct Point {
	int x, y;
};

char getMapPoint(const char* map, int x, int y)
{
	if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
		return *(map + y * WIDTH + x);
	else
		return '0';
}

int comparePoint(Point p1, Point p2)
{
	return p1.x == p2.x && p1.y == p2.y;
}
// выгрузка карты из базы данных
int loadMapFromFile(char* map)
{
	FILE* levelDB;
	levelDB = fopen("level_1.txt", "r");
	if (levelDB == NULL) return 0;
	char symbol;

	rewind(levelDB);
	for (int y = 0; y < HEIGHT; y++)
		for (int x = 0; x <= WIDTH; x++)
		{
			symbol = getc(levelDB);
			//putchar(symbol);
			if (symbol == EOF) { return 0; }
			else if (symbol == NULL) { return 0; }
			else if (symbol == '\n') { break; }
			else
			{
				*(map + y * WIDTH + x) = symbol;
			}
		}

	fclose(levelDB);
}

// очистка консоли
void clearScreen()
{
	HANDLE                     hStdOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD                      count;
	DWORD                      cellCount;
	COORD                      homeCoords = { 0, 0 };

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut == INVALID_HANDLE_VALUE) return;

	/* Get the number of cells in the current buffer */
	if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
	cellCount = csbi.dwSize.X * csbi.dwSize.Y;

	/* Fill the entire buffer with spaces */
	if (!FillConsoleOutputCharacter(
		hStdOut,
		(TCHAR)' ',
		cellCount,
		homeCoords,
		&count
	)) return;

	/* Fill the entire buffer with the current colors and attributes */
	if (!FillConsoleOutputAttribute(
		hStdOut,
		csbi.wAttributes,
		cellCount,
		homeCoords,
		&count
	)) return;

	/* Move the cursor home */
	SetConsoleCursorPosition(hStdOut, homeCoords);
}
// отрисовка карты
void drawMap(const char* map, Point player, int countEnemies, Point* enemies)
{
	//system("CLS"); // выдает ошибку
	clearScreen();
	int isDrawCharacter = 0;

	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			//if (x >= WIDTH || x >= WIDTH) return;
			isDrawCharacter = 0;
			if (player.x == x && player.y == y)
			{
				putchar(1); putchar(' '); isDrawCharacter = 1;
			}

			if (isDrawCharacter == 0)
				for (int i = 0; i < countEnemies; i++)
					if ((*(enemies + i)).x == x && (*(enemies + i)).y == y)
					{
						putchar('}'); putchar('{'); isDrawCharacter = 1;
					}

			if (isDrawCharacter == 0)
				switch (getMapPoint(map, x, y))
				{
					case EMPTY_TAG: {
						putchar(' '); putchar(' ');
						break;
					}
					case WALL_TAG: {
						putchar(219); putchar(219);
						break;
					}
					case ENEMY_TAG: {
						putchar(' '); putchar(' ');
						break;
					}
					case START_TAG: {
						putchar(' '); putchar(' ');
						break;
					}
					case FINISH_TAG: {
						putchar(177); putchar(177);
						break;
					}
					default: {
						putchar('*'); putchar('*');
						break;
					}
				}
		}
		putchar('\n');
	}
}

// проверка на столкновение с монстром
int playerCollisionEnemy(Point player, int countEnemies, Point* enemies)
{
	for (int i = 0; i < countEnemies; i++)
		if (comparePoint(player, enemies[i]))
			return 1;
	return 0;
}

// ход монстров
void enemyMove(const char* map, Point player, int countEnemies, Point* enemies)
{ // ИИ противника
	int _x, _y;
	int enemyMove;
	/*
	enemyMove
	-1 между игроком и противником есть преграды - ход не сделан
	0 поиск препятсвий в сторону игрока
	1 между игроком и противником нет преград - ход сделан
	*/


	for (int i = 0; i < countEnemies; i++)
	{
		enemyMove = 0;
		_x = enemies[i].x;
		_y = enemies[i].y;
		//if (enemies[i].x == player.x || enemies[i].y == player.y)
		if (_y == player.y)
		{
			while (enemyMove == 0) 
			{
				if (_x < player.x) // игрок справа
				{
					// игрок не в прямой видимости
					if (getMapPoint(map, _x + 1, _y) == WALL_TAG)
						enemyMove = -1;

					// игрок в прямой видимости - движемся в его сторону
					if (_x + 1 == player.x) {
						enemies[i].x++; enemyMove = 1;
					}

					// проверяем следующию клетку
					if (_x == WIDTH - 1) enemyMove = -1;
					else _x++;

				}
				else // игрок слева
				{
					// игрок не в прямой видимости
					if (getMapPoint(map, _x - 1, _y) == WALL_TAG)
						enemyMove = -1;

					// игрок в прямой видимости - движемся в его сторону
					if (_x - 1 == player.x) {
						enemies[i].x--; enemyMove = 1;
					}

					// проверяем следующию клетку
					if (_x == 0) enemyMove = -1;
					else _x--;
				}
			}
		}
		else if (_x == player.x)
		{
			while (enemyMove == 0)
			{
				if (_y < player.y) // игрок снизу
				{
					// игрок не в прямой видимости
					if (getMapPoint(map, _x, _y + 1) == WALL_TAG)
						enemyMove = -1;

					// игрок в прямой видимости - движемся в его сторону
					if (_y + 1 == player.y) {
						enemies[i].y++; enemyMove = 1;
					}

					// проверяем следующию клетку
					if (_y == HEIGHT + 1) enemyMove = -1;
					else _y++;
				}
				else // игрок сверху
				{
					// игрок не в прямой видимости
					if (getMapPoint(map, _x, _y - 1) == WALL_TAG)
						enemyMove = -1;

					// игрок в прямой видимости - движемся в его сторону
					if (_y - 1 == player.y) {
						enemies[i].y--; enemyMove = 1;
					}
					// проверяем следующию клетку
					if (_y == 0) enemyMove = -1;
					else _y--;
				}
			}
		}
		else enemyMove = -1;

		if (enemyMove == -1) { //рандом ход
				int countVectors = 0,
					vectors[4] = { 0, 0, 0, 0 };
				if (enemies[i].y != 0 && getMapPoint(map, enemies[i].x, enemies[i].y - 1) != WALL_TAG)
				{
					vectors[0] = 1; countVectors++;
				}
				if (enemies[i].x != 0 && getMapPoint(map, enemies[i].x - 1, enemies[i].y) != WALL_TAG)
				{
					vectors[1] = 1; countVectors++;
				}
				if (enemies[i].y != HEIGHT - 1 && getMapPoint(map, enemies[i].x, enemies[i].y + 1) != WALL_TAG)
				{
					vectors[2] = 1; countVectors++;
				}
				if (enemies[i].x != WIDTH - 1 && getMapPoint(map, enemies[i].x + 1, enemies[i].y) != WALL_TAG)
				{
					vectors[3] = 1; countVectors++;
				}

				if (countVectors != 0)
				{
					int j = 0, randomVector = rand() % countVectors;
					for (int k = 0; k < 4 && enemyMove == -1; k++)
					{
						if (vectors[k] != 0)
						{

							if (randomVector == j)
							{
								switch (k)
								{
								case 0: {
									enemies[i].y--; break;
								}
								case 1: {
									enemies[i].x--; break;
								}
								case 2: {
									enemies[i].y++; break;
								}
								case 3: {
									enemies[i].x++; break;
								}
								default: break;
								}
								enemyMove = 1;
							}
							else j++;
						}
					}


				}
				enemyMove = 1;

			}
		
	}
}

// расстановка монстров на карте
int initEnemies(const char* map, Point **enemies)
{
	int count = 0;

	for (int y = 0; y < HEIGHT; y++)
		for (int x = 0; x <= WIDTH; x++)
			if (getMapPoint(map, x, y) == ENEMY_TAG)
				count++;

	*enemies = (Point*)calloc(count, sizeof(Point));

	count = 0;
	for (int y = 0; y < HEIGHT; y++)
		for (int x = 0; x <= WIDTH; x++)
			if (getMapPoint(map, x, y) == ENEMY_TAG)
			{
				(*(*enemies + count)).x = x;
				(*(*enemies + count)).y = y;
				count++;
			}
	return count;
}

// определдение позиции игрока на карте
void initPlayer(const char* map, Point* player)
{
	for (int y = 0; y < HEIGHT; y++)
		for (int x = 0; x <= WIDTH; x++)
			if (getMapPoint(map, x, y) == START_TAG)
				*player = {x, y};
}
/* endGame
* 0 игра продоложается
* 1 игрок проиграл
* 2 выиграл
*/
// игровой ход - ожидание действия игрока
int gameMove(const char* map)
{
	int isMovePlayer, countEnemies, endGame = 0;

	Point player;
	Point* enemies = NULL;

	countEnemies = initEnemies(map, &enemies);
	initPlayer(map, &player);

	drawMap(map, player, countEnemies, enemies);
	char code;

	while (endGame == 0)
	{
		isMovePlayer = 0;
		code = _getwch();
		switch (code)
		{
		case 'w': {
			if (player.y != 0 && getMapPoint(map, player.x, player.y - 1) != WALL_TAG)
			{
				player.y--; isMovePlayer = 1;
			}
			break;
		}
		case 'a': {
			if (player.x != 0 && getMapPoint(map, player.x - 1, player.y) != WALL_TAG)
			{
				player.x--; isMovePlayer = 1;
			}
			break;
		}
		case 's': {
			if (player.y != HEIGHT - 1 && getMapPoint(map, player.x, player.y + 1) != WALL_TAG)
			{
				player.y++; isMovePlayer = 1;
			}
			break;
		}
		case 'd': {
			if (player.x != WIDTH - 1 && getMapPoint(map, player.x + 1, player.y) != WALL_TAG)
			{
				player.x++; isMovePlayer = 1;
			}
			break;
		}
		default: break;
		}
		if (isMovePlayer)
		{
			if (playerCollisionEnemy(player, countEnemies, enemies)) endGame = 1;

			// Игрок сделал ход - теперь нужно чтобы сходил противник (бот) 
			enemyMove(map, player, countEnemies, enemies);

			if (playerCollisionEnemy(player, countEnemies, enemies)) endGame = 1;

			if (getMapPoint(map, player.x, player.y) == FINISH_TAG) endGame = 2;


			drawMap(map, player, countEnemies, enemies);
		}
	}

	free(enemies);
	return endGame;
}

void saveRate(char* nick, int time)
{
	FILE* rateDB;
	rateDB = fopen("rate.txt", "r+");
	if (rateDB == NULL) return;

	char* line = (char*)calloc(22, sizeof(char)),
		* _nick = NULL,
		* _timeStr = NULL;
	int _time, isFind = 0;
	long offset = 0;

	while (fgets(line, 22, rateDB) != NULL)
	{
		_nick = strtok(line, " ");
		if (_nick == NULL) continue;
		
		_timeStr = strtok(NULL, " ");
		if (_timeStr == NULL || atoi(_timeStr) == NULL) continue;
		_time = atoi(_timeStr);

		if (strcmp(_nick, nick) == 0)
		{
			if (time >= _time)
				isFind = -1;
			else {
				offset = ftell(rateDB) - 21;
				isFind = 1;
			}
			break;
		}
	}

	if (isFind != -1)
	{
		if (isFind == 1 && fseek(rateDB, offset, SEEK_SET) == 0)
			fprintf(rateDB, "%10.10s %.10d", nick, time);
		else if (isFind == 0 && fseek(rateDB, 0, SEEK_END) == 0)
			fprintf(rateDB, "%10.10s %.10d", nick, time);
	}

	free(line);
	fclose(rateDB);
}

void printRateTable()
{
	FILE* rateDB;
	rateDB = fopen("rate.txt", "r");
	if (rateDB == NULL) return;

	char* nick = NULL, *time = NULL, * line = (char*)calloc(22, sizeof(char));

	printf("\n\tTable of rate\n");
	for (int i = 0; i < 27; i++) putchar('~');
	printf("\n| %-10.10s | %10.10s |\n", "Nick", "best time");

	while (fgets(line, 22, rateDB) != NULL)
	{
		nick = strtok(line, " ");
		time = strtok(NULL, " ");
		for (int i = 0; i < 27; i++) putchar('~');
		printf("\n| %-10.10s | %10.d |\n", nick, atoi(time));
	}
	for (int i = 0; i < 27; i++) putchar('~');
	putchar('\n');


	free(line);
	fclose(rateDB);
}

void menu()
{
	char* map = (char*)calloc(HEIGHT * WIDTH, sizeof(Point));
	loadMapFromFile(map);

	time_t startTime = time(NULL);
	int lastGameStatus = 0;
	char nick[20];

	while (1)
	{
		switch (lastGameStatus)
		{
			case 0: { // игр еще не было
				break;
			}
			case 1: { // проиграл
				printf(COLOR_RED "\nYou lose!\n" ANSI_RESET_ALL);
				break;
			}
			case 2: { // выиграл
				int curTime = int(time(NULL) - startTime);
				printf(COLOR_GREEN "\nYou win!\n" ANSI_RESET_ALL);
				printf("\nComplietion time = %ld sec\n", curTime);
				saveRate(nick, curTime);
				break;
			}
			default:break;
		}
		printRateTable();

		printf("\nEnter your Nick to start game:");
		while (scanf("%s", nick) == 0);

		startTime = time(NULL);
		lastGameStatus = gameMove(map);
	}

	free(map);
}

int main() { menu(); }