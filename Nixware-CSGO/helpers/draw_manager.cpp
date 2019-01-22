#pragma once
#include "draw_manager.hpp"

// We don't use these anywhere else, no reason for them to be
// available anywhere else

enum FontRenderFlag_t
{
	FONT_LEFT = 0,
	FONT_RIGHT = 1,
	FONT_CENTER = 2
};

// Initialises the rendering system, setting up fonts etc
void CDraw::SetupFonts()
{
	font.Default = 0x1D; // MainMenu Font from vgui_spew_fonts 
	font.ESP = Interfaces::Surface->CreateFont_();
	font.ESPMini = Interfaces::Surface->CreateFont_();
	font.ESPFLAG = Interfaces::Surface->CreateFont_();
	font.Defuse = Interfaces::Surface->CreateFont_();
	font.DroppedGuns = Interfaces::Surface->CreateFont_();
	font.LBY = Interfaces::Surface->CreateFont_();
	font.AAIndicator = Interfaces::Surface->CreateFont_();
	font.WeaponIcons = Interfaces::Surface->CreateFont_();
	font.HitMarker = Interfaces::Surface->CreateFont_();

	Interfaces::Surface->SetFontGlyphSet(font.WeaponIcons, ("AstriumWep"), 16, 500, 0, 0, FONTFLAG_OUTLINE);
	Interfaces::Surface->SetFontGlyphSet(font.AAIndicator, ("Arial"), 60, 500, 0, 0, FONTFLAG_ANTIALIAS);
	Interfaces::Surface->SetFontGlyphSet(font.ESP, ("Arial"), 16, 500, 0, 0, FONTFLAG_OUTLINE);
	Interfaces::Surface->SetFontGlyphSet(font.ESPFLAG, ("Verdana"), 12, 100, 0, 0, FONTFLAG_OUTLINE); /////
	Interfaces::Surface->SetFontGlyphSet(font.Defuse, ("Tahoma"), 15, 700, 0, 0, FONTFLAG_DROPSHADOW);
	Interfaces::Surface->SetFontGlyphSet(font.ESPMini, ("Arial"), 16, 500, 0, 0, FONTFLAG_OUTLINE); // Visitor TT2 BRK
	Interfaces::Surface->SetFontGlyphSet(font.DroppedGuns, ("Tahoma"), 8, 700, 0, 0, FONTFLAG_DROPSHADOW);
	Interfaces::Surface->SetFontGlyphSet(font.LBY, ("Verdana"), 36, 900, 0, 0, FONTFLAG_ANTIALIAS);
	Interfaces::Surface->SetFontGlyphSet(font.HitMarker, ("Arial"), 25, FW_NORMAL, NULL, NULL, FONTFLAG_ANTIALIAS | FONTFLAG_OUTLINE);
}

auto box = [](int x, int y, int w, int h, Color c) {
	CDraw::Get().Line(x, y, x, y + h, c);
	CDraw::Get().Line(x, y + h, x + w + 1, y + h, c);
	CDraw::Get().Line(x + w, y, x + w, y + h, c);
	CDraw::Get().Line(x, y, x + w, y, c);
};

void CDraw::custom_box(int x, int y, int w, int h)
{
	DrawRectFilled(x, y, w, h, Color(30, 30, 30));
	box(x - 0, y - 0, w + 0 * 2, h + 0 * 2, Color::Gray_v2);
	box(x - 1, y - 1, w + 1 * 2, h + 1 * 2, Color::Black);
	box(x - 2, y - 2, w + 2 * 2, h + 2 * 2, Color::Gray_v3);
	box(x - 3, y - 3, w + 3 * 2, h + 3 * 2, Color::Gray_v3);
	box(x - 4, y - 4, w + 4 * 2, h + 4 * 2, Color::Gray_v3);
	box(x - 5, y - 5, w + 5 * 2, h + 5 * 2, Color::Black);
	box(x - 6, y - 6, w + 6 * 2, h + 6 * 2, Color::Gray_v2);
}

