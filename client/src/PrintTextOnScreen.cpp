#include "PrintTextOnScreen.h"
#include "hgeFont.h"


PrintText::PrintText()
{
	font_.reset(new hgeFont("font1.fnt"));
	font_->SetScale(0.5);
	txt = "SCORE :";
}
void PrintText::Render(float x,float y)
{
	font_->printf(x + 5, y + 5, HGETEXT_LEFT, "%s", txt.c_str());
}
PrintText::~PrintText()
{
	font_.release();
}