#include "Page.h"

void kInitializePageTables(void)
{
	PML4TENTRY* pstPML4TEntry;
	PDPTENTRY* pstPDPTEntry;
	PDENTRY* pstPDEntry;	//for Page Directory Entry setting
	PDENTRY* pstPTEntry;	//for Page Tabel Entry setting
	DWORD dwMappingAddress;
	DWORD dwMappingAddress2;	// 0x1000 = 4kb size
	int i;

	//PML4Entry seting
	pstPML4TEntry = (PML4TENTRY*) 0x100000;
	kSetPageEntryData(&(pstPML4TEntry[0]), 0x00, 0x101000, PAGE_FLAGS_DEFAULT,0);
	
	for(i = 1; i < PAGE_MAXENTRYCOUNT; i++)
		kSetPageEntryData(&(pstPML4TEntry[i]), 0,0,0,0);

	//PDPTEntry setting
	pstPDPTEntry = (PDPTENTRY*) 0x101000;
	
	for(i = 0; i < 64; i++)
		kSetPageEntryData(&(pstPDPTEntry[i]),0,0x102000 + (i * PAGE_TABLESIZE), PAGE_FLAGS_DEFAULT, 0);
	
	for(i = 64; i < PAGE_MAXENTRYCOUNT; i++)
		kSetPageEntryData(&(pstPDPTEntry[i]),0,0,0,0);

	
	//Set 4kb PTEntry
	pstPDEntry = (PDENTRY*)0x102000;
	dwMappingAddress = 0;
	pstPTEntry = (PTENTRY*)0x142000;
	dwMappingAddress2 = 0;

	kSetPageEntryData(&(pstPDEntry[0]), 0, 0x142000, PAGE_FLAGS_DEFAULT, 0);
	dwMappingAddress += PAGE_DEFAULTSIZE;

	//4kb PTEntry 0 ~ 510 == 0x00 ~ 0x1FE000 : R/W, 511 == 0x1FF000 : Read Only
	for(i = 0; i < 511; i++)
	{
		kSetPageEntryData(&(pstPTEntry[i]), 0, dwMappingAddress2, PAGE_FLAGS_DEFAULT, 0);
		dwMappingAddress2 += 0x1000;
	}
	kSetPageEntryData(&(pstPTEntry[511]), 0, dwMappingAddress2, 0, 0);
	

	//Set PDEntry
	for(i = 1; i < 5; i++)
	{
		kSetPageEntryData(&(pstPDEntry[i]),(i * (PAGE_DEFAULTSIZE >> 20)) >> 12, dwMappingAddress,
		PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
		dwMappingAddress += PAGE_DEFAULTSIZE;
	}

	kSetPageEntryData(&(pstPDEntry[5]),(0 * (PAGE_DEFAULTSIZE >> 20)) >> 12, 0, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS,0);
	dwMappingAddress += PAGE_DEFAULTSIZE;
	

	for(i = 6; i < PAGE_MAXENTRYCOUNT * 64; i++)
	{
		kSetPageEntryData(&(pstPDEntry[i]), (i * (PAGE_DEFAULTSIZE >> 20)) >> 12, dwMappingAddress,
		PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
		dwMappingAddress += PAGE_DEFAULTSIZE;
	}
	
	for(int i = 0; i < PAGE_MAXENTRYCOUNT * 64; i++)
	{
		kSetPageEntryData(&(pstPDEntry[i]), (i * (PAGE_DEFAULTSIZE >> 20)) >> 12, dwMappingAddress,
		PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
		dwMappingAddress += PAGE_DEFAULTSIZE;
	}
}

void kSetPageEntryData(PTENTRY * pstEntry, DWORD dwUpperBaseAddress, DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags)
{
	pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
	pstEntry->dwUpperBaseAddressAndEXB = (dwUpperBaseAddress & 0xFF) | dwUpperFlags;
}
