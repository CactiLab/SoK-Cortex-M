USE cortexm_firmware;
CREATE TABLE IF NOT EXISTS firmware 
(
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	hash VARCHAR(256) NOT NULL UNIQUE,
	bin_name VARCHAR(256) NOT NULL, 
    vendor VARCHAR(128),
    model VARCHAR(128),
    base_addr INT DEFAULT 0,
	ctr_read_only BOOLEAN DEFAULT 0,
	ctr_set_to_0x0 BOOLEAN DEFAULT 0,  # set the default value
	ctr_set_to_0x2 BOOLEAN DEFAULT 0,  # change stack to PSP
	ctr_set_to_0x4 BOOLEAN DEFAULT 0,  # FP related
	ctr_bit_clear BOOLEAN DEFAULT 0,   # keep the second or third bit and clear others
	ctr_set_to_0x3 BOOLEAN DEFAULT 0,  # privilege-related
	handler_SVCall BOOLEAN DEFAULT 0,  # check if these handler are dummy ones
	handler_PendSV BOOLEAN DEFAULT 0,
	handler_SysTick BOOLEAN DEFAULT 0,
	write_psp BOOLEAN DEFAULT 0,
	read_psp BOOLEAN DEFAULT 0,
	write_msp BOOLEAN DEFAULT 0,
	read_msp BOOLEAN DEFAULT 0,
	use_mpu BOOLEAN DEFAULT 0,
    use_smpu BOOLEAN DEFAULT 0,
	use_multi_stacks BOOLEAN DEFAULT 0,
	have_context_switch BOOLEAN DEFAULT 0,
	have_priv_seperation BOOLEAN DEFAULT 0,
    notes VARCHAR(256)
	# TODO: field for DSB/ISB instructions
);
-- SHOW TABLES;
-- DESCRIBE firmware_with_control;
ALTER TABLE firmware
ADD COLUMN ctr_store BOOLEAN DEFAULT 0 AFTER ctr_read_only;
ALTER TABLE cortexm_firmware.firmware RENAME COLUMN write_psp TO psp_store;
ALTER TABLE cortexm_firmware.firmware RENAME COLUMN read_psp TO psp_read;
ALTER TABLE cortexm_firmware.firmware RENAME COLUMN write_msp TO msp_store;
ALTER TABLE cortexm_firmware.firmware RENAME COLUMN read_msp TO msp_read;
ALTER TABLE cortexm_firmware.firmware RENAME COLUMN ctr_read_only TO ctr_read;
ALTER TABLE cortexm_firmware.firmware RENAME COLUMN handler_SVCall TO use_svc;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN app VARCHAR(128) AFTER vendor;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN category VARCHAR(128) AFTER app;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN is_unique BOOLEAN DEFAULT 0 AFTER vendor;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN readback BOOLEAN DEFAULT 0 AFTER handler_SysTick;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN readback_2 BOOLEAN DEFAULT 0 AFTER readback;
ALTER TABLE cortexm_firmware.firmware DROP COLUMN readback_3;
ALTER TABLE cortexm_firmware.firmware RENAME COLUMN readback TO readback_1;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN msplim BOOLEAN DEFAULT 0 AFTER msp_read;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN psplim BOOLEAN DEFAULT 0 AFTER msplim;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN elf BOOLEAN DEFAULT 0 AFTER model;

ALTER TABLE cortexm_firmware.firmware DROP COLUMN elf;
ALTER TABLE cortexm_firmware.firmware MODIFY model VARCHAR(128) AFTER notes;
ALTER TABLE cortexm_firmware.firmware MODIFY category VARCHAR(128) AFTER notes;
ALTER TABLE cortexm_firmware.firmware MODIFY app VARCHAR(128) AFTER notes;
ALTER TABLE cortexm_firmware.firmware MODIFY is_unique BOOLEAN DEFAULT 0 AFTER notes;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN fw_type VARCHAR(128) AFTER vendor;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN fw_release VARCHAR(128) AFTER fw_type;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN linked_fw TEXT AFTER base_addr;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN fw_path TEXT AFTER notes;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN top_dir VARCHAR(128) AFTER fw_path;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN sub_dir VARCHAR(128) AFTER top_dir;
ALTER TABLE cortexm_firmware.firmware RENAME COLUMN use_multi_stacks TO stack_canary;
ALTER TABLE cortexm_firmware.firmware ADD COLUMN isb_ctr BOOLEAN DEFAULT 0 AFTER ctr_read;

ALTER TABLE cortexm_firmware.merged_fw DROP COLUMN have_context_switch;
ALTER TABLE cortexm_firmware.merged_fw DROP COLUMN have_priv_seperation;
ALTER TABLE cortexm_firmware.merged_fw ADD COLUMN os_freertos BOOLEAN DEFAULT 0 AFTER stack_canary;
ALTER TABLE cortexm_firmware.merged_fw ADD COLUMN os_cmsis BOOLEAN DEFAULT 0 AFTER stack_canary;
ALTER TABLE cortexm_firmware.merged_fw ADD COLUMN os_zephyr BOOLEAN DEFAULT 0 AFTER stack_canary;
ALTER TABLE cortexm_firmware.merged_fw ADD COLUMN os_mbed BOOLEAN DEFAULT 0 AFTER stack_canary;
ALTER TABLE cortexm_firmware.merged_fw ADD COLUMN os_keilrtx BOOLEAN DEFAULT 0 AFTER stack_canary;
ALTER TABLE cortexm_firmware.merged_fw ADD COLUMN stack_ovf_chk BOOLEAN DEFAULT 0 AFTER stack_canary;
ALTER TABLE cortexm_firmware.merged_fw ADD PRIMARY KEY (id);
ALTER TABLE cortexm_firmware.merged_fw ADD COLUMN fw_bytes INT DEFAULT 0 AFTER model;
ALTER TABLE cortexm_firmware.merged_fw ADD COLUMN tz_bxns BOOLEAN DEFAULT 0 AFTER stack_canary
ALTER TABLE cortexm_firmware.merged_fw ADD COLUMN tz_blxns BOOLEAN DEFAULT 0 AFTER stack_canary