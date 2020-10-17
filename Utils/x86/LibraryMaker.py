
import argparse
import pathlib

parser = argparse.ArgumentParser()

parser.add_argument('exec', type=str, help="Executable file to turn into a library")
parser.add_argument('-i', dest='include', type=str, required=True, help="Path to include files")
parser.add_argument('-o', dest='output', type=str, required=True, help="Output .c file")
parser.add_argument('-n', dest='libname', type=str, required=True, help="library name")
parser.add_argument('-t', dest='table', type=str, required=True, help="symbol offset table file name")
parser.add_argument('-s', dest='symbolList', type=str, default=None, help="Optional list of symbols that should be found; otherwise inferred from header files")

args = parser.parse_args()

def getSymbolTable(file):

    def getInt(offset, size):
        return int.from_bytes(file[offset:offset + size], 'little')

    def getString(offset):
        startOffset = offset

        while file[offset] != 0:
            offset += 1
        
        return file[startOffset:offset].decode('utf-8')

    sectionHeaderOffset = getInt(0x20, 4)
    nSections = getInt(0x30, 2)

    stringHeaderTableLocPtr = sectionHeaderOffset + 0x28 * getInt(0x32, 2) + 0x10
    stringHeaderTableLoc = getInt(stringHeaderTableLocPtr, 4)

    stringTableLoc = None
    symbolTableLoc = None
    nSymbols = None

    for i in range(nSections):
        nameLoc = stringHeaderTableLoc + getInt(sectionHeaderOffset, 4)

        flags = getInt(sectionHeaderOffset + 0x4, 4)
        location = getInt(sectionHeaderOffset + 0x10, 4)

        name = getString(nameLoc)
        if name == '.strtab':
            stringTableLoc = location
        elif name == '.symtab':
            symbolTableLoc = location
            nSymbols = getInt(sectionHeaderOffset + 0x14, 4) // 16

        sectionHeaderOffset += 0x28

    symTable = {}
    
    for symbolLoc in [symbolTableLoc + 16 * i for i in range(nSymbols)]:
        name = getString(stringTableLoc + getInt(symbolLoc, 4))
        location = getInt(symbolLoc + 4, 4)

        #if location != 0:
        symTable[name] = location
    
    if args.symbolList is not None:
        symbolList = []
        with open(args.symbolList, 'r') as symListFile:
            for line in symListFile:
                symbolList.append(line.strip())
    else:
        symbolList = symTable.keys()
    
    headerOffset = 0
    headerSize = len(symbolList) * 4
    for symbolName in symbolList:
        location = symTable[symbolName]
        location += headerSize
        symTable[symbolName] = (headerOffset, location)
        headerOffset += 4
    
    return symTable

def processHeader(path, rootPath, dynamicSrc):
    outputFileName = str(dynamicSrc / path.relative_to(rootPath)).replace('.h', '.c')
    outputFile = open(outputFileName, 'w')

    includePath = str(path.relative_to(rootPath))
    outputFile.write(f"#include \"{includePath}\"\n\n")
    outputFile.write(f"extern void * _{libraryName}_LoadLocation;\n\n")

    with open(path, 'r') as headerFile:
        statements = headerFile.read().replace('\n','').replace('\r','').split(';')

        for statement in statements:
            # rule out non-function statments (preprocessor, struct declations)
            if (
                statement.startswith('#')
                or not statement.endswith(')')
                or '{' in statement
                or '(' not in statement
                or statement[0].isspace()
            ):

                continue

            # get the function name
            openPIndex = statement.index('(')
            nameStart = openPIndex
            while nameStart > 1 and statement[nameStart - 1].isspace():
                nameStart -= 1
            
            while nameStart > 1:
                c = statement[nameStart - 1]
                if not (c.isalnum() or c == '_'):
                    break
                nameStart -= 1
            
            # parse each argument name and type
            name = statement[nameStart:openPIndex].strip()

            if name not in symbolTable:
                continue

            argsRaw = statement[openPIndex + 1:-1].split(',')
            argNames = []
            argTypes = []
            for argRaw in argsRaw:
                argRaw = argRaw.strip()
                if argRaw == '':
                    continue
                
                argNameStart = len(argRaw)
                while argNameStart > 1:
                    c = argRaw[argNameStart - 1] 
                    if not (c.isalnum() or c == '_'):
                        break
                    argNameStart -= 1

                argName = argRaw[argNameStart:].strip()
                argType = argRaw[:argNameStart].strip()

                argNames.append(argName)
                argTypes.append(argType)
            
            returnType = statement[:nameStart].strip()

            if isinstance(symbolTable[name], int):
                print(f"WARNING: symbol '{name}' not requested, skipping")
                continue

            offset = symbolTable[name][0]
            funcAssign = f"\t{returnType} (*func)({','.join(argTypes)}) = _{libraryName}_LoadLocation + *(unsigned int*)(_{libraryName}_LoadLocation + {offset});\n"
            if returnType == "void":
                funcCall = f"\tfunc({','.join(argNames)});\n"
            else:
                funcCall = f"\treturn func({','.join(argNames)});\n"

            outputFile.write(statement + " {\n")
            outputFile.write(funcAssign)
            outputFile.write(funcCall)
            outputFile.write("}\n\n")
    
    outputFile.close()


def processHeaders(path, rootPath, dynPath):
    if isinstance(path, str):
        path = pathlib.Path(path)
    if isinstance(dynPath, str):
        dynPath = pathlib.Path(dynPath)
    if isinstance(rootPath, str):
        rootPath = pathlib.Path(rootPath)
    
    if path.is_dir():
        dynDir = dynPath / path.relative_to(rootPath)
        if not dynDir.exists():
            dynDir.mkdir()

        for subPath in path.iterdir():
            processHeaders(subPath, rootPath, dynPath)
    
    elif str(path).endswith('.h'):
        processHeader(path, rootPath, dynPath)

def generateSymbleOffsetTable(path, symbols):

    if args.symbolList is not None:
        symbolList = []
        with open(args.symbolList, 'r') as symListFile:
            for line in symListFile:
                symbolList.append(line.strip())
    else:
        symbolList = symbols.keys()

    table = [-1] * len(symbolList)

    for symbol in symbolList:
        headerOffset = symbols[symbol][0] // 4
        symbolLocation = symbols[symbol][1]
        print(symbolLocation, symbol)

        table[headerOffset] = symbolLocation
    
    tableBytes = bytes()

    for loc in table:
        tableBytes += loc.to_bytes(4, 'little')

    with open(path, 'wb') as offsetFile:
        offsetFile.write(tableBytes)
        

with open(args.exec, 'rb') as execFile:
    symbolTable = getSymbolTable(execFile.read())

libraryName = args.libname
processHeaders(args.include, args.include, args.output)
generateSymbleOffsetTable(args.table, symbolTable)