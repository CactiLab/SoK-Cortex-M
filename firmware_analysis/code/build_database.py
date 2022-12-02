import os
import json
import hashlib

import mysql.connector

DATASET_DIR = 'Nordic_DFU'
BASE_FILE = 'base.txt'

cnx = mysql.connector.connect(user='andy', password='00544',
                              host='127.0.0.1',
                              database='cortexm_firmware')
cursor = cnx.cursor(buffered=True)


def db_get_linked_fw(fw_hash):
    cursor.execute("SELECT linked_fw FROM firmware WHERE hash = %s", (fw_hash,))
    result = cursor.fetchone()[0]
    if result is None:
        return set()
    return set(result.split(','))

    
def db_update_row(fw_dict):
    if 'linked_fw' in fw_dict:
        fw_set = db_get_linked_fw(fw_dict['fw_hash'])
        fw_set.add(fw_dict['linked_fw'])
        linked_fw = ','.join(fw_set)
        sql = "UPDATE firmware SET bin_name = %s, vendor = %s, fw_type = %s, fw_release = %s, base_addr= %s, linked_fw = %s, fw_path = %s, top_dir = %s, sub_dir = %s WHERE hash = %s"
        val = (fw_dict['fw_name'], 'Nordic', fw_dict['fw_type'], fw_dict['release'], fw_dict['base_addr'], linked_fw, fw_dict['fw_path'], fw_dict['top_dir'], fw_dict['sub_dir'], fw_dict['fw_hash'])
    else:
        sql = "UPDATE firmware SET bin_name = %s, vendor = %s, fw_type = %s, fw_release = %s, base_addr= %s, fw_path = %s, top_dir = %s, sub_dir = %s WHERE hash = %s"
        val = (fw_dict['fw_name'], 'Nordic', fw_dict['fw_type'], fw_dict['release'], fw_dict['base_addr'], fw_dict['fw_path'], fw_dict['top_dir'], fw_dict['sub_dir'], fw_dict['fw_hash'])
    cursor.execute(sql, val)
    cnx.commit()


def db_insert_row(fw_dict):
    if 'linked_fw' in fw_dict:
        sql = "INSERT INTO firmware (hash, bin_name, vendor, fw_type, fw_release, base_addr, linked_fw, fw_path, top_dir, sub_dir) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)"
        val = (fw_dict['fw_hash'], fw_dict['fw_name'], 'Nordic', fw_dict['fw_type'], fw_dict['release'], fw_dict['base_addr'], fw_dict['linked_fw'], fw_dict['fw_path'], fw_dict['top_dir'], fw_dict['sub_dir'])
    else:
        sql = "INSERT INTO firmware (hash, bin_name, vendor, fw_type, fw_release, base_addr, fw_path, top_dir, sub_dir) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)"
        val = (fw_dict['fw_hash'], fw_dict['fw_name'], 'Nordic', fw_dict['fw_type'], fw_dict['release'], fw_dict['base_addr'], fw_dict['fw_path'], fw_dict['top_dir'], fw_dict['sub_dir'])
    cursor.execute(sql, val)
    cnx.commit()


def check_vendor(fw_hash):
    cursor.execute("SELECT vendor FROM firmware WHERE hash = %s", (fw_hash,))
    result = cursor.fetchone()[0]
    if result is 'Nordic':
        return False
    return True


def update_database(fw_list):
    for fw in fw_list:
        sql = "SELECT count(*) FROM firmware WHERE hash = %s"
        val = (fw['fw_hash'],)
        cursor.execute(sql, val)
        if cursor.fetchone()[0]:
            # hash exists in database
            if 'linked_fw' in fw or check_vendor(fw['fw_hash']):
                db_update_row(fw)
        else:
            db_insert_row(fw)


def get_md5(file):
    with open(file, 'rb') as f:
        file_hash = hashlib.md5()
        while chunk := f.read(8192):
            file_hash.update(chunk)
    hash_str = file_hash.hexdigest()
    return hash_str


def get_base_addr(hash):
    with open(BASE_FILE, 'r') as f:
        for line in f:
            if line == '\n': continue
            hash_val = line.split('\t')[1]
            base_addr = line.split('\t')[2]
            if hash_val == hash:
                return base_addr

# exit()

for top_dir in os.listdir(DATASET_DIR):
    print(top_dir)
    path = os.path.join(DATASET_DIR, top_dir)
    for sub_dir in os.listdir(path):
        print(sub_dir)
        path = os.path.join(DATASET_DIR, top_dir, sub_dir)
        for release in os.listdir(path):
            print(release)
            manifest_data = {}
            manifest_path = os.path.join(path, release, 'manifest.json')
            with open(manifest_path) as f:
                manifest_data = json.load(f)
            
            fw_parts = 0
            fws = []
            for fw_type in manifest_data['manifest']:
                if fw_type == 'dfu_version': continue
                dic = {}
                dic['fw_type'] = fw_type
                dic['fw_name'] = manifest_data['manifest'][fw_type]['bin_file']
                dic['fw_path'] = os.path.join(path, release, dic['fw_name'])
                dic['fw_hash'] = get_md5(dic['fw_path'])
                dic['base_addr'] = get_base_addr(dic['fw_hash'])
                dic['top_dir'] = top_dir
                dic['sub_dir'] = sub_dir
                dic['release'] = release
                fws.append(dic)
                fw_parts += 1
            
            if fw_parts == 2:
                fws[0]['linked_fw'] = fws[1]['fw_hash']
                fws[1]['linked_fw'] = fws[0]['fw_hash']
            elif fw_parts > 2:
                print('More than 2 fws in manifest {manifest_path}!')
                exit(-1)
            
            print(fws)
            update_database(fws)
                