void CDraw::DrawFilled3DBox(Vector origin, int width, int height, Color outline, Color filling)
{
	float difw = float(width / 2);
	float difh = float(height / 2);
	Vector boxVectors[8] =
	{
		Vector(origin.x - difw, origin.y - difh, origin.z - difw),
		Vector(origin.x - difw, origin.y - difh, origin.z + difw),
		Vector(origin.x + difw, origin.y - difh, origin.z + difw),
		Vector(origin.x + difw, origin.y - difh, origin.z - difw),
		Vector(origin.x - difw, origin.y + difh, origin.z - difw),
		Vector(origin.x - difw, origin.y + difh, origin.z + difw),
		Vector(origin.x + difw, origin.y + difh, origin.z + difw),
		Vector(origin.x + difw, origin.y + difh, origin.z - difw),
	};

	static Vector vec0, vec1, vec2, vec3,
		vec4, vec5, vec6, vec7;

	if (Math::WorldToScreen(boxVectors[0], vec0) &&
		Math::WorldToScreen(boxVectors[1], vec1) &&
		Math::WorldToScreen(boxVectors[2], vec2) &&
		Math::WorldToScreen(boxVectors[3], vec3) &&
		Math::WorldToScreen(boxVectors[4], vec4) &&
		Math::WorldToScreen(boxVectors[5], vec5) &&
		Math::WorldToScreen(boxVectors[6], vec6) &&
		Math::WorldToScreen(boxVectors[7], vec7))
	{
		Vector lines[12][2];
		lines[0][0] = vec0;
		lines[0][1] = vec1;
		lines[1][0] = vec1;
		lines[1][1] = vec2;
		lines[2][0] = vec2;
		lines[2][1] = vec3;
		lines[3][0] = vec3;
		lines[3][1] = vec0;

		// top of box
		lines[4][0] = vec4;
		lines[4][1] = vec5;
		lines[5][0] = vec5;
		lines[5][1] = vec6;
		lines[6][0] = vec6;
		lines[6][1] = vec7;
		lines[7][0] = vec7;
		lines[7][1] = vec4;

		lines[8][0] = vec0;
		lines[8][1] = vec4;

		lines[9][0] = vec1;
		lines[9][1] = vec5;

		lines[10][0] = vec2;
		lines[10][1] = vec6;

		lines[11][0] = vec3;
		lines[11][1] = vec7;

		for (int i = 0; i < 12; i++)
			Line(lines[i][0].x, lines[i][0].y, lines[i][1].x, lines[i][1].y, outline);
	}
}

RECT CDraw::GetViewport()
{
	RECT Viewport = { 0, 0, 0, 0 };
	Viewport.right = Globals::ScreenWeight; Viewport.bottom = Globals::ScreenHeight;
	return Viewport;
}

void CDraw::Clear(int x, int y, int w, int h, Color color)
{
	Interfaces::Surface->DrawSetColor(color);
	Interfaces::Surface->DrawFilledRect(x, y, x + w, y + h);
}

void CDraw::DrawOutlineRect(int x, int y, int w, int h, Color color)
{
	DrawRect(x, y, w, h, Color::Black);
	DrawRect(x + 1, y + 1, w - 2, h - 2, color);
	DrawRect(x + 2, y + 2, w - 4, h - 4, Color::Black);
}

void CDraw::DrawString(DWORD font, int x, int y, Color color, DWORD alignment, const char* msg, ...)
{
	va_list va_alist;
	char buf[1024];
	va_start(va_alist, msg);
	_vsnprintf(buf, sizeof(buf), msg, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];
	MultiByteToWideChar(CP_UTF8, 0, buf, 256, wbuf, 256);

	int r = 255, g = 255, b = 255, a = 255;
	color.GetColor(r, g, b, a);

	int width, height;

	Interfaces::Surface->GetTextSize(font, wbuf, width, height);

	if (alignment & FONT_RIGHT)
		x -= width;
	if (alignment & FONT_CENTER)
		x -= width / 2;

	Interfaces::Surface->DrawSetTextFont(font);
	Interfaces::Surface->DrawSetTextColor(r, g, b, a);
	Interfaces::Surface->DrawSetTextPos(x, y - height / 2);
	Interfaces::Surface->DrawPrintText(wbuf, wcslen(wbuf));
}

