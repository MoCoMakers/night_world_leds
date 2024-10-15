import cv2
import dlib
from feed import Feed
import multiprocessing

ip_list = [
    '192.168.134.120',
    '192.168.134.121',
    '192.168.134.122',
    '192.168.134.123',
    '192.168.134.124',
    '192.168.134.125',
    '192.168.134.126',
    '192.168.134.127',
    '192.168.134.128',
    '192.168.134.129',
    '192.168.134.130',
    '192.168.134.131',
    '192.168.134.132',
    '192.168.134.133',
    '192.168.134.134',
    '192.168.134.135',
    '192.168.134.136',
    '192.168.134.137',
    '192.168.134.138',
    '192.168.134.139',
    '192.168.134.140',
    '192.168.134.141',
    ]

"""
from PIL import Image
import requests
IP = '192.168.134.121'
import numpy as np
while True:
    img = Image.open(requests.get("http://"+IP+"/jpg", stream=True).raw)
    img.save("TESTING.jpg")
    img = cv2.cvtColor(np.array(img), cv2.COLOR_RGB2BGR)
    img = cv2.resize(img, (640,480))
    cv2.imshow('live feed image', img)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
"""

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
if __name__ == "__main__":

    # Debugging use this instead:
    # update_feed(ip_list[1])

    with multiprocessing.Pool(processes=len(ip_list)) as pool:
            pool.map(update_feed, ip_list)
        
    cv2.destroyAllWindows()