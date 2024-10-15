import cv2
import dlib
from feed import Feed

ip_list = [
    '192.168.134.186',
    ]

face_classifier = cv2.CascadeClassifier(
    cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
)

eye_classifier = cv2.CascadeClassifier(
    cv2.data.haarcascades + "haarcascade_eye.xml"
)

# Load the dlib face detector
frontal_detector = dlib.get_frontal_face_detector()

feeds_list = []

for ip in ip_list:
    next_feed = Feed(ip,face_classifier,eye_classifier, frontal_detector)
    feeds_list.append(next_feed)


while True:
    count = 0
    current_feed: Feed
    for current_feed in feeds_list:
        img = current_feed.load_image(doReturn = True)
        face_count, eye_count, frontal_count, rectangles = current_feed.detect_faces()
        for rectangle in rectangles:
            for (x, y, w, h) in rectangle:
                cv2.rectangle(img, (x, y), (x + w, y + h), (0, 255, 0), 4)
        
        cv2.imshow('result image', img)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
        count = count + 1

    # Press 'q' to exit the feed
    


cv2.destroyAllWindows()