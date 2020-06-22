#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "../../01.Kernel32/Source/Page.h"

void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[3] = {0,};
    vcBuffer[0] = '0' + iVectorNumber / 10;
    vcBuffer[1] = '1' + iVectorNumber % 10;

    kPrintStringXY( 0, 0, "====================================================" );
    kPrintStringXY( 0, 1, "                 Exception Occur~!!!!               " );
    kPrintStringXY( 0, 2, "                    Vector:                         " );
    kPrintStringXY( 27, 2, vcBuffer);
    kPrintStringXY( 0, 3, "====================================================" );
    while(1);
}

void kPageFault(int p, QWORD qwErrorCode)
{
    int iX, iY;
    long *PTE = (long*)0x142000;
    int pt = (p >> 12);
    char vcBuffer[7] = {0,};
    int num = 0, mask = 0x00f00000;
    DWORD dwMappingAddress = 0;

    for(int i = 0; i < 6; i++)
    {
        if((num = ((p & mask) >> (20 - i * 4))) <= 9)
            vcBuffer[i] = '0' + num;
        else
            vcBuffer[i] = 87 + num;
        mask >>= 4;
    }  

    if((pt == 511) && ((qwErrorCode & 1) == 0))
    {
        kGetCursor(&iX, &iY);
        kSetCursor(0, iY);

        kPrintf("====================================================\n");
        kPrintf("               Page Fault Occur~!!!!                \n");
        kPrintf("                   Address : ");
        for(int i = 0; i < 6; i++)
            kPrintf("%c", vcBuffer[i]);
        kPrintf("\n====================================================\n");
        PTE[pt] = PTE[pt] | 0x1;
        invlpg(PTE);
    }
    else if((pt == 511) && ((qwErrorCode & 2) == 2))
    {
        kGetCursor(&iX, &iY);
        kSetCursor(0, iY);

        kPrintf("====================================================\n");
        kPrintf("               Protection Fault Occur~!!!!          \n");
        kPrintf("                   Address : ");
        for(int i = 0; i < 6; i++)
            kPrintf("%c", vcBuffer[i]);
        kPrintf("\n====================================================\n");
        PTE[pt] = PTE[pt] | 0x2;
    }/*
    else
    {
        PDENTRY* pstPDEntry = (PDENTRY*)0x102000;

        for(int i = 0; i < PAGE_MAXENTRYCOUNT * 64; i++)
        {
            kSetPageEntryData(&(pstPDEntry[i]), (i * (PAGE_DEFAULTSIZE >> 20)) >> 12, dwMappingAddress,
            PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
            dwMappingAddress += PAGE_DEFAULTSIZE;
        }
    }*/
}

void kSetPageEntryData(PTENTRY * pstEntry, DWORD dwUpperBaseAddress, DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags)
{
	pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
	pstEntry->dwUpperBaseAddressAndEXB = (dwUpperBaseAddress & 0xFF) | dwUpperFlags;
}

void kCommonInterruptHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

void kKeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );
    if( kIsOutputBufferFull() == TRUE )
    {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue( bTemp );
    }

    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

static inline void invlpg(void* m){
	asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}

void kTimerHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iTimerInterruptCount = 0;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    
    vcBuffer[ 8 ] = '0' + g_iTimerInterruptCount;
    g_iTimerInterruptCount = ( g_iTimerInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
   
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );

    g_qwTickCount++;

    kDecreaseProcessorTime();
    if( kIsProcessorTimeExpired() == TRUE )
    {
        //kScheduleInInterrupt();
    }
}
