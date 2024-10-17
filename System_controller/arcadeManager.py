from networkHelper import do_simple_get
from system_context import SystemContext
import requests

class ArcadeManager():
    def __init__(self, ip, context):
       self.ip = ip
       self.context: SystemContext
       self.context = context

    def read_state_requested(self):
        url = "http://"+self.ip+":5000/getButtonStatus"
        response = do_simple_get(url, self.ip, timeout=(1,10))
        request_args = {}
        state_requested = None
        if response:
            try:
                if response.status_code == 200:
                    data = response.json()
                    button = data["button_value"]
                    direction = data["direction"]

                    if self.context.get_last_active_direction() != direction:
                        direction_change = True
                    else:
                        direction_change = False

                    self.context.set_last_active_direction(direction)
                    
                    if button:
                        state_requested = self.context.translate_button_to_display_state(button)
                    
                    if direction != "middle":
                        self.context.march_hue_to_direction(direction, direction_change)
                    else:
                        self.context.unwide_hue_march()
            except requests.exceptions.RequestException as e:
                print("Network timeout error")
                pass
        
        return (state_requested, request_args)