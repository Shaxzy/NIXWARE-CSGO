#pragma once

#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"
#include "input.hpp"
#include "math.hpp"

struct DrawData
{
	DrawData(int h, int w, int l, int u, int r, int d)
	{
		Height = h;
		Width = w;
		Left = l;
		Up = u;
		Right = r;
		Down = d;
		rec_type = 1;
	}
	DrawData(int l, int u, int r, int d)
	{
		Left = l;
		Up = u;
		Right = r;
		Down = d;
		rec_type = 2;
	}
	const DrawData Outline()
	{
		if (rec_type == 1)
		{
			return DrawData(Height, Width, Left - 1, Up - 1, Right + 1, Down + 1);
		}
		else if (rec_type == 2)
		{
			return DrawData(Left - 1, Up - 1, Right + 1, Down + 1);
		}
	}
	const DrawData Inline()
	{
		if (rec_type == 1)
		{
			return DrawData(Height, Width, Left + 1, Up + 1, Right - 1, Down - 1);
		}
		else if (rec_type == 2)
		{
			return DrawData(Left + 1, Up + 1, Right - 1, Down - 1);
		}
	}

	int Height, Width, Left, Up, Right, Down, rec_type;
};

class CDraw : public Singleton<CDraw>
{
public:
	void SetupFonts();

	void custom_box(int x, int y, int w, int h);

	void D3D_ImDrawRectRainbow(int x, int y, int width, int height, float flSpeed, float & flRainbow);

	void DrawFilled3DBox(Vector origin, int width, int height, Color outline, Color filling);

	// Normal Drawing functions
	void Clear(int x, int y, int w, int h, Color color);
	void DrawRect(int x, int y, int w, int h, Color col);
	void DrawCorners(RECT rect, Color Col, Color Col2);
	void DrawOutlineRect(int x, int y, int w, int h, Color color);
	void DrawString(DWORD font, int x, int y, Color color, DWORD alignment, const char* msg, ...);
	void DrawHeader(int x, int y, int w, int HealthBarWidth, Color color);
	void DrawVerBar(int x, int y, int w, int h, int val, Color color1, Color color2);
	void OutlineRainbow(int x, int y, int w, int h, float flSpeed, float &flRainbow);
	void Line(int x, int y, int x2, int y2, Color color);
	void PolyLine(int *x, int *y, int count, Color color);
	void Polygon(int count, Vertex_t* Vertexs, Color color);
	void PolygonOutline(int count, Vertex_t* Vertexs, Color color, Color colorLine);
	void PolyLine(int count, Vertex_t* Vertexs, Color colorLine);
	void DrawWave(Vector loc, float radius, Color color);
	void DrawCircle3D(Vector position, float points, float radius, Color Color);
	void Circle(int x, int y, int r, int seg, Color color);
	void OutlineCircle(int x, int y, int r, int seg, Color color);
	void Pixel(int x, int y, Color col);

	void DrawCoalBox(int x, int y, int w, int h, Color color);

	void DrawOutlineCoalBox(int x, int y, int w, int h, Color color);

	void BoxDefault(DrawData Data, Color pColor);

	// Gradient Functions
	void GradientV(int x, int y, int w, int h, Color c1, Color c2);
	void GradientH(int x, int y, int w, int h, Color c1, Color c2);

	// Text functions
	void Text(int x, int y, Color color, DWORD font, const char* text, ...);
	void OutlinedBox2(int x, int y, int w, int h, Color color);
	void DrawRectFilled(int x0, int y0, int x1, int y1, Color color);
	void FillRGBA(int x, int y, int w, int h, Color c);
	void TexturedPolygon(int n, std::vector<Vertex_t> vertice, Color color);
	void Textf(int x, int y, Color color, DWORD font, const char* fmt, ...);
	void DrawFilledCircle(Vector2D center, Color color, float radius, float points);
	void Text(int x, int y, Color color, DWORD font, const wchar_t* text);
	void Text(int x, int y, DWORD font, const wchar_t* text);

	// Other rendering functions
	RECT GetViewport();
	class Fonts
	{
	public:
		DWORD Default;
		DWORD ESP;
		DWORD ESPFLAG;
		DWORD ESPMini;
		DWORD Defuse;
		DWORD DroppedGuns;
		DWORD LBY;
		DWORD AAIndicator;
		DWORD WeaponIcons;
		DWORD HitMarker;
	} font;
};