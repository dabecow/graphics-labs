#ifndef UNICODE
#define UNICODE
#endif

#define VIEWPORT_LEFT 5
#define VIEWPORT_TOP 5
#define VIEWPORT_RIGHT 605
#define VIEWPORT_BOTTOM 605
#define CHOSEN_PADDING 2
#include <windows.h>
#include <iostream>
#include <conio.h>
#include <vector>
#include <stack>
using namespace std;

// int WIDTH = VIEWPORT_RIGHT - VIEWPORT_LEFT;
// int HEIGHT = VIEWPORT_BOTTOM - VIEWPORT_TOP;
// int PADDING = VIEWPORT_LEFT;

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

enum FigureType {
	FT_LINE = 0,
	FT_RECTANGLE,
	FT_CIRCLE
};

struct fPOINT {
	float x;
	float y;
};

struct Figure {
	fPOINT pointBegin,
		   pointEnd;
	int figureType;
};

struct ClientAreaConfig {
	POINT topLeft;
	const Mode mode = MODE_HORIZONTAL;
};

struct MenuConfig {	
	const DWORD height = (VIEWPORT_BOTTOM - VIEWPORT_TOP) / 10,
	width = (VIEWPORT_RIGHT - VIEWPORT_LEFT) / 3;
	const DWORD padding = VIEWPORT_LEFT;

	int currentMenu = MT_LINE;
	const int numberOfElements = 4;
	const char* menuElements[4] = {"Line", "Rectangle", "Circle", "Unzoom"};
	POINT topLeftPoints[4];
};

struct Line {
	POINT pointBegin;
	POINT pointEnd;
};

struct fLine {
	fPOINT pointBegin;
	fPOINT pointEnd;
};

stack<fLine> zoomingStack;

vector<Figure> figures;
Line line;

ClientAreaConfig areaConfig;
MenuConfig menuConfig;

CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };

HWND hwnd;
HANDLE rHnd;
DWORD SelectedItem = 0;
BOOL leftButtonPressed = false;
BOOL rightButtonPressed = false;
HPEN redPen;
HPEN greenPen;

void initMenu();
void initPens();

void drawMenu(HDC hdc);

bool inMenu(POINT cursor);
bool inDrawArea(POINT cursor);
bool pointInRect(POINT point, POINT topLeftRectPoint);
int chosenType();

void addModel(int type, POINT p1, POINT p2);

void drawModels(HDC hdc);
void drawMenuRectangle(HDC hdc, POINT topLeft, const char* text, bool isChosen);

void drawLineStart(HDC hdc, POINT point);
void drawLineMoved(HDC hdc, POINT point);
void drawLineEnd(HDC hdc, POINT point);

void drawRectStart(HDC hdc, POINT point);
void drawRectMoved(HDC hdc, POINT point);
void drawRectEnd(HDC hdc, POINT point);

void redraw(HDC hdc);

fPOINT toNDC(POINT p);
POINT toDC(fPOINT fp);

void processZooming();

void drawViewPort(HDC hdc)
{
	SelectObject(hdc, redPen);
	Rectangle(hdc, VIEWPORT_LEFT, VIEWPORT_TOP, VIEWPORT_RIGHT, VIEWPORT_BOTTOM); // Рисуем квадрат
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
				if (_chosenType == MT_ZOOM) {
					_chosenType = MT_LINE;
					if (zoomingStack.size() <= 1) 
						return;
					zoomingStack.pop(); 
					redraw(hdc);
				}
				if (_chosenType != MT_NONE) {
					menuConfig.currentMenu = _chosenType;
					drawMenu(hdc);
				}
			} 
			if (!leftButtonPressed && inDrawArea(cursorPoint)) {
				drawLineStart(hdc, cursorPoint);
				leftButtonPressed = true;
			}
		} else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED) {
			if (!leftButtonPressed && inDrawArea(cursorPoint)) {
				drawRectStart(hdc, cursorPoint);
				rightButtonPressed = true;
			}
		} else	
			if (mer.dwButtonState == 0) { 
				if (leftButtonPressed) {
					drawLineEnd(hdc, cursorPoint);
					if (inDrawArea(cursorPoint)) {
						addModel(menuConfig.currentMenu, line.pointBegin, line.pointEnd);
					}
					redraw(hdc);
					leftButtonPressed = false;
				} else if (rightButtonPressed) {
					drawRectEnd(hdc, cursorPoint);
					rightButtonPressed = false;
					processZooming();
					redraw(hdc);
				}
				
			}
		break;
	case MOUSE_MOVED:
		if (inDrawArea(cursorPoint)) {
			if (leftButtonPressed) {
				drawLineMoved(hdc, cursorPoint);
			} else if (rightButtonPressed) {
				drawRectMoved(hdc, cursorPoint);
			}
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

	fLine currentNDC = {
		.pointBegin = {.x = 0, .y =0}, 
		.pointEnd = {.x = 1, .y = 1}
	};

	zoomingStack.push(currentNDC);
	drawViewPort(hdc); 
	drawMenu(hdc);
	
	DWORD numEvents = 0;
	DWORD numEventsRead = 0;
	bool isRunning = true;

	
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
					redraw(hdc);
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
	areaConfig.topLeft.x = VIEWPORT_LEFT;
	if (areaConfig.mode == MODE_HORIZONTAL) {
		areaConfig.topLeft.y = VIEWPORT_LEFT;
	} else {	
		areaConfig.topLeft.y = VIEWPORT_TOP * 2 + menuConfig.height;
	}

	POINT topLeft = {};
	for (int i = 0; i < menuConfig.numberOfElements; i++) {
		if (areaConfig.mode == MODE_HORIZONTAL) {
			topLeft.x = areaConfig.topLeft.x + (VIEWPORT_RIGHT - VIEWPORT_LEFT) + VIEWPORT_LEFT * 2;
			topLeft.y = areaConfig.topLeft.y + VIEWPORT_TOP + (menuConfig.height + menuConfig.padding) * i;
		} else {
			topLeft.x = VIEWPORT_LEFT + (menuConfig.width + menuConfig.padding) * i;
			topLeft.y = VIEWPORT_TOP;
		}
		menuConfig.topLeftPoints[i] = topLeft;
	}
}