void CDraw::DrawHeader(int x, int y, int w, int HealthBarWidth, Color color)
{
	int clr[3];

	clr[0] = color.r() - 85;
	clr[1] = color.g() - 85;
	clr[2] = color.b() - 85;

	for (int i = 0; i < 3; i++)
	{
		if (clr[i] < 0)
			clr[i] = 0;
	}

	int i = 0;
	int xCoord = x;
	int yCoord = y;

	for (i = 0; i < 5; i++)
	{
		FillRGBA(x - i, y + i, w, 1, Color(0, 0, 0, 185));

		FillRGBA(x - i, y + i, HealthBarWidth + 2, 1, color);
		 
		FillRGBA(x - 3, y + 3, HealthBarWidth + 2, 1, Color(clr[0], clr[1], clr[2]));

		FillRGBA(x - 4, y + 4, HealthBarWidth + 2, 1, Color(clr[0], clr[1], clr[2]));
	}

	FillRGBA(x, y, w, 1, Color(0, 0, 0, 185));

	FillRGBA((x + 1) - 5, y + 5, w, 1, Color(0, 0, 0, 185));

	for (i = 0; i < 5; i++)
	{
		FillRGBA(x, y, 1, 1, Color(0, 0, 0, 185));
		x--;
		y++;
	}

	x = xCoord;
	y = yCoord;

	for (i = 0; i < 5; i++)
	{
		if (i != 0)
			FillRGBA(x + w, y, 1, 1, Color(0, 0, 0, 185));

		x--;
		y++;
	}
}

void CDraw::OutlineRainbow(int x, int y, int width, int height, float flSpeed, float &flRainbow)
{
	Color colColor(0, 0, 0);

	flRainbow += flSpeed;
	if (flRainbow > 1.f) flRainbow = 0.f;

	for (int i = 0; i < width; i++)
	{
		float hue = (1.f / (float)width) * i;
		hue -= flRainbow;
		if (hue < 0.f) hue += 1.f;

		Color colRainbow = colColor.FromHSB(hue, 1.f, 1.f);
		DrawOutlineRect(x + i, y, 1, height, colRainbow);
	}
}

void CDraw::Pixel(int x, int y, Color col)
{
	Interfaces::Surface->DrawSetColor(col);
	Interfaces::Surface->DrawFilledRect(x, y, x + 1, y + 1);
}

void CDraw::Line(int x, int y, int x2, int y2, Color color)
{
	Interfaces::Surface->DrawSetColor(color);
	Interfaces::Surface->DrawLine(x, y, x2, y2);
}

void CDraw::PolyLine(int *x, int *y, int count, Color color)
{
	Interfaces::Surface->DrawSetColor(color);
	Interfaces::Surface->DrawPolyLine(x, y, count);
}

void CDraw::Text(int x, int y, Color color, DWORD font, const char* text, ...)
{
	size_t origsize = strlen(text) + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t wcstring[newsize];
	mbstowcs_s(&convertedChars, wcstring, origsize, text, _TRUNCATE);

	Interfaces::Surface->DrawSetTextFont(font);

	Interfaces::Surface->DrawSetTextColor(color);
	Interfaces::Surface->DrawSetTextPos(x, y);
	Interfaces::Surface->DrawPrintText(wcstring, wcslen(wcstring));
}

void CDraw::OutlinedBox2(int x, int y, int w, int h, Color color)
{
	DrawRect(x, y, w, h, Color::Black);
	DrawRect(x + 1, y + 1, w - 2, h - 2, color);
}

void CDraw::DrawRectFilled(int x0, int y0, int x1, int y1, Color color)
{
	Interfaces::Surface->DrawSetColor(color);
	Interfaces::Surface->DrawFilledRect(x0, y0, x1, y1);
}

void CDraw::DrawVerBar(int x, int y, int w, int h, int val, Color color1, Color color2)
{
	if (val > 100) val = 100;
	int size = (h * val) / 100;

	DrawRect(x, y, w, h, Color(0, 0, 0));
	FillRGBA(x + 1, y + 1, w - 2, h - 2, color2);

	if (size == h)
		FillRGBA(x + 1, y + 1 + h - size, w - 2, size - 2, color1);
	else
		FillRGBA(x + 1, y + 1 + h - size, w - 2, size, color1);
}

void CDraw::FillRGBA(int x, int y, int w, int h, Color c)
{
	Interfaces::Surface->DrawSetColor(c);
	Interfaces::Surface->DrawFilledRect(x, y, x + w, y + h);
}

