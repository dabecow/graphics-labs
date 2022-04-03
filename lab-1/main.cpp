#ifndef UNICODE
#define UNICODE
#endif

#define WIDTH 600
#define HEIGHT 600
#define PADDING 5
#define CHOSEN_PADDING 2
#include <windows.h>
#include <iostream>
#include <conio.h>
using namespace std;

struct MyPOINT {
	DWORD x;
	DWORD y;
};

enum Mode {
	MODE_HORIZONTAL,
	MODE_VERTICAL
};

enum MenuType {
	MT_LINE = 0,
	MT_RECTANGLE,
	MT_CIRCLE,
	MT_ZOOM,
	MT_NONE
};

struct MenuConfig {	
	const DWORD height = HEIGHT / 10,
	width = WIDTH / 3;
	const DWORD padding = PADDING;

	const Mode mode = MODE_HORIZONTAL;

	int currentMenu = MT_LINE;
	const int numberOfElements = 4;
	const char* menuElements[4] = {"Line", "Rectangle", "Circle", "Zoom"};
	POINT topLeftPoints[4];
	// MenuConfig(DWORD _height, DWORD _width, DWORD _padding, Mode _mode): height(_height), width(_width), padding(_padding), mode(_mode) {};
};

struct {
	POINT pointBegin;
	POINT pointEnd;
} Line;

MenuConfig config;

CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
HWND hwnd;
HANDLE rHnd;
DWORD SelectedItem = 0;
BOOL isButtonPressed = false;
HPEN redPen;
HPEN greenPen;

bool inMenu(POINT cursor);
bool inDrawArea(POINT cursor);

void initMenu();
void initPens();
void drawMenu(HDC hdc);
bool pointInRect(POINT point, POINT topLeftRectPoint);
int chosenType();
void drawMenuRectangle(HDC hdc, POINT topLeft, const char* text, bool isChosen);
void drawFigure(HDC hdc);

void drawLineStart(HDC hdc, POINT point);
void drawLineMoved(HDC hdc, POINT point);
void drawLineEnd(HDC hdc, POINT point);

// Функция рисования. Помещайте сюда всю графику
void drawA(HDC hdc)
{
	Rectangle(hdc, PADDING, PADDING, PADDING + WIDTH, PADDING + HEIGHT); // Рисуем квадрат
	drawMenu(hdc);
}

void setConsoleSize() // Задание размеров окна консоли
{
	const int colConsole = 90;
	const int rowConsole = 40;
	HANDLE hNdl = GetStdHandle(STD_OUTPUT_HANDLE);
	SMALL_RECT windowSize = { 0,0,colConsole - 1,rowConsole - 1 };
	SetConsoleWindowInfo(hNdl, TRUE, &windowSize);
	COORD bufferSize = { colConsole, rowConsole }; // размеры буфера
	SetConsoleScreenBufferSize(hNdl, bufferSize);
}

VOID MouseEventProc(MOUSE_EVENT_RECORD mer, HDC hdc)
{
	tagPOINT cursorPoint;

	HCURSOR hCurs1 = GetCursor();

	GetCursorPos((LPPOINT)&cursorPoint);
	ScreenToClient(hwnd, (LPPOINT)&cursorPoint);

	switch (mer.dwEventFlags)
	{
	case 0:

		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			if (inMenu(cursorPoint)) {
				int _chosenType = chosenType();
				if (_chosenType != MT_NONE) {
					config.currentMenu = _chosenType;
					drawMenu(hdc);
				}
			} 
			if (!isButtonPressed && inDrawArea(cursorPoint)) {
				drawLineStart(hdc, cursorPoint);
				isButtonPressed = true;
			}
		}
		else	
			if (mer.dwButtonState == 0 && isButtonPressed)
		    { 
				drawLineEnd(hdc, cursorPoint);
				if (inDrawArea(cursorPoint))
					drawFigure(hdc);
				isButtonPressed = false;
			}
		break;
	case MOUSE_MOVED:
		if (isButtonPressed && inDrawArea(cursorPoint))
		{
			drawLineMoved(hdc, cursorPoint);
		}
  	}
}

int main()
{
	hwnd = GetConsoleWindow();
	HDC hdc = GetDC(hwnd);
	rHnd = GetStdHandle(STD_INPUT_HANDLE);
	DWORD fdwMode;
	SetConsoleTitle(L"Simple Rectangle Drawing");
	setConsoleSize();
	fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
	SetConsoleMode(rHnd, fdwMode);

	initPens();
	initMenu();
	SelectObject(hdc, redPen); // загружаем созданное перо в контекст
	drawA(hdc); 

	DWORD numEvents = 0;
	DWORD numEventsRead = 0;
	bool isRunning = true;

	// initMenu();
	while (isRunning) {
		// Определить количество событий консоли
		GetConsoleScreenBufferInfo(rHnd, &csbi);
		GetNumberOfConsoleInputEvents(rHnd, &numEvents);

		if (numEvents != 0) {

			INPUT_RECORD* eventBuffer = new INPUT_RECORD[numEvents];
			// Извлечение данных во временный буфер событий eventBuffer[]
			ReadConsoleInput(rHnd, eventBuffer, numEvents, &numEventsRead);

			for (DWORD i = 0; i < numEventsRead; ++i) {

				if (eventBuffer[i].EventType == KEY_EVENT) {
					if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
						isRunning = false; 
				}

				if (eventBuffer[i].EventType == FOCUS_EVENT) {
					Sleep(300);
					drawA(hdc); 
				}
				
				else if (eventBuffer[i].EventType == MOUSE_EVENT) {
					MouseEventProc(eventBuffer[i].Event.MouseEvent, hdc);	
				}
			}
			delete[] eventBuffer;
		}
	}

	ReleaseDC(hwnd, hdc); //освобождаем дескрипторы
	DeleteObject(redPen);
	DeleteObject(greenPen);
}

