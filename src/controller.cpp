#include <Arduino.h>
#include "../include/controller.h"
#include "../include/model.h"
#include "../include/view.h"
#include <SD.h>
#include <FS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_BusIO_Register.h>
#include <Ds1302.h>

// DECLARING VARIABLES FOR BUTTONS
#define DOWN_BUTTON 39
#define SELECT_BUTTON 34
#define UP_BUTTON 35

// DECLARING VARIABLES FOR RTC
#define PIN_ENA 14
#define PIN_CLK 26
#define PIN_DAT 27

const static char *WeekDays[] =
    {
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
        "Sunday"};

// DECLARING VARIABLES FOR OUTPUT DEVICES
#define BUZZER 23

// DECLARING VARIABLES FOR ADS
#define ALERT_PIN 32

int dataRateValues[] = {8, 16, 32, 64, 128, 250, 475, 860};

// DECLARING VARIABLES FOR MODE AND CHANNEL
MODE currentMode = DISPLAY_ONLY;
CHANNEL currentChannel = VOLTAGE;

uint16_t currentSampleRate = 860;

unsigned long time_now = 0;
unsigned long time_old = 0;
int count = 0;

Measurement measurement(8);

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

/**
 * @brief Indicates whether new data is available.
 */
volatile bool new_data = false;
/**
 * @brief Interrupt service routine for handling new data ready event.
 *
 * This function is called when new data is ready to be processed.
 * It sets the `new_data` flag to true.
 */
void IRAM_ATTR NewDataReadyISR()
{
    new_data = true;
}

// DECLARING VARIABLES FOR WIFI
const char *ssid = "Logger-Access-Point";
const char *password = "logger1234";

bool isExecuted = false;

File file;

Ds1302 rtc(PIN_ENA, PIN_CLK, PIN_DAT);

Adafruit_ADS1115 ads;

/**
 * @brief Initializes the serial monitor.
 *
 * This function initializes the serial monitor with a baud rate of 115200.
 * It waits for the serial port to connect before proceeding.
 * It also prints an "Initializing..." message to the serial monitor.
 */
void initializeSerial()
{
    // INITIALIZING SERIAL MONITOR
    Serial.begin(250000);

    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    // Print contributors
    Serial.println("-------------------------------");
    Serial.print("Logger\n");
    Serial.println("-------------------------------");
    Serial.println("Contributors:");
    Serial.println("- Vincenzo Pio Florio");
    Serial.println("- Francesco Stasi");
    Serial.println("- Davide Tonti");
    Serial.println("-------------------------------\n");
    Serial.println("Initializing...\n");
}

/**
 * @brief Initializes the output devices.
 *
 * This function initializes the output devices by setting the pinMode of the BUZZER pin to OUTPUT.
 *
 * @return true if the output devices are successfully initialized, false otherwise.
 */

boolean initializeOutputDevices()
{
    Serial.println("Initializing output devices...");

    pinMode(BUZZER, OUTPUT);

    Serial.println("Output devices initialized\n");

    return true;
}

/**
 * @brief Initializes the input devices.
 *
 * This function sets the pin modes for the UP_BUTTON, SELECT_BUTTON, and DOWN_BUTTON pins to INPUT.
 *
 * @return true if the input devices are successfully initialized, false otherwise.
 */
boolean initializeInputDevices()
{

    Serial.println("Initializing input devices...");

    pinMode(UP_BUTTON, INPUT);
    pinMode(SELECT_BUTTON, INPUT);
    pinMode(DOWN_BUTTON, INPUT);

    Serial.println("Input devices initialized\n");

    return true;
}

boolean initializeSDcard()
{
    Serial.println("Initializing SD card...\n");

    if (!SD.begin(5)) // is CS pin in ESP32!!!
    {
        return false;
    }
    else
    {
        /*Serial.println("Creating example.txt...");
        myFile = SD.open("example.txt", FILE_WRITE);
        myFile.close();*/

        if (SD.exists("example.txt"))
        {
            SD.remove("example.txt");
        }
        return true;
    }
}

boolean initializeRTC()
{
    rtc.init();
    return true;
}

boolean initializeWifi()
{

    Serial.println("Initializing Wifi...");

    // IP Address details
    IPAddress local_ip(192, 168, 1, 1);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);

    WebServer server(80);

    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);

    Serial.print("Connect to My access point: ");
    Serial.println(ssid);

    // server.on("/", server.send(200, "text/html", HTML));

    server.begin();
    Serial.println("HTTP server started");

    Serial.println("Wifi initialized\n");
    return true;
}

