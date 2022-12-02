SELECT * FROM cortexm_firmware.merged_fw;

SELECT 
	id, hash, bin_name, vendor, fw_type, base_addr, tz_bxns, tz_blxns, fw_path
FROM
    cortexm_firmware.merged_fw;
    
select count(*) from cortexm_firmware.merged_fw where os_mbed and id < 1142 and is_unique;
SELECT hash, bin_name, vendor FROM cortexm_firmware.merged_fw WHERE fw_path IS NULL;
select 
	id, hash, bin_name, vendor, fw_type, stack_ovf_chk, os_freertos, os_cmsis, os_zephyr, os_mbed, os_keilrtx, fw_path
from
	cortexm_firmware.merged_fw
WHERE
	os_freertos or os_cmsis or os_zephyr or os_mbed or os_keilrtx;
    
select 
	#avg(fw_bytes) 'Average Size'
    min(fw_bytes)
from
	cortexm_firmware.merged_fw
where
	id < 1142 or (id > 1142 and fw_type != 'softdevice_bootloader' and fw_type != 'softdevice' and fw_type != 'bootloader' and fw_type != 'applicationOtherCpu');
    
SET @row_index := -1;

select 
	#avg(fw_bytes) 'Average Size'
    count(*)
from
	cortexm_firmware.merged_fw
where
	id < 1142 or (id > 1142 and fw_type != 'softdevice_bootloader' and fw_type != 'softdevice' and fw_type != 'bootloader' and fw_type != 'applicationOtherCpu');


select 
    count(*)
from
	cortexm_firmware.merged_fw
where
	id < 1142 and vendor = 'fitbit' and ctr_store;


SELECT AVG(subq.fw_bytes) as median_value
FROM (
    SELECT @row_index:=@row_index + 1 AS row_index, fw_bytes
    FROM cortexm_firmware.merged_fw
    ORDER BY fw_bytes
  ) AS subq
  WHERE subq.row_index 
  IN (FLOOR(@row_index / 2) , CEIL(@row_index / 2));
