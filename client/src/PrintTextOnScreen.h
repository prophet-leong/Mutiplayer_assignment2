#pragma once

#include <memory>
#include <string>

class hgeFont;
class PrintText
{
public:
	PrintText();
	~PrintText();
	std::auto_ptr<hgeFont> font_;
	std::string txt;
	void Render(float x,float y);
private:

};

