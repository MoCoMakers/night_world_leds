import os
import cv2
import dlib
import socket
import re
import time

# Get the current working directory
cwd = os.getcwd()

try:
    system_path = os.path.dirname(os.path.abspath(__file__))
    os.chdir(system_path)
    print(f"Changed directory to: {system_path}")
except FileNotFoundError:
    print("System folder not found")
    
from feed import Feed
from ledGrid import LEDGrid
from system_context import SystemContext
from arcadeManager import ArcadeManager
from networkHelper import do_bulk_esp32_action

from gridContext import global_grid_set, ip_prefix, ip_list
# See IP Address conguration in gridContext

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

def blackout_single_line(grid):
    grid: LEDGrid
    grid.do_full_blackout() 

def blackout_all_lines(global_grid_set):
    do_bulk_esp32_action(blackout_single_line, global_grid_set)

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
    nosignal = cv2.imread('no_signal.png')
    
    if not check_valid_ip(ip_prefix):
        raise Exception("Check your connected WiFi Network for prefix: "+ip_prefix)

    # Initialize game state variables
    running = True
    frame_rate = 60  # Frames per second
    frame_duration = 1.0 / frame_rate  # Duration of each frame in seconds

    # Game loop
    last_frame_time = time.time()

    starting_rgb = (255,0,0) # Red
    sysCnt = SystemContext(starting_rgb)
    arcade = ArcadeManager()

    blackout_all_lines(global_grid_set)

    while running:
        current_time = time.time()
        elapsed_time = current_time - last_frame_time

        if elapsed_time >= frame_duration:
            print("Frame loop")
            last_frame_time = current_time

            state_requested, request_args = arcade.read_state_requested()
            if state_requested:
                sysCnt.update_meta_state(state_requested, request_args, global_grid_set)

            # Update system state
        else:
            raise Exception("Undertimed loop actually found!")
            pass


        

    while True:
       myGrid.do_animation("wave_pattern_matrix", brightness=100)

    # Debugging use this instead:
    # update_feed(ip_list[1])

    
        
    cv2.destroyAllWindows()