boolean initializeADC() // TODO finish function
{
    Serial.println("Initializing ADC...");

    Serial.println("ADC initialized");

    ads.setGain(GAIN_TWOTHIRDS); // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)

    return true;
}

/**
 * @brief Initializes the devices required for the correct operation of the logger.
 *
 * @return true if all essential devices are successfully initialized, false otherwise.
 */
boolean initializeDevices()
{
    Serial.println("Initializing devices...");

    initializeSerial();

    Serial.println("Initialized devices!\n");
    // Initialize only essential devices to correct work of logger
    return initializeOutputDevices() &&
           initializeInputDevices() &&
           initializeScreen() && initializeADC() && initializeRTC();
}

/**
 * @brief Function to check if the device should go up.
 *
 * This function checks if the UP_BUTTON is pressed or if the character 'u' is received from Serial.
 *
 * @return true if the device should go up, false otherwise.
 */
boolean goUp()
{
    if (Serial.available() > 0)
    {
        char ch = Serial.read();
        return (digitalRead(UP_BUTTON) || ch == 'u');
    }
    return digitalRead(UP_BUTTON);
}

/**
 * @brief Function to check if the device should go down.
 *
 * This function checks if there is data available on the Serial port. If there is, it reads the data and checks if it is equal to 'd'.
 * If the DOWN_BUTTON is pressed or the data is equal to 'd', it returns true. Otherwise, it returns false.
 *
 * @return true if the device should go down, false otherwise.
 */
boolean goDown()
{
    if (Serial.available() > 0)
    {
        char ch = Serial.read();
        return (digitalRead(DOWN_BUTTON) || ch == 'd');
    }
    return digitalRead(DOWN_BUTTON);
}

/**
 * @brief Checks if the select button is pressed or if there is data available on the Serial port.
 *
 * @return true if the select button is pressed or if there is data available on the Serial port, false otherwise.
 */
boolean select()
{
    if (Serial.available() > 0)
    {
        char ch = Serial.read();
        return (digitalRead(SELECT_BUTTON) || ch == 's');
    }
    return digitalRead(SELECT_BUTTON);
}

/**
 * @brief Function to sound the buzzer in a scrolling pattern.
 *
 * This function plays a tone on the buzzer for a duration of 1000 milliseconds (1 second),
 * followed by a delay of 40 milliseconds. Then, the buzzer is turned off using the noTone() function,
 * followed by a delay of 100 milliseconds.
 */
void soundBuzzerScroll()
{
    tone(BUZZER, 200, 1000);

    delay(40);

    noTone(BUZZER);

    delay(100);
}

/**
 * Function to sound the buzzer with a specific frequency and duration.
 */
void soundBuzzerSelect()
{
    tone(BUZZER, 1000, 300);

    delay(40);

    noTone(BUZZER);

    delay(250);
}

String getTimeStamp()
{

    Ds1302::DateTime now;
    rtc.getDateTime(&now);

    String currentTime = "";
    if (now.hour < 10)
        currentTime = currentTime + "0";
    currentTime = currentTime + now.hour + ":"; // 00-23
    if (now.minute < 10)
        currentTime = currentTime + "0";
    currentTime = currentTime + now.minute + ":"; // 00-59
    if (now.second < 10)
        currentTime = currentTime + "0";
    currentTime = currentTime + now.second; // 00-59

    return currentTime;
}

/**
 * @brief This function handles the output mode activation.
 */
