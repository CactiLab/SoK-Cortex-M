import binascii
import os
import re
import fileinput

from ghidra.program.flatapi import FlatProgramAPI
from ghidra.program.model.lang import LanguageID
from ghidra.program.model.data import PointerDataType
from ghidra.program.model.listing import Program
from ghidra.program.model.symbol import SourceType
from ghidra.program.util import DefaultLanguageService, GhidraProgramUtilities
from ghidra.program.disassemble import Disassembler
from ghidra.app.plugin.core.analysis import AutoAnalysisManager
from ghidra.app.util.importer import LcsHintLoadSpecChooser, SingleLoaderFilter, MessageLog, AutoImporter
from ghidra.app.util.exporter import AsciiExporter
from ghidra.app.util.bin import RandomAccessByteProvider
from ghidra.app.util.opinion import LoaderService, BinaryLoader
from ghidra.util.exception import CancelledException
from ghidra.util.task import TaskMonitor, TimeoutTaskMonitor
from java.io import File
from java.lang import Object
from java.util.concurrent import TimeUnit
    

def get_pointer_at_addr(listing, addr):
    addrData = listing.getDefinedDataAt(addr)
    if addrData == None:
        listing.clearCodeUnits(addr, addr.add(3), False)
        # api.clearListing(addr, addr.add(3))  # This also works
        addrData = listing.createData(addr, PointerDataType.dataType, 4)
    return addrData


def define_other_functions_in_vt(api, program, vt_offset, prog_ranges, current_addr):
    listing = program.getListing()
    fm = program.getFunctionManager()
    # check the end address of the vector table
    zero_count = 0
    end_addr = api.getFunctionAfter(current_addr).getEntryPoint()
    
    # print(current_addr)
    # print(end_addr)
    while current_addr < end_addr:
        addrData = get_pointer_at_addr(listing, current_addr)
        if addrData.getDataType().getDisplayName() == 'string':
            zero_count = 0
        elif addrData.getValue().toString() == "00000000":
            zero_count += 1
        elif not any(lower <= int(addrData.getValue().toString(), 16) <= upper for (lower, upper) in prog_ranges):
            zero_count = 5
            # !! important
            listing.clearCodeUnits(current_addr, current_addr.add(3), False)
            api.disassemble(current_addr)
            api.analyzeChanges(program)
            symbol_str = 'LAB_' + current_addr.toString()
            func = api.createFunction(current_addr, symbol_str)
            # func = fm.getFunctionAt(current_addr)
            if func is None:
                print('WARN_USER  Failed to create at: ' + symbol_str)
        else:
            zero_count = 0
        if zero_count == 5:
            end_addr = current_addr
        current_addr = current_addr.add(4)
    current_addr = program.getImageBase().add(vt_offset)
    print('INFO  Vector table end: 0x' + end_addr.toString())
    while current_addr < end_addr:
        addrData = get_pointer_at_addr(listing, current_addr)
        # print(current_addr)
        # print(addrData.getValue())
        current_addr = current_addr.add(4)
        if addrData.getDataType().getDisplayName() == 'string': continue
        if addrData.getValue().toString() == "00000000": continue
        if not any(lower <= int(addrData.getValue().toString(), 16) <= upper for (lower, upper) in prog_ranges): continue
        
        api.disassemble(addrData.getValue().add(-1))
        api.analyzeChanges(program)
        
        symbol_str = 'FUNC_VT_' + addrData.getValue().add(-1).toString()
        
        func = fm.getFunctionAt(addrData.getValue().add(-1))
        if func == None:
            func = api.createFunction(addrData.getValue().add(-1), symbol_str)
            if func == None:
                func_name = 'FUNC_VT_' + addrData.getValue().toString()
                func = api.createFunction(addrData.getValue(), func_name)
                if func == None:
                    func = fm.getFunctionAt(addrData.getValue().add(-3))
                if func == None:
                    data = listing.getDefinedDataAt(addrData.getValue())
                    if data is not None:
                        end_addr = current_addr
                        continue
                if func == None:
                    print("WARN_USER  Cannot resolve function at 0x" + addrData.getValue().add(-1).toString())
                    continue
        elif 'FUNC' not in func.getName():
            func.setName(symbol_str, SourceType.USER_DEFINED)


