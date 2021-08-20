#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/IoLib.h>

VOID output_c(UINT32 c[], UINT32 bus, UINT32 device, UINT32 fun);
VOID output(UINT32 bus, UINT32 device, UINT32 fun);
EFI_STATUS
EFIAPI

UefiMain(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE* SystemTable
)
{
	UINT32 pci[128];
	UINT32 bus, device, fun;
	UINT32 InputStr,uintkey[8];
	UINT32 pcic = 0, i, keynum = 0;
	UINT32 ioaddnum, iodatanum;
	UINTN Index;
	EFI_INPUT_KEY key;
	for (bus = 0; bus < 0x100; bus++) {
		for (device = 0; device < 0x20; device++) {
			for (fun = 0; fun < 8; fun++) {
				ioaddnum = 0x80000000L | (bus << 16) | (device << 11) | (fun << 8);
				IoWrite32(0xCF8, ioaddnum);
				iodatanum = IoRead32(0xCFC);
				if (iodatanum != 0xffffffff) {
					pci[pcic] = bus * 256 + device * 8 + fun;
					pcic++;
				}
			}
		}
	}
	Print(L"The PC has [ %d ]  PCI devices,which device's,which PCI device do you want to check the first 256 bytes of the configuration space ? (0-%d)\n", pcic,pcic-1);
	//SystemTable->BootServices->WaitForEvent(1, &(SystemTable->ConIn->WaitForKey), &InputStr);
	InputStr = 0;
	//InputStr = Index;
	while (1) {
		SystemTable->BootServices->WaitForEvent(1, &(SystemTable->ConIn->WaitForKey), &Index);
		SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key);
		InputStr = key.UnicodeChar;
		if (InputStr > 47 && InputStr < 58) {
			Print(L"%d", InputStr - 48);
		}
		else {
			Print(L"\n");
		}
		uintkey[keynum] = InputStr;
		if (InputStr == 13) {
			break;
		}
		keynum++;
		if (keynum >= 8) {
			break;
		}
	}
	InputStr = 0;
	for (i = 0; i < keynum; i++) {
		InputStr = InputStr * 10 + uintkey[i] - 48;
	}
	if (InputStr >= 0 && InputStr < pcic) {
		bus = pci[InputStr] / 256;
		device = (pci[InputStr] / 8) % 32;
		fun = pci[InputStr] % 8;
		Print(L"The device %d:%x %x %x:\n", InputStr, bus, device, fun);
		output(bus, device, fun);
	}
	else if (InputStr == pcic) {
		for (i = 0; i < pcic; i++) {
			bus = pci[i] / 256;
			device = (pci[i] / 8) % 32;
			fun = pci[i] % 8;
			Print(L"The device %d:%x %x %x:\n", i, bus, device, fun);
			output(bus, device, fun);
		}
	}
	else {
		Print(L"ERROR!\n");
	}
	
//	IoWrite32(0xCF8, 0x80000000);
//	Print(L"freedebug bus:0 dev:0 fun:0 offset 0 = %x\n", IoRead32(0xCFC));
	return EFI_SUCCESS;
}
VOID output_c(UINT32 c[], UINT32 bus, UINT32 device, UINT32 fun) {
	Print(L"The following shows the first 256Byte configuration space of the PCIE device bus:%x,device:%x,function:%x:\n", bus, device, fun);
	Print(L"  ");
	for (UINT32 i = 0; i < 0x10; i++) {
		Print(L"%x  ", i);
	}
	Print(L"\n0 ");
	for (UINT32 i = 0; i < 512; i++) {
		Print(L"%x", c[i]);
		if (i % 2 == 1 && i % 32 != 31) {
			Print(L" ");
		}
		else if (i % 32 == 31) {
			Print(L"\n");
			if (i != 511) {
				Print(L"%x ", i / 32 + 1);
			}
		}
	}
}

VOID output(UINT32 bus, UINT32 device, UINT32 fun) {
	UINT32 ioaddnum, iodatanum, r, count, i, iodatanum2;
	UINT32 c[512], temp[8];
	for (r = 0; r < 0x40; r++) {
		ioaddnum = 0x80000000L | (bus << 16) | (device << 11) | (fun << 8) | (r << 2);
		IoWrite32(0xCF8, ioaddnum);
		iodatanum = IoRead32(0xCFC);
		if (iodatanum == 0xffffffff) {
			Print(L"The PCIE device bus:%x,device:%x,function:%x does not exist!\n", bus, device, fun);
			break;
		}
		//Print(L"offset%x=%x    \n",  r, iodatanum);
		count = 0;
		iodatanum2 = iodatanum;
		while (iodatanum2 != 0)
		{
			iodatanum2 /= 16;
			++count;
		}
		if (count == 0) {
			count = 1;
		}
		for (i = 0; i < 8; i++) {
			if (i >= count) {
				temp[7 - i] = 0;
			}
			else {
				temp[7 - i] = iodatanum % 0x10;
				iodatanum = iodatanum / 16;
			}
		}
		for (i = 0; i < 4; i++) {
			c[r * 8 + i * 2] = temp[6 - i * 2];
			c[r * 8 + i * 2 + 1] = temp[7 - i * 2];
		}
		if (r == 63) {
			output_c(c, bus, device, fun);
		}
	}
	Print(L"\n");
}
