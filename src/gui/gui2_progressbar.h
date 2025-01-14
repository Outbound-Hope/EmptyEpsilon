#ifndef GUI2_PROGRESSBAR_H
#define GUI2_PROGRESSBAR_H

#include "gui2_element.h"

class GuiProgressbar : public GuiElement
{
private:
    float min_value;
    float max_value;
    float value;
    sf::Color color;
    sf::Color text_color;
    bool drawBackground;
    bool bi_directional;
    float text_size;
    string text;
public:
    GuiProgressbar(GuiContainer* owner, string id, float min_value, float max_value, float start_value);

    virtual void onDraw(sf::RenderTarget& window);

    GuiProgressbar* setValue(float value);
    float getValue();
    GuiProgressbar* setRange(float min_value, float max_value);
    GuiProgressbar* setText(string text);
    GuiProgressbar* setColor(sf::Color color);
    GuiProgressbar* setTextColor(sf::Color color);
    GuiProgressbar* setTextSize(float size);
    GuiProgressbar* setDrawBackground(bool drawBackground);
    GuiProgressbar* setBiDirectional(bool biDirectional);
};

#endif//GUI2_PROGRESSBAR_H
