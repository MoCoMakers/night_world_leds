import cv2
import dlib
from feed import Feed
import multiprocessing
import requests
from ledGrid import LEDGrid
import socket
import re

ip_prefix = "192.168.147."
last_octed_list = [
    '120',
    '121',
    '122',
    '123',
    '124',
    '125',
    '126',
    '127',
    '128',
    '129',
    '130',
    '131',
    '132',
    '133',
    '134',
    '135',
    '136',
    '137',
    '138',
    '139',
    '140',
    '141',
    ]

ip_list = []
for octet in last_octed_list:
    ip_list.append(ip_prefix+octet)

nosignal = cv2.imread('no_signal.png')

def update_feed(ip):
    face_classifier = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")
    eye_classifier = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_eye.xml")
    frontal_detector = dlib.get_frontal_face_detector()
    current_feed = Feed(ip, face_classifier, eye_classifier, frontal_detector)

    while True:
        img = current_feed.load_image(doReturn = True)
        if current_feed.status == "ACTIVE":
            face_count, eye_count, frontal_count, rectangles = current_feed.detect_faces()
            print(face_count+eye_count+frontal_count)
            face = rectangles['face']
            eye = rectangles['eye']
            frontal = rectangles['frontal']
            for (x, y, w, h) in face:
                cv2.rectangle(img, (x, y), (x + w, y + h), (0, 255, 0), 4)
            
            for (x, y, w, h) in eye:
                cv2.rectangle(img, (x, y), (x + w, y + h), (255, 255, 0), 4)

            for face in frontal:
                x, y, w, h = face.left(), face.top(), face.width(), face.height()
                cv2.rectangle(img, (x, y), (x + w, y + h), (0, 255, 255), 2)
            
            cv2.imshow('FEED: '+str(current_feed.ip), img)
            # Needed to update the display
            if cv2.waitKey(1) & 0xFF == ord('q'):
                pass

        else:
            print("Disconnected at: "+current_feed.ip + " with: "+current_feed.status)
            cv2.imshow('FEED: '+str(current_feed.ip), nosignal)
            # Needed to update the display
            if cv2.waitKey(1) & 0xFF == ord('q'):
                pass

def check_valid_ip(prefix):
    # Get the local IP address
    hostname = socket.gethostname()
    local_ip = socket.gethostbyname(hostname)
    print(local_ip)
    
    # Define the regex pattern for the IP prefix
    pattern = f"^{prefix}"
    
    # Check if the local IP address starts with the specified prefix
    if re.match(pattern, local_ip):
        return True
    else:
        return False

if __name__ == "__main__":
    
    if not check_valid_ip(ip_prefix):
        raise Exception("Check your connected WiFi Network for prefix: "+ip_prefix)

    myGrid = LEDGrid(ip=ip_list[0],n_leds=190)
    spacer = 4
    myGrid.define_row(21,31, direction="forward")
    myGrid.define_row(37,47, direction="backward")
    myGrid.define_row(57,67, direction="forward")
    myGrid.define_row(73,83, direction="backward")
    myGrid.define_row(92,102, direction="forward")
    myGrid.define_row(107,117, direction="backward")
    myGrid.define_row(125,135, direction="forward")
    myGrid.define_row(140,150, direction="backward")
    myGrid.define_row(157,167, direction="forward")
    myGrid.define_row(174,184, direction="backward")



    while True:
       myGrid.do_animation("wave_pattern_matrix", brightness=100)

    # Debugging use this instead:
    # update_feed(ip_list[1])

    with multiprocessing.Pool(processes=len(ip_list)) as pool:
            pool.map(update_feed, ip_list)
        
    cv2.destroyAllWindows()