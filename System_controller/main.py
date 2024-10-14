import cv2
import requests
from PIL import Image
import numpy as np
import dlib

#FEED_RESOLUTION = (320,240)
FEED_RESOLUTION = (640,480)
#FEED_RESOLUTION = (800,600)
#FEED_RESOLUTION = (1024,768)
IP = "192.168.134.186"

size = FEED_RESOLUTION[0]*FEED_RESOLUTION[1]
img = Image.open(requests.get("http://"+IP+"/jpg", stream=True).raw)
avg_frame = None

hog = cv2.HOGDescriptor()
hog.setSVMDetector(cv2.HOGDescriptor_getDefaultPeopleDetector())

face_classifier = cv2.CascadeClassifier(
    cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
)

eye_classifier = cv2.CascadeClassifier(
    cv2.data.haarcascades + "haarcascade_eye.xml"
)

# Load the dlib face detector
detector = dlib.get_frontal_face_detector()

while True:
    
    img = Image.open(requests.get("http://"+IP+"/jpg", stream=True).raw)

    # read the image
    img = cv2.cvtColor(np.array(img), cv2.COLOR_RGB2BGR)
    img = cv2.resize(img, FEED_RESOLUTION)

    # Adjust the brightness and contrast 
    # Adjusts the brightness by adding 10 to each pixel value 
    brightness = 10 
    # Adjusts the contrast by scaling the pixel values by 2.3 
    contrast = 1.3  
    img = cv2.addWeighted(img, contrast, np.zeros(img.shape, img.dtype), 0, brightness) 

    # convert to grayscale
    img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    face = face_classifier.detectMultiScale(
        img_gray, scaleFactor=1.1, minNeighbors=5, minSize=(40, 40)
    )

    for (x, y, w, h) in face:
        cv2.rectangle(img, (x, y), (x + w, y + h), (0, 255, 0), 4)

    eye = eye_classifier.detectMultiScale(
        img_gray, scaleFactor=1.1, minNeighbors=5, minSize=(40, 40)
    )

    for (x, y, w, h) in eye:
        cv2.rectangle(img, (x, y), (x + w, y + h), (255, 255, 0), 4)
    
    faces = detector(img_gray)
    # Draw rectangles around faces
    for face in faces:
        x, y, w, h = face.left(), face.top(), face.width(), face.height()
        cv2.rectangle(img, (x, y), (x + w, y + h), (0, 255, 255), 2)


    # (humans, _) = hog.detectMultiScale(img, winStride=(10, 10), padding=(32, 32), scale=1.1)
    # print('Human Detected : ', len(humans))
    # # loop over all detected humans
    # for (x, y, w, h) in humans:
    #     pad_w, pad_h = int(0.15 * w), int(0.01 * h)
    #     cv2.rectangle(img, (x + pad_w, y + pad_h), (x + w - pad_w, y + h - pad_h), (0, 255, 0), 2)

    # Apply non-local means denoising
    # img = cv2.fastNlMeansDenoisingColored(img, None, 10, 10, 7, 10)

    # display the image
    # cv2.imshow('original image', img)

    
    # blur the image for better detection
    # blur_img = cv2.GaussianBlur(img, (3,3), 0)

    # display the blur image
    cv2.imshow('blur image', img)

    # canny edge detection
    # canny = cv2.Canny(blur_img, 250, 255)

    # display the canny image
    # cv2.imshow('canny image', canny)
    # cv2.waitKey(0)

    # laplacian edge detection
    # laplacian = cv2.Laplacian(blur_img, cv2.CV_64F)

    # if avg_frame is None:
    #     avg_frame = laplacian

    # display the laplacian image
    # cv2.imshow('laplacian image', laplacian) 

    #result = cv2.accumulateWeighted(laplacian, avg_frame, 0.01)
    #result = cv2.subtract(result,laplacian)

    #cv2.imshow('result image', result) 
        
    # Press 'q' to exit the feed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break


cv2.destroyAllWindows()