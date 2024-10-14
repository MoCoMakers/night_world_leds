import colorspace

def transition_color_get_list(start_color, end_color, list_size):
    # Define the start and end colors in RGB as a tuple - e.g start_color = (255, 0, 0)  # Red

    # Create a ColorLib object
    cl = colorspace.colorlib()

    # Create a list to hold the transition colors
    transition_colors = []

    for step in range(list_size):
        # Compute the intermediate color
        intermediate_color = interpolate_colors(start_color, end_color, step / list_size )
        transition_colors.append(intermediate_color)

    # Print the transition colors
    for color in transition_colors:
        print(color)

def interpolate_colors(start_color, end_color, ratio):
    r = int(start_color[0] + (end_color[0] - start_color[0]) * ratio)
    g = int(start_color[1] + (end_color[1] - start_color[1]) * ratio)
    b = int(start_color[2] + (end_color[2] - start_color[2]) * ratio)
    return (r, g, b)

COLOR_WHEEL_DIRECTIONS = {
     "north": (255, 0, 0), #Red
     "northeast": (255, 127, 0), #Orange
     "east": (255, 255, 0), # Yellow
     "southeast": (127, 255, 0),  # yellow-green
     "south": (0, 255, 0), # Green
     "southwest": (0, 255, 255), #Cyan
     "west": (0, 0, 255), #Blue
     "northwest": (255, 0, 255) #Magenta
 }


if __name__ == "__main__":
    my_list = transition_color_get_list((255,0,0),(0,0,255), 10)
    print(my_list)