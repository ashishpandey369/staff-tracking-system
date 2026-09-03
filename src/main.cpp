#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Firebase_ESP_Client.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

//================ WIFI =================
const char *ssid = "Galaxy_S20";
const char *password = "123456789";

//=============== FIREBASE ===============
#define API_KEY "AIzaSyBTBZ24dFaIPl3Ary8p7XpmKwADSyohj3A"
#define DATABASE_URL "https://smart-track-f8a8b-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

//=============== TIME ===================
WiFiUDP ntpUDP;

NTPClient timeClient(
    ntpUDP,
    "pool.ntp.org",
    19800,
    60000);

//=============== DEVICE =================
#define DEVICE_NAME "BME Dept."
#define ROOM_FLOOR "Room 319 | Floor 3"
// First board:
// #define DEVICE_NAME "SCE Dept."

bool firebaseReady = false;

//=============== OLED ==================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 25
#define OLED_SCL 26

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1);

//=============== RFID ==================
#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

//========================================

void drawWifi(bool connected)
{
    display.fillRect(108, 0, 20, 12, BLACK);

    if (connected)
    {
        display.drawCircle(118, 4, 2, WHITE);
        display.drawCircle(118, 4, 4, WHITE);
        display.drawCircle(118, 4, 6, WHITE);
    }
}

void showScreen(String l1, String l2, bool wifi)
{
    display.clearDisplay();

    drawWifi(wifi);

    display.setTextColor(WHITE);

    display.setTextSize(2);
    display.setCursor(0, 15);
    display.println(l1);

    display.setTextSize(1);
    display.setCursor(0, 45);
    display.println(l2);

    display.display();
}

void sendFirebase(String id, String name, String contact)
{
    timeClient.update();

    String currentTime =
        timeClient.getFormattedTime();

    FirebaseJson json;

    json.set("name", name);
    json.set("contact", contact);
    json.set("currentLocation", DEVICE_NAME);
    json.set("roomFloor", ROOM_FLOOR);
    json.set("lastSeen", currentTime);
    json.set("status", "Present");

    String path =
        "/employees/89A72B51/employees/";

    path += id;

    Serial.println("==============");
    Serial.println("Uploading...");
    Serial.print("Path: ");
    Serial.println(path);

    bool ok =
        Firebase.RTDB.updateNode(
            &fbdo,
            path.c_str(),
            &json);

    if (ok)
    {
        Serial.println("Firebase Success");

        Serial.print("Scan Time: ");
        Serial.println(currentTime);
    }
    else
    {
        Serial.println("Firebase Failed");
        Serial.println(
            fbdo.errorReason());
    }
}

void setup()
{
    Serial.begin(115200);

    Serial.println();
    Serial.println("===== SYSTEM START =====");

    Wire.begin(
        OLED_SDA,
        OLED_SCL);

    delay(1000);

    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            0x3C))
    {
        Serial.println("OLED FAILED");

        while (true)
            ;
    }

    Serial.println("OLED Ready");

    SPI.begin(
        18,
        21,
        23,
        5);

    rfid.PCD_Init();

    Serial.println(
        "RFID Ready");

    showScreen(
        "Connecting",
        "Please wait",
        false);

    Serial.println(
        "Connecting WiFi...");

    WiFi.mode(
        WIFI_STA);

    WiFi.begin(
        ssid,
        password);

    while (
        WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println();

    Serial.println(
        "WiFi Connected");

    Serial.print(
        "IP: ");

    Serial.println(
        WiFi.localIP());

    showScreen(
        "Connected",
        DEVICE_NAME,
        true);

    config.api_key =
        API_KEY;

    config.database_url =
        DATABASE_URL;

    Serial.println(
        "Starting Firebase Login...");

    if (
        Firebase.signUp(
            &config,
            &auth,
            "",
            ""))
    {
        firebaseReady = true;

        Serial.println(
            "Firebase Login Success");
    }
    else
    {
        Serial.println(
            "Firebase Login Failed");

        Serial.println(
            config.signer.signupError.message.c_str());
    }

    Firebase.begin(
        &config,
        &auth);

    Firebase.reconnectWiFi(true);

    timeClient.begin();

    Serial.println(
        "Firebase Ready");

    delay(2000);

    showScreen(
        DEVICE_NAME,
        "Tap Card",
        true);

    Serial.println(
        "Waiting for Card...");
}

void loop()
{
    if (
        !rfid.PICC_IsNewCardPresent())
        return;

    if (
        !rfid.PICC_ReadCardSerial())
        return;

    String uid = "";

    for (
        byte i = 0;
        i < rfid.uid.size;
        i++)
    {
        char temp[4];

        sprintf(
            temp,
            "%02X",
            rfid.uid.uidByte[i]);

        uid += temp;

        if (i < rfid.uid.size - 1)
            uid += " ";
    }

    uid.trim();

    Serial.print(
        "UID: ");

    Serial.println(uid);

    String name = "";
    String id = "";
    String contact = "";

    if (uid == "7A DC 6A 05")
    {
        name = "Staf 1";
        id = "RF001";
        contact = "+91 7259685574";
    }

    else if (uid == "99 E5 69 05")
    {
        name = "Staf 2";
        id = "RF002";
        contact = "+91 9663616601";
    }

    else if (uid == "B5 88 A9 04")
    {
        name = "Staf 3";
        id = "RF003";
        contact = "+91  7483049662";
    }

    else if (uid == "E3 A1 87 04")
    {
        name = "Staf 4";
        id = "RF004";
        contact = "+91 7259685574";
    }

    if (name != "")
    {
        showScreen(
            "WELCOME",
            name,
            true);

        Serial.print(
            "Welcome ");

        Serial.println(name);

        if (
            firebaseReady &&
            Firebase.ready())
        {
            sendFirebase(
                id,
                name,
                contact);
        }
    }
    else
    {
        showScreen(
            "UNKNOWN",
            uid,
            true);

        Serial.println(
            "Unknown Card");
    }

    delay(3000);

    showScreen(
        DEVICE_NAME,
        "Tap Card",
        true);

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}