void CDraw::TexturedPolygon(int n, std::vector<Vertex_t> vertice, Color color)
{
	static int texture_id = Interfaces::Surface->CreateNewTextureID(true); // 
	static unsigned char buf[4] = { 255, 255, 255, 255 };
	Interfaces::Surface->DrawSetTextureRGBA(texture_id, buf, 1, 1); //
	Interfaces::Surface->DrawSetColor(color); //
	Interfaces::Surface->DrawSetTexture(texture_id); //
	Interfaces::Surface->DrawTexturedPolygon(n, vertice.data()); //
}

void CDraw::DrawFilledCircle(Vector2D center, Color color, float radius, float points) {
	std::vector<Vertex_t> vertices;
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
		vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + center.x, radius * sinf(a) + center.y)));

	TexturedPolygon((int)points, vertices, color);
}

void CDraw::Text(int x, int y, Color color, DWORD font, const wchar_t* text)
{
	Interfaces::Surface->DrawSetTextFont(font);
	Interfaces::Surface->DrawSetTextColor(color);
	Interfaces::Surface->DrawSetTextPos(x, y);
	Interfaces::Surface->DrawPrintText(text, wcslen(text));
}

void CDraw::Text(int x, int y, DWORD font, const wchar_t* text)
{
	Interfaces::Surface->DrawSetTextFont(font);
	Interfaces::Surface->DrawSetTextPos(x, y);
	Interfaces::Surface->DrawPrintText(text, wcslen(text));
}

void CDraw::Textf(int x, int y, Color color, DWORD font, const char* fmt, ...)
{
	if (!fmt) return; //if the passed string is null return
	if (strlen(fmt) < 2) return;

	//Set up va_list and buffer to hold the params 
	va_list va_alist;
	char logBuf[256] = { 0 };

	//Do sprintf with the parameters
	va_start(va_alist, fmt);
	_vsnprintf_s(logBuf + strlen(logBuf), 256 - strlen(logBuf), sizeof(logBuf) - strlen(logBuf), fmt, va_alist);
	va_end(va_alist);

	Text(x, y, color, font, logBuf);
}

void CDraw::DrawRect(int x, int y, int w, int h, Color col)
{
	Interfaces::Surface->DrawSetColor(col);
	Interfaces::Surface->DrawOutlinedRect(x, y, x + w, y + h);
}

void CDraw::DrawCorners(RECT rect, Color Col, Color Col2)
{
	int x1, y1, x2, y2, w, h;
	x1 = rect.left;
	y1 = rect.top;
	x2 = rect.right;
	y2 = rect.bottom;

	w = x2;
	h = y2;

	int Line_Size = (y1 - h) / 6;
	int Line_Size2 = (y1 - h) / 6;

	int red = 0;
	int green = 0;
	int blue = 0;
	int alpha = 0;
	Col.GetColor(red, green, blue, alpha);
	Interfaces::Surface->DrawSetColor(red, green, blue, alpha);

	//top inwards
	Interfaces::Surface->DrawLine(w, y1, w + Line_Size, y1);
	Interfaces::Surface->DrawLine(x1, y1, x1 - Line_Size, y1);

	//top downwards
	Interfaces::Surface->DrawLine(x1, y1, x1, y1 - Line_Size);
	Interfaces::Surface->DrawLine(w, y1, w, y1 - Line_Size);

	//bottom inwards
	Interfaces::Surface->DrawLine(x1, h, x1 - Line_Size, h);
	Interfaces::Surface->DrawLine(w, h, w + Line_Size, h);

	//bottom upwards
	Interfaces::Surface->DrawLine(x1, h, x1, h + Line_Size);
	Interfaces::Surface->DrawLine(w, h, w, h + Line_Size);

	//outlines

	Interfaces::Surface->DrawSetColor(Col2);

	//top inwards
	Interfaces::Surface->DrawLine(w, y1 - 1, w + Line_Size, y1 - 1);
	Interfaces::Surface->DrawLine(x1, y1 - 1, x1 - Line_Size, y1 - 1);
	//inlines
	Interfaces::Surface->DrawLine(w - 1, y1 + 1, w + Line_Size, y1 + 1);
	Interfaces::Surface->DrawLine(x1 + 1, y1 + 1, x1 - Line_Size, y1 + 1);

	// top downwards
	Interfaces::Surface->DrawLine(x1 - 1, y1 - 1, x1 - 1, y1 - Line_Size);
	Interfaces::Surface->DrawLine(w + 1, y1 - 1, w + 1, y1 - Line_Size);
	//inlines
	Interfaces::Surface->DrawLine(x1 + 1, y1, x1 + 1, y1 - Line_Size);
	Interfaces::Surface->DrawLine(w - 1, y1, w - 1, y1 - Line_Size);

	//bottom inwards
	Interfaces::Surface->DrawLine(x1, h + 1, x1 - Line_Size, h + 1);
	Interfaces::Surface->DrawLine(w, h + 1, w + Line_Size, h + 1);
	//inlines
	Interfaces::Surface->DrawLine(x1 + 1, h - 1, x1 - Line_Size, h - 1);
	Interfaces::Surface->DrawLine(w - 1, h - 1, w + Line_Size, h - 1);

	//bottom upwards
	Interfaces::Surface->DrawLine(x1 - 1, h + 1, x1 - 1, h + Line_Size);
	Interfaces::Surface->DrawLine(w + 1, h + 1, w + 1, h + Line_Size);
	//inlines
	Interfaces::Surface->DrawLine(x1 + 1, h, x1 + 1, h + Line_Size);
	Interfaces::Surface->DrawLine(w - 1, h, w - 1, h + Line_Size);
}

