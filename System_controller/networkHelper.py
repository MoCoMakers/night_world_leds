import requests
import multiprocessing
import time
import threading


def do_bulk_esp32_action_2(function_handle, ip_list, has_returns=False, return_dict=None, has_queue=False, queue = False):

    if not has_returns:
        with multiprocessing.Pool(processes=len(ip_list)) as pool:
                pool.map(function_handle, ip_list)
    else:
        with multiprocessing.Pool(processes=len(ip_list)) as pool:
            pool.starmap(function_handle, [(ip, return_dict) for ip in ip_list])

def do_bulk_esp32_action(function_handle, ip_list, has_returns=False, return_dict=None, has_queue=False, queue=None):
    threads = []
    
    for ip in ip_list:
        if not has_returns:
            thread = threading.Thread(target=function_handle, args=(ip,))
        else:
            if has_queue:
                thread = threading.Thread(target=function_handle, args=(ip, return_dict, queue))
            else:
                thread = threading.Thread(target=function_handle, args=(ip, return_dict))
        threads.append(thread)
        thread.start()
    
    for thread in threads:
        thread.join()

def do_simple_get(url, ip, timeout=(1, 1)):
    try:
        response = requests.get(url, timeout=timeout) # 1 second to connect, 10 seconds to
        time_wait_ms = 100
        time.sleep(time_wait_ms / 1000.0)
        return response
    except requests.exceptions.RequestException as e:
        print("Failed connection on "+ip)