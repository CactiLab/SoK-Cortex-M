reg_armcc_pro='push( *){(.*)}(.*(\n)){1,3}(.*?)ldr( *)(\w*),( *)\[(.*)\](.*?)\n(.*?)ldr( *)(\w*),( *)\[(.*),( *)#(0x0|0)\](.*?)\n(.*?)str( *)(\w*),( *)\[(\w){2},( *)#(0x([\w]*)|(\d*))\]'
reg_armclang_pro='push( *){(.*)}(.*?)\n(.*?)sub( *?)[a-z]{2},( *?)#(0x([\w]*)|(\d*))(.*(\n)){1,3}(.*?)movw( *?)r\d,( *?)#(0x([\w]*)|(\d*))(.*?)\n(.*?)movt( *?)r\d,( *?)#(0x([\w]*)|(\d*))(.*?)\n(.*?)ldr( *)(\w*),( *)\[r\d,( *)#(0x([\w]*)|(\d*))\]'
reg_gcc_pro='push( *){(.*)}(.*?)\n(.*?)sub( *?)[a-z]{2},( *?)#(0x([\w]*)|(\d*))(.*(\n)){1,3}(.*?)ldr( *?)r\d,( *?)\[(.*)\](.*?)\n(.*?)ldr( *?)r\d,( *?)\[r\d,( *)#(0x0|0)\](.*?)\n(.*?)str'
reg_exp_pro='push( *){(.*)}(.*?)\n(.*?)ldr( *)r\d,( *)\[(.*)\](.*(\n)){1,2}(.*?)ldr( *)r\d,( *)\[r\d,(.*)\](.*(\n)){1,4}(.*?)sub(.*)(\w){2},(.*?)(.*(\n)){1,2}(.*?)str( *)r\d,( *)\[(\w){2},(.*)\]'

reg_armcc_epi='ldr(.*),( *)\[[a-z]{2},( *)#(0x([\w]*)|(\d*))\](.*?)\n(.*?)ldr(.*),( *)\[(.*)\](.*?)\n(.*?)ldr(.*),( *)\[(.*),( *)#(0x0|0)\](.*?)\n(.*?)cmp(.*),(.*)(.*(\n)){1,2}(.*?)beq(.*?)\n(.*?)bl'
reg_armclang_epi='ldr(.*),( *)\[r\d,( *)#(0x([\w]*)|(\d*))\](.*?)\n(.*?)ldr(.*),( *)\[([a-z]{2}),( *)#(.*)\](.*?)\n(.*?)cmp(.*),(.*)r\d(.*(\n)){1,3}(.*?)i(t*)( *)eq(.*(\n)){1,5}(.*?)bl'
reg_gcc_epi='ldr( *)r\d,( *)\[(.*)](.*?)\n(.*?)ldr(.*),( *)\[r\d,( *)#(0x([\w]*)|(\d*))\](.*?)\n(.*?)ldr( *)r\d,( *)\[r\d,( *)#(.*)\](.*?)\n(.*?)eors(.*),( *)r\d(.*(\n)){1,3}(.*?)beq(.*(\n)){1,3}(.*?)bl'
reg_exp_epi='ldr( *)r\d,( *)\[(\D){2},(.*)](.*?)\n(.*?)ldr( *)r\d,( *)\[r\d.(.*)](.*?)\n(.*?)cmp( *)r\d,( *)r\d(.*(\n)){1,2}(.*?)bne'

test='(.*)ldr( *)r\d,( *)\[[a-z]{2},( *)#(0x([\w]*)|(\d*))\](.*?)\n(.*?)ldr( *)r\d,( *)\[([a-z]{2},( *)#(0x([\w]*)|(\d*))|0x(\w*))\](.*?)\n(.*?)ldr( *)r\d,( *)\[r\d,( *)#(0x([\w]*)|(\d*))\](.*?)\n(.*?)cmp(.*)r\d,(.*)r\d(.*(\n)){1,2}(.*?)beq'

reg_ctr_0x1='mov(.*)r\d,#0x1(.*?)\n(.*?)msr(.*)control,r\d'
reg_ctr_0x2_1='mov(.*)r\d,#0x2(.*?)\n(.*?)msr(.*)control,r\d'
reg_ctr_0x2_2='orr(.*)r\d,r\d,#0x2(.*(\n)){1,2}(.*?)msr(.*)control,r\d'
reg_ctr_0x2_3='mov(.*)r\d,#0x2(.*?)\n(.*?)orr(.*)r\d,r\d(.*?)\n(.*?)msr(.*)control,r\d'
reg_ctr_0x3='mov(.*)r\d,#0x3(.*?)\n(.*?)msr(.*)control,r\d'

reg_ctr_store='msr(.*)control,r\d'
reg_ctr_read='mrs(.*)r\d,control'
reg_isb_ctr='msr(.*)control,r\d(.*(\n)){1,10}(.*?)isb'

reg_psp_store='msr(.*)psp,r\d'
reg_psp_read='mrs(.*)r\d,psp'
reg_msp_store='msr(.*)msp,r\d'
reg_msp_read='mrs(.*)r\d,msp'

reg_mpu_ctrl='e000ed94'
reg_smpu='4000052c'

reg_svc='svc(.*)0x(\w*)'

reg_readback_1='10001004' # nRF51
reg_readback_2='10001208' # nRF52
reg_readback_3='00ff8000' # nRF53

reg_stack_limit_msp='msplim'
reg_stack_limit_psp='psplim'

reg_tz_bxns='bxns'
reg_tz_blxns='blxns'