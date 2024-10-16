from ledRow import LEDRow
import math
import requests

class LEDGrid:
    def __init__(self, ip, n_leds):
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

    def define_row(self, start_index, end_index, direction):
        length = end_index - start_index
        newRow = LEDRow(length=length, offset=start_index, direction=direction)
        self.row_strips.append(newRow)

    def do_animation(self, function_name, brightness=None):
        if function_name == "wave_pattern_matrix":
            self.wave_pattern_matrix(brightness)

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
        self.full_data[index] = {"r": r, "g": g, "b": b, "brightness": brightness}

    def buildOutputURL(self):
        url = r"http://" + self.ip + "/colorRange?"
        for key in self.full_data.keys():
            pixel = self.full_data[key]
            r = str(pixel["r"])
            g = str(pixel["g"])
            b = str(pixel["b"])
            brightness = str(pixel["brightness"])
            index = str(key)
            url = url + "&i" + index + "r=" + r + "&i" + index + "g=" + g + "&i" + index + "b=" + b + "&i" + index + "brightness=" + brightness
        return url