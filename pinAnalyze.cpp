#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <map>
#include <getopt.h>

//using namespace std;

#include "pin.H"
#include "instlib.H"

int Count_Insns = 0;
int Count_Branches = 0;
int Count_CndDirJMP = 0;
int Count_CndDirJMP_Targ = 0;
int Count_CndDirJMP_Fall = 0;
int Count_UncDirJMP = 0;
int Count_UncIndJMP = 0;
int Count_DirCALL = 0;
int Count_IndCALL = 0;
int Count_RET = 0;

bool verbose = true;

/* Inserted on every instruction. */

void Insn_Default(){
    Count_Insns++;
}

void Insn_CndDirJMP(bool taken){
    Count_CndDirJMP++;

    if (taken) Count_CndDirJMP_Targ++;
    else Count_CndDirJMP_Fall++;

    Count_Insns++;
}

void Insn_UncDirJMP(){
    Count_UncDirJMP++;
    Count_Insns++;
}

void Insn_UncIndJMP(){
    Count_UncIndJMP++;
    Count_Insns++;
}

void Insn_DirCALL(){
    Count_DirCALL++;
    Count_Insns++;
}

void Insn_IndCALL(){
    Count_IndCALL++;
    Count_Insns++;
}

void Insn_RET(){
    Count_RET++;
    Count_Insns++;
}


/* Inserted on every insn. */

void Instruction(INS ins, void *v)
{
    /* Insert a call to docount before every instruction, no arguments are passed. */

    /*
     * BOOL LEVEL_CORE::INS_IsBranch (INS ins)
     * BOOL LEVEL_CORE::INS_IsDirectBranch (INS ins)
     * BOOL LEVEL_CORE::INS_IsDirectCall (INS ins)
     * BOOL LEVEL_CORE::INS_IsCall (INS ins)
     * BOOL LEVEL_CORE::INS_IsRet (INS ins)
     * BOOL LEVEL_CORE::INS_IsControlFlow (INS ins)
     * BOOL LEVEL_CORE::INS_HasFallThrough (   INS     ins )   
    */

    if (INS_IsControlFlow(ins)) {

        if (INS_IsBranch(ins)) {

            /* Conditional jump; only direct on x86. */

            if (INS_HasFallThrough(ins)) {
                if (INS_IsDirectControlFlow(ins))
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)Insn_CndDirJMP, IARG_BRANCH_TAKEN, IARG_END);
            }

            /* Unconditional jumps; can be direct/indirect. */

            else {
                if (INS_IsDirectControlFlow(ins))
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)Insn_UncDirJMP, IARG_END);
                else   
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)Insn_UncIndJMP, IARG_END);  
            }
        }

        else {

            /* Calls; can be direct/indirect. */

            if (INS_IsCall(ins)) {
                if (INS_IsDirectControlFlow(ins))
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)Insn_DirCALL, IARG_END);
                else
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)Insn_IndCALL, IARG_END);
            }

            /* Returns; always indirect. */

            if (INS_IsRet(ins)) {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)Insn_RET, IARG_END);
            }            
        }
    } 

    else   
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)Insn_Default, IARG_END);
}


/* Inserted on program exit. */

void Fini(int code, void *v)
{
    Count_Branches = Count_CndDirJMP_Targ + Count_CndDirJMP_Fall 
                + Count_UncDirJMP + Count_UncIndJMP 
                + Count_DirCALL + Count_IndCALL + Count_RET;

    cout << endl;
    cout << "## --- Control-flow Stats ---" << endl;
    cout << "## Total Branches   : " << Count_Branches << endl;
    cout << "##  Cnd. Dir. Jumps : " << Count_CndDirJMP << endl;
    cout << "##       Targ Edges : " << Count_CndDirJMP_Targ << endl;
    cout << "##       Fall Edges : " << Count_CndDirJMP_Fall << endl;
    cout << "##  Unc. Dir. Jumps : " << Count_UncDirJMP << endl;
    cout << "##  Unc. Ind. Jumps : " << Count_UncIndJMP << endl;
    cout << "##  Dir. Calls      : " << Count_DirCALL << endl;
    cout << "##  Ind. Calls      : " << Count_IndCALL << endl;
    cout << "##  Returns         : " << Count_RET << endl;
    cout << endl;

    if (verbose){
        cout << "Branches, CndDirJMP, CndDirJMPTarg, CndDirJMPFall, UncDirJMP, UncIndJMP, DirCALL, IndCALL, RET" << endl;
        cout << Count_Branches << ", ";
        cout << Count_CndDirJMP << ", ";
        cout << Count_CndDirJMP_Targ << ", ";
        cout << Count_CndDirJMP_Fall << ", ";
        cout << Count_UncDirJMP << ", ";
        cout << Count_UncIndJMP << ", ";
        cout << Count_DirCALL << ", ";
        cout << Count_IndCALL << ", ";
        cout << Count_RET << endl;
        cout << endl;
    }
}


int main(int argc, char **argv)
{
    // TODO - add function blacklisting like in Zipr/dyntool.
    // Want to basically capture everything PAST forkserver.

    /* Initialize pin. */

    if (PIN_Init(argc, argv) == 1)
    {
        cout << "Error PIN_Init!" << endl;
        return 1;
    }

    /* Register instruction callback. */

    INS_AddInstrumentFunction(Instruction, 0);

    /* Register application exit callback. */

    PIN_AddFiniFunction(Fini, 0);

    /* Start the program, never returns. */

    PIN_StartProgram();

    return 0;
}
