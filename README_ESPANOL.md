# 📽️ ESP32 Video Streaming a Pantalla ST7789

Este proyecto permite transmitir video desde un PC a una pantalla **ST7789** usando un **ESP32** a través de UART.

## 🚀 Características
- ✅ Transmisión de video en tiempo real desde un PC al ESP32.
- ✅ Conversión y compresión de frames a **JPEG** para optimizar la transmisión.
- ✅ Decodificación en el ESP32 y visualización en la pantalla ST7789.
- ✅ Ajuste de color y rotación automática de la imagen en **Python**.
- ✅ Optimización de FPS (~10-15 FPS con UART estable a 921600 baudios).

## 📌 Requisitos
### 🔹 **Hardware**
- 📟 **Pantalla ST7789** (135x240 píxeles, SPI)
- 🎛️ **ESP32** (preferiblemente con PSRAM si deseas más rendimiento)
- 🖥️ **PC con Linux o Windows** (para enviar video)
- 🔌 **Cable USB** para programar y comunicar con el ESP32

### 🔹 **Software y Librerías**
#### 📂 **En el ESP32 (Arduino)**
Asegúrate de instalar las siguientes librerías en el **Arduino IDE**:
- [`TFT_eSPI`](https://github.com/Bodmer/TFT_eSPI) → Para manejar la pantalla ST7789
- [`JPEGDecoder`](https://github.com/Bodmer/JPEGDecoder) → Para decodificar imágenes JPEG

📌 **Configuraciones necesarias en `TFT_eSPI`**:
1. Editar `User_Setup.h` y asegurarse de definir:
   ```cpp
   #define ST7789_DRIVER
   #define TFT_WIDTH  135
   #define TFT_HEIGHT 240
   #define SPI_FREQUENCY  40000000  // Opcionalmente, 80 MHz si la pantalla lo soporta
   #define USE_DMA  // Para mejor rendimiento
   ```

#### 🐍 **En Python (PC)**
Instalar las dependencias con:
```bash
pip install -r requirements.txt
```
📌 **Contenido de `requirements.txt`:**
```
opencv-python
pyserial
numpy
```

## 🔧 Instalación y Uso
### 1️⃣ **Configurar y Cargar el Código en el ESP32**
1. **Abrir Arduino IDE** y cargar el código en `display.ino`.
2. **Cerrar el Monitor Serie** de Arduino antes de ejecutar el script en Python.
3. **Conectar la pantalla ST7789 al ESP32** usando SPI:
   | **ESP32** | **ST7789** |
   |-----------|-----------|
   | **3.3V**  | **VCC**   |
   | **GND**   | **GND**   |
   | **18**    | **SCK**   |
   | **19**    | **MOSI**  |
   | **5**     | **DC**    |
   | **23**    | **CS**    |
   | **4**     | **RST**   |

### 2️⃣ **Ejecutar el Código en el PC (Python)**
1. **Abrir la terminal** y ejecutar:
   ```bash
   python3 send.py
   ```
2. El video comenzará a transmitirse a la pantalla ST7789 a través del ESP32.

## 🛠️ Solución de Problemas
### ❌ **Colores distorsionados**
✔️ Solución: Asegurarse de que en el ESP32 esté activado `tft.setSwapBytes(true);`

### ❌ **Imagen rotada**
✔️ Solución: En Python, modificar:
   ```python
   frame = cv2.rotate(frame, cv2.ROTATE_90_CLOCKWISE)
   ```

### ❌ **La imagen no ocupa toda la pantalla**
✔️ Solución: Asegurarse de que el tamaño del frame en Python es `135x240`:
   ```python
   frame = cv2.resize(frame, (135, 240))
   ```

## 📜 Licencia
Este proyecto está bajo la licencia **MIT**, puedes usarlo y modificarlo libremente.

Desarrollado por Pablo Toledo
---


