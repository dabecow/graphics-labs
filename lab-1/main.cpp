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

struct ClientAreaConfig {
	POINT topLeft;
	const Mode mode = MODE_HORIZONTAL;
};

struct MenuConfig {	
	const DWORD height = HEIGHT / 10,
	width = WIDTH / 3;
	const DWORD padding = PADDING;

	int currentMenu = MT_LINE;
	const int numberOfElements = 4;
	const char* menuElements[4] = {"Line", "Rectangle", "Circle", "Zoom"};
	POINT topLeftPoints[4];
};

struct Line {
	POINT pointBegin;
	POINT pointEnd;
};

Line line;
ClientAreaConfig areaConfig;
MenuConfig menuConfig;

CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
HWND hwnd;
HANDLE rHnd;
DWORD SelectedItem = 0;
BOOL isButtonPressed = false;
HPEN redPen;
HPEN greenPen;

void initMenu();
void initPens();

void drawMenu(HDC hdc);

bool inMenu(POINT cursor);
bool inDrawArea(POINT cursor);
bool pointInRect(POINT point, POINT topLeftRectPoint);
int chosenType();

void drawMenuRectangle(HDC hdc, POINT topLeft, const char* text, bool isChosen);
void drawFigure(HDC hdc);
void drawLineStart(HDC hdc, POINT point);
void drawLineMoved(HDC hdc, POINT point);
void drawLineEnd(HDC hdc, POINT point);

void drawA(HDC hdc)
{
	DWORD offsetX = areaConfig.topLeft.x + PADDING;
	DWORD offsetY = areaConfig.topLeft.y + PADDING;
	Rectangle(hdc, offsetX, offsetY, offsetX + WIDTH, offsetY + HEIGHT); // Рисуем квадрат
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
					menuConfig.currentMenu = _chosenType;
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
	areaConfig.topLeft.x = PADDING;
	if (areaConfig.mode == MODE_HORIZONTAL) {
		areaConfig.topLeft.y = PADDING;
	} else {	
		areaConfig.topLeft.y = PADDING * 2 + menuConfig.height;
	}

	POINT topLeft = {};
	for (int i = 0; i < menuConfig.numberOfElements; i++) {
		if (areaConfig.mode == MODE_HORIZONTAL) {
			topLeft.x = areaConfig.topLeft.x + WIDTH + PADDING * 2;
			topLeft.y = areaConfig.topLeft.y + PADDING + (menuConfig.height + menuConfig.padding) * i;
		} else {
			topLeft.x = PADDING + (menuConfig.width + menuConfig.padding) * i;
			topLeft.y = PADDING;
		}
		menuConfig.topLeftPoints[i] = topLeft;
	}
}

void drawMenu(HDC hdc) {
	POINT topLeft = {};
	for (int i = 0; i < menuConfig.numberOfElements; i++) {
		topLeft = menuConfig.topLeftPoints[i];

		if (menuConfig.currentMenu == i)
			SelectObject(hdc, greenPen);

		Rectangle(hdc, topLeft.x, topLeft.y, topLeft.x + menuConfig.width, topLeft.y + menuConfig.height);
		SelectObject(hdc, redPen);
		
		TextOutA(hdc, topLeft.x + menuConfig.width / 10, topLeft.y + menuConfig.height / 2, menuConfig.menuElements[i], strlen(menuConfig.menuElements[i]));

	}
}

bool pointInRect(POINT point, POINT topLeftRectPoint) {
	return 
		point.x >= topLeftRectPoint.x 
		&& point.x <= topLeftRectPoint.x + menuConfig.width 
		&& point.y >= topLeftRectPoint.y
		&& point.y <= topLeftRectPoint.y + menuConfig.height;
}

int chosenType() {
	POINT tagPOINT;
	GetCursorPos((LPPOINT)&tagPOINT);
	ScreenToClient(hwnd, (LPPOINT)&tagPOINT);

	POINT topLeft = {};
	for (int i = 0; i < menuConfig.numberOfElements; i++) {
		if (pointInRect(tagPOINT, menuConfig.topLeftPoints[i]))
			return i;
	}

	return MT_NONE;
}

bool inMenu(POINT cursor) {
	return areaConfig.mode == MODE_HORIZONTAL 
		&& cursor.x >= menuConfig.topLeftPoints[0].x
		&& cursor.y >= menuConfig.topLeftPoints[0].y
		&& cursor.x <= menuConfig.topLeftPoints[0].x + menuConfig.width
		&& cursor.y <= menuConfig.topLeftPoints[menuConfig.numberOfElements - 1].y + HEIGHT
		||
		areaConfig.mode == MODE_VERTICAL 
		&& cursor.y <= menuConfig.topLeftPoints[0].y + menuConfig.height;
}

bool inDrawArea(POINT cursor) {
	return cursor.x >= areaConfig.topLeft.x 
		&& cursor.y >= areaConfig.topLeft.y
		&& cursor.x <= areaConfig.topLeft.x + WIDTH 
		&& cursor.y <= areaConfig.topLeft.y + HEIGHT;
}

void drawFigure(HDC hdc) {

	switch (menuConfig.currentMenu) {
		case MT_LINE:
			MoveToEx(hdc, line.pointBegin.x, line.pointBegin.y, NULL);
			LineTo(hdc, line.pointEnd.x, line.pointEnd.y); 
			break;
		case MT_RECTANGLE:
			Rectangle(hdc, line.pointBegin.x, line.pointBegin.y, line.pointEnd.x, line.pointEnd.y);
			break;
		case MT_CIRCLE:
			Ellipse(hdc, line.pointBegin.x, line.pointBegin.y, line.pointEnd.x, line.pointEnd.y);
			break;
		default:
			break;
	}
}

void drawLineStart(HDC hdc, POINT point) {
	line.pointBegin = point;
	line.pointEnd = point;
	SetROP2(hdc, R2_NOTXORPEN);
	MoveToEx(hdc, line.pointBegin.x, line.pointBegin.y, NULL);
	LineTo(hdc, line.pointEnd.x, line.pointEnd.y);
}

void drawLineMoved(HDC hdc, POINT point) {
	MoveToEx(hdc, line.pointBegin.x, line.pointBegin.y, NULL);
	LineTo(hdc, line.pointEnd.x, line.pointEnd.y);
	line.pointEnd = point;
	MoveToEx(hdc, line.pointBegin.x, line.pointBegin.y, NULL);
	LineTo(hdc, line.pointEnd.x, line.pointEnd.y);
}
void drawLineEnd(HDC hdc, POINT point) {
	MoveToEx(hdc, line.pointBegin.x, line.pointBegin.y, NULL);
	LineTo(hdc, line.pointEnd.x, line.pointEnd.y);
	SetROP2(hdc, R2_COPYPEN);
}