void outputModeAct()
{
    /*Attention, before selecting the data storage method, it is necessary to verify and Initialization of devices*/
    outputModeGraphic(currentMode);
    switch (currentMode)
    {
    case SD_ONLY:

        if (goDown())
        {
            soundBuzzerScroll();
            currentMode = DISPLAY_ONLY;
            Serial.println("Mode selected: DISPLAY_ONLY\n");
        }
        if (goUp())
        {
            soundBuzzerScroll();

            currentMode = WIFI_ONLY;
            Serial.println("Mode selected: WIFI_ONLY\n");
        }
        break;
    case DISPLAY_ONLY:

        if (goDown())
        {
            soundBuzzerScroll();
            currentMode = WIFI_ONLY;
            Serial.println("Mode selected: WIFI_ONLY\n");
        }
        if (goUp()) // Check on sd card
        {
            soundBuzzerScroll();
            currentMode = SD_ONLY;
            Serial.println("Mode selected: SD_ONLY\n");
        }
        break;

    case WIFI_ONLY:

        if (goDown())
        {
            soundBuzzerScroll();
            currentMode = SD_ONLY;
            Serial.println("Mode selected: SD_ONLY\n");
        }
        if (goUp())
        {
            soundBuzzerScroll();
            currentMode = DISPLAY_ONLY;
            Serial.println("Mode selected: DISPLAY_ONLY\n");
        }
        break;

    default:
        Serial.println("\n\n\n\n-----------------------------");
        Serial.println("Error selecting channel\n");
        Serial.println("\n\n\n\n-----------------------------");
        break;
    }

    delay(10);
}

/**
 * @brief Performs the action for the input mode.
 *
 * This function is responsible for handling the input mode action.
 * It performs the necessary operations based on the current input mode.
 *
 * @return void
 */
void inputModeAct()
{
    inputModeGraphic(currentChannel);
    switch (currentChannel)
    {
    case VOLTAGE:

        if (goDown())
        {
            soundBuzzerScroll();
            currentChannel = CURRENT;
            Serial.println("Input selected: CURRENT\n");
        }
        if (goUp())
        {
            soundBuzzerScroll();
            currentChannel = RESISTANCE;
            Serial.println("Input selected: RESISTANCE\n");
        }
        break;

    case CURRENT:

        if (goDown())
        {
            soundBuzzerScroll();
            currentChannel = RESISTANCE;
            Serial.println("Input selected: RESISTANCE\n");
        }
        if (goUp())
        {
            soundBuzzerScroll();
            currentChannel = VOLTAGE;
            Serial.println("Input selected: VOLTAGE\n");
        }
        break;

    case RESISTANCE:

        if (goDown())
        {
            soundBuzzerScroll();
            currentChannel = VOLTAGE;
            Serial.println("Input selected: VOLTAGE\n");
        }
        if (goUp())
        {
            soundBuzzerScroll();
            currentChannel = CURRENT;
            Serial.println("Input selected: CURRENT\n");
        }
        break;

    default:
        Serial.println("\n\n\n\n-----------------------------");
        Serial.println("Error selecting channel");
        Serial.println("\n\n\n\n-----------------------------");
        break;
    }
    delay(10);
}

/**
 * Sets the channel for ADC readings.
 *
 * The function starts ADC reading based on the current channel value.
 * If the current channel is VOLTAGE, it starts ADC reading for single-ended channel 0.
 * If the current channel is CURRENT, it starts ADC reading for differential channel 2-3.
 * If the current channel is RESISTANCE, it starts ADC reading for single-ended channel 1.
 * If the current channel is none of the above, it prints an error message.
 *
 * @return void
 */
void setChannel()
{
    if (currentChannel == VOLTAGE)
    {
        ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, true);
        Serial.println("Reading channel A0\n");
    }

    else if (currentChannel == CURRENT)
    {
        ads.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_2_3, true);
        Serial.println("Reading channel A2-A3\n");
    }

    else if (currentChannel == RESISTANCE)
    {
        ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_3, true);
        Serial.println("Reading channel A1\n");
    }

    else
    {
        Serial.println("\n\n\n\n-----------------------------");
        Serial.println("Error selecting channel");
        Serial.println("\n\n\n\n-----------------------------");
    }
}

/**
 * @brief Performs the information action.
 *
 * This function checks if the subSetup flag is set or if the goUp or goDown functions return true.
 * If any of these conditions are true, it calls the infoGraphic function with the results of the initializeWifi, initializeSDcard, and initializeRTC functions as arguments.
 * It also resets the subSetup flag to 0.
 *
 * @param subSetup A boolean flag indicating if the submenu setup is required.
 */
void infoAct(boolean subSetup)
{
    if (subSetup || goUp() || goDown())
    {
        // infoGraphic(initializeWifi(), initializeSDcard(), initializeRTC()); //TODO fix methods
        //  submenu setup
        subSetup = 0;
    }
}

/**
 * @brief Converts an integer value to a string representation of the ADS1115 conversion rate.
 *
 * @param value The ADS1115 conversion rate value.
 * @return The string representation of the ADS1115 conversion rate.
 */