void drawMenu(HDC hdc) {
	SelectObject(hdc, redPen);
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
		&& cursor.y <= menuConfig.topLeftPoints[menuConfig.numberOfElements - 1].y + (VIEWPORT_BOTTOM - VIEWPORT_TOP)
		||
		areaConfig.mode == MODE_VERTICAL 
		&& cursor.y <= menuConfig.topLeftPoints[0].y + menuConfig.height;
}

bool inDrawArea(POINT cursor) {
	return cursor.x >= areaConfig.topLeft.x 
		&& cursor.y >= areaConfig.topLeft.y
		&& cursor.x <= areaConfig.topLeft.x + (VIEWPORT_RIGHT - VIEWPORT_LEFT) 
		&& cursor.y <= areaConfig.topLeft.y + (VIEWPORT_BOTTOM - VIEWPORT_TOP);
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

void drawRectStart(HDC hdc, POINT point) {
	line.pointBegin = point;
	line.pointEnd = point;
	SetROP2(hdc, R2_NOTXORPEN);
	Rectangle(hdc, point.x, point.y, point.x, point.y);
}

void drawRectMoved(HDC hdc, POINT point) {
	Rectangle(hdc, line.pointBegin.x, line.pointBegin.y, line.pointEnd.x, line.pointEnd.y);
	line.pointEnd.x = point.x;
	line.pointEnd.y = line.pointBegin.y + point.x - line.pointBegin.x;
	Rectangle(hdc, line.pointBegin.x, line.pointBegin.y, line.pointEnd.x, line.pointEnd.y);
}

void drawRectEnd(HDC hdc, POINT point) {
	Rectangle(hdc, line.pointBegin.x, line.pointBegin.y, line.pointEnd.x, line.pointEnd.y);
	SetROP2(hdc, R2_COPYPEN);
}

void addModel(int type, POINT p1, POINT p2) {
	Figure figure;
	switch (type) {
		case MT_LINE:
			figure.figureType = FT_LINE;
			break;
		case MT_RECTANGLE:
			figure.figureType = FT_RECTANGLE;
			break;
		case MT_CIRCLE:
			figure.figureType = FT_CIRCLE;
			break;
		default:
			return;
	}

	figure.pointBegin = toNDC(p1);
	figure.pointEnd = toNDC(p2);
	figures.push_back(figure);
}

void drawModels(HDC hdc) {
	for (size_t i = 0; i < figures.size(); i++)
	{
		Figure* figure = &figures.at(i);
		POINT p1 = toDC(figure->pointBegin), p2 = toDC(figure->pointEnd);
		switch (figure->figureType)
		{
		case FT_LINE:
			MoveToEx(hdc, p1.x, p1.y, NULL);
			LineTo(hdc, p2.x, p2.y); 
			break;
		case FT_RECTANGLE:
			Rectangle(hdc, p1.x, p1.y, p2.x, p2.y);
			break;
		case FT_CIRCLE:
			Ellipse(hdc, p1.x, p1.y, p2.x, p2.y);	
			break;
		default:
			break;
		}		
	}
	
}

fPOINT toNDC(POINT p) {
	fLine currentNDC = zoomingStack.top();
	fPOINT sx = { 
		.x = (float) (VIEWPORT_RIGHT - VIEWPORT_LEFT)/(currentNDC.pointEnd.x - currentNDC.pointBegin.x),
		.y = (float) (VIEWPORT_BOTTOM - VIEWPORT_TOP)/(currentNDC.pointEnd.y - currentNDC.pointBegin.y)
	};
	fPOINT fp = {
		.x = (float) (p.x - VIEWPORT_LEFT)/sx.x + currentNDC.pointBegin.x, 
		.y = (float) (p.y - VIEWPORT_TOP)/sx.y +  + currentNDC.pointBegin.y
	};
	return fp;
}

POINT toDC(fPOINT fp) {
	fLine currentNDC = zoomingStack.top();
	fPOINT sx = { 
		.x = (float) (VIEWPORT_RIGHT - VIEWPORT_LEFT)/(currentNDC.pointEnd.x - currentNDC.pointBegin.x),
		.y = (float) (VIEWPORT_BOTTOM - VIEWPORT_TOP)/(currentNDC.pointEnd.y - currentNDC.pointBegin.y)
	};
	POINT p = {
		.x = VIEWPORT_LEFT + sx.x * (fp.x - currentNDC.pointBegin.x),
		.y = VIEWPORT_TOP + sx.y * (fp.y - currentNDC.pointBegin.y)
	};
	return p;
}

void processZooming() {
	fLine currentNDC = {
		.pointBegin = toNDC(line.pointBegin),
		.pointEnd = toNDC(line.pointEnd)
	};
	zoomingStack.push(currentNDC);
	menuConfig.currentMenu = MT_LINE;
}

void redraw(HDC hdc) {
	drawMenu(hdc);
	HRGN rgn = CreateRectRgn(VIEWPORT_LEFT, VIEWPORT_TOP, VIEWPORT_RIGHT, VIEWPORT_BOTTOM);
	SelectClipRgn(hdc, rgn);
	drawViewPort(hdc);
	drawModels(hdc);
	SelectClipRgn(hdc, NULL);
}