void CDraw::DrawCoalBox(int x, int y, int w, int h, Color color)
{
	int iw = w / 4;
	int ih = h / 4;
	// top
	Line(x, y, x + iw, y, color);					// left
	Line(x + w - iw, y, x + w, y, color);			// right
	Line(x, y, x, y + ih, color);					// top left
	Line(x + w - 1, y, x + w - 1, y + ih, color);	// top right
															// bottom
	Line(x, y + h, x + iw, y + h, color);			// left
	Line(x + w - iw, y + h, x + w, y + h, color);	// right
	Line(x, y + h - ih, x, y + h, color);			// bottom left
	Line(x + w - 1, y + h - ih, x + w - 1, y + h, color);	// bottom right
}

void CDraw::DrawOutlineCoalBox(int x, int y, int w, int h, Color color)
{
	int iw = w / 4;
	int ih = h / 4;
	DrawCoalBox(x, y, w, h, Color::Black);

	// top left
	Line(x + 1, y + 1, x + iw, y + 1, color);
	Line(x + iw, y, x + iw, y + 2, Color::Black);
	Line(x + 1, y + 1, x + 1, y + ih, color);
	Line(x, y + ih, x + 2, y + ih, Color::Black);

	// top right
	Line(x + w - iw, y + 1, x + w - 1, y + 1, color);
	Line(x + w - iw - 1, y, x + w - iw, y + 2, Color::Black);
	Line(x + w - 2, y + 1, x + w - 2, y + ih, color);
	Line(x + w - 2, y + ih, x + w, y + ih, Color::Black);

	// bottom left
	Line(x + 1, y + h - ih, x + 1, y + h, color);
	Line(x, y + h - ih - 1, x + 2, y + h - ih - 1, Color::Black);
	Line(x + 1, y + h - 1, x + iw, y + h - 1, color);
	Line(x + iw, y + h - 1, x + iw, y + h + 1, Color::Black);

	// bottom right
	Line(x + w - iw, y + h - 1, x + w - 1, y + h - 1, color);
	Line(x + w - iw - 1, y + h - 1, x + w - iw, y + h + 1, Color::Black);
	Line(x + w - 2, y + h - ih, x + w - 2, y + h, color);
	Line(x + w - 2, y + h - ih - 1, x + w, y + h - ih - 1, Color::Black);

	DrawCoalBox(x + 2, y + 2, w - 4, h - 4, Color::Black);
}

void CDraw::BoxDefault(DrawData Data, Color pColor)
{
	Interfaces::Surface->DrawSetColor(pColor);
	Interfaces::Surface->DrawOutlinedRect(Data.Left, Data.Up, Data.Right, Data.Down);
}

void CDraw::GradientV(int x, int y, int w, int h, Color c1, Color c2)
{
	Clear(x, y, w, h, c1);
	BYTE first = c2.r();
	BYTE second = c2.g();
	BYTE third = c2.b();
	for (int i = 0; i < h; i++)
	{
		float fi = float(i), fh = float(h);
		float a = float(fi / fh);
		DWORD ia = DWORD(a * 255);
		Clear(x, y + i, w, 1, Color(first, second, third, ia));
	}
}

