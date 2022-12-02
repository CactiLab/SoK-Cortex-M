import os
import re
import mysql.connector
from tqdm import tqdm

root_dir = '/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/monolithic/D_FITBIT/results'

cnx = mysql.connector.connect(user='andy', password='00544',
                              host='127.0.0.1',
                              database='cortexm_firmware')

cursor = cnx.cursor(buffered=True)


def lookup_hash(filename):
    sql = "SELECT hash FROM firmware WHERE bin_name = %s"
    val = (filename,)
    cursor.execute(sql, val)
    result = cursor.fetchone()[0]
    return result


for subdir, _, files in os.walk(root_dir):
    for filename in tqdm(files):
        disass_path = os.path.join(subdir, filename)
        
        if filename.endswith('.txt'):
            os.rename(disass_path, disass_path[:-4])
            disass_path = disass_path[:-4]
            filename = filename[:-4]
        
        if '@' not in filename:
            fw_hash = lookup_hash(filename)
            os.rename(disass_path, disass_path + '@' + fw_hash)
            