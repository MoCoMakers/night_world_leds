class LEDRow():
    def __init__(self, length, offset, direction="forward"):
        self.length = length
        self.offset = offset
        self.direction = direction
    
    def get_column_led_index(self, column):
        if self.direction =="forward":
            index = self.offset+column
        elif self.direction=="backward":
            index = self.offset+self.length - column
        else:
            raise Exception("Unsupported direction")
        return index