// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscommon.h"
#include "syscall.h"
#include "thread.h"
#include "ksyscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void k_exec(int arg_vaddr[]) {
    unsigned char* name;
    int len, k_argc, k_opt;
    unsigned char** k_argv;
    int errno;

    // first check the location of filename is valid
    errno = fname_addrck((char*)arg_vaddr[0]);
    //printf("errno = %d\n",errno);
    if (errno <= 0) {    
        ASSERT(false);
    }
    len = ustrlen(arg_vaddr[0]);
    name = new unsigned char[len];
    u2kmemcpy(name,arg_vaddr[0],len);
    printf("name = %s\n",name);
    if (fexist_ck(name) == -1) {
        ASSERT(false);
    }
    k_argc = arg_vaddr[1];
    printf("argc value=%d\n",k_argc);
    k_opt = arg_vaddr[3];
    printf("opt value=%d\n",k_opt);

    k_argv = new unsigned char*[k_argc];
    u2kmatrixcpy(k_argv,arg_vaddr[2],k_argc);
    for (int i = 0; i< k_argc; i++) {
        printf("argv[%d] value = %s\n",i,k_argv[i]);
    }
}

void
StartUserProcess(int argv)
{
    // TODO: use test file path, should pass in filename
    char * filename = "../test/thread_yield";
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return;
    }
    space = new AddrSpace();
    space->Initialize(executable); // use locks
    currentThread->space = space;

    delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"
}



void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    int buffer, size, id;
    //int arg_vaddr[4] = {0};

    Thread * t;
    if (which == SyscallException) {
        switch (type) {
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;

            case SC_Exit:
                printf("exit\n");
                int sp;
                for (int i=0; i<5; i++) {
                machine->ReadMem(machine->ReadRegister(StackReg)+4*i,4,&sp);
                printf("%d %d\n",i,sp);
                }
                PushPC();
                //interrupt->Halt();
                break;

            case SC_Exec:
                printf("exec\n"); // notice: current implementation is still buggy
                t = new Thread("exec new thread");
                t -> Fork(StartUserProcess, 1); // use fake arg
                machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
                machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4);
                machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg) + 8);
                currentThread->Yield(); 
                break;

            case SC_Fork:
                printf("fork\n");
                interrupt->Halt();
                break;

            case SC_Yield:
                printf("yield\n");
                currentThread->Yield();
                PushPC();
                //machine->Run();
                
                //interrupt->Halt();
                break;
            
            case SC_Read:
                buffer = machine->ReadRegister(4);
                size = machine->ReadRegister(5);
                id = machine->ReadRegister(6);
                kread((char*)buffer, size, id);
                PushPC();
                //buffer = machine->ReadRegister(4);
                //k_read(buffer, 0, 0);
                /*buffer = machine->ReadRegister(4);
                arg2_vaddr= machine->ReadRegister(5);
                printf("arg1 addr=%d\n",buffer);
                printf("arg2 addr=%d\n",arg2_vaddr);
                k_read(buffer, arg2_vaddr, 0);*/
                break;

            case SC_Write:
                printf("write\n");
                buffer = machine->ReadRegister(4);
                size = machine->ReadRegister(5);
                id = machine->ReadRegister(6);

                //TODO: Checker!

                kwrite((char*)buffer, size, id);
                PushPC();
                break;

            default:
                printf("Unexpected user mode exception %d %d\n", which, type);
                interrupt->Halt();
                ASSERT(FALSE);
        }
    }
    else if (which == NumExceptionTypes) {
        //TODO handle arithematic exception
        printf("number exception\n");
        ASSERT(false);
    }
    else if (which == IllegalInstrException) {
        //TODO handle illegal instruction
        printf("illegal instruction exception\n");
        ASSERT(false);
    } 
    else if (which == OverflowException) {
        //TODO handle interger overflow
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(false);
    }
    else if (which == AddressErrorException) {
        //TODO handle address error 
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(false);
    } else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}
