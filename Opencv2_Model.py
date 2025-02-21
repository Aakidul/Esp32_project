import cv2
import time
import requests
import numpy as np

# ESP32 Web Server URL
server_url = "http://192.168.1.1/update"

# Open the default camera (webcam)
cap = cv2.VideoCapture(0)

# Variables for periodic POST sending
post_interval = 5  # seconds
last_post_time = time.time()

def detect_faces(frame):
    """
    Detects faces using edge detection and contour filtering.
    This is a naive approach that approximates face-like structures.
    """
    # Convert to grayscale
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # Apply Gaussian blur to remove noise
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)

    # Apply Canny edge detection
    edges = cv2.Canny(blurred, 50, 150)

    # Find contours
    contours, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    valid_faces = []
    for contour in contours:
        x, y, w, h = cv2.boundingRect(contour)

        # Simple filter based on face-like proportions
        aspect_ratio = w / float(h)
        if 0.8 < aspect_ratio < 1.2 and 50 < w < 300:
            valid_faces.append((x, y, w, h))
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 0, 255), 2)  # Red rectangle

    return valid_faces

# Main loop for video processing
while True:
    ret, frame = cap.read()
    if not ret:
        print("Failed to grab frame. Exiting.")
        break

    # Detect faces
    faces = detect_faces(frame)
    face_count = len(faces)

    # Display detected face count
    cv2.putText(frame, f"Faces: {face_count}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
    
    # Show frame with detected faces
    cv2.imshow("Face Detection (No ML)", frame)
    
    # Send data every 5 seconds
    current_time = time.time()
    if current_time - last_post_time >= post_interval:
        payload = {"person": face_count}
        try:
            response = requests.post(server_url, json=payload, timeout=2)
            print(f"Sent: {payload} | Response: {response.text}")
        except Exception as e:
            print("Error sending POST request:", e)
        last_post_time = current_time

    # Exit if 'q' key is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release camera and close windows
cap.release()
cv2.destroyAllWindows()