uint16_t adsToStringRate(int value)
{

    return ("RATE_ADS1115_%dSPS", value);
}

/**
 * @brief Sets the sample act.
 */
void sampleSetAct()
{
    sampleSetGraphic(currentSampleRate);

    if (goDown())
    {
        int i = 0;
        soundBuzzerScroll();
        for (int j = 0; j < 8; j++)
        {
            if (dataRateValues[j] == currentSampleRate)
                i = j;
        }
        if (i > 0)
        {
            i--;
            currentSampleRate = dataRateValues[i];
        }
        sampleSetSelectorGraphic(0);
        Serial.println("Selected sample rate: " + String(currentSampleRate) + "\n");
    }
    if (goUp())
    {
        int i = 0;
        soundBuzzerScroll();
        for (int j = 0; j < 8; j++)
        {
            if (dataRateValues[j] == currentSampleRate)
                i = j;
        }
        Serial.println(goUp());
        if (i < 7)
        {
            i++;
            currentSampleRate = dataRateValues[i];
        }
        sampleSetSelectorGraphic(1);
        Serial.println("Selected sample rate: " + String(currentSampleRate) + "\n");
    }
    delay(10);
}

/**
 * @brief Sets up the ADC (Analog-to-Digital Converter).
 *
 * This function initializes the ADC module and configures its settings.
 * It prepares the ADC for reading analog values from sensors or other sources.
 */
void adcSetup()
// TODO pass the conversion value to labview<
{
    Serial.println("\n\n\n\n-----------------------------");
    Serial.println("ENTERED IN ADC SETUP\n\n\n\n");
    // We get a falling edge every time a new sample is ready.
    attachInterrupt(digitalPinToInterrupt(ALERT_PIN), NewDataReadyISR, FALLING);
    Serial.println("Interrupt attached (falling edge for new data ready)))");

    // Initialize the ADC module.
    ads.setDataRate(RATE_ADS1115_860SPS);
    Serial.println("Sample rate setting done!\n");

    if (!ads.begin())
    {
        Serial.println("Failed to initialize ADS.");
        while (1)
            ;
    }

    Serial.println("Sample rate: " + String(currentSampleRate) + "SPS\n");

    setChannel();
    Serial.println("Channel setting done!\n");

    measurement.setLength(currentSampleRate);

    Serial.println("Array length: " + String(measurement.getLength()) + "\n");

    Serial.println("\n\n\n\n-----------------------------");

    // Start continuous conversions.
    Serial.print(ads.getDataRate());

    loggerGraphic(currentMode, currentChannel, getTimeStamp());
}

// Executive Actions
void loggerAct()
{

    if (!new_data)
    {
        //Serial.println("No new data ready!");
        return;
    }

    //Serial.println("New data ready!");

    if (!measurement.isArrayFull())
    {
        // Serial.println(ads.getLastConversionResults());
        measurement.insertMeasurement(ads.getLastConversionResults());
    }
    else
    {
        //Serial.write(measurement.getMeasurements(), sizeof(measurement.getMeasurements()));
        Serial.println("Mean: " + String(measurement.getMean()));
        Serial.println("------------------------------------------------------------------------------");
        //Serial.println("Std: " + String(measurement.getStd()));
        Serial.println("Array full, resetted");
        Serial.println("Array length: " + String(measurement.getLength()) + "\n");
        measurement.setArrayFull(false);
        loggerGraphic(currentMode, currentChannel, getTimeStamp());
    }

    new_data = false;
    //delay(1);

    /*
     static uint8_t last_second = 0;
     if (last_second != now.second)
     {
         last_second = now.second;

         Serial.print("20");
         Serial.print(now.year); // 00-99
         Serial.print('-');
         if (now.month < 10)
             Serial.print('0');
         Serial.print(now.month); // 01-12
         Serial.print('-');
         if (now.day < 10)
             Serial.print('0');
         Serial.print(now.day); // 01-31
         Serial.print(' ');
         Serial.print(WeekDays[now.dow - 1]); // 1-7
         Serial.print(' ');
         if (now.hour < 10)
             Serial.print('0');
         Serial.print(now.hour); // 00-23
         Serial.print(':');
         if (now.minute < 10)
             Serial.print('0');
         Serial.print(now.minute); // 00-59
         Serial.print(':');
         if (now.second < 10)
             Serial.print('0');
         Serial.print(now.second); // 00-59
         Serial.println();
     }
     */
}