def disass_by_symbol(program, vt_off):
    vectorTableFuncs = ['Reset', 'NMI', 'HardFault', 'MemManage', 'BusFault', 'UsageFault', 'SVCall', 'PendSV', 'SysTick', 'IRQ']
    api = FlatProgramAPI(program)
    
    # get the ranges
    prog_ranges = []
    for blk in program.getMemory().getBlocks():
        start = blk.getStart().getOffset()
        end = blk.getEnd().getOffset()
        prog_ranges.append((start, end))
    
    print(prog_ranges)
    
    fm = program.getFunctionManager()
    listing = program.getListing()
    symbolTable = program.getSymbolTable()
    symbols = symbolTable.getAllSymbols(True)
    vt_offset = vt_off
    addrAnalyzed = []
    print('INFO  Analyze all.....')
    # api.analyzeAll(program)
    # print('Second Analysis Done!')
    
    if vt_offset != -1:
        print('INFO  Vector table offset: ' + hex(vt_offset))
        
    for symbol in symbols:
        addr = symbol.getAddress()
        symbolStr = symbol.toString()
        symbol_addr = None
        
        # print(addr, symbolStr)
        # get the vector table offset
        if symbolStr == 'MasterStackPointer' and api.getFirstFunction() is not None and vt_offset == -1:
            # recursively find the MSP address
            current_addr = addr
            end_addr = api.getFirstFunction().getEntryPoint()
            while current_addr < end_addr:
                pointer = get_pointer_at_addr(listing, current_addr)
                # print(pointer.getValue())
                try:
                    _ = pointer.getValue().toString()
                except:
                    current_addr = current_addr.add(4)
                    continue
                if int('0x20000000', 16) <= int(pointer.getValue().toString(), 16) <= int('0x3FFFFFFF', 16):
                    # double check if this is the MSP address
                    next_pointer = get_pointer_at_addr(listing, current_addr.add(4))
                    if any(lower <= int(next_pointer.getValue().toString(), 16) <= upper for (lower, upper) in prog_ranges):
                        vt_offset = current_addr.getOffset() - program.getImageBase().getOffset()
                        print('INFO  Vector table offset: ' + hex(vt_offset))
                        break
                current_addr = current_addr.add(4)
            if vt_offset == -1:
                print('WARN_USER  Vector table not found!')
        
        if addr.toString().startswith('ffff') or 'PTR' in symbolStr:
            continue
        
        if '_' in symbolStr:
            try:
                symbol_addr = int(symbolStr.split('_')[-1][0:8], 16)
            except:
                print('WARN_USER  Invalid symbol address: ' + symbolStr)
                continue
            
            if not any(lower <= symbol_addr <= upper for (lower, upper) in prog_ranges): continue
            if getAddress(program, hex(symbol_addr)) in addrAnalyzed: continue
        
        # print(addr, symbolStr) # debug
        if symbolStr in vectorTableFuncs:
            if vt_offset == -1: continue
            addrData = get_pointer_at_addr(listing, addr.add(vt_offset))
            
            if addrData.getValue().toString() == "00000000" or \
            not any(lower <= int(addrData.getValue().toString(), 16) <= upper for (lower, upper) in prog_ranges):
                if symbolStr == vectorTableFuncs[-1]:
                    define_other_functions_in_vt(api, program, vt_offset, prog_ranges, addr.add(vt_offset))
                continue
            
            try:
                api.disassemble(addrData.getValue().add(-1))
            except Exception as e:
                print(e)
                listing.clearCodeUnits(addr.add(vt_offset), addr.add(vt_offset).add(3), False)
                addrData = listing.createData(addr.add(vt_offset), PointerDataType.dataType, 4)
                
                api.disassemble(addrData.getValue().add(-1))
            api.analyzeChanges(program)
            
            func_name = 'FUNC_' + symbolStr + '_' + addrData.getValue().add(-1).toString()
            func = api.createFunction(addrData.getValue().add(-1), func_name)
            if func == None:
                func = api.createFunction(addrData.getValue(), func_name)
            if func == None:                
                func = fm.getFunctionAt(addrData.getValue().add(-1))
                if func is not None:
                    func.setName(func_name, SourceType.USER_DEFINED)
                else:
                    # listing.clearCodeUnits(addrData.getValue().add(-1), addrData.getValue().add(2), True)
                    api.disassemble(addrData.getValue().add(-1))
                    api.analyzeChanges(program)
                    func = api.createFunction(addrData.getValue().add(-1), func_name)
            
            if symbolStr == vectorTableFuncs[-1]:                    
                define_other_functions_in_vt(api, program, vt_offset, prog_ranges, addr.add(vt_offset))
            
        elif 'LAB' in symbolStr:           
            func = api.createFunction(getAddress(program, hex(symbol_addr)), symbolStr)

            if func == None:
                print('WARN_USER  Fail to create a function! Skip: ' + getAddress(program, hex(symbol_addr)).toString())
                continue
            addrSet = func.getBody()
            codeUnits = listing.getCodeUnits(addrSet, True) # true means 'forward'
            for codeUnit in codeUnits:
                if codeUnit.getAddress() in addrAnalyzed: break
                code_str = codeUnit.toString()
                if code_str.split(' ')[0] is 'b' or 'bx':
                    api.disassemble(codeUnit.getAddress().add(2))
                    api.analyzeChanges(program)
                addrAnalyzed.append(codeUnit.getAddress())
        elif 'FUN' in symbolStr and not symbolStr.endswith('+1'):
            # func = api.createFunction(getAddress(program, hex(symbol_addr)), symbolStr)
            func = fm.getFunctionAt(getAddress(program, hex(symbol_addr)))
            if func == None:
                print('WARN_USER  Fail to create a function! Skip: ' + getAddress(program, hex(symbol_addr)).toString())
                continue
            addrSet = func.getBody()
            codeUnits = listing.getCodeUnits(addrSet, True) # true means 'forward'
            for codeUnit in codeUnits:
                if codeUnit.getAddress() in addrAnalyzed: break
                code_str = codeUnit.toString()
                if code_str.split(' ')[0] is 'b' or 'bx':
                    api.disassemble(codeUnit.getAddress().add(2))
                    api.analyzeChanges(program)
                addrAnalyzed.append(codeUnit.getAddress())
                
    listing = program.getListing()
    func = api.getFirstFunction()
    if func == None:
        return program
    entryPoint = func.getEntryPoint()
    instructions = listing.getInstructions(entryPoint, True)

    for instruction in instructions:
        addr = instruction.getAddress()
        oper = instruction.getMnemonicString()
        if (oper == 'b' or oper == 'bx' or oper == 'pop'):
            if api.disassemble(addr.add(2)):
                api.analyzeChanges(program)
    return program


