from ghidra.program.model.listing import Program
from ghidra.program.model.address import AddressSet
from ghidra.program.util import GhidraProgramUtilities
from ghidra.app.plugin.core.analysis import AutoAnalysisManager
from ghidra.app.util.exporter import AsciiExporter
from ghidra.util.task import TaskMonitor, TimeoutTaskMonitor
from java.io import File
from java.util.concurrent import TimeUnit

def auto_analyze(program):
    print('INFO  Processor used : ' + program.getLanguage().getProcessor().toString())
    _ = program.startTransaction("Analysis")
    mgr = AutoAnalysisManager.getAnalysisManager(program)
    an_opts = program.getOptions(Program.ANALYSIS_PROPERTIES)
    an_opts.setBoolean('Decompiler Parameter ID', True)
    an_opts.setBoolean('ELF Scalar Operand References', True)
    mgr.initializeOptions()
    mgr.reAnalyzeAll(None)
    print("INFO  Analyzing...")
    mgr.startAnalysis(TimeoutTaskMonitor.timeoutIn(30, TimeUnit.SECONDS))
    GhidraProgramUtilities.setAnalyzedFlag(program, True)
    
    return program


def main():
    if isRunningHeadless() and len(getScriptArgs()) == 2:
        target_file = getScriptArgs()[0]
        output_file = getScriptArgs()[1]
    else:
        print(len(getScriptArgs()))
        print('Please run this file in Ghidra headless mode and provide 2 args!')
        return
    
    print('INFO  Running on {}'.format(target_file))
    
    program = importFile(File(target_file))
    program = auto_analyze(program)
    
    memory = program.getMemory()
    text_block = memory.getBlock('.text')
    if text_block is None:
        text_block = memory.getBlock('text')

    addr_set = None
    if text_block is not None:
        addr_set = AddressSet(text_block.getStart(), text_block.getEnd())
    
    exporter = AsciiExporter()
    exporter.export(File(output_file), program, addr_set, TaskMonitor.DUMMY)
    
    for consumer in program.getConsumerList():
        program.release(consumer)
    closeProgram(program)
    
    print('*** end of the log ***')
    
main()