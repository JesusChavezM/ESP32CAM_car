import cv2 #opencv
import urllib.request #para abrir y leer URL
import numpy as np
import requests
from tkinter import *

# URL of the ESP32-CAM
url = 'http://172.20.10.12/cam-hi.jpg'
winName = 'ESP32 CAMERA'
cv2.namedWindow(winName, cv2.WINDOW_AUTOSIZE)
base_url = 'http://172.20.10.12'


# Definimos el tamaño del vector para identificar objetos
def determine_slot_number(box):
    x_center = box[0] + box[2] / 2
    if x_center < 160:
        return 0
    elif x_center < 320:
        return 1
    elif x_center < 480:
        return 2
    else:
        return 3

# Load class names from coco.names
classNames = []
classFile = 'coco.names'
with open(classFile, 'rt') as f:
    classNames = f.read().rstrip('\n').split('\n')

configPath = 'ssd_mobilenet_v3_large_coco_2020_01_14.pbtxt'
weightsPath = 'frozen_inference_graph.pb'

net = cv2.dnn_DetectionModel(weightsPath, configPath)
net.setInputSize(320, 320)
net.setInputScale(1.0 / 127.5)
net.setInputMean((127.5, 127.5, 127.5))
net.setInputSwapRB(True)

def process_image():
    imgResponse = urllib.request.urlopen(url)  # Open URL
    imgNp = np.array(bytearray(imgResponse.read()), dtype=np.uint8)
    img = cv2.imdecode(imgNp, -1)  # Decode image

    img = cv2.rotate(img, cv2.ROTATE_90_COUNTERCLOCKWISE)  # Rotate if needed

    classIds, confs, bbox = net.detect(img, confThreshold=0.5)
    print(classIds, bbox)

    # Reset parking slots status
    Object = [0] * 2

    if len(classIds) != 0:
        for classId, confidence, box in zip(classIds.flatten(), confs.flatten(), bbox):
            label = classNames[classId - 1]
            if label == 'laptop' or label == 'bottle' or label == 'book' or label == 'cell phone':  # Update based on class
                slot_number = determine_slot_number(box)  # Implement this function based on your logic
                Object[slot_number] = 1
                cv2.rectangle(img, box, color=(0, 255, 0), thickness=3)
                cv2.putText(img, label, (box[0] + 10, box[1] + 30), cv2.FONT_HERSHEY_COMPLEX, 1, (0, 255, 0), 2)

    print(f"Object: {Object}")

    cv2.imshow(winName, img)  # Show image


def send_command(command):
    # Construye la URL del comando
    command_url = f"{base_url}/{command}"
    # Envía la solicitud HTTP GET a la URL del comando
    response = requests.get(command_url)
    # Imprime la respuesta del servidor
    print(f"Response from {command_url}: {response.text}")

# Crea una ventana
window = Tk()

# Crea botones para cada comando
Button(window, text="Adelante", command=lambda: send_command("forward")).pack()
Button(window, text="Atrás", command=lambda: send_command("backward")).pack()
Button(window, text="Izquierda", command=lambda: send_command("left")).pack()
Button(window, text="Derecha", command=lambda: send_command("right")).pack()
Button(window, text="Detener", command=lambda: send_command("stop")).pack()
Button(window, text="Encender LED", command=lambda: send_command("led_on")).pack()
Button(window, text="Apagar LED", command=lambda: send_command("led_off")).pack()

# Llama a process_image una vez para iniciar el ciclo
process_image()

# Ejecuta la ventana
window.mainloop()