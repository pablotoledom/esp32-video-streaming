#include <TFT_eSPI.h>
#include <JPEGDecoder.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
    Serial.begin(921600);
    Serial.setRxBufferSize(16384);
    if (psramInit()) {
        Serial.println("PSRAM enabled!");
    } else {
        Serial.println("No PSRAM available, using normal RAM.");
    }

    tft.init();
    tft.setRotation(0);
    tft.setSwapBytes(true);  // Ensures that color bytes are in the correct order
    tft.fillScreen(TFT_BLACK);
}

// Function to draw the JPEG image on the ST7789 screen
void drawJpeg(int xpos, int ypos) {
    while (JpegDec.read()) {
        uint16_t *pImg = JpegDec.pImage;
        tft.pushImage(xpos + JpegDec.MCUx * JpegDec.MCUWidth, 
                      ypos + JpegDec.MCUy * JpegDec.MCUHeight, 
                      JpegDec.MCUWidth, JpegDec.MCUHeight, pImg);
        yield();
    }
}

void loop() {
    if (Serial.available() >= 6) {  // Wait for header + size
        if (Serial.read() == 0xAA && Serial.read() == 0x55) {
            Serial.println("Header received...");

            uint8_t lenBytes[4];
            Serial.readBytes((char*)lenBytes, 4);
            uint32_t frameSize = ((uint32_t)lenBytes[0]) |
                                 (((uint32_t)lenBytes[1]) << 8) |
                                 (((uint32_t)lenBytes[2]) << 16) |
                                 (((uint32_t)lenBytes[3]) << 24);
            Serial.print("Frame size: ");
            Serial.println(frameSize);

            uint8_t *buffer = (uint8_t*)malloc(frameSize);
            if (!buffer) {
                Serial.println("Not enough memory.");
                return;
            }

            int bytesRead = 0;
            unsigned long startTime = millis();
            while (bytesRead < frameSize && (millis() - startTime < 1000)) {
                if (Serial.available()) {
                    bytesRead += Serial.readBytes((char*)buffer + bytesRead, frameSize - bytesRead);
                }
            }

            Serial.print("Bytes received: ");
            Serial.println(bytesRead);

            if (bytesRead == frameSize) {
                if (JpegDec.decodeArray(buffer, frameSize)) {
                    drawJpeg(0, 0);
                    Serial.println("Image displayed.");
                    Serial.write(0x01);
                } else {
                    Serial.println("Error decoding JPEG.");
                    Serial.write(0x00);
                }
            } else {
                Serial.println("Incomplete frame received.");
            }

            free(buffer);
        }
    }
}
