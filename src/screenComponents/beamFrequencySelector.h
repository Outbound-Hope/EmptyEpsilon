#ifndef BEAM_FREQUENCY_SELECTOR_H
#define BEAM_FREQUENCY_SELECTOR_H

#include "gui/gui2_selector.h"
#include "spaceObjects/playerSpaceship.h"

class GuiBeamFrequencySelector : public GuiSelector
{
public:
    GuiBeamFrequencySelector(GuiContainer* owner, string id, EWeaponFrontDirection weaponDirection);
    
    virtual void onHotkey(const HotkeyResult& key) override;
};

#endif//BEAM_FREQUENCY_SELECTOR_H
