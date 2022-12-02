import re
import os
import subprocess

import mysql.connector

cnx = mysql.connector.connect(user='andy', password='00544',
                              host='127.0.0.1',
                              database='cortexm_firmware')
cursor = cnx.cursor(buffered=True)

root_dir = '/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/firmware'

def get_path_list():
    cursor.execute("SELECT hash, fw_path FROM merged_fw")
    result = cursor.fetchall()
    return result


def search_string(fw_hash, fw_path):
    # if fw_path.startswith('Nordic_DFU'):
    #     return
    fw_path_full = os.path.join(root_dir, fw_path)

    analyzer = subprocess.Popen(['strings', fw_path_full], stdout=subprocess.PIPE, universal_newlines=True)
    found = False
    while True:
        line = analyzer.stdout.readline()
        if not line:
            break
        if re.search(r"freertos", line, re.IGNORECASE):
            print(fw_path)
            print(line)
            found = True
            
    return found


def update_db(fw_hash, col):
    cursor.execute("UPDATE merged_fw SET {} = 1 WHERE hash = '{}'".format(col, fw_hash))
    cnx.commit()


count = 0
total = 0
fw_list = get_path_list()
for fw in fw_list:
    total += 1
    if search_string(fw[0], fw[1]):
        # update_db(fw[0], 'stack_ovf_chk')
        count += 1
print('Total:', total)
print('Found:', count)