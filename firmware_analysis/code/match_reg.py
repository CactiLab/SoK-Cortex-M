import os
import re
import mysql.connector
import regex_pattern as pt
from tqdm import tqdm

root_dir = '/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/new'

cnx = mysql.connector.connect(user='root', password='00544',
                              host='127.0.0.1',
                              database='cortexm_firmware')

mycursor = cnx.cursor()

def check_stack_canary(fw_path):
    with open(fw_path) as f:
        datafile = f.read()
        if re.search(pt.reg_armcc_pro, datafile) and re.search(pt.reg_armcc_epi, datafile):
            return True
        elif re.search(pt.reg_armclang_pro, datafile) and re.search(pt.reg_armclang_epi, datafile):
            return True
        elif re.search(pt.reg_gcc_pro, datafile) and re.search(pt.reg_gcc_epi, datafile):
            return True
        elif re.search(pt.reg_exp_pro, datafile) and re.search(pt.reg_exp_epi, datafile):
            if len(re.findall(pt.reg_exp_pro, datafile)) > 1 and len(re.findall(pt.reg_exp_epi, datafile)) > 1:
                return True
        else:
            return False

result_dic = {}

result_dic['stack_canary'] = []
result_dic['ctr_set_to_0x1'] = []
result_dic['ctr_set_to_0x2'] = []
result_dic['ctr_set_to_0x3'] = []
result_dic['ctr_store'] = []
result_dic['ctr_read'] = []
result_dic['isb_ctr'] = []
result_dic['psp_store'] = []
result_dic['psp_read'] = []
result_dic['msp_store'] = []
result_dic['msp_read'] = []
result_dic['use_mpu'] = []
result_dic['use_smpu'] = []
result_dic['use_svc'] = []
result_dic['readback_1'] = []  # nRF51
result_dic['readback_2'] = []  # nRF52 & nRF53
result_dic['msplim'] = []
result_dic['psplim'] = []
result_dic['tz_bxns'] = []
result_dic['tz_blxns'] = []

for subdir, _, files in os.walk(root_dir):
    for filename in tqdm(files):
        fw_path = os.path.join(subdir, filename)
        fw_hash = os.path.basename(filename).split('@')[1]
        
        
        if check_stack_canary(fw_path):
            result_dic['stack_canary'].append((fw_hash,))
        
        #continue
        with open(fw_path) as f:
            datafile = f.read()
            if re.search(pt.reg_tz_bxns, datafile):
                # print(fw_path)
                result_dic['tz_bxns'].append((fw_hash,))
            
            if re.search(pt.reg_tz_blxns, datafile):
                result_dic['tz_blxns'].append((fw_hash,))
            
            # continue
            if re.search(pt.reg_ctr_0x1, datafile):
                result_dic['ctr_set_to_0x1'].append((fw_hash,))
                
            if re.search(pt.reg_ctr_0x2_1, datafile) or re.search(pt.reg_ctr_0x2_2, datafile) or re.search(pt.reg_ctr_0x2_3, datafile):
                result_dic['ctr_set_to_0x2'].append((fw_hash,))
            
            if re.search(pt.reg_ctr_0x3, datafile):
                result_dic['ctr_set_to_0x3'].append((fw_hash,))
            
            if re.search(pt.reg_ctr_store, datafile):
                result_dic['ctr_store'].append((fw_hash,))
            
            if re.search(pt.reg_ctr_read, datafile):
                result_dic['ctr_read'].append((fw_hash,))
            
            if re.search(pt.reg_isb_ctr, datafile):
                result_dic['isb_ctr'].append((fw_hash,))
            
            if re.search(pt.reg_psp_store, datafile):
                result_dic['psp_store'].append((fw_hash,))
            
            if re.search(pt.reg_psp_read, datafile):
                result_dic['psp_read'].append((fw_hash,))
                
            if re.search(pt.reg_msp_store, datafile):
                result_dic['msp_store'].append((fw_hash,))
                
            if re.search(pt.reg_msp_read, datafile):
                result_dic['msp_read'].append((fw_hash,))
            
            if re.search(pt.reg_mpu_ctrl, datafile):
                result_dic['use_mpu'].append((fw_hash,))
            
            if re.search(pt.reg_smpu, datafile):
                result_dic['use_smpu'].append((fw_hash,))
            
            if re.search(pt.reg_svc, datafile):
                result_dic['use_svc'].append((fw_hash,))
            
            if re.search(pt.reg_readback_1, datafile):
                result_dic['readback_1'].append((fw_hash,))
            
            if re.search(pt.reg_readback_2, datafile):
                result_dic['readback_2'].append((fw_hash,))
            
            if re.search(pt.reg_readback_3, datafile):
                result_dic['readback_2'].append((fw_hash,))
            
            if re.search(pt.reg_stack_limit_msp, datafile):
                result_dic['msplim'].append((fw_hash,))
            
            if re.search(pt.reg_stack_limit_psp, datafile):
                result_dic['psplim'].append((fw_hash,))

print('tz_bxns:', len(result_dic['tz_bxns']))
# exit()
for col, hash_list in result_dic.items():
    print(col, len(hash_list))
    if len(hash_list) == 0:
        continue
    sql = 'UPDATE merged_fw SET ' + col + ' = 1 WHERE (hash=%s)'
    mycursor.executemany(sql, hash_list)
    cnx.commit()
    print(mycursor.rowcount, "{} records updated.".format(col))