import requests
from PIL import Image
import cv2
import numpy as np
import time

class Feed:
    def __init__(self, ip, face_classifier, eye_classifier, frontal_detector):
        self.ip = ip
        self.status = "INACTIVE"
        self.latest_image = None
        self.feed_resolution = (640,480)
        self.face_classifier = face_classifier
        self.eye_classifier = eye_classifier
        self.frontal_detector = frontal_detector
  
    def set_feed_resolution(self, resolution):
        # FEED_RESOLUTION = (320,240)
        # FEED_RESOLUTION = (640,480)
        # FEED_RESOLUTION = (800,600)
        # FEED_RESOLUTION = (1024,768)
        self.feed_resolution = resolution

    def load_image(self, doReturn=False):
        try:
            img = Image.open(requests.get("http://"+self.ip+"/jpg", stream=True).raw)
            img.convert('RGB')
            # read the image
            img = cv2.cvtColor(np.array(img), cv2.COLOR_RGB2BGR)
            img = cv2.resize(img, self.feed_resolution)
            self.latest_image = img
            self.status = "ACTIVE"
        except requests.exceptions.ConnectTimeout:
            self.status = "DISCONNECTED 1"
        except requests.exceptions.ConnectionError:
            self.status = "DISCONNECTED 2"
        except requests.exceptions.ReadTimeout:
            self.status = "DISCONNECTED 3"
        except:  # noqa: E722
            self.status = "FAILED"
        

        if doReturn:
            return self.latest_image
    
    def detect_faces(self):
        img = self.latest_image
        # Adjust the brightness and contrast 
        # Adjusts the brightness by adding 10 to each pixel value 
        # brightness = 10 
        # # Adjusts the contrast by scaling the pixel values by 2.3 
        # contrast = 1.3  
        # img = cv2.addWeighted(img, contrast, np.zeros(img.shape, img.dtype), 0, brightness) 

        # convert to grayscale
        img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)    

        face = self.face_classifier.detectMultiScale(
            img_gray, scaleFactor=1.1, minNeighbors=5, minSize=(40, 40)
        )

        eye = self.eye_classifier.detectMultiScale(
            img_gray, scaleFactor=1.1, minNeighbors=5, minSize=(40, 40)
        )

        frontal = self.frontal_detector(img_gray)

        face_count = len(face)
        eye_count = len(eye)
        frontal_count = len(frontal)

        rectangles = {
            "face": face,
            "eye": eye,
            "frontal": frontal
        }

        return (face_count, eye_count, frontal_count, rectangles)


        
        