import requests
import multiprocessing


def do_bulk_esp32_action(function_handle, ip_list):
    with multiprocessing.Pool(processes=len(ip_list)) as pool:
            pool.map(function_handle, ip_list)

def do_simple_get(url, ip):
    try:
        response = requests.get(url)
        # time.sleep(time_wait_ms / 1000.0)
    except:
        print("Failed connection on "+ip)