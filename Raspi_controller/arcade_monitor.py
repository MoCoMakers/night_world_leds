from evdev import InputDevice, categorize, ecodes
from multiprocessing import Process, Manager
import time

def joystick_reader(shared_data):
    joystick = InputDevice('/dev/input/event4')
    print("Joystick detected:", joystick.path)
    
    button_map = {
        299: 1, 298: 2, 297: 3, 296: 4, 295: 5,
        294: 6, 293: 7, 292: 8
    }

    east = False
    west = False
    north = False
    south = False

    for event in joystick.read_loop():
        if event.type == ecodes.EV_KEY:
            button_event = categorize(event)
            if button_event.event.code in button_map:
                if event.value == 1:
                    value = button_map[button_event.event.code]
                    shared_data["button_value"] = value
                else:
                    shared_data["button_value"] = None
        elif event.type == ecodes.EV_ABS:
            absevent = categorize(event)
            if absevent.event.code == ecodes.ABS_X:
                value = absevent.event.value
                east = value > 200
                west = value < 90
                if not east and not west:
                    east, west = False, False
            elif absevent.event.code == ecodes.ABS_Y:
                value = absevent.event.value
                north = value > 200
                south = value < 90
                if not north and not south:
                    north, south = False, False
                
            if north:
                if west:
                    final_direction = "northwest"
                elif east:
                    final_direction = "northeast"
                else:
                    final_direction = "north"
            elif south:
                if west:
                    final_direction = "southwest"
                elif east:
                    final_direction = "southeast"
                else:
                    final_direction = "south"
            elif west:
                final_direction = "west"
            elif east:
                final_direction = "east"
            else:
                final_direction = "middle"

            shared_data["direction"] = final_direction
                
