import cv2
import serial
import time
import struct
import numpy as np

# UART port configuration
SERIAL_PORT = "/dev/ttyUSB0"  # On Windows use "COM3" or "COM4"
BAUD_RATE = 921600  # Keep a stable speed

# Open the serial port
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)

# Load the video
video_path = "video.mp4"
cap = cv2.VideoCapture(video_path)

# Variables for frame comparison
prev_frame = None
frame_skip_threshold = 500  # If the change is below this value, we do not send the frame

print("Starting video transmission...")

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        cap.set(cv2.CAP_PROP_POS_FRAMES, 0)  # Restart video when reaching the end
        continue

    # Resize the frame to optimize performance
    frame = cv2.rotate(frame, cv2.ROTATE_90_CLOCKWISE)
    frame = cv2.resize(frame, (135, 240))

    # Convert to grayscale to detect changes (more efficient than color)
    gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # Compare with the previous frame, and if the change is below the threshold, skip it
    if prev_frame is not None:
        diff = cv2.absdiff(prev_frame, gray_frame)  # Detect differences between frames
        if cv2.countNonZero(diff) < frame_skip_threshold:  # If the change is minimal, skip the frame
            continue

    # Save the current frame for the next comparison
    prev_frame = gray_frame.copy()

    # Compress the image to JPEG with optimized quality
    _, buffer = cv2.imencode(".jpg", frame, [cv2.IMWRITE_JPEG_QUALITY, 25])
    jpeg_bytes = buffer.tobytes()
    frame_size = len(jpeg_bytes)

    print(f"Sending JPEG of {frame_size} bytes...")

    # Clear the input buffer before sending new data
    ser.reset_input_buffer()

    # Send synchronization header: 0xAA 0x55
    ser.write(b'\xAA\x55')

    # Send the frame size as a 4-byte integer (little-endian)
    ser.write(struct.pack('<I', frame_size))

    # Send JPEG data in larger chunks (2048 bytes)
    chunk_size = 2048
    for i in range(0, frame_size, chunk_size):
        ser.write(jpeg_bytes[i:i+chunk_size])
        time.sleep(0.0003)  # Small delay to avoid saturating UART

    # Wait for the ESP32 response
    response = ser.read()
    print(f"ESP32 response: {response}")

    # Try to increase FPS by reducing delay
    time.sleep(1/30)  # Attempt to reach 30 FPS

cap.release()
ser.close()
