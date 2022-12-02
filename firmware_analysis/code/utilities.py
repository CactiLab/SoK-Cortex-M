import os
from claripy import true
import mysql.connector
from tqdm import tqdm

root_dir = '/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/firmware'

cnx = mysql.connector.connect(user='andy', password='00544',
                              host='127.0.0.1',
                              database='cortexm_firmware')
cursor = cnx.cursor(buffered=True)

def get_fw_list():
    cursor.execute("SELECT hash, fw_path, tz_bxns, tz_blxns FROM merged_fw")
    result = cursor.fetchall()
    # print(result)
    return result
        

def update_fw_size(fw_hash, fw_path):
    full_path = os.path.join(root_dir, fw_path)
    fw_bytes = os.path.getsize(full_path)
    sql = 'UPDATE merged_fw SET fw_bytes = %s WHERE hash = %s'
    val = (fw_bytes, fw_hash)
    cursor.execute(sql, val)
    cnx.commit()


def search_bytes(fw_hash, fw_path, tz_bxns, tz_blxns):
    full_path = os.path.join(root_dir, fw_path)

    with open(full_path, 'rb') as f:
        fw_bytes = f.read()
        if fw_bytes.find(b'\x7f\xe9\x7f\xe9') and tz_bxns == 1:
            print(fw_path)
            return True
    

fw_list = get_fw_list()
result_list = []
for fw in tqdm(fw_list):
    if search_bytes(fw[0], fw[1], fw[2], fw[3]):
        result_list.append(fw[1])

print(len(result_list))