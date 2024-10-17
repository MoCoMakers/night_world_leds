import os
import cv2
import dlib
import socket
import re
import time
import requests
from display_state import DisplayState

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
from networkHelper import do_bulk_esp32_action, do_simple_get

from gridContext import global_grid_set, ip_prefix, ip_list
# See IP Address conguration in gridContext

ARCADE_IP = "192.168.147.93"
MAX_MARCH_COUNT = 5 # This speeds up joystick color changes when it's smaller

def update_feed(grid, return_dict):
    grid: LEDGrid
    ip = grid.get_ip()
    face_classifier = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")
    eye_classifier = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_eye.xml")
    frontal_detector = dlib.get_frontal_face_detector()
    current_feed = Feed(ip, face_classifier, eye_classifier, frontal_detector)

    img = current_feed.load_image(doReturn = True)
    if current_feed.status == "ACTIVE":
        face_count, eye_count, frontal_count, rectangles = current_feed.detect_faces()
        print("Face count:"+str(face_count+eye_count+frontal_count))
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

        return_dict[ip] = (img, face_count, eye_count, frontal_count)

    else:
        print("Disconnected at: "+current_feed.ip + " with: "+current_feed.status)
        nosignal = cv2.imread('no_signal.png')
        return_dict[ip] = (nosignal, 0,0,0)

def update_feed_ORIG(ip):
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

def paint_single_grid(grid):
    # grid.update_face_detection_effect() # Fade to all white. MOVED TO ARDUINO
    url = grid.buildOutputURL()
    do_simple_get(url, grid.get_ip())

def do_paint_all_grids(global_grid_set):
    do_bulk_esp32_action(paint_single_grid, global_grid_set)

def notify_face_detected(ip):
    url = "http://"+ip+"/notifyFaceDetected"
    
    do_simple_get(url, grid.get_ip())

def do_notify_all_face_detections(ips_with_face):
    do_bulk_esp32_action(notify_face_detected, ips_with_face)

def print_camera_feed(image_composite):
    img = image_composite[0]
    ip = image_composite[1]

    cv2.imshow('FEED: '+str(ip), img)
    # Needed to update the display
    if cv2.waitKey(1) & 0xFF == ord('q'):
        pass

    time.sleep(10)


def do_update_all_camera_feeds(camera_grid_set):
    return_dict = {}
    do_bulk_esp32_action(update_feed, camera_grid_set, has_returns=True, return_dict=return_dict)

    #At this time return_dict should hold all data

    composite_set = []

    for grid in camera_grid_set:
        current_ip = grid.get_ip()
        response = return_dict[current_ip]
        img = response[0]
        face_count = response[1]
        eye_count = response[2]
        frontal_count = response[3]

        composite_set.append((img, current_ip, face_count, eye_count, frontal_count))
    return_dict.clear()

    return composite_set

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

def check_arcade_running(ip):
    url = "http://"+ip+":5000"
    response = do_simple_get(url, ip, timeout=(1,10))
    response: requests.models.Response
    return response and response.status_code == 200 and "Arcade System Running" in response.text

