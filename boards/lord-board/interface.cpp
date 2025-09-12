#include "core/powerSave.h"
#include "core/utils.h"
#include <CYD28_TouchscreenR.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <interface.h>
#include <soc/adc_channel.h>
#include <soc/soc_caps.h>
CYD28_TouchR touch(320, 240);

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
/*void _setup_gpio() {
    bruceConfig.colorInverted = 0;
    pinMode(CC1101_SS_PIN, OUTPUT);
    pinMode(NRF24_SS_PIN, OUTPUT);
    digitalWrite(CC1101_SS_PIN, HIGH);
    digitalWrite(NRF24_SS_PIN, HIGH);
    pinMode(TFT_BL, OUTPUT);

    bruceConfig.rfModule = CC1101_SPI_MODULE;
    bruceConfig.irRx = RXLED;
    bruceConfig.irTx = LED;
    Wire.setPins(SDA, SCL);
    Wire.begin();
}
*/
// Power handler for battery detection
#include <Wire.h>
#include <XPowersLib.h>
XPowersPPM PPM;

void _setup_gpio() {

  

    pinMode(CC1101_SS_PIN, OUTPUT);
    pinMode(NRF24_SS_PIN, OUTPUT);

    digitalWrite(CC1101_SS_PIN, HIGH);
    digitalWrite(NRF24_SS_PIN, HIGH);
    // Starts SPI instance for CC1101 and NRF24 with CS pins blocking communication at start

    bruceConfig.rfModule = CC1101_SPI_MODULE;
    bruceConfig.irRx = RXLED;
    Wire.setPins(GROVE_SDA, GROVE_SCL);
    Wire.begin();
    bool pmu_ret = false;

    pmu_ret = PPM.init(Wire, GROVE_SDA, GROVE_SCL, BQ25896_SLAVE_ADDRESS);
    if (pmu_ret) {
        PPM.setSysPowerDownVoltage(3300);
        PPM.setInputCurrentLimit(3250);
        Serial.printf("getInputCurrentLimit: %d mA\n", PPM.getInputCurrentLimit());
        PPM.disableCurrentLimitPin();
        PPM.setChargeTargetVoltage(4208);
        PPM.setPrechargeCurr(64);
        PPM.setChargerConstantCurr(832);
        PPM.getChargerConstantCurr();
        Serial.printf("getChargerConstantCurr: %d mA\n", PPM.getChargerConstantCurr());
        PPM.enableADCMeasure();
        PPM.enableCharge();
        PPM.enableOTG();
        PPM.disableOTG();
    }
}
/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() {
    if (!touch.begin(&tft.getSPIinstance())) {
        Serial.println("Touch IC not Started");
        log_i("Touch IC not Started");
    } else Serial.println("Touch IC Started");
}

/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() { return 0; }
// int getBattery() {}
//  int8_t percent = 0;
//  percent = (PPM.getSystemVoltage() - 3300) * 100 / (float)(4150 - 3350);

// return (percent < 0) ? 0 : (percent >= 100) ? 100 : percent;

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    if (brightval == 0) {
        analogWrite(TFT_BL, brightval);
    } else {
        int bl = MINBRIGHT + round(((255 - MINBRIGHT) * brightval / 100));
        analogWrite(TFT_BL, bl);
    }
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/

void InputHandler(void) {
    static long tm = millis();

    // debounce touch events
    if (millis() - tm > 300 || LongPress) {
        if (touch.touched()) {
            auto raw = touch.getPointScaled();
            tm = millis();

            // --- Rotation compensation ---
            int16_t tx = raw.x;
            int16_t ty = raw.y;

            switch (bruceConfig.rotation) {
                case 2: // portrait
                {
                    int tmp = tx;
                    tx = tftWidth - ty;
                    ty = tmp;
                } break;
                case 3: // landscape
                    // no swap needed
                    break;
                case 0: // portrait inverted
                {
                    int tmp = tx;
                    tx = ty;
                    ty = (tftHeight + 0) - tmp; //calibrate in real time
                } break;
                case 1: // landscape inverted
                    ty = (tftHeight + 0) - ty; //calibrate in real time
                    tx = tftWidth - tx;
                    break;
            }

            // Serial.printf( "Touch: raw=(%d,%d) mapped=(%d,%d) rot=%d\n", raw.x, raw.y, tx, ty,
            // bruceConfig.rotation" );

            // wake screen if off
            if (!wakeUpScreen()) {
                AnyKeyPress = true;
            } else {
                return;
            }

            // store in global touch point
            touchPoint.x = tx;
            touchPoint.y = ty;
            touchPoint.pressed = true;

            // optional: heatmap logging
            touchHeatMap(touchPoint);

        } else {
            touchPoint.pressed = false;
        }
    }
}

/*********************************************************************
** Function: keyboard
** location: mykeyboard.cpp
** Starts keyboard to type data
**********************************************************************/
// String keyboard(String mytext, int maxSize, String msg) {}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() {}

/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to tornoff the device (name is odd btw)
**********************************************************************/
void checkReboot() {}
