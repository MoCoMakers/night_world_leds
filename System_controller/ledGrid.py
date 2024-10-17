from ledRow import LEDRow
import math
import requests
from networkHelper import do_simple_get
from color_extras import interpolate_colors

class LEDGrid:
    def __init__(self, ip, n_leds, current_state_brightness, rotate_order):
        pixel_raw = {}
        for index in range(n_leds):
            pixel_raw[index] = {
                "r": 0,
                "g": 0,
                "b": 0,
                "brightness": 0
            }
        self.full_data = pixel_raw
        self.row_strips = []
        self.ip = ip
        self.theme = None
        self.status = "inactive"
        self.current_state_brightness = current_state_brightness
        self.uses_camera = False
        self.face_detected_status = False
        self.face_detection_fade_direction = "down"
        self.face_detection_ratio = 0.0
        self.face_detection_lock = False
        self.face_detection_origional_color = (0,0,255)
        self.grid_enabled = True
        self.override_brightness = 0
        self.rotate_order = rotate_order

    def do_full_blackout(self):
        url = "http://"+self.ip+"/black"
        do_simple_get(url, self.ip)

    def define_row(self, start_index, end_index, direction):
        length = end_index - start_index
        newRow = LEDRow(length=length, offset=start_index, direction=direction)
        self.row_strips.append(newRow)

    def do_animation(self, function_name, brightness=None):
        if function_name == "wave_pattern_matrix":
            self.wave_pattern_matrix(brightness)

    def do_set_solid_color(self, color, brightness=None):
        if not brightness:
            brightness = self.current_state_brightness
        for y in range(len(self.row_strips)):
            for x in range(self.row_strips[y].length):
                self.setPixelColorXY(x, y, color, brightness)

    def wave_pattern_matrix(self, brightness, time_wait_ms=0.0):
        print("Building URL Set for: wave_pattern_matrix")
        url_set = []
        for j in range(256):
            for y in range(len(self.row_strips)):
                for x in range(self.row_strips[y].length):
                    color_value = int((math.sin(x + j / 10.0) + 1) * 127.5)
                    self.setPixelColorXY(x, y, (color_value, 0, 255 - color_value), brightness)
            url = self.buildOutputURL()
            url_set.append(url)
        
        print("Now Running Frames")
        count = 0
        for url in url_set:
            try:
                response = requests.get(url)
                # time.sleep(time_wait_ms / 1000.0)
            except:
                print("Failed connection on "+self.ip)
            print("Frame count: "+str(count))
            count=count+1

    def setPixelColorXY(self, x, y, color, brightness):
        index = self.row_strips[y].get_column_led_index(x)
        r, g, b = color
        # print("Color: "+str(color))
        self.full_data[index] = {"r": r, "g": g, "b": b, "brightness": brightness}

    def buildOutputURL(self):
        url = r"http://" + self.ip + "/colorRange?"
        for key in self.full_data.keys():
            pixel = self.full_data[key]
            r = str(pixel["r"])
            g = str(pixel["g"])
            b = str(pixel["b"])
            brightness = str(pixel["brightness"])

            # Enable theme control of what's on
            if not self.grid_enabled:
                brightness = str(0)
            
            if self.override_brightness:
                brightness = str(self.override_brightness)

            if r == 0 and g == 0 and b == 0 or brightness == 0:
                continue
            index = str(key)
            url = url + "&i" + index + "r=" + r + "&i" + index + "g=" + g + "&i" + index + "b=" + b + "&i" + index + "brightness=" + brightness
        return url
    
    def set_theme(self, theme):
        self.theme = theme
    
    def set_active(self, status):
        self.status = status
    
    def get_theme(self):
        return self.theme
    
    def get_ip(self):
        return self.ip
    
    def set_uses_camera(self, uses_camera):
        self.uses_camera = uses_camera
    
    def get_uses_camera(self):
        return self.uses_camera
    
    def set_face_detected_status(self, status):
        self.face_detected_status = status

    def set_face_detection_ratio(self, ratio):
        self.face_detection_ratio = ratio
    
    def get_face_detection_ratio(self):
        return self.face_detection_ratio
    
    def set_face_detection_fade_direction(self, direction):
        self.face_detection_fade_direction = direction
    
    def get_face_detection_fade_direction(self):
        return self.face_detection_fade_direction
    
    def get_color_at_xy(self, x, y):
        index = self.row_strips[y].get_column_led_index(x)
        pixel = self.full_data[index]
        return pixel
    
    def set_grid_enabled(self, status):
        self.grid_enabled = status
    
    def set_override_brightness(self, brightness):
        self.override_brightness = brightness
    
    def set_rotate_order(self, order):
        self.rotate_order = order
    
    def get_rotate_order(self):
        return self.rotate_order
    
    def update_face_detection_effect(self):
        brightness_step = 0.1 #counts up to 1.0
        if self.face_detection_lock: #Working through a face animation
            if self.face_detection_fade_direction == "up":
                if self.face_detection_ratio <= 1.0:
                    self.face_detection_ratio = self.face_detection_ratio + brightness_step
                    if self.face_detection_ratio > 1.0:
                        self.get_face_detection_ratio = 1.0
                        self.face_detection_fade_direction = "down" #Start going back down
                    for y in range(len(self.row_strips)):
                        for x in range(self.row_strips[y].length):
                            pixel = self.get_color_at_xy(x,y)
                            brightness = pixel["brightness"]
                            new_color = interpolate_colors((pixel["r"],pixel["g"],pixel["b"]), (255,255,255), self.face_detection_ratio)
                            self.setPixelColorXY(x, y, new_color, brightness)

            else: # fade direction is down
                self.face_detection_ratio = self.face_detection_ratio - brightness_step
                if self.face_detection_ratio < 0.0:
                        self.get_face_detection_ratio = 0.0
                        self.face_detection_lock = False

                for y in range(len(self.row_strips)):
                        for x in range(self.row_strips[y].length):
                            pixel = self.get_color_at_xy(x,y)
                            brightness = pixel["brightness"]
                            new_color = interpolate_colors((pixel["r"],pixel["g"],pixel["b"]), self.face_detection_origional_color, self.face_detection_ratio)
                            self.setPixelColorXY(x, y, new_color, brightness)
                
        else: # Check for new animation
            if self.face_detected_status: #Start new animation
                self.face_detection_fade_direction = "up"
                self.face_detection_lock = True
                pixel = self.get_color_at_xy(0,0)
                self.face_detection_origional_color = (pixel["r"], pixel["g"], pixel["b"]) # Sample one pixel's color