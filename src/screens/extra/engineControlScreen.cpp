#include "playerInfo.h"
#include "gameGlobalInfo.h"
#include "engineControlScreen.h"

#include "screenComponents/shipInternalView.h"
#include "screenComponents/selfDestructButton.h"
#include "screenComponents/alertOverlay.h"
#include "screenComponents/customShipFunctions.h"
#include "screenComponents/shipsLogControl.h"
#include "screenComponents/globalMessage.h"

#include "gui/gui2_keyvaluedisplay.h"
#include "gui/gui2_autolayout.h"
#include "gui/gui2_togglebutton.h"
#include "gui/gui2_slider.h"
#include "gui/gui2_progressbar.h"
#include "gui/gui2_arrow.h"
#include "gui/gui2_image.h"
#include "gui/gui2_panel.h"

EngineControlScreen::EngineControlScreen(GuiContainer* owner)
: GuiOverlay(owner, "ENGINEERING_SCREEN", colorConfig.background)
{
    // Render the background decorations.
    background_crosses = new GuiOverlay(this, "BACKGROUND_CROSSES", sf::Color::White);
    background_crosses->setTextureTiled("gui/BackgroundCrosses");

    // Render the alert level color overlay.
    (new AlertLevelOverlay(this));

    GuiAutoLayout* global_info_layout = new GuiAutoLayout(this, "GLOBAL_INFO", GuiAutoLayout::LayoutVerticalTopToBottom);
    global_info_layout->setPosition(40, 40, ATopLeft)->setSize(240, 200);

    energy_display = new GuiKeyValueDisplay(global_info_layout, "ENERGY_DISPLAY", 0.45, "Energy", "");
    energy_display->setIcon("gui/icons/energy")->setTextSize(20)->setSize(GuiElement::GuiSizeMax, 40);
    hull_display = new GuiKeyValueDisplay(global_info_layout, "HULL_DISPLAY", 0.45, "Hull", "");
    hull_display->setIcon("gui/icons/hull")->setTextSize(20)->setSize(GuiElement::GuiSizeMax, 40);
    front_shield_display = new GuiKeyValueDisplay(global_info_layout, "SHIELDS_DISPLAY", 0.45, "Front", "");
    front_shield_display->setIcon("gui/icons/shields-fore")->setTextSize(20)->setSize(GuiElement::GuiSizeMax, 40);
    rear_shield_display = new GuiKeyValueDisplay(global_info_layout, "SHIELDS_DISPLAY", 0.45, "Rear", "");
    rear_shield_display->setIcon("gui/icons/shields-aft")->setTextSize(20)->setSize(GuiElement::GuiSizeMax, 40);

    GuiAutoLayout* system_row_layouts = new GuiAutoLayout(this, "SYSTEM_ROWS", GuiAutoLayout::LayoutVerticalBottomToTop);
    system_row_layouts->setPosition(40, -40, ABottomLeft)->setSize(800, SYS_COUNT * 50);
    for(int n=0; n<SYS_COUNT; n++)
    {
        string id = "SYSTEM_ROW_" + getSystemName(ESystem(n));
        SystemRow info;
        info.layout = new GuiAutoLayout(system_row_layouts, id, GuiAutoLayout::LayoutHorizontalLeftToRight);
        info.layout->setSize(GuiElement::GuiSizeMax, 50);

        info.name_label = new GuiLabel(info.layout, id + "_NAME_LABEL", getSystemName(ESystem(n)), 20);
        info.name_label->addBackground()->setSize(200, GuiElement::GuiSizeMax);

        info.damage_bar = new GuiProgressbar(info.layout, id + "_DAMAGE", 0.0, 1.0, 0.0);
        info.damage_bar->setTextSize(20)->setSize(80, GuiElement::GuiSizeMax);
        info.heat_bar = new GuiProgressbar(info.layout, id + "_HEAT", 0.0, 1.0, 0.0);
        info.heat_bar->setSize(100, GuiElement::GuiSizeMax);
        info.heat_arrow = new GuiArrow(info.heat_bar, id + "_HEAT_ARROW", 0);
        info.heat_arrow->setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMax);
        info.heat_icon = new GuiImage(info.heat_bar, "", "gui/icons/status_overheat");
        info.heat_icon->setColor(colorConfig.overlay_overheating)->setPosition(0, 0, ACenter)->setSize(GuiElement::GuiSizeMatchHeight, GuiElement::GuiSizeMax);
        info.power_bar = new GuiProgressbar(info.layout, id + "_POWER", 0.0, 3.0, 0.0);
        info.power_bar->setTextSize(20)->setColor(sf::Color(192, 192, 32, 128))->setSize(80, GuiElement::GuiSizeMax);
        info.coolant_bar = new GuiProgressbar(info.layout, id + "_COOLANT", 0.0, my_spaceship->max_coolant, 0.0);
        info.coolant_bar->setColor(sf::Color(32, 128, 128, 128))->setSize(100, GuiElement::GuiSizeMax);
        info.effectiveness_bar = new GuiProgressbar(info.layout, id + "_EFFECTIVENESS", 0.0, 3.0, 0.0);
        info.effectiveness_bar->setColor(sf::Color::White)->setSize(100, GuiElement::GuiSizeMax);
        
        info.energy_label = new GuiLabel(info.layout, id + "_ENERGY", getSystemName(ESystem(n)), 20);
        info.energy_label->addBackground()->setSize(70, GuiElement::GuiSizeMax);

        info.layout->moveToBack();
        system_rows.push_back(info);
    }

    GuiAutoLayout* headers_layout = new GuiAutoLayout(system_row_layouts, "", GuiAutoLayout::LayoutHorizontalLeftToRight);
    headers_layout->setSize(GuiElement::GuiSizeMax, 50);
    (new GuiLabel(headers_layout, "NAME_TITLE", "Name", 20))->setSize(200, GuiElement::GuiSizeMax);
    (new GuiLabel(headers_layout, "HEALTH_TITLE", "Health", 20))->setSize(80, GuiElement::GuiSizeMax);
    (new GuiLabel(headers_layout, "HEAT_TITLE", "Heat", 20))->setSize(100, GuiElement::GuiSizeMax);
    (new GuiLabel(headers_layout, "POWER_TITLE", "Power", 20))->setSize(80, GuiElement::GuiSizeMax);
    (new GuiLabel(headers_layout, "COOLANT_TITLE", "Coolant", 20))->setSize(100, GuiElement::GuiSizeMax);
    (new GuiLabel(headers_layout, "EFFECTIVENESS_TITLE", "Effectiveness", 20))->setSize(100, GuiElement::GuiSizeMax);
    (new GuiLabel(headers_layout, "ENERGY_TITLE", "Energy", 20))->setSize(70, GuiElement::GuiSizeMax);

    system_effects_container = new GuiAutoLayout(this, "SYSTEM_EFFECTS", GuiAutoLayout::LayoutVerticalTopToBottom);
    system_effects_container->setPosition(-50, 50, ATopRight)->setSize(270, GuiElement::GuiSizeMax);

    energy_deriv.last_measurement = 0.0;
    energy_deriv.average = 0.0;
    last_measurement_time = 0.0;
    
    new GuiGlobalMessage(this);
}

