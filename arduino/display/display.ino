#include <TFT_eSPI.h>
#include <JPEGDecoder.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(969900);               // Configuramos UART0 a 3 Mbps
  Serial.setRxBufferSize(8192);         // Aumentamos el tamaño del buffer UART
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  Serial.println("ESP32 JPEG Display Ready...");
}

// Función para dibujar la imagen JPEG decodificada en la pantalla ST7789
void drawJpeg(int xpos, int ypos) {
  while (JpegDec.read()) {
    uint16_t *pImg = JpegDec.pImage;
    tft.pushImage(xpos + JpegDec.MCUx * JpegDec.MCUWidth, 
                  ypos + JpegDec.MCUy * JpegDec.MCUHeight, 
                  JpegDec.MCUWidth, JpegDec.MCUHeight, pImg);
    yield();  // Permite que el sistema realice otras tareas
  }
}

void loop() {
  // Se espera que haya al menos 2 bytes (encabezado) + 4 bytes (tamaño del frame)
  if (Serial.available() >= 6) {
    // Verificamos el encabezado de sincronización (0xAA 0x55)
    if (Serial.read() == 0xAA && Serial.read() == 0x55) {
      Serial.println("Sync header received.");
      
      // Leemos 4 bytes que indican el tamaño del frame JPEG
      uint8_t lenBytes[4];
      int lenRead = Serial.readBytes((char*)lenBytes, 4);
      if (lenRead != 4) {
        Serial.println("Error reading frame length.");
        return;
      }
      // Convertimos los 4 bytes a un entero (suponemos orden little-endian)
      uint32_t frameSize = ((uint32_t)lenBytes[0]) |
                           (((uint32_t)lenBytes[1]) << 8) |
                           (((uint32_t)lenBytes[2]) << 16) |
                           (((uint32_t)lenBytes[3]) << 24);
      Serial.print("Frame size: ");
      Serial.println(frameSize);
      
      // Reservamos memoria para el frame
      uint8_t *buffer = (uint8_t*)malloc(frameSize);
      if (buffer == NULL) {
        Serial.println("Failed to allocate memory for frame.");
        return;
      }
      
      int bytesRead = 0;
      unsigned long startTime = millis();
      // Esperamos hasta recibir todos los bytes o un timeout de 1000 ms
      while (bytesRead < frameSize && (millis() - startTime < 1000)) {
        if (Serial.available()) {
          bytesRead += Serial.readBytes((char*)buffer + bytesRead, frameSize - bytesRead);
        }
      }
      Serial.print("Bytes read: ");
      Serial.println(bytesRead);
      
      if (bytesRead == frameSize) {
        if (JpegDec.decodeArray(buffer, frameSize)) {
          Serial.println("JPEG decoded successfully.");
          drawJpeg(0, 0);
          Serial.println("Image displayed.");
          Serial.write(0x01);  // Enviamos confirmación al PC
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
