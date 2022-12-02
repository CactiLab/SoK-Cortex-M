import datetime
import os
import json
import hashlib
import subprocess
from tqdm import tqdm
import fileinput
import re

import mysql.connector

ANALYZER='/home/andy/libs/ghidra_10.2.2_PUBLIC/support/analyzeHeadless'
SCRIPT_PATH='/home/andy/libs/ghidra_scripts'
RUN_PATH = '/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/monolithic/export_ascii.py'
BASE_PATH = '/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/firmware'
RESULT_PATH = '/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/new'

cnx = mysql.connector.connect(user='root', password='00544',
                              host='127.0.0.1',
                              database='cortexm_firmware')
cursor = cnx.cursor(buffered=True)

def get_fw_list():
    cursor.execute("SELECT hash, base_addr, fw_path FROM merged_fw")
    result = cursor.fetchall()
    # print(result)
    return result

# remove all lines matching the regex in a file
def remove_invalid_lines(file):
    for line in fileinput.input(file, inplace = True):
        if not re.search(r'\w\w(\s*)\?\?(\s*)\w\wh', line):
            print(line, end="")

def run_analyzer(fw_hash, fw_path, base_addr):
    fw_path_full = os.path.join(BASE_PATH, fw_path)
    basename = os.path.basename(fw_path) + '@' + fw_hash
    print(basename)
    if os.path.exists(os.path.join(RESULT_PATH, basename)):
        print('Already analyzed!\n')
        return
    result_path = os.path.join(RESULT_PATH, basename)
    if ' ' not in fw_path_full and ('(' in fw_path_full or ')' in fw_path_full):
        fw_path_full = '\'' + fw_path_full + '\''
    if ' ' not in result_path and ('(' in result_path or ')' in result_path):
        result_path = '\'' + result_path + '\''
    
    os.system('rm -rf /tmp/demo.bin*')
    analyzer = subprocess.Popen([ANALYZER, '/tmp', 'demo.bin', '-scriptPath', SCRIPT_PATH, '-preScript', RUN_PATH, fw_path_full, str(base_addr), result_path, '-noanalysis', '-deleteProject'],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                universal_newlines=True)
    print("[Command Line] {}\n".format(subprocess.list2cmdline(analyzer.args)))
    return_code = 0
    end_signal = False
    while True:
        output = analyzer.stdout.readline()
        print(output.strip())
        if output.strip() == '*** end of the log ***':
            end_signal = True
            
        return_code = analyzer.poll()
        if return_code is not None:
            print('RETURN CODE', return_code)
            # Process has finished, read rest of the output 
            for output in analyzer.stdout.readlines():
                print(output.strip())
            break
    
    for error in analyzer.stderr.readlines():
        if not error.lower().startswith('openjdk'):
            print(error.strip())
    
    if return_code != 0 or not end_signal:
        print('Error during analysis! Exiting...')
        exit(-1)
    
    remove_invalid_lines(result_path)
    # print date and time
    print(datetime.datetime.now())
    print()


fw_list = get_fw_list()

for fw in tqdm(fw_list):
    run_analyzer(fw[0], fw[2], fw[1])
    
