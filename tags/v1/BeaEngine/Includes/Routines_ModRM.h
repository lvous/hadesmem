/* Copyright 2006-2009, BeatriX
 * File coded by BeatriX
 *
 * This file is part of BeaEngine.
 *
 *    BeaEngine is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    BeaEngine is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with BeaEngine.  If not, see <http://www.gnu.org/licenses/>.
 * =======================================
 *
 * ======================================= */
void __bea_callspec__ MOD_RM(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    GV.DECALAGE_EIP = 0;
    if (!Security(1, pMyDisasm)) return;
    GV.MOD_ = ((*((UInt8*)(UIntPtr) (GV.EIP_+1))) >> 6) & 0x3;
    GV.RM_  = (*((UInt8*)(UIntPtr) (GV.EIP_+1))) & 0x7;
    if (GV.MOD_ == 0) {
        ModRM_0[GV.RM_](pMyArgument, pMyDisasm);
    }
    else if (GV.MOD_ == 1) {
        GV.DECALAGE_EIP++;
        ModRM_1[GV.RM_](pMyArgument, pMyDisasm);
    }
    else if (GV.MOD_ == 2) {
        if (GV.AddressSize >= 32) {
            GV.DECALAGE_EIP += 4;
        }
        else {
            GV.DECALAGE_EIP += 2;
        }
        ModRM_2[GV.RM_](pMyArgument, pMyDisasm);
    }
    else {
        ModRM_3[GV.RM_](pMyArgument, pMyDisasm);
    }

}
/* =======================================
 *
 * ======================================= */
void __bea_callspec__ Reg_Opcode(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0;
    if (!Security(1, pMyDisasm)) return;
    GV.REGOPCODE = ((*((UInt8*)(UIntPtr) (GV.EIP_+1))) >> 3) & 0x7;
    if (GV.REX.R_ == 1) {
        GV.REGOPCODE += 0x8;
    }
    if (GV.MMX_  == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersMMX[GV.REGOPCODE]);
        (*pMyArgument).ArgType = REGISTER_TYPE+MMX_REG+REGS[GV.REGOPCODE];
        (*pMyArgument).ArgSize = 64;
    }
    else if (GV.SEG_ == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSEG[GV.REGOPCODE]);
        (*pMyArgument).ArgType = REGISTER_TYPE+SEGMENT_REG+REGS[GV.REGOPCODE];
        (*pMyArgument).ArgSize = 16;
    }
    else if (GV.CR_ == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersCR[GV.REGOPCODE]);
        (*pMyArgument).ArgType = REGISTER_TYPE+CR_REG+REGS[GV.REGOPCODE];
        (*pMyArgument).ArgSize = 32;
    }
    else if (GV.DR_ == 1) {
        if (GV.SYNTAX_ == ATSyntax) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersDR_AT[GV.REGOPCODE]);
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersDR[GV.REGOPCODE]);
        }
        (*pMyArgument).ArgType = REGISTER_TYPE+DR_REG+REGS[GV.REGOPCODE];
        (*pMyArgument).ArgSize = 32;
    }
    else if (GV.SSE_ == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[GV.REGOPCODE]);
        (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[GV.REGOPCODE];
        (*pMyArgument).ArgSize = 128;
    }
    else if (GV.OperandSize == 8) {
        if (GV.REX.state == 0) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8BitsLegacy[GV.REGOPCODE]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS8BITS[GV.REGOPCODE];
            (*pMyArgument).ArgSize = 8;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[GV.REGOPCODE]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[GV.REGOPCODE];
            (*pMyArgument).ArgSize = 8;
        }
    }
    else if (GV.OperandSize == 16) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[GV.REGOPCODE]);
        (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[GV.REGOPCODE];
        (*pMyArgument).ArgSize = 16;
    }
    else if (GV.OperandSize == 32) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.REGOPCODE]);
        (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[GV.REGOPCODE];
        (*pMyArgument).ArgSize = 32;
    }
    else if (GV.OperandSize == 64) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.REGOPCODE]);
        (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[GV.REGOPCODE];
        (*pMyArgument).ArgSize = 64;
    }
}
/* =======================================
 *          ModRM_0
 * ======================================= */
void __bea_callspec__ Addr_EAX(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0;
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic, "(%");
        i += 2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[8]);
            (*pMyArgument).Memory.BaseRegister = REGS[8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[0]);
            (*pMyArgument).Memory.BaseRegister = REGS[0];
        }
    }
    else if (GV.AddressSize == 32) {

        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[8]);
            (*pMyArgument).Memory.BaseRegister = REGS[8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[0]);
            (*pMyArgument).Memory.BaseRegister = REGS[0];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BXSI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }

}
/* =======================================
 *
 * ======================================= */
void __bea_callspec__ Addr_ECX(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0;
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic, "(%");
        i += 2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[9]);
            (*pMyArgument).Memory.BaseRegister = REGS[1+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[1]);
            (*pMyArgument).Memory.BaseRegister = REGS[1];
        }
    }
    else if (GV.AddressSize == 32) {

        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[9]);
            (*pMyArgument).Memory.BaseRegister = REGS[1+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[1]);
            (*pMyArgument).Memory.BaseRegister = REGS[1];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BXDI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }

}

/* =======================================
 *
 * ======================================= */
void __bea_callspec__ Addr_EDX(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0;
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic, "(%");
        i += 2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[10]);
            (*pMyArgument).Memory.BaseRegister = REGS[2+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[2]);
            (*pMyArgument).Memory.BaseRegister = REGS[2];
        }
    }
    else if (GV.AddressSize == 32) {

        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[10]);
            (*pMyArgument).Memory.BaseRegister = REGS[2+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[2]);
            (*pMyArgument).Memory.BaseRegister = REGS[2];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BPSI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }

}


