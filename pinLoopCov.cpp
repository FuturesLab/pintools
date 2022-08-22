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
ADDRINT textStart, 
        textEnd;
BBL prevBbl;
INS prevInsn;
ADDRINT prevAddr;



set <string> skipFunctions;

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
    skipFunctions.insert("__x86.get_pc_thunk.bx");
}

void Trace(TRACE trace, void *v){

    BBL bbl = TRACE_BblHead(trace);
    
    for (; BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
        ADDRINT addr = BBL_Address(bbl);

        if (addr < textStart || addr > textEnd)
            continue;

        //cout << hex << BBL_Address(prevBbl) << " " << BBL_Address(bbl) << endl;

        INS insn = BBL_InsTail(bbl);

        /*
        if ((addr <= prevAddr) && INS_IsDirectBranch(insn)){
            cout << INS_IsDirectBranch(insn) << " ";
            cout << INS_IsRet(insn) << " ";
            cout << prevAddr << "," << addr << " ";
            cout << INS_Disassemble(prevInsn) << endl;
        }
        */

        cout << addr << " " << INS_Disassemble(insn) << endl;

        //prevInsn = insn;
        //prevAddr = INS_Address(insn);



        //INS ins = BBL_InsTail(bbl);

        //cout << "[LoopCov]    insn: " << hex << BBL_Address(bbl) << " " << INS_Disassemble(ins) << endl;


    }
}

/* Retrieve .TEXT section boundaries. */

void getTextBounds(void *ptr){
    IMG img = APP_ImgHead();

    for(SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)){
        if (SEC_IsExecutable(sec) && SEC_Name(sec) == ".text"){
            ADDRINT start = SEC_Address(sec);
            ADDRINT size  = SEC_Size(sec);

            if (start != 0){
                textStart = start;
                textEnd = start + size;
            }

            if (getenv("LOOPCOV_DEBUG")){
                cout << "[LoopCov] section: " << SEC_Name(sec) << endl;
                cout << "[LoopCov]   start: " << hex << textStart << endl;
                cout << "[LoopCov]     end: " << hex << textEnd << endl;
            }  

            return;        
        }
    }
}

int main(int argc, char **argv)
{
    // Reset output log, if present. 

    ofstream outfile;
    outfile.open("/tmp/pinLoopCov.log", ios::trunc);
    outfile.close();

    // Initialize pin. 

    if (PIN_Init(argc, argv) == 1)
    {
        cout << "Error PIN_Init!" << endl;
        return 1;
    }

    initSkipFunctions();
    PIN_InitSymbols();


    PIN_AddApplicationStartFunction(getTextBounds,0);

    TRACE_AddInstrumentFunction(Trace, 0);

    /* Start the program, never returns. */

    PIN_StartProgram();

    return 0;
}
