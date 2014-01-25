/*
 *  main.c
 *  SkeletonAddition
 */

#include <Carbon/Carbon.h>


#pragma mark Declarations of external entry points

#pragma GCC visibility push(default)

#ifdef __cplusplus
extern "C" {
#endif

OSErr MyEventHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
OSErr SAInitialize(CFBundleRef theBundle);
void SATerminate();
Boolean SAIsBusy();

#ifdef __cplusplus
}
#endif

#pragma GCC visibility pop

#pragma mark Event handler

// echo function 
// satest "hello osax"

OSErr MyEventHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon)
{
	OSErr			theErr = noErr; 	
	DescType 		typeCode;
	Size 			sizeOfParam, actualSize;
	char			*directstr;
	
	theErr = AESizeOfParam(	ev,
						   keyDirectObject,
						   &typeCode,
						   &sizeOfParam);
	if (theErr != noErr){    		
		return theErr;
	}     
	directstr = (char *) malloc (sizeOfParam);
	theErr = AEGetParamPtr(ev, keyDirectObject, typeChar, 
						   &typeCode, (Ptr) directstr, 
						   sizeOfParam, &actualSize);
	theErr = AEPutParamPtr(reply, keyDirectObject, typeChar,
						   directstr, actualSize );
	free(directstr);
	return noErr;
}

#pragma mark Pre-Leopard infrastructure

static OSErr InstallMyEventHandlers();
static void RemoveMyEventHandlers();

OSErr SAInitialize(CFBundleRef theBundle)
{
	// Any other setup you need here...

	fputs("Init!\n", stderr);
	return InstallMyEventHandlers();
}

void SATerminate()
{
	fputs("Terminate!\n", stderr);
	RemoveMyEventHandlers();
}

Boolean SAIsBusy()
{
	fputs("Busy!\n", stderr);
	return false;
}

#pragma mark handler installation and removal (pre-Leopard only)

struct AEEventHandlerInfo {
	FourCharCode			evClass, evID;
	AEEventHandlerProcPtr	proc;
};
typedef struct AEEventHandlerInfo AEEventHandlerInfo;

static const AEEventHandlerInfo gEventInfo[] = {
	{ 'SKEL', 'SKEL', MyEventHandler }
	// Add more suite/event/handler triplets here if you define more than one command.
};
#define kEventHandlerCount  (sizeof(gEventInfo) / sizeof(AEEventHandlerInfo))

static AEEventHandlerUPP gEventUPPs[kEventHandlerCount];

static OSErr InstallMyEventHandlers()
{
	OSErr err;
	size_t i;
	
	for (i = 0; i < kEventHandlerCount; ++i) {
		if ((gEventUPPs[i] = NewAEEventHandlerUPP(gEventInfo[i].proc)) != NULL)
			err = AEInstallEventHandler(gEventInfo[i].evClass, gEventInfo[i].evID, gEventUPPs[i], 0, true);
		else
			err = memFullErr;
		
		if (err != noErr) {
			SATerminate();  // Call the termination function ourselves, because the loader won't once we fail.
			return err;
		}
	}

	return noErr; 
}

static void RemoveMyEventHandlers()
{
	OSErr err;
	size_t i;

	for (i = 0; i < kEventHandlerCount; ++i) {
		if (gEventUPPs[i] &&
			(err = AERemoveEventHandler(gEventInfo[i].evClass, gEventInfo[i].evID, gEventUPPs[i], true)) == noErr)
		{
			DisposeAEEventHandlerUPP(gEventUPPs[i]);
			gEventUPPs[i] = NULL;
		}
	}
}