def auto_importer_test(target, langID):
    language = DefaultLanguageService.getLanguageService().getLanguage(LanguageID(str(langID)))
    default_cspec = language.getDefaultCompilerSpec()
    program = importFileAsBinary(File(target), language, default_cspec)
    return program

def auto_importer(target, langID):
    lang = DefaultLanguageService.getLanguageService().getLanguage(LanguageID(str(langID)))
    load_spec_chooser = LcsHintLoadSpecChooser(lang, None)
    loader_filter = SingleLoaderFilter(BinaryLoader)
    byte_provider = RandomAccessByteProvider(File(target))
    loader_map = LoaderService.getSupportedLoadSpecs(byte_provider, loader_filter)
    load_spec = load_spec_chooser.choose(loader_map)
    lang_cspec_pair = load_spec.getLanguageCompilerSpec()
    cspec = lang.getCompilerSpecByID(lang_cspec_pair.compilerSpecID)
    log = MessageLog()
    program = AutoImporter.importByUsingSpecificLoaderClassAndLcs(File(target), getProjectRootFolder(),
                                                                  BinaryLoader, None, lang, cspec, Object(), log,
                                                                  TaskMonitor.DUMMY)
    byte_provider.close()
    return program

def auto_analyze(program):
    print('INFO  Processor used : ' + program.getLanguage().getProcessor().toString())
    txId = program.startTransaction("Analysis")
    mgr = AutoAnalysisManager.getAnalysisManager(program)
    anOpts = program.getOptions(Program.ANALYSIS_PROPERTIES)
    anOpts.setBoolean('Decompiler Parameter ID', True)
    anOpts.setBoolean('Scalar Operand References', True)
    mgr.initializeOptions()
    mgr.reAnalyzeAll(None)
    print("INFO  Analyzing...")
    mgr.startAnalysis(TimeoutTaskMonitor.timeoutIn(30, TimeUnit.SECONDS))
    GhidraProgramUtilities.setAnalyzedFlag(program, True)
    
    return program


def getAddress(program, offset):
    return program.getAddressFactory().getDefaultAddressSpace().getAddress(offset)


# def remove_invalid_lines(file):
#     for line in fileinput.input(file, inplace = True):
#         if not re.search(r'00(\s*)\?\?(\s*)\d\dh', line):
#             print(line, end='')
# remove all lines matching the regex in a file


def main():
    if isRunningHeadless() and len(getScriptArgs()) == 3:
        target_file = getScriptArgs()[0]
        base = getScriptArgs()[1]
        output_file = getScriptArgs()[2]
        vt_offset = -1
    elif isRunningHeadless() and len(getScriptArgs()) == 4:
        target_file = getScriptArgs()[0]
        base = getScriptArgs()[1]
        output_file = getScriptArgs()[2]
        vt_offset = int(getScriptArgs()[3], 16)
    else:
        print('Please run this file in Ghidra headless mode and provide 3 args!')
        return

    print('INFO  Running test on {}'.format(target_file))
    
    base_addr = hex(int(base))
    print('INFO  Base: ' + base_addr)
    program = auto_importer_test(target_file, "ARM:LE:32:Cortex")
    program.setImageBase(getAddress(program, base_addr), False)
    
    program = auto_analyze(program)
    program = disass_by_symbol(program, vt_offset)
    
    exporter = AsciiExporter()
    exporter.export(File(output_file), program, None, TaskMonitor.DUMMY)
    
    # remove_invalid_lines(output_file)
    
    for consumer in program.getConsumerList():
        program.release(consumer)
    closeProgram(program)
    
    print('*** end of the log ***')

main()
