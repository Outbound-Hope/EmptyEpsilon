#include "playerInfo.h"
#include "gameGlobalInfo.h"
#include "beamTargetSelector.h"

GuiBeamTargetSelector::GuiBeamTargetSelector(GuiContainer* owner, string id, EWeaponFrontDirection weaponDirection)
: GuiSelector(owner, id, [weaponDirection](int index, string value) { if (my_spaceship) weaponDirection == RearAndLeft? my_spaceship->commandSetRearBeamSystemTarget(ESystem(index + SYS_None)) : my_spaceship->commandSetBeamSystemTarget(ESystem(index + SYS_None)); })
{
    addEntry("Hull", "-1");
    for(int n=0; n<SYS_COUNT; n++)
        addEntry(getSystemName(ESystem(n)), string(n));
    if (my_spaceship)
        setSelectionIndex((weaponDirection == RearAndLeft? my_spaceship->rear_beam_system_target : my_spaceship->beam_system_target) - SYS_None);
    if (!gameGlobalInfo->use_system_damage)
        hide();
}

void GuiBeamTargetSelector::onHotkey(const HotkeyResult& key)
{
    if (key.category == "WEAPONS" && my_spaceship && gameGlobalInfo->use_system_damage)
    {
        if (key.hotkey == "BEAM_SUBSYSTEM_TARGET_NEXT")
        {
            if (getSelectionIndex() >= (int)entries.size() - 1)
                setSelectionIndex(0);
            else
                setSelectionIndex(getSelectionIndex() + 1);
            callback();
        }
        if (key.hotkey == "BEAM_SUBSYSTEM_TARGET_PREV")
        {
            if (getSelectionIndex() <= 0)
                setSelectionIndex(entries.size() - 1);
            else
                setSelectionIndex(getSelectionIndex() - 1);
        }
    }
}