void initPens() {
	redPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
	greenPen = CreatePen(PS_SOLID, 3, RGB(0, 255, 0));
}

void initMenu() {
	POINT topLeft = {};
	for (int i = 0; i < config.numberOfElements; i++) {
		if (config.mode == MODE_HORIZONTAL) {
			topLeft.x = WIDTH + PADDING * 2;
			topLeft.y = PADDING + (config.height + config.padding) * i;
		} else {
			topLeft.x = PADDING + (config.width + config.padding) * i;
			topLeft.y = PADDING;
		}
		config.topLeftPoints[i] = topLeft;
	}
}

void drawMenu(HDC hdc) {
	POINT topLeft = {};
	for (int i = 0; i < config.numberOfElements; i++) {
		topLeft = config.topLeftPoints[i];

		if (config.currentMenu == i)
			SelectObject(hdc, greenPen);

		Rectangle(hdc, topLeft.x, topLeft.y, topLeft.x + config.width, topLeft.y + config.height);
		SelectObject(hdc, redPen);
		
		TextOutA(hdc, topLeft.x + config.width / 10, topLeft.y + config.height / 2, config.menuElements[i], strlen(config.menuElements[i]));

	}
}

bool pointInRect(POINT point, POINT topLeftRectPoint) {
	return 
		point.x >= topLeftRectPoint.x 
		&& point.x <= topLeftRectPoint.x + config.width 
		&& point.y >= topLeftRectPoint.y
		&& point.y <= topLeftRectPoint.y + config.height;
}

int chosenType() {
	POINT tagPOINT;
	GetCursorPos((LPPOINT)&tagPOINT);
	ScreenToClient(hwnd, (LPPOINT)&tagPOINT);

	POINT topLeft = {};
	for (int i = 0; i < config.numberOfElements; i++) {
		if (pointInRect(tagPOINT, config.topLeftPoints[i]))
			return i;
	}

	return MT_NONE;
}

bool inMenu(POINT cursor) {
	return cursor.x >= config.topLeftPoints[0].x
		&& cursor.y >= config.topLeftPoints[0].y
		&& cursor.x <= config.topLeftPoints[0].x + config.width
		&& cursor.y <= config.topLeftPoints[config.numberOfElements - 1].y + HEIGHT;
}

bool inDrawArea(POINT cursor) {
	return cursor.x >= PADDING 
		&& cursor.y >= PADDING
		&& cursor.x <= PADDING + WIDTH 
		&& cursor.y <= PADDING + HEIGHT;
}

void drawFigure(HDC hdc) {

	switch (config.currentMenu) {
		case MT_LINE:
			MoveToEx(hdc, Line.pointBegin.x, Line.pointBegin.y, NULL);
			LineTo(hdc, Line.pointEnd.x, Line.pointEnd.y); 
			break;
		case MT_RECTANGLE:
			Rectangle(hdc, Line.pointBegin.x, Line.pointBegin.y, Line.pointEnd.x, Line.pointEnd.y);
			break;
		case MT_CIRCLE:
			Ellipse(hdc, Line.pointBegin.x, Line.pointBegin.y, Line.pointEnd.x, Line.pointEnd.y);
			break;
		default:
			break;
	}
}

void drawLineStart(HDC hdc, POINT point) {
	Line.pointBegin = point;
	Line.pointEnd = point;
	SetROP2(hdc, R2_NOTXORPEN);
	MoveToEx(hdc, Line.pointBegin.x, Line.pointBegin.y, NULL);
	LineTo(hdc, Line.pointEnd.x, Line.pointEnd.y);
}

void drawLineMoved(HDC hdc, POINT point) {
	MoveToEx(hdc, Line.pointBegin.x, Line.pointBegin.y, NULL);
	LineTo(hdc, Line.pointEnd.x, Line.pointEnd.y);
	Line.pointEnd = point;
	MoveToEx(hdc, Line.pointBegin.x, Line.pointBegin.y, NULL);
	LineTo(hdc, Line.pointEnd.x, Line.pointEnd.y);
}
void drawLineEnd(HDC hdc, POINT point) {
	MoveToEx(hdc, Line.pointBegin.x, Line.pointBegin.y, NULL);
	LineTo(hdc, Line.pointEnd.x, Line.pointEnd.y);
	SetROP2(hdc, R2_COPYPEN);
}