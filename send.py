import cv2
import serial
import time
import struct

# Configuración del puerto serial
SERIAL_PORT = "/dev/ttyUSB0"   # En Windows usa "COM3" o "COM4"
BAUD_RATE = 969900            # 3 Mbps

# Abrir el puerto serial
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)

# Abrir el video
video_path = "video.mp4"
cap = cv2.VideoCapture(video_path)

print("Iniciando transmisión de video...")

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        cap.set(cv2.CAP_PROP_POS_FRAMES, 0)  # Reiniciar video al llegar al final
        continue

    # Redimensionar al tamaño de la pantalla (135x240)
    frame = cv2.resize(frame, (135, 240))

    # Comprimir la imagen a JPEG con calidad 20 (reduce el tamaño del frame)
    _, buffer = cv2.imencode(".jpg", frame, [cv2.IMWRITE_JPEG_QUALITY, 20])
    jpeg_bytes = buffer.tobytes()
    frame_size = len(jpeg_bytes)
    print(f"Enviando JPEG de {frame_size} bytes...")

    # Vaciar el buffer de entrada antes de enviar
    ser.reset_input_buffer()

    # Enviar encabezado de sincronización: 0xAA 0x55
    ser.write(b'\xAA\x55')

    # Enviar el tamaño del frame como un entero de 4 bytes (little-endian)
    ser.write(struct.pack('<I', frame_size))

    # Enviar los datos JPEG en bloques
    chunk_size = 1024
    for i in range(0, frame_size, chunk_size):
        ser.write(jpeg_bytes[i:i+chunk_size])
        time.sleep(0.0005)  # Pequeña pausa para mejorar la estabilidad

    # Esperar la respuesta del ESP32
    response = ser.read()
    print(f"Respuesta del ESP32: {response}")

    # Ajusta el delay para intentar aumentar los FPS (p.ej., 1/25 para 25 FPS)
    time.sleep(1/25)

cap.release()
ser.close()