/* =======================================
 *
 * ======================================= */
void __bea_callspec__ Addr_EBX(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0;
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic, "(%");
        i += 2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[11]);
            (*pMyArgument).Memory.BaseRegister = REGS[3+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[3]);
            (*pMyArgument).Memory.BaseRegister = REGS[3];
        }
    }
    else if (GV.AddressSize == 32) {

        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[11]);
            (*pMyArgument).Memory.BaseRegister = REGS[3+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[3]);
            (*pMyArgument).Memory.BaseRegister = REGS[3];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BPDI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }

}

/* =======================================
 *
 * ======================================= */
void __bea_callspec__ Addr_SIB(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    if (!Security(2, pMyDisasm)) return;
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize >= 32) {
        GV.DECALAGE_EIP++;
        GV.BASE_  = ((UInt8) *((UInt8*) (UIntPtr) (GV.EIP_+2))) & 0x7;
        GV.SCALE_  = (((UInt8) *((UInt8*) (UIntPtr)(GV.EIP_+2))) & 0xc0) >> 6;
        GV.INDEX_  = (((UInt8) *((UInt8*) (UIntPtr)(GV.EIP_+2))) & 0x38) >> 3;
        (void) SIB[GV.SCALE_ ](pMyArgument, 0, pMyDisasm);
    }
    else {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic, Registers16Bits[6]);
        (*pMyArgument).Memory.BaseRegister = REGS[6];
    }
}

/* =======================================
 *
 * ======================================= */
void __bea_callspec__ Addr_disp32(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    Int32 MyNumber;
    UInt64 MyAddress;
    size_t i = 0;
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize >= 32) {
        if (!Security(6, pMyDisasm)) return;
        GV.DECALAGE_EIP+=4;
        MyNumber = *((Int32*)(UIntPtr) (GV.EIP_+2));
        (*pMyArgument).Memory.Displacement = MyNumber;
        if (GV.Architecture == 64) {
            MyNumber += 6;
            MyNumber += GV.NB_PREFIX;
            CalculateRelativeAddress(&MyAddress, (Int64)MyNumber, pMyDisasm);
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%I64X", (Int64)MyAddress);
        }
        else {
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X", (Int64)MyNumber);
        }
    }
    else {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic, Registers16Bits[7]);
        (*pMyArgument).Memory.BaseRegister = REGS[7];
    }
}

/* =======================================
 *
 * ======================================= */
void __bea_callspec__ Addr_ESI(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    long MyNumber;
    size_t i = 0;
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic, "(%");
        i += 2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[14]);
            (*pMyArgument).Memory.BaseRegister = REGS[6+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[6]);
            (*pMyArgument).Memory.BaseRegister = REGS[6];
        }
    }
    else if (GV.AddressSize == 32) {

        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[14]);
            (*pMyArgument).Memory.BaseRegister = REGS[6+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[6]);
            (*pMyArgument).Memory.BaseRegister = REGS[6];
        }
    }
    else {
        GV.DECALAGE_EIP+=2;
        MyNumber = *((UInt16*)(UIntPtr) (GV.EIP_+2));
        (*pMyArgument).Memory.Displacement = MyNumber;
        if (!Security(2, pMyDisasm)) return;
        (void) CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.4X", (Int64)MyNumber);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
}

/* =======================================
 *
 * ======================================= */
void __bea_callspec__ Addr_EDI(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0;
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic, "(%");
        i += 2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[15]);
            (*pMyArgument).Memory.BaseRegister = REGS[7+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[7]);
            (*pMyArgument).Memory.BaseRegister = REGS[7];
        }
    }
    else if (GV.AddressSize == 32) {

        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[15]);
            (*pMyArgument).Memory.BaseRegister = REGS[7+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[7]);
            (*pMyArgument).Memory.BaseRegister = REGS[7];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[3]);
            (*pMyArgument).Memory.BaseRegister = REGS[3];
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
}

/* =======================================
 *          ModRM_1
 * ======================================= */
void __bea_callspec__ Addr_EAX_disp8(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int8*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[8]);
            (*pMyArgument).Memory.BaseRegister = REGS[8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[0]);
            (*pMyArgument).Memory.BaseRegister = REGS[0];
        }
    }
    else if (GV.AddressSize == 32) {

        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[8]);
            (*pMyArgument).Memory.BaseRegister = REGS[8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[0]);
            (*pMyArgument).Memory.BaseRegister = REGS[0];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BXSI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *          ModRM_1
 * ======================================= */
void __bea_callspec__ Addr_ECX_disp8(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = (Int8) *((UInt8*) (UIntPtr)GV.EIP_+2);
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[9]);
            (*pMyArgument).Memory.BaseRegister = REGS[1+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[1]);
            (*pMyArgument).Memory.BaseRegister = REGS[1];
        }
    }
    else if (GV.AddressSize == 32) {

        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[9]);
            (*pMyArgument).Memory.BaseRegister = REGS[1+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[1]);
            (*pMyArgument).Memory.BaseRegister = REGS[1];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BXDI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *          ModRM_1
 * ======================================= */
void __bea_callspec__ Addr_EDX_disp8(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int8*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[10]);
            (*pMyArgument).Memory.BaseRegister = REGS[2+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[2]);
            (*pMyArgument).Memory.BaseRegister = REGS[2];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[10]);
            (*pMyArgument).Memory.BaseRegister = REGS[2+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[2]);
            (*pMyArgument).Memory.BaseRegister = REGS[2];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BPSI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *          ModRM_1
 * ======================================= */