void CDraw::GradientH(int x, int y, int w, int h, Color c1, Color c2)
{
	Clear(x, y, w, h, c1);
	BYTE first = c2.r();
	BYTE second = c2.g();
	BYTE third = c2.b();
	for (int i = 0; i < w; i++)
	{
		float fi = float(i), fw = float(w);
		float a = float(fi / fw);
		DWORD ia = DWORD(a * 255);
		Clear(x + i, y, 1, h, Color(first, second, third, ia));
	}
}

void CDraw::Polygon(int count, Vertex_t* Vertexs, Color color)
{
	static int Texture = Interfaces::Surface->CreateNewTextureID(true); //need to make a texture with procedural true
	unsigned char buffer[4] = { 255, 255, 255, 255 };//{ color.r(), color.g(), color.b(), color.a() };

	Interfaces::Surface->DrawSetTextureRGBA(Texture, buffer, 1, 1); //Texture, char array of texture, width, height
	Interfaces::Surface->DrawSetColor(color); // keep this full color and opacity use the RGBA @top to set values.
	Interfaces::Surface->DrawSetTexture(Texture); // bind texture

	Interfaces::Surface->DrawTexturedPolygon(count, Vertexs);
}

void CDraw::PolygonOutline(int count, Vertex_t* Vertexs, Color color, Color colorLine)
{
	static int x[128];
	static int y[128];

	Polygon(count, Vertexs, color);

	for (int i = 0; i < count; i++)
	{
		x[i] = int(Vertexs[i].m_Position.x);
		y[i] = int(Vertexs[i].m_Position.y);
	}

	PolyLine(x, y, count, colorLine);
}

void CDraw::PolyLine(int count, Vertex_t* Vertexs, Color colorLine)
{
	static int x[128];
	static int y[128];

	for (int i = 0; i < count; i++)
	{
		x[i] = int(Vertexs[i].m_Position.x);
		y[i] = int(Vertexs[i].m_Position.y);
	}

	PolyLine(x, y, count, colorLine);
}

void CDraw::DrawWave(Vector loc, float radius, Color color)
{
	static float Step = M_PI * 3.0f / 40;
	Vector prev;
	for (float lat = 0; lat <= M_PI * 3.0f; lat += Step)
	{
		float sin1 = sin(lat);
		float cos1 = cos(lat);
		float sin3 = sin(0.0);
		float cos3 = cos(0.0);

		Vector point1;
		point1 = Vector(sin1 * cos3, cos1, sin1 * sin3) * radius;
		Vector point3 = loc;
		Vector Out;
		point3 += point1;

		if (Math::WorldToScreen(point3, Out))
		{
			if (lat > 0.000) {

				Line(prev.x, prev.y, Out.x, Out.y, color);
			}
		}
		prev = Out;
	}
}

void CDraw::DrawCircle3D(Vector position, float points, float radius, Color Color)
{
	float Step = (float)M_PI * 2.0f / points;

	std::vector<Vector> points3d;

	for (float a = 0; a < (M_PI * 2.0f); a += Step)
	{
		Vector Start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
		Vector End(radius * cosf(a + Step) + position.x, radius * sinf(a + Step) + position.y, position.z);

		Vector start2d, end2d;

		if (!Math::WorldToScreen(Start, start2d) || !Math::WorldToScreen(End, end2d))
			return;

		Line(start2d.x, start2d.y, end2d.x, end2d.y, Color);
	}
}

void CDraw::Circle(int x, int y, int r, int seg, Color color)
{
	Interfaces::Surface->DrawSetColor(0, 0, 0, 255);
	Interfaces::Surface->DrawSetColor(color);
	Interfaces::Surface->DrawOutlinedCircle(x, y, r, seg);
}

void CDraw::OutlineCircle(int x, int y, int r, int seg, Color color)
{
	Interfaces::Surface->DrawSetColor(0, 0, 0, 255);
	Interfaces::Surface->DrawOutlinedCircle(x, y, r - 1, seg);
	Interfaces::Surface->DrawOutlinedCircle(x, y, r + 1, seg);
	Interfaces::Surface->DrawSetColor(color);
	Interfaces::Surface->DrawOutlinedCircle(x, y, r, seg);
}

struct CUSTOMVERTEX {
	FLOAT x, y, z;
	FLOAT rhw;
	DWORD color;
	//  FLOAT tu, tv;   
};