if __name__ == "__main__":
    nosignal = cv2.imread('no_signal.png')
    
    if not check_valid_ip(ip_prefix):
        raise Exception("Check your connected WiFi Network for prefix: "+ip_prefix)
    
    if not check_arcade_running(ARCADE_IP):
        raise Exception("Make sure arcade is on, and that it's running at address: http://"+ARCADE_IP+":5000")

    # Initialize game state variables
    running = True
    frame_rate = 60  # Frames per second
    frame_duration = 1.0 / frame_rate  # Duration of each frame in seconds

    # Game loop
    last_frame_time = time.time()

    starting_rgb = (255,255,0) # Red
    sysCnt = SystemContext(starting_rgb, max_march_count=MAX_MARCH_COUNT)
    arcade = ArcadeManager(ip = ARCADE_IP, context = sysCnt)

    blackout_all_lines(global_grid_set)

    camera_grid_set = []
    for grid in global_grid_set:
        grid: LEDGrid
        if grid.get_uses_camera():
            camera_grid_set.append(grid)


    # Intialize persistent feed windows:
    for grid in camera_grid_set:
        cv2.imshow('FEED: '+str(grid.get_ip()), nosignal)
        # Needed to update the display
        if cv2.waitKey(1) & 0xFF == ord('q'):
            pass
    

    while running:
        current_time = time.time()
        elapsed_time = current_time - last_frame_time

        if True:
            print("Frame loop")
            last_frame_time = current_time
            oldState = sysCnt.get_active_display_state()
            print("Current theme: "+str(oldState))

            state_requested, request_args = arcade.read_state_requested()
            print(state_requested)
            print(request_args)

            if state_requested:
                if state_requested not in [DisplayState.BRIGHT, DisplayState.DARK]:
                    sysCnt.set_active_display_state(state_requested)
                    sysCnt.set_restore_state(state_requested)
                    global_grid_set = sysCnt.updated_global_grid_set_to_theme(sysCnt.get_active_display_state(), global_grid_set)

                    if state_requested == DisplayState.TERR:
                        #Green
                        color = (0,255,0)
                        sysCnt.set_live_rgb(color)
                        sysCnt.update_baseline_rgb(color)
                    elif state_requested == DisplayState.GAL:
                        #Purple
                        color = (168,52,235)
                        sysCnt.set_live_rgb(color)
                        sysCnt.update_baseline_rgb(color)
                    elif state_requested == DisplayState.SUB:
                        #Redish
                        color = (235, 52, 91)
                        sysCnt.set_live_rgb(color)
                        sysCnt.update_baseline_rgb(color)
                    elif state_requested == DisplayState.LIFE:
                        #Light blue
                        color = (176, 210, 255)
                        sysCnt.set_live_rgb(color)
                        sysCnt.update_baseline_rgb(color)
                else:
                    global_grid_set = sysCnt.updated_global_grid_set_to_theme(state_requested, global_grid_set)

            else:
                global_grid_set = sysCnt.updated_global_grid_set_to_theme(sysCnt.get_restore_state(), global_grid_set)
            



            # if state_requested:
            #     if state_requested != DisplayState.BRIGHT or state_requested != DisplayState.DARK:
            #         sysCnt.set_active_display_state(state_requested)
            #         global_grid_set = sysCnt.updated_global_grid_set_to_theme(sysCnt.get_active_display_state(), global_grid_set)
            #     else:
            #         if oldState!= DisplayState.BRIGHT or oldState != DisplayState.DARK:
            #             sysCnt.set_restore_state(oldState)
            #         global_grid_set = sysCnt.updated_global_grid_set_to_theme(state_requested, global_grid_set)
            # else:
            #     global_grid_set = sysCnt.updated_global_grid_set_to_theme(sysCnt.get_restore_state(), global_grid_set)

            # sysCnt.update_meta_state(state_requested, request_args, global_grid_set)
            
            for grid in global_grid_set:
                grid: LEDGrid
                if grid.get_theme() != "led":
                    print(sysCnt.get_live_rgb())
                    grid.do_set_solid_color(sysCnt.get_live_rgb())
                else:
                    #TODO: get animation.update() or next()
                    pass

            composite_set = do_update_all_camera_feeds(camera_grid_set)

            ips_with_face = []
            for entry in composite_set:
                img = entry[0]
                ip = entry[1]
                cv2.imshow('FEED: '+str(ip), img)
                # Needed to update the display
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    pass

                face_count = entry[2]
                eye_count = entry[3]
                frontal_count = entry[4]

                if face_count+eye_count+frontal_count > 0:
                    face_detected = True
                    ips_with_face.append(ip)
                
                do_notify_all_face_detections(ips_with_face)

                
                # for grid in global_grid_set:
                #     if grid.get_ip() == ip:
                #         grid.set_face_detected_status(True)
                

            # Paint final
            do_paint_all_grids(global_grid_set)
           
        else:
            raise Exception("Undertimed loop actually found!")
            pass


        

    # while True:
    #    myGrid.do_animation("wave_pattern_matrix", brightness=100)

    # Debugging use this instead:
    # update_feed(ip_list[1])

    
        
    cv2.destroyAllWindows()