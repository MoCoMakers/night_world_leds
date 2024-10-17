from display_state import DisplayState
import time
from color_extras import COLOR_WHEEL_DIRECTIONS, interpolate_colors
from ledGrid import LEDGrid
import random

class SystemContext():
    def __init__(self, live_rgb, max_march_count):
        self.live_rgb = live_rgb
        self.baseline_rgb = live_rgb
        self.active_theme_set = []
        self.active_display_state = DisplayState.ROTATE
        self.last_display_state_update = time.time()
        self.hue_march_count = 0
        self.max_march_count = max_march_count
        self.restore_state = DisplayState.ROTATE
        self.last_active_direction = "middle"
    
    def update_active_theme_set(self, theme_set):
        self.active_theme_set = theme_set
    
    def update_live_rgb(self, live_rgb):
        self.live_rgb = live_rgb
    
    def update_baseline_rgb(self, rgb):
        self.baseline_rgb = rgb
    
    def update_meta_state(self, state_requested, request_args, global_grid_set):
        pass

    def get_live_rgb(self):
        return self.live_rgb
    
    def set_live_rgb(self, color):
        self.live_rgb = color
    
    def get_active_display_state(self):
        return self.active_display_state
    
    def set_active_display_state(self, state):
        self.active_display_state = state

    def translate_button_to_display_state(self, buttonVal):
        if not buttonVal:
            return None
        button_dict = {
            1: DisplayState.FLASH,
            2: DisplayState.BRIGHT,
            3: DisplayState.ROTATE,
            4: DisplayState.LIFE,
            5: DisplayState.DARK,
            6: DisplayState.GAL,
            7: DisplayState.TERR,
            8: DisplayState.SUB
        }
        return button_dict[buttonVal]
    
    def march_hue_to_direction(self, direction, direction_change):
        current_color = self.live_rgb
        if direction_change:
            self.hue_march_count=0
        target_color = COLOR_WHEEL_DIRECTIONS[direction]
        update_ratio = (self.hue_march_count * 1.0) / self.max_march_count
        new_color = interpolate_colors(start_color=current_color, end_color=target_color, ratio=update_ratio)
        self.live_rgb = new_color
        self.hue_march_count = self.hue_march_count + 1
        if self.hue_march_count > self.max_march_count:
            self.hue_march_count = self.max_march_count

    def unwide_hue_march(self):
        current_color = self.live_rgb
        target_color = self.baseline_rgb
        self.hue_march_count = self.hue_march_count - 1
        if self.hue_march_count < 0:
            self.hue_march_count = 0
        update_ratio = (self.hue_march_count * 1.0) / self.max_march_count
        new_color = interpolate_colors(start_color=current_color, end_color=target_color, ratio=update_ratio)
        self.live_rgb = new_color
    
    def updated_global_grid_set_to_theme(self, state_requested, global_grid_set):
        if state_requested == DisplayState.BRIGHT:
            for grid in global_grid_set:
                grid: LEDGrid
                grid.set_override_brightness(0)
                grid.set_grid_enabled(True)
        elif state_requested == DisplayState.DARK:
            for grid in global_grid_set:
                grid: LEDGrid
                grid.set_override_brightness(0)
                grid.set_grid_enabled(False)
        elif state_requested == DisplayState.GAL:
            for grid in global_grid_set:
                grid.set_override_brightness(0)
                if grid.get_theme() == "gal":
                    grid.set_grid_enabled(True)
                else:
                    grid.set_grid_enabled(False)
        elif state_requested == DisplayState.TERR:
            for grid in global_grid_set:
                grid.set_override_brightness(0)
                if grid.get_theme() == "terr":
                    grid.set_grid_enabled(True)
                else:
                    grid.set_grid_enabled(False)
        elif state_requested == DisplayState.LIFE:
            for grid in global_grid_set:
                grid.set_override_brightness(0)
                if grid.get_theme() == "life":
                    grid.set_grid_enabled(True)
                else:
                    grid.set_grid_enabled(False)
        elif state_requested == DisplayState.SUB:
            for grid in global_grid_set:
                grid.set_override_brightness(0)
                if grid.get_theme() == "sub":
                    grid.set_grid_enabled(True)
                else:
                    grid.set_grid_enabled(False)
        elif state_requested == DisplayState.ROTATE:
            for grid in global_grid_set:
                rotate_order = grid.get_rotate_order()

                #Update the roation order for next time:
                new_order = rotate_order + 1
                if new_order > len(global_grid_set):
                    new_order = 0
                grid.set_rotate_order(new_order)

                grid.set_override_brightness(0)

                halfway = len(global_grid_set)/2
                if rotate_order<= halfway:
                    new_order = rotate_order
                else:
                    new_order = len(global_grid_set) - (rotate_order + 1)
                ratio = (new_order*1.0)/halfway
                brightness = round(ratio * 255)
                grid.set_override_brightness(brightness)
                grid.set_grid_enabled(True)

        elif state_requested == DisplayState.FLASH:
            for grid in global_grid_set:
                random_value = random.randint(0, 255)
                grid.set_override_brightness(random_value)
                grid.set_grid_enabled(True)
        return global_grid_set
    
    def get_restore_state(self):
        return self.restore_state
    
    def set_restore_state(self, state):
        self.restore_state = state

    def get_last_active_direction(self):
        return self.last_active_direction
    
    def set_last_active_direction(self, direction):
        self.last_active_direction = direction