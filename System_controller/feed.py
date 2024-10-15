import requests
from PIL import Image
import cv2
import numpy as np

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
        url = f"http://{self.ip}/jpg"

        try:
            response = requests.get(url, stream=True)
            self.latest_image = Image.open(response.raw)
        except requests.exceptions.ConnectTimeout:
            self.status = "DISCONNECTED"
        

        if doReturn:
            return self.latest_image
    
    def detect_faces(self):
        img = self.latest_image
        # read the image
        img = cv2.cvtColor(np.array(img), cv2.COLOR_RGB2BGR)
        img = cv2.resize(img, self.feed_resolution)

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


        
        