void EngineControlScreen::RollingDeriviateAvg::apply(float deltaTime, float currentMeasurement){
    float delta_e = currentMeasurement - last_measurement;
    float delta_e_per_second = delta_e / deltaTime;
    average = average * 0.99 + delta_e_per_second * 0.01;
    last_measurement = currentMeasurement;
}

void EngineControlScreen::onDraw(sf::RenderTarget& window)
{
    if (my_spaceship)
    {
        //Update the energy usage.
        if (last_measurement_time == 0.0) {
            energy_deriv.last_measurement = my_spaceship->energy_level;
        } else if (last_measurement_time != engine->getElapsedTime()) {
            float delta_t = engine->getElapsedTime() - last_measurement_time;
            energy_deriv.apply(delta_t, my_spaceship->energy_level);
            for(int n=0; n<SYS_COUNT; n++) {
                system_rows[n].heat_deriv.apply(delta_t, my_spaceship->systems[n].heat_level);
            }
        }
        last_measurement_time = engine->getElapsedTime();

        energy_display->setValue(string(int(my_spaceship->energy_level)) + " (" + string(int(energy_deriv.average * 60.0f)) + "/m)");
        if (my_spaceship->energy_level < 100)
            energy_display->setColor(sf::Color::Red);
        else
            energy_display->setColor(sf::Color::White);
        hull_display->setValue(string(int(100 * my_spaceship->hull_strength / my_spaceship->hull_max)) + "%");
        if (my_spaceship->hull_strength < my_spaceship->hull_max / 4.0f)
            hull_display->setColor(sf::Color::Red);
        else
            hull_display->setColor(sf::Color::White);
        front_shield_display->setValue(string(my_spaceship->getShieldPercentage(0)) + "%");
        rear_shield_display->setValue(string(my_spaceship->getShieldPercentage(1)) + "%");
        system_effects_index = 0;
        for(int n=0; n<SYS_COUNT; n++)
        {
            ESystem selected_system = ESystem(n);
            ShipSystem& system = my_spaceship->systems[n];
            SystemRow info = system_rows[n];
            info.layout->setVisible(my_spaceship->hasSystem(selected_system));
            
            float health = system.health;
            if (health < 0.0)
                info.damage_bar->setValue(-health)->setColor(sf::Color(128, 32, 32, 192));
            else
                info.damage_bar->setValue(health)->setColor(sf::Color(64, 128 * health, 64 * health, 192));
            info.damage_bar->setText(string(int(health * 100)) + "%");

            float heat = system.heat_level;
            info.heat_bar->setValue(heat)->setColor(sf::Color(128, 32 + 96 * (1.0 - heat), 32, 192));
            float heating_diff = info.heat_deriv.average;
            if (heating_diff > 0)
                info.heat_arrow->setAngle(90);
            else
                info.heat_arrow->setAngle(-90);
            info.heat_arrow->setVisible(heat > 0);
            info.heat_arrow->setColor(sf::Color(255, 255, 255, std::min(255, int(255 * fabs(heating_diff / PlayerSpaceship::system_heatup_per_second)))));
            if (heat > 0.9 && fmod(engine->getElapsedTime(), 0.5) < 0.25)
                info.heat_icon->show();
            else
                info.heat_icon->hide();

            info.power_bar->setValue(system.power_level);
            info.power_bar->setText(string(int(system.power_level * 100)) + "%")->setTextColor(system.power_level > 1 ? sf::Color::Red : sf::Color::White);
            info.coolant_bar->setValue(system.coolant_level);
            
            float effectiveness = my_spaceship->getSystemEffectiveness(selected_system);
            float energy = 0.f;
            const float powerFactor = PlayerSpaceship::system_power_user_factor[n];
            if (powerFactor < 0) {
                if (effectiveness > 1.0f)
                    effectiveness = (1.0f + effectiveness) / 2.0f;
                energy = powerFactor * effectiveness;
            } else {
                energy = powerFactor * system.power_level;
            }
            info.energy_label->setText(string(-energy * 60.0, 1) + "/m");

            info.effectiveness_bar->setValue(effectiveness);

            switch(selected_system)
            {
            case SYS_BeamWeapons:
                addSystemEffect("Railguns Firing rate", string(int(effectiveness * 100)) + "%");
                // If the ship has a turret, also note that the rotation rate
                // is affected.
                for(int n = 0; n < max_beam_weapons; n++)
                {
                    if (my_spaceship->beam_weapons[n].getTurretArc() > 0)
                    {
                        addSystemEffect("Turret rotation rate", string(int(effectiveness * 100)) + "%");
                        break;
                    }
                }
                break;
            case SYS_MissileSystem:
                addSystemEffect("Missile Reload rate", string(int(effectiveness * 100)) + "%");
                break;
            case SYS_Maneuver:
                addSystemEffect("Turning speed", string(int(effectiveness * 100)) + "%");
                if (my_spaceship->combat_maneuver_boost_speed > 0.0 || my_spaceship->combat_maneuver_strafe_speed)
                    addSystemEffect("Combat recharge rate", string(int(((my_spaceship->getSystemEffectiveness(SYS_Maneuver) + my_spaceship->getSystemEffectiveness(SYS_Impulse)) / 2.0) * 100)) + "%");
                break;
            case SYS_Impulse:
                addSystemEffect("Ion speed", string(int(effectiveness * 100)) + "%");
                break;
            case SYS_Warp:
                addSystemEffect("Grav drive speed", string(int(effectiveness * 100)) + "%");
                addSystemEffect("Grav Calibration speed", string(int(effectiveness * 100)) + "%");
                break;
            case SYS_JumpDrive:
                addSystemEffect("Skip drive recharge rate", string(int(my_spaceship->getJumpDriveRechargeRate() * 100)) + "%");
                addSystemEffect("Skip drive speed", string(int(effectiveness * 100)) + "%");
                break;
            case SYS_FrontShield:
                if (gameGlobalInfo->use_beam_shield_frequencies)
                    addSystemEffect("Shield Calibration speed", string(int((my_spaceship->getSystemEffectiveness(SYS_FrontShield) + my_spaceship->getSystemEffectiveness(SYS_RearShield)) / 2.0 * 100)) + "%");
                addSystemEffect("Front Charge rate", string(int(effectiveness * 100)) + "%");
                {
                    DamageInfo di;
                    di.type = DT_Kinetic;
                    float damage_negate = 1.0f - my_spaceship->getShieldDamageFactor(di, 0);
                    if (damage_negate < 0.0)
                        addSystemEffect("Front Extra damage", string(int(-damage_negate * 100)) + "%");
                    else
                        addSystemEffect("Front Damage negate", string(int(damage_negate * 100)) + "%");
                }
                break;
            case SYS_RearShield:
                addSystemEffect("Rear Charge rate", string(int(effectiveness * 100)) + "%");
                {
                    DamageInfo di;
                    di.type = DT_Kinetic;
                    float damage_negate = 1.0f - my_spaceship->getShieldDamageFactor(di, my_spaceship->shield_count - 1);
                    if (damage_negate < 0.0)
                        addSystemEffect("Rear Extra damage", string(int(-damage_negate * 100)) + "%");
                    else
                        addSystemEffect("Rear Damage negate", string(int(damage_negate * 100)) + "%");
                }
                break;
            default:
                break;
            }
        }
        for(unsigned int idx=system_effects_index; idx<system_effects.size(); idx++)
            system_effects[idx]->hide();
    }
    GuiOverlay::onDraw(window);
}

void EngineControlScreen::addSystemEffect(string key, string value)
{
    if (system_effects_index == system_effects.size())
    {
        GuiKeyValueDisplay* item = new GuiKeyValueDisplay(system_effects_container, "", 0.75, key, value);
        item->setTextSize(20)->setSize(GuiElement::GuiSizeMax, 40);
        system_effects.push_back(item);
    }else{
        system_effects[system_effects_index]->setKey(key);
        system_effects[system_effects_index]->setValue(value);
        system_effects[system_effects_index]->show();
    }
    system_effects_index++;
}
