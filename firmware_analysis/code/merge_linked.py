import mysql.connector


cnx = mysql.connector.connect(user='root', password='00544',
                              host='127.0.0.1',
                              database='cortexm_firmware')
cursor = cnx.cursor(buffered=True)

col_list = ['stack_canary', 'ctr_set_to_0x1', 'ctr_set_to_0x2', 'ctr_set_to_0x3', 'ctr_store', 'ctr_read',
            'psp_store', 'psp_read', 'msp_store', 'msp_read', 'use_mpu', 'use_smpu', 'use_svc',
            'readback_1', 'readback_2', 'msplim', 'psplim']

def get_hash_list():
    cursor.execute("SELECT hash FROM merged_fw WHERE BINARY vendor = 'Nordic' AND fw_type != 'application'")
    result = cursor.fetchall()
    return [x[0] for x in result]


def get_elememt(fw_hash, col):
    sql = 'SELECT ' + col + ' FROM merged_fw WHERE hash = %s'
    val = (fw_hash,)
    cursor.execute(sql, val)
    result = cursor.fetchone()[0]
    return result


def update_element(fw_hash, col):
    sql = 'UPDATE merged_fw SET ' + col + ' = 1 WHERE hash = %s'
    cursor.execute(sql, (fw_hash,))
    cnx.commit()


# exit()
hash_list = get_hash_list()
for hash in hash_list:
    cursor.execute("SELECT linked_fw FROM merged_fw WHERE hash = %s", (hash,))
    result = cursor.fetchone()[0]
    if result is None:
        print('No linked fw for hash: ' + hash)
        continue
    linked_fw = result.split(',')
    
    for col in col_list:
        result = get_elememt(hash, col)
        if result != 0 and result is not None:
            for fw_hash in linked_fw:
                update_element(fw_hash, col)
                print('Update ' + col + ' for hash: ' + fw_hash)
    