void __bea_callspec__ Addr_EBX_disp8(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int8*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[11]);
            (*pMyArgument).Memory.BaseRegister = REGS[3+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[3]);
            (*pMyArgument).Memory.BaseRegister = REGS[3];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[11]);
            (*pMyArgument).Memory.BaseRegister = REGS[3+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[3]);
            (*pMyArgument).Memory.BaseRegister = REGS[3];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BPDI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *
 * ======================================= */
void __bea_callspec__ Addr_SIB_disp8(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    if (!Security(2, pMyDisasm)) return;
    if (GV.AddressSize >= 32) {
        MyNumber = *((Int8*)(UIntPtr) (GV.EIP_+3));
    }
    else {
        MyNumber = *((Int8*)(UIntPtr) (GV.EIP_+2));
    }
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) MyNumber);
        }
        /*(void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
         *i+=2; */
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize >= 32) {
        GV.DECALAGE_EIP++;
        GV.BASE_  = (*((UInt8*)(UIntPtr) (GV.EIP_+2))) & 0x7;
        GV.SCALE_  = ((*((UInt8*)(UIntPtr) (GV.EIP_+2))) & 0xc0) >> 6;
        GV.INDEX_  = ((*((UInt8*)(UIntPtr) (GV.EIP_+2))) & 0x38) >> 3;
        j = i;
        i += SIB[GV.SCALE_ ](pMyArgument, j, pMyDisasm);
    }
    else {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic, Registers16Bits[6]);
        i += strlen (Registers16Bits[6]);
    }

    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i++;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) MyNumber);
        }
    }

}

/* =======================================
 *          ModRM_1
 * ======================================= */
void __bea_callspec__ Addr_EBP_disp8(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int8*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[13]);
            (*pMyArgument).Memory.BaseRegister = REGS[5+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[5]);
            (*pMyArgument).Memory.BaseRegister = REGS[5];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[13]);
            (*pMyArgument).Memory.BaseRegister = REGS[5+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[5]);
            (*pMyArgument).Memory.BaseRegister = REGS[5];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[7]);
            (*pMyArgument).Memory.BaseRegister = REGS[7];
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *          ModRM_1
 * ======================================= */
void __bea_callspec__ Addr_ESI_disp8(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int8*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[14]);
            (*pMyArgument).Memory.BaseRegister = REGS[6+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[6]);
            (*pMyArgument).Memory.BaseRegister = REGS[6];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[14]);
            (*pMyArgument).Memory.BaseRegister = REGS[6+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[6]);
            (*pMyArgument).Memory.BaseRegister = REGS[6];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[5]);
            (*pMyArgument).Memory.BaseRegister = REGS[5];
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *          ModRM_1
 * ======================================= */
void __bea_callspec__ Addr_EDI_disp8(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int8*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.2X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[15]);
            (*pMyArgument).Memory.BaseRegister = REGS[7+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[7]);
            (*pMyArgument).Memory.BaseRegister = REGS[7];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[15]);
            (*pMyArgument).Memory.BaseRegister = REGS[7+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[7]);
            (*pMyArgument).Memory.BaseRegister = REGS[7];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[3]);
            (*pMyArgument).Memory.BaseRegister = REGS[3];
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.2X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *          ModRM_2
 * ======================================= */
void __bea_callspec__ Addr_EAX_disp32(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int32*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i ++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            j = i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[8]);
            (*pMyArgument).Memory.BaseRegister = REGS[8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[0]);
            (*pMyArgument).Memory.BaseRegister = REGS[0];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[8]);
            (*pMyArgument).Memory.BaseRegister = REGS[8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[0]);
            (*pMyArgument).Memory.BaseRegister = REGS[0];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BXSI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) MyNumber);
        }
    }
}


/* =======================================
 *          ModRM_2
 * ======================================= */
void __bea_callspec__ Addr_ECX_disp32(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int32*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i ++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            j = i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[9]);
            (*pMyArgument).Memory.BaseRegister = REGS[1+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[1]);
            (*pMyArgument).Memory.BaseRegister = REGS[1];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[9]);
            (*pMyArgument).Memory.BaseRegister = REGS[1+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[1]);
            (*pMyArgument).Memory.BaseRegister = REGS[1];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BXDI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *          ModRM_2
 * ======================================= */
void __bea_callspec__ Addr_EDX_disp32(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int32*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i ++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            j = i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[10]);
            (*pMyArgument).Memory.BaseRegister = REGS[2+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[2]);
            (*pMyArgument).Memory.BaseRegister = REGS[2];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[10]);
            (*pMyArgument).Memory.BaseRegister = REGS[2+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[2]);
            (*pMyArgument).Memory.BaseRegister = REGS[2];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BPSI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *          ModRM_2
 * ======================================= */
void __bea_callspec__ Addr_EBX_disp32(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int32*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i ++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            j = i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[11]);
            (*pMyArgument).Memory.BaseRegister = REGS[3+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[3]);
            (*pMyArgument).Memory.BaseRegister = REGS[3];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[11]);
            (*pMyArgument).Memory.BaseRegister = REGS[3+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[3]);
            (*pMyArgument).Memory.BaseRegister = REGS[3];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, BPDI_);
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *
 * ======================================= */
void __bea_callspec__ Addr_SIB_disp32(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    if (!Security(2, pMyDisasm)) return;
    if (GV.AddressSize >= 32) {
        MyNumber = *((Int32*)(UIntPtr) (GV.EIP_+3));
    }
    else {
        MyNumber = *((Int32*)(UIntPtr) (GV.EIP_+2));
    }

    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i ++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            j = i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) MyNumber);
        }
        /*(void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
         *i+=2; */
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize >= 32) {
        GV.DECALAGE_EIP++;
        GV.BASE_  = ((UInt8) *((UInt8*) (UIntPtr)GV.EIP_+2)) & 0x7;
        GV.SCALE_  = (((UInt8) *((UInt8*) (UIntPtr)GV.EIP_+2)) & 0xc0) >> 6;
        GV.INDEX_  = (((UInt8) *((UInt8*) (UIntPtr)GV.EIP_+2)) & 0x38) >> 3;
        j = i;
        i += SIB[GV.SCALE_ ](pMyArgument, j, pMyDisasm);
    }
    else {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic, Registers16Bits[6]);
        (*pMyArgument).Memory.BaseRegister = REGS[6];
        i += strlen (Registers16Bits[6]);
    }

    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) MyNumber);
        }
    }

}

/* =======================================
 *          ModRM_2
 * ======================================= */
void __bea_callspec__ Addr_EBP_disp32(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int32*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i ++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            j = i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[13]);
            (*pMyArgument).Memory.BaseRegister = REGS[5+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[5]);
            (*pMyArgument).Memory.BaseRegister = REGS[5];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[13]);
            (*pMyArgument).Memory.BaseRegister = REGS[5+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[5]);
            (*pMyArgument).Memory.BaseRegister = REGS[5];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[7]);
            (*pMyArgument).Memory.BaseRegister = REGS[7];
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *          ModRM_2
 * ======================================= */
void __bea_callspec__ Addr_ESI_disp32(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int32*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i ++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            j = i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[14]);
            (*pMyArgument).Memory.BaseRegister = REGS[6+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[6]);
            (*pMyArgument).Memory.BaseRegister = REGS[6];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[14]);
            (*pMyArgument).Memory.BaseRegister = REGS[6+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[6]);
            (*pMyArgument).Memory.BaseRegister = REGS[6];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[5]);
            (*pMyArgument).Memory.BaseRegister = REGS[5];
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *          ModRM_2
 * ======================================= */
void __bea_callspec__ Addr_EDI_disp32(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    size_t i = 0, j;
    long MyNumber;
    MyNumber = *((Int32*)(UIntPtr) (GV.EIP_+2));
    (*pMyArgument).Memory.Displacement = MyNumber;
    if (GV.SYNTAX_ == ATSyntax) {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i ++;
            j=i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            j = i;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) MyNumber);
        }
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
        i+=2;
    }
    (*pMyArgument).ArgType = MEMORY_TYPE;
    if (GV.AddressSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[15]);
            (*pMyArgument).Memory.BaseRegister = REGS[7+8];
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[7]);
            (*pMyArgument).Memory.BaseRegister = REGS[7];
        }
    }
    else if (GV.AddressSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[15]);
            (*pMyArgument).Memory.BaseRegister = REGS[7+8];

        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[7]);
            (*pMyArgument).Memory.BaseRegister = REGS[7];
        }
    }
    else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[3]);
            (*pMyArgument).Memory.BaseRegister = REGS[3];
    }
    i = strlen ((char*) &(*pMyArgument).ArgMnemonic);
    if (GV.SYNTAX_ == ATSyntax) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i += 1;
    }
    else {
        if (MyNumber < 0) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "-");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) ~MyNumber+1);
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i++;
            i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+i,"%.8X",(Int64) MyNumber);
        }
    }
}

/* =======================================
 *          ModRM_3
 * ======================================= */
void __bea_callspec__ _rEAX(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    int i = 0;
    GV.MemDecoration = 0;
    if (GV.MMX_  == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersMMX[0]);
        (*pMyArgument).ArgType = REGISTER_TYPE+MMX_REG+REGS[0];
        (*pMyArgument).ArgSize = 64;
        return;
    }

    if (GV.SSE_ == 1) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[8];
            (*pMyArgument).ArgSize = 128;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[0];
            (*pMyArgument).ArgSize = 128;
        }
        return;
    }
    if (GV.OperandSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[8];
            (*pMyArgument).ArgSize = 64;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[0];
            (*pMyArgument).ArgSize = 64;
        }
    }
    else if (GV.OperandSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[8];
            (*pMyArgument).ArgSize = 32;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[0];
            (*pMyArgument).ArgSize = 32;
        }
    }
    else if (GV.OperandSize == 16) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[8];
            (*pMyArgument).ArgSize = 16;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[0];
            (*pMyArgument).ArgSize = 16;
        }
    }
    else if (GV.OperandSize == 8) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[8];
            (*pMyArgument).ArgSize = 8;
        }
        else {
            if (GV.REX.state == 0) {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8BitsLegacy[0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS8BITS[0];
                (*pMyArgument).ArgSize = 8;
            }
            else {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[0];
                (*pMyArgument).ArgSize = 8;
            }
        }
    }
}


/* =======================================
 *          ModRM_3
 * ======================================= */
void __bea_callspec__ _rECX(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    int i = 0;
    GV.MemDecoration = 0;
    if (GV.MMX_  == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersMMX[1]);
        (*pMyArgument).ArgType = REGISTER_TYPE+MMX_REG+REGS[1];
        (*pMyArgument).ArgSize = 64;
        return;
    }

    if (GV.SSE_ == 1) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[1+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[1+8];
            (*pMyArgument).ArgSize = 128;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[1]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[1];
            (*pMyArgument).ArgSize = 128;
        }
        return;
    }
    if (GV.OperandSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[1+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[1+8];
            (*pMyArgument).ArgSize = 64;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[1]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[1];
            (*pMyArgument).ArgSize = 64;
        }
    }
    else if (GV.OperandSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[1+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[1+8];
            (*pMyArgument).ArgSize = 32;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[1]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[1];
            (*pMyArgument).ArgSize = 32;
        }
    }
    else if (GV.OperandSize == 16) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[1+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[1+8];
            (*pMyArgument).ArgSize = 16;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[1]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[1];
            (*pMyArgument).ArgSize = 16;
        }
    }
    else if (GV.OperandSize == 8) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[1+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[1+8];
            (*pMyArgument).ArgSize = 8;
        }
        else {
            if (GV.REX.state == 0) {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8BitsLegacy[1]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS8BITS[1];
                (*pMyArgument).ArgSize = 8;
            }
            else {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[1]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[1];
                (*pMyArgument).ArgSize = 8;
            }
        }
    }
}



/* =======================================
 *          ModRM_3
 * ======================================= */
void __bea_callspec__ _rEDX(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    int i = 0;
    GV.MemDecoration = 0;
    if (GV.MMX_  == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersMMX[2+0]);
        (*pMyArgument).ArgType = REGISTER_TYPE+MMX_REG+REGS[2+0];
        (*pMyArgument).ArgSize = 64;
        return;
    }

    if (GV.SSE_ == 1) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[2+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[2+8];
            (*pMyArgument).ArgSize = 128;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[2+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[2+0];
            (*pMyArgument).ArgSize = 128;
        }
        return;
    }
    if (GV.OperandSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[2+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[2+8];
            (*pMyArgument).ArgSize = 64;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[2+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[2+0];
            (*pMyArgument).ArgSize = 64;
        }
    }
    else if (GV.OperandSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[2+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[2+8];
            (*pMyArgument).ArgSize = 32;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[2+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[2+0];
            (*pMyArgument).ArgSize = 32;
        }
    }
    else if (GV.OperandSize == 16) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[2+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[2+8];
            (*pMyArgument).ArgSize = 16;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[2+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[2+0];
            (*pMyArgument).ArgSize = 16;
        }
    }
    else if (GV.OperandSize == 8) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[2+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[2+8];
            (*pMyArgument).ArgSize = 8;
        }
        else {
            if (GV.REX.state == 0) {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8BitsLegacy[2+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS8BITS[2+0];
                (*pMyArgument).ArgSize = 8;
            }
            else {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[2+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[2+0];
                (*pMyArgument).ArgSize = 8;
            }
        }
    }
}



/* =======================================
 *          ModRM_3
 * ======================================= */
void __bea_callspec__ _rEBX(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    int i = 0;
    GV.MemDecoration = 0;
    if (GV.MMX_  == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersMMX[3+0]);
        (*pMyArgument).ArgType = REGISTER_TYPE+MMX_REG+REGS[3+0];
        (*pMyArgument).ArgSize = 64;
        return;
    }

    if (GV.SSE_ == 1) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[3+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[3+8];
            (*pMyArgument).ArgSize = 128;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[3+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[3+0];
            (*pMyArgument).ArgSize = 128;
        }
        return;
    }
    if (GV.OperandSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[3+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[3+8];
            (*pMyArgument).ArgSize = 64;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[3+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[3+0];
            (*pMyArgument).ArgSize = 64;
        }
    }
    else if (GV.OperandSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[3+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[3+8];
            (*pMyArgument).ArgSize = 32;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[3+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[3+0];
            (*pMyArgument).ArgSize = 32;
        }
    }
    else if (GV.OperandSize == 16) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[3+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[3+8];
            (*pMyArgument).ArgSize = 16;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[3+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[3+0];
            (*pMyArgument).ArgSize = 16;
        }
    }
    else if (GV.OperandSize == 8) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[3+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[3+8];
            (*pMyArgument).ArgSize = 8;
        }
        else {
            if (GV.REX.state == 0) {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8BitsLegacy[3+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS8BITS[3+0];
                (*pMyArgument).ArgSize = 8;
            }
            else {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[3+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[3+0];
                (*pMyArgument).ArgSize = 8;
            }
        }
    }
}



/* =======================================
 *          ModRM_3
 * ======================================= */
void __bea_callspec__ _rESP(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    int i = 0;
    GV.MemDecoration = 0;
    if (GV.MMX_  == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersMMX[4+0]);
        (*pMyArgument).ArgType = REGISTER_TYPE+MMX_REG+REGS[4+0];
        (*pMyArgument).ArgSize = 64;
        return;
    }

    if (GV.SSE_ == 1) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[4+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[4+8];
            (*pMyArgument).ArgSize = 128;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[4+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[4+0];
            (*pMyArgument).ArgSize = 128;
        }
        return;
    }
    if (GV.OperandSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[4+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[4+8];
            (*pMyArgument).ArgSize = 64;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[4+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[4+0];
            (*pMyArgument).ArgSize = 64;
        }
    }
    else if (GV.OperandSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[4+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[4+8];
            (*pMyArgument).ArgSize = 32;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[4+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[4+0];
            (*pMyArgument).ArgSize = 32;
        }
    }
    else if (GV.OperandSize == 16) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[4+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[4+8];
            (*pMyArgument).ArgSize = 16;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[4+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[4+0];
            (*pMyArgument).ArgSize = 16;
        }
    }
    else if (GV.OperandSize == 8) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[4+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[4+8];
            (*pMyArgument).ArgSize = 8;
        }
        else {
            if (GV.REX.state == 0) {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8BitsLegacy[4+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS8BITS[4+0];
                (*pMyArgument).ArgSize = 8;
            }
            else {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[4+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[4+0];
                (*pMyArgument).ArgSize = 8;
            }
        }
    }
}



/* =======================================
 *          ModRM_3
 * ======================================= */
void __bea_callspec__ _rEBP(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    int i = 0;
    GV.MemDecoration = 0;
    if (GV.MMX_  == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersMMX[5+0]);
        (*pMyArgument).ArgType = REGISTER_TYPE+MMX_REG+REGS[5+0];
        (*pMyArgument).ArgSize = 64;
        return;
    }

    if (GV.SSE_ == 1) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[5+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[5+8];
            (*pMyArgument).ArgSize = 128;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[5+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[5+0];
            (*pMyArgument).ArgSize = 128;
        }
        return;
    }
    if (GV.OperandSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[5+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[5+8];
            (*pMyArgument).ArgSize = 64;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[5+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[5+0];
            (*pMyArgument).ArgSize = 64;
        }
    }
    else if (GV.OperandSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[5+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[5+8];
            (*pMyArgument).ArgSize = 32;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[5+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[5+0];
            (*pMyArgument).ArgSize = 32;
        }
    }
    else if (GV.OperandSize == 16) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[5+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[5+8];
            (*pMyArgument).ArgSize = 16;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[5+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[5+0];
            (*pMyArgument).ArgSize = 16;
        }
    }
    else if (GV.OperandSize == 8) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[5+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[5+8];
            (*pMyArgument).ArgSize = 8;
        }
        else {
            if (GV.REX.state == 0) {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8BitsLegacy[5+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS8BITS[5+0];
                (*pMyArgument).ArgSize = 8;
            }
            else {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[5+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[5+0];
                (*pMyArgument).ArgSize = 8;
            }
        }
    }
}



/* =======================================
 *          ModRM_3
 * ======================================= */
void __bea_callspec__ _rESI(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    int i = 0;
    GV.MemDecoration = 0;
    if (GV.MMX_  == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersMMX[6+0]);
        (*pMyArgument).ArgType = REGISTER_TYPE+MMX_REG+REGS[6+0];
        (*pMyArgument).ArgSize = 64;
        return;
    }

    if (GV.SSE_ == 1) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[6+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[6+8];
            (*pMyArgument).ArgSize = 128;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[6+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[6+0];
            (*pMyArgument).ArgSize = 128;
        }
        return;
    }
    if (GV.OperandSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[6+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[6+8];
            (*pMyArgument).ArgSize = 64;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[6+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[6+0];
            (*pMyArgument).ArgSize = 64;
        }
    }
    else if (GV.OperandSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[6+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[6+8];
            (*pMyArgument).ArgSize = 32;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[6+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[6+0];
            (*pMyArgument).ArgSize = 32;
        }
    }
    else if (GV.OperandSize == 16) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[6+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[6+8];
            (*pMyArgument).ArgSize = 16;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[6+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[6+0];
            (*pMyArgument).ArgSize = 16;
        }
    }
    else if (GV.OperandSize == 8) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[6+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[6+8];
            (*pMyArgument).ArgSize = 8;
        }
        else {
            if (GV.REX.state == 0) {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8BitsLegacy[6+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS8BITS[6+0];
                (*pMyArgument).ArgSize = 8;
            }
            else {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[6+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[6+0];
                (*pMyArgument).ArgSize = 8;
            }
        }
    }
}



/* =======================================
 *          ModRM_3
 * ======================================= */
void __bea_callspec__ _rEDI(ARGTYPE* pMyArgument, PDISASM pMyDisasm)
{
    int i = 0;
    GV.MemDecoration = 0;
    if (GV.MMX_  == 1) {
        (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersMMX[7+0]);
        (*pMyArgument).ArgType = REGISTER_TYPE+MMX_REG+REGS[7+0];
        (*pMyArgument).ArgSize = 64;
        return;
    }

    if (GV.SSE_ == 1) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[7+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[7+8];
            (*pMyArgument).ArgSize = 128;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, RegistersSSE[7+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+SSE_REG+REGS[7+0];
            (*pMyArgument).ArgSize = 128;
        }
        return;
    }
    if (GV.OperandSize == 64) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[7+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[7+8];
            (*pMyArgument).ArgSize = 64;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[7+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[7+0];
            (*pMyArgument).ArgSize = 64;
        }
    }
    else if (GV.OperandSize == 32) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[7+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[7+8];
            (*pMyArgument).ArgSize = 32;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[7+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[7+0];
            (*pMyArgument).ArgSize = 32;
        }
    }
    else if (GV.OperandSize == 16) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[7+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[7+8];
            (*pMyArgument).ArgSize = 16;
        }
        else {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers16Bits[7+0]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[7+0];
            (*pMyArgument).ArgSize = 16;
        }
    }
    else if (GV.OperandSize == 8) {
        if (GV.REX.B_ == 1) {
            (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[7+8]);
            (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[7+8];
            (*pMyArgument).ArgSize = 8;
        }
        else {
            if (GV.REX.state == 0) {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8BitsLegacy[7+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS8BITS[7+0];
                (*pMyArgument).ArgSize = 8;
            }
            else {
                (void) strcpy ((char*) (*pMyArgument).ArgMnemonic+i, Registers8Bits[7+0]);
                (*pMyArgument).ArgType = REGISTER_TYPE+GENERAL_REG+REGS[7+0];
                (*pMyArgument).ArgSize = 8;
            }
        }
    }
}


/* =======================================
 *
 * ======================================= */
size_t __bea_callspec__ SIB_0(ARGTYPE* pMyArgument, size_t i, PDISASM pMyDisasm)
{

    /* ========================= Interpret Base */

    if ((GV.BASE_  == 5) && (GV.MOD_ == 0)) {
        GV.DECALAGE_EIP += 4;
        if (!Security(7, pMyDisasm)) return i;
        (void) sprintf((char*) (*pMyArgument).ArgMnemonic+i, "%.8X",*((UInt32*)(UIntPtr) (GV.EIP_+3)));
        (*pMyArgument).Memory.Displacement = *((UInt32*)(UIntPtr) (GV.EIP_+3));
        i += 4;
    }
    else {

        if (GV.SYNTAX_ == ATSyntax) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
            i += 2;
        }
        if (GV.AddressSize == 64) {
            if (GV.REX.B_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.BASE_ ]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ ];
                i += strlen(Registers64Bits[GV.BASE_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.BASE_ +8]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ +8];
                i += strlen( Registers64Bits[GV.BASE_ +8]);
            }
        }
        else if (GV.AddressSize == 32) {
            if (GV.REX.B_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.BASE_ ]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ ];
                i += strlen( Registers32Bits[GV.BASE_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.BASE_ +8]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ +8];
                i += strlen( Registers32Bits[GV.BASE_ +8]);
            }
        }
    }
    /* ======================== Interpret Index */


    if (GV.INDEX_  != 4) {
        if (GV.SYNTAX_ == ATSyntax) {
            if (GV.BASE_  == 5) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(,%");
                i+=3;
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ",%");
                i+=2;
            }
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i+=1;
        }

        if (GV.AddressSize == 64) {
            if (GV.REX.X_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.INDEX_ ]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ ];
                i += strlen( Registers64Bits[GV.INDEX_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.INDEX_ +8]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ +8];
                i += strlen( Registers64Bits[GV.INDEX_ +8]);
            }
        }
        else if (GV.AddressSize == 32) {
            if (GV.REX.X_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.INDEX_ ]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ ];
                i += strlen( Registers32Bits[GV.INDEX_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.INDEX_ +8]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ +8];
                i += strlen( Registers32Bits[GV.INDEX_ +8]);
            }
        }
    }
    (*pMyArgument).Memory.Scale = 1;
    if ((GV.SYNTAX_ == ATSyntax) && ((GV.BASE_  != 5) || (GV.INDEX_  != 4))) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i++;
    }
    return i;
}

/* =======================================
 *
 * ======================================= */
size_t __bea_callspec__ SIB_1(ARGTYPE* pMyArgument, size_t i, PDISASM pMyDisasm)
{
    size_t j;
    /* ========================= Interpret Base */

    if ((GV.BASE_  == 5) && (GV.MOD_ == 0)) {
        GV.DECALAGE_EIP += 4;
        if (!Security(7, pMyDisasm)) return i;
        j = i;
        i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) *((UInt32*)(UIntPtr) (GV.EIP_+3)));
        (*pMyArgument).Memory.Displacement = *((UInt32*)(UIntPtr) (GV.EIP_+3));
    }
    else {

        if (GV.SYNTAX_ == ATSyntax) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
            i += 2;
        }
        if (GV.AddressSize == 64) {
            if (GV.REX.B_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.BASE_ ]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ ];
                i += strlen( Registers64Bits[GV.BASE_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.BASE_ +8]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ +8];
                i += strlen( Registers64Bits[GV.BASE_ +8]);
            }
        }
        else if (GV.AddressSize == 32) {
            if (GV.REX.B_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.BASE_ ]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ ];
                i += strlen( Registers32Bits[GV.BASE_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.BASE_ +8]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ +8];
                i += strlen( Registers32Bits[GV.BASE_ +8]);
            }
        }
    }
    /* ======================== Interpret Index */

    if (GV.INDEX_  != 4) {
        if (GV.SYNTAX_ == ATSyntax) {
            if (GV.BASE_  == 5) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(,%");
                i+=3;
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ",%");
                i+=2;
            }
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i+=1;
        }

        if (GV.AddressSize == 64) {
            if (GV.REX.X_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.INDEX_ ]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ ];
                i += strlen( Registers64Bits[GV.INDEX_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.INDEX_ +8]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ +8];
                i += strlen( Registers64Bits[GV.INDEX_ +8]);
            }
        }
        else if (GV.AddressSize == 32) {
            if (GV.REX.X_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.INDEX_ ]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ ];
                i += strlen( Registers32Bits[GV.INDEX_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.INDEX_ +8]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ +8];
                i += strlen( Registers32Bits[GV.INDEX_ +8]);
            }
        }
        (*pMyArgument).Memory.Scale = 2;
        if (GV.SYNTAX_ == ATSyntax) {
            if ((GV.BASE_  != 5) || (GV.INDEX_  != 4)) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ",2");
                i+=2;
            }
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "*2");

            i+=2;
        }
    }
    if ((GV.SYNTAX_ == ATSyntax) && ((GV.BASE_  != 5) || (GV.INDEX_  != 4))) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i++;
    }
    return i;
}

/* =======================================
 *
 * ======================================= */
size_t __bea_callspec__ SIB_2(ARGTYPE* pMyArgument, size_t i, PDISASM pMyDisasm)
{
    size_t j;
    /* ========================= Interpret Base */

    if ((GV.BASE_  == 5) && (GV.MOD_ == 0)) {
        GV.DECALAGE_EIP += 4;
        if (!Security(7, pMyDisasm)) return i;
        j = i;
        i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) *((UInt32*)(UIntPtr) (GV.EIP_+3)));
        (*pMyArgument).Memory.Displacement = *((UInt32*)(UIntPtr) (GV.EIP_+3));
    }
    else {

        if (GV.SYNTAX_ == ATSyntax) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
            i += 2;
        }
        if (GV.AddressSize == 64) {
            if (GV.REX.B_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.BASE_ ]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ ];
                i += strlen( Registers64Bits[GV.BASE_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.BASE_ +8]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ +8];
                i += strlen( Registers64Bits[GV.BASE_ +8]);
            }
        }
        else if (GV.AddressSize == 32) {
            if (GV.REX.B_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.BASE_ ]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ ];
                i += strlen( Registers32Bits[GV.BASE_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.BASE_ +8]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ +8];
                i += strlen( Registers32Bits[GV.BASE_ +8]);
            }
        }
    }
    /* ======================== Interpret Index */

    if (GV.INDEX_  != 4) {
        if (GV.SYNTAX_ == ATSyntax) {
            if (GV.BASE_  == 5) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(,%");
                i+=3;
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ",%");
                i+=2;
            }
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i+=1;
        }

        if (GV.AddressSize == 64) {
            if (GV.REX.X_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.INDEX_ ]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ ];
                i += strlen( Registers64Bits[GV.INDEX_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.INDEX_ +8]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ +8];
                i += strlen( Registers64Bits[GV.INDEX_ +8]);
            }
        }
        else if (GV.AddressSize == 32) {
            if (GV.REX.X_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.INDEX_ ]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ ];
                i += strlen( Registers32Bits[GV.INDEX_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.INDEX_ +8]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ +8];
                i += strlen( Registers32Bits[GV.INDEX_ +8]);
            }
        }
        (*pMyArgument).Memory.Scale = 4;
        if (GV.SYNTAX_ == ATSyntax) {
            if ((GV.BASE_  != 5) || (GV.INDEX_  != 4)) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ",4");
                i+=2;
            }
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "*4");
            i+=2;
        }
    }
    if ((GV.SYNTAX_ == ATSyntax) && ((GV.BASE_  != 5) || (GV.INDEX_  != 4))) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i++;
    }
    return i;
}

/* =======================================
 *
 * ======================================= */
size_t __bea_callspec__ SIB_3(ARGTYPE* pMyArgument, size_t i, PDISASM pMyDisasm)
{
    size_t j;
    /* ========================= Interpret Base */

    if ((GV.BASE_  == 5) && (GV.MOD_ == 0)) {
        GV.DECALAGE_EIP += 4;
        if (!Security(7, pMyDisasm)) return i;
        j = i;
        i+= CopyFormattedNumber(pMyDisasm, (char*) (*pMyArgument).ArgMnemonic+j,"%.8X",(Int64) *((UInt32*)(UIntPtr) (GV.EIP_+3)));
        (*pMyArgument).Memory.Displacement = *((UInt32*)(UIntPtr) (GV.EIP_+3));
    }
    else {

        if (GV.SYNTAX_ == ATSyntax) {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(%");
            i += 2;
        }
        if (GV.AddressSize == 64) {
            if (GV.REX.B_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.BASE_ ]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ ];
                i += strlen( Registers64Bits[GV.BASE_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.BASE_ +8]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ +8];
                i += strlen( Registers64Bits[GV.BASE_ +8]);
            }
        }
        else if (GV.AddressSize == 32) {
            if (GV.REX.B_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.BASE_ ]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ ];
                i += strlen( Registers32Bits[GV.BASE_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.BASE_ +8]);
                (*pMyArgument).Memory.BaseRegister = REGS[GV.BASE_ +8];
                i += strlen( Registers32Bits[GV.BASE_ +8]);
            }
        }
    }
    /* ======================== Interpret Index */

    if (GV.INDEX_  != 4) {
        if (GV.SYNTAX_ == ATSyntax) {
            if (GV.BASE_  == 5) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "(,%");
                i+=3;
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ",%");
                i+=2;
            }
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "+");
            i+=1;
        }

        if (GV.AddressSize == 64) {
            if (GV.REX.X_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.INDEX_ ]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ ];
                i += strlen( Registers64Bits[GV.INDEX_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers64Bits[GV.INDEX_ +8]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ +8];
                i += strlen( Registers64Bits[GV.INDEX_ +8]);
            }
        }
        else if (GV.AddressSize == 32) {
            if (GV.REX.X_ == 0) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.INDEX_ ]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ ];
                i += strlen( Registers32Bits[GV.INDEX_ ]);
            }
            else {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, Registers32Bits[GV.INDEX_ +8]);
                (*pMyArgument).Memory.IndexRegister = REGS[GV.INDEX_ +8];
                i += strlen( Registers32Bits[GV.INDEX_ +8]);
            }
        }
        (*pMyArgument).Memory.Scale = 8;
        if (GV.SYNTAX_ == ATSyntax) {
            if ((GV.BASE_  != 5) || (GV.INDEX_  != 4)) {
                (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ",8");
                i+=2;
            }
        }
        else {
            (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, "*8");
            i+=2;
        }
    }
    if ((GV.SYNTAX_ == ATSyntax) && ((GV.BASE_  != 5) || (GV.INDEX_  != 4))) {
        (void) strcpy((char*) (*pMyArgument).ArgMnemonic+i, ")");
        i++;
    }
    return i;
}
