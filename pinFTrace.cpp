#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <map>
#include <getopt.h>
#include <vector>
#include <set>
#include <string>
#include <stdlib.h>
#include <climits>
#include <getopt.h>

using namespace std;

#include "pin.H"
#include "instlib.H"

bool verbose = true;
set <string> skipFunctions;
RTN prevRtn;
INS prevIns;
string prevInsMnem;
string prevRtnName;
bool found_main = false;

void initSkipFunctions(){
    skipFunctions.insert(".plt");
    skipFunctions.insert("__libc_start_main@plt");
    skipFunctions.insert(".init");
    skipFunctions.insert("init");
    skipFunctions.insert("_init");
    skipFunctions.insert("start");
    skipFunctions.insert("_start");
    skipFunctions.insert("fini");
    skipFunctions.insert("_fini");
    skipFunctions.insert("register_tm_clones");
    skipFunctions.insert("deregister_tm_clones");
    skipFunctions.insert("frame_dummy");
    skipFunctions.insert("__do_global_ctors_aux");
    skipFunctions.insert("__do_global_dtors_aux");
    skipFunctions.insert("__libc_csu_init");
    skipFunctions.insert("__libc_csu_fini");
    skipFunctions.insert("__libc_csu_fini");
    skipFunctions.insert("__libc_start_main");
    skipFunctions.insert("__gmon_start__");
    skipFunctions.insert("__cxa_atexit");
    skipFunctions.insert("__cxa_finalize");
    skipFunctions.insert("__assert_fail");
    skipFunctions.insert("free");
    skipFunctions.insert("fnmatch");
    skipFunctions.insert("readlinkat");
    skipFunctions.insert("malloc");
    skipFunctions.insert("calloc");
    skipFunctions.insert("realloc");
    skipFunctions.insert(".text");
    skipFunctions.insert("__x86.get_pc_thunk.bx");
}


/* Helper function to omit functions we don't care about
 * (e.g., LIBC functions or compiler thunks). */

bool RTN_IsBlacklisted(RTN rtn){
    string name = RTN_Name(rtn);
    long addr = RTN_Address(rtn);

    if (skipFunctions.find(name) != skipFunctions.end() \
        || name.find("__libc") != string::npos \
        || name.find("__afl") != string::npos \
        || name.find("@plt") != string::npos) {
        return true;
    }

    else if (addr < 0 || addr > 130000000000000)
        return true;

    else
        return false;
}

string INS_GetTransferType(INS ins){
    if (INS_IsCall(ins)){
        if (INS_IsDirectCall(ins))
            return "TRANSFER_DIRECT_CALL";
        else
            return "TRANSFER_INDIRECT_CALL";
    }

    else if (INS_IsBranch(ins)){
        if (INS_IsDirectBranch(ins))
            return "TRANSFER_DIRECT_BRANCH";
        else
            return "TRANSFER_INDIRECT_BRANCH";
    }

    else if (INS_IsRet(ins)){
        return "TRANSFER_RETURN";
    }

    else 
        return "TRANSFER_UNKNOWN";
}

/* Records each function encountered. Omits 
 * functions not within the main binary
 * based on pre-specified addr boundaries. 
 * Resulting log will be /tmp/ftrace.log. */

void Trace(INS ins, void *v){
    ofstream outfile;
    outfile.open("/tmp/pinFTrace.log", ios::app); 

    if (INS_Valid(ins) && RTN_Valid(INS_Rtn(ins)) && !INS_Valid(INS_Next(ins))){
        if (RTN_Name(INS_Rtn(ins)) == "main") 
            found_main = true;
        if ((INS_GetTransferType(ins)) == "TRANSFER_UNKNOWN") 
            return;
        if (!found_main) 
            return;

        cout << hex << INS_Address(ins) << ", ";
        cout << RTN_Name(INS_Rtn(ins)) << ", ";
        cout << INS_Mnemonic(ins) << ", ";
        cout << INS_GetTransferType(ins) << endl;
    }

    outfile.close();
}

int main(int argc, char **argv)
{
    /* Reset output log, if present. */

    ofstream outfile;
    outfile.open("/tmp/pinFTrace.log", ios::trunc);
    outfile.close();

    /* Initialize pin. */

    if (PIN_Init(argc, argv) == 1)
    {
        cout << "Error PIN_Init!" << endl;
        return 1;
    }

    initSkipFunctions();

    PIN_InitSymbols();

    INS_AddInstrumentFunction(Trace, 0);

    /* Start the program, never returns. */

    PIN_StartProgram();

    return 0;
}