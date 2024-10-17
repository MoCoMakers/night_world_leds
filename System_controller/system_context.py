class SystemContext():
    def __init__(self, global_rgb):
        self.global_rgb = global_rgb
    
    def update_global_rgb(self, global_rgb):
        self.global_rgb = global_rgb
    
    def update_meta_state(self, state_requested, request_args):
        pass
