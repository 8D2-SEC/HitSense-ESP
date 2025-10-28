import pymem
import pymem.process
from flask import Flask, jsonify
import threading
import time
PROCESS_NAME = "GTA5.exe"
HEALTH_OFFSET = "aca_pones_tu_offset"
#recuerda se saca con cheatengine
app = Flask(__name__)
current_health = 100
pm = pymem.Pymem(PROCESS_NAME)
module_base = pymem.process.module_from_name(pm.process_handle, PROCESS_NAME).lpBaseOfDll
health_address = module_base + HEALTH_OFFSET
def update_health():
    global current_health
    while True:
        try:
            current_health = pm.read_int(health_address)
        except:
            current_health = 0
        time.sleep(0.05)
@app.route('/health', methods=['GET'])
def get_health():
    return jsonify({"health": current_health})
if __name__ == "__main__":
    t = threading.Thread(target=update_health)
    t.daemon = True
    t.start()
    app.run(host='0.0.0.0', port=5000)#pense en hacer un mqtt pero me dio pereza jaja
