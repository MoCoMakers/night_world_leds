from multiprocessing import Process, Manager
from arcade_monitor import joystick_reader
import time
from flask import Flask, jsonify
import atexit

app = Flask(__name__)
manager = Manager()
shared_data = manager.dict()
shared_data["button_value"] = None
shared_data["direction"] = "middle"

@app.route('/', methods=['GET'])
def index():
    return "<h1>Arcade System Running</h1>"

@app.route('/getButtonStatus', methods=['GET'])
def get_button_status():
    
    return jsonify({
        "button_value": shared_data.get("button_value", None),
        "direction": shared_data.get("direction", "middle")
    })

if __name__ == '__main__':
    p = Process(target=joystick_reader, args=(shared_data,))
    p.start()

    def cleanup():
        p.terminate()
        p.join()
    atexit.register(cleanup)

    app.run(host='0.0.0.0', debug=True, use_reloader=True)