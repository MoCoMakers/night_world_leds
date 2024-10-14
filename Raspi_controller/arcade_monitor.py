from evdev import InputDevice, categorize, ecodes

# Replace '/dev/input/eventX' with the correct event device for your joystick
joystick = InputDevice('/dev/input/event4')

print("Joystick detected:", joystick.path)

# Initialize direciton vars
east = False
west = False
north = False
south = False

# Define a dictionary to map button codes to values
button_map = {
    #top row
    299: 1, 
    298: 2,
    # middle row
    297: 3,
    296: 4,
    295: 5,
    # bottom row
    294: 6,
    293: 7,
    292: 8
}

for event in joystick.read_loop():
    if event.type == ecodes.EV_KEY:
        button_event = categorize(event)
        if button_event.event.code in button_map:
            if event.value == 1:  # Check for "down" events only
                value = button_map[button_event.event.code]
                print(f"Button {button_event.event.code} is mapped to value {value}")
                # Perform actions based on the mapped value
        else:
            print(categorize(event)) 
            print(event.value)
    elif event.type == ecodes.EV_ABS:
        absevent = categorize(event)
        if absevent.event.code == ecodes.ABS_X:
            value = absevent.event.value
            print(f"X axis: {absevent.event.value}")
            if value > 200: # 0 = left, 127 = middle, 255 = right
                east = True
                west = False
            elif value < 90:
                west = True
                east = False
            else:
                east = False
                west = False
        elif absevent.event.code == ecodes.ABS_Y:
            value = absevent.event.value
            print(f"Y axis: {absevent.event.value}")
            if value > 200: # 0 = left, 127 = middle, 255 = right
                north = True
                south = False
            elif value < 90:
                south = True
                north = False
            else:
                north = False
                south = False
        
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
        
        if (final_direction !="middle"):
            print(final_direction)
