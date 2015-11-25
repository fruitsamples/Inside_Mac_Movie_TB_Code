/*	File:		mtb.c	Contains:		Written by: 		Copyright:	Copyright � 1992-1999 by Apple Computer, Inc., All Rights Reserved.				You may incorporate this Apple sample source code into your program(s) without				restriction. This Apple sample source code has been provided "AS IS" and the				responsibility for its operation is yours. You are not permitted to redistribute				this Apple sample source code as "Apple sample source code" after having made				changes. If you're going to re-distribute the source, we require that you make				it clear in the source that the code was descended from Apple sample source				code, but that you've made changes.	Change History (most recent first):				8/16/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1				12/4/94		khs				changed the format of the file to the new look and feel*/// INCLUDES#include "mtb.h"#include <TextUtils.h>// FUNCTIONSvoid CheckError(OSErr error,				Str255 displayString){	if (error == noErr)		return;	if (displayString[0] > 0)		DebugStr(displayString);	ExitToShell();}void DrawFrame(const Rect* trackFrame,			   long curSample){	Str255 numStr;	ForeColor(redColor);	PaintRect(trackFrame);	ForeColor(blueColor);	NumToString(curSample, numStr);	MoveTo(trackFrame->right / 2, trackFrame->bottom / 2);	TextSize(trackFrame->bottom / 3);	DrawString(numStr);}void AddVideoSamplesToMedia(Media theMedia,							const Rect* trackFrame){	long maxCompressedSize;	GWorldPtr theGWorld = nil;	long curSample;	Handle compressedData = nil;	Ptr compressedDataPtr;	ImageDescriptionHandle imageDesc = nil;	CGrafPtr oldPort;	GDHandle oldGDeviceH;	OSErr err = noErr;	err = NewGWorld(&theGWorld, 16,				// pixel depth 								trackFrame, nil, nil, (GWorldFlags)0);	CheckError(err, "\pNewGWorld");	LockPixels(theGWorld->portPixMap);	err = GetMaxCompressionSize(theGWorld->portPixMap, trackFrame, 0,// let ICM choose depth								codecNormalQuality, 'rle ', (CompressorComponent)anyCodec, &maxCompressedSize);	CheckError(err, "\pGetMaxCompressionSize");	compressedData = NewHandle(maxCompressedSize);	CheckError(MemError(), "\pNewHandle");	MoveHHi(compressedData);	HLock(compressedData);	compressedDataPtr = StripAddress(*compressedData);	imageDesc = (ImageDescriptionHandle)NewHandle(4);	CheckError(MemError(), "\pNewHandle");	GetGWorld(&oldPort, &oldGDeviceH);	SetGWorld(theGWorld, nil);	//������� changed to <= 30	for (curSample = 1; curSample <= 30; curSample++)	{		EraseRect(trackFrame);		DrawFrame(trackFrame, curSample);		err = CompressImage(theGWorld->portPixMap, trackFrame, codecNormalQuality, 'rle ', imageDesc, compressedDataPtr);		CheckError(err, "\pCompressImage");		err = AddMediaSample(theMedia, compressedData, 0,// no offset in data							 (**imageDesc).dataSize, 60,// frame duration = 1/10 sec							 (SampleDescriptionHandle)imageDesc, 1,// one sample							 0,					// self-contained samples							 nil);		CheckError(err, "\pAddMediaSample");	}	SetGWorld(oldPort, oldGDeviceH);	if (imageDesc)		DisposeHandle((Handle)imageDesc);	if (compressedData)		DisposeHandle(compressedData);	if (theGWorld)		DisposeGWorld(theGWorld);}void CreateMyVideoTrack(Movie theMovie){	Track theTrack;	Media theMedia;	OSErr err = noErr;	Rect trackFrame =	{		0,  0, 100, 320	}	;	theTrack = NewMovieTrack(theMovie, FixRatio(trackFrame.right, 1), FixRatio(trackFrame.bottom, 1), kNoVolume);	CheckError(GetMoviesError(), "\pNewMovieTrack");	theMedia = NewTrackMedia(theTrack, VideoMediaType, 600,// Video Time Scale							 nil, 0);	CheckError(GetMoviesError(), "\pNewTrackMedia");	err = BeginMediaEdits(theMedia);	CheckError(err, "\pBeginMediaEdits");	AddVideoSamplesToMedia(theMedia, &trackFrame);	err = EndMediaEdits(theMedia);	CheckError(err, "\pEndMediaEdits");	err = InsertMediaIntoTrack(theTrack, 0,		// track start time							   0,				// media start time							   GetMediaDuration(theMedia), fixed1);	CheckError(err, "\pInsertMediaIntoTrack");}void CreateMyCoolMovie(void){	Point where =	{		100,  100	}	;	SFReply theSFReply;	Movie theMovie = nil;	FSSpec mySpec;	short resRefNum = 0;	short resId = 0;	OSErr err = noErr;	SFPutFile(where, "\pEnter movie file name:", "\pMovie File", nil, &theSFReply);	if (!theSFReply.good)		return;	FSMakeFSSpec(theSFReply.vRefNum, 0, theSFReply.fName, &mySpec);	err = CreateMovieFile(&mySpec, 'TVOD', smCurrentScript, createMovieFileDeleteCurFile, &resRefNum, &theMovie);	CheckError(err, "\pCreateMovieFile");	CreateMyVideoTrack(theMovie);	CreateMySoundTrack(theMovie);	err = AddMovieResource(theMovie, resRefNum, &resId, theSFReply.fName);	CheckError(err, "\pAddMovieResource");	if (resRefNum)		CloseMovieFile(resRefNum);	DisposeMovie(theMovie);}void InitMovieToolbox(void){	OSErr err;	InitGraf(&qd.thePort);	InitFonts();	InitWindows();	InitMenus();	TEInit();	InitDialogs(nil);	MaxApplZone();	if (!IsQuickTimeInstalled())	{		CheckError(-1, "\pPlease install QuickTime and try again.");	}	err = EnterMovies();	CheckError(err, "\pEnterMovies");}// MAIN FUNCTIONvoid MainCreate(void);void MainCreate(void){	InitMovieToolbox();	CreateMyCoolMovie();}void MainPlay1(void);void MainPlay1(void){	WindowPtr aWindow;	Rect windowRect;	Rect movieBox;	Movie aMovie;	Boolean done = false;	OSErr err;	EventRecord theEvent;	WindowPtr whichWindow;	short part;	InitGraf(&qd.thePort);	InitFonts();	InitWindows();	InitMenus();	TEInit();	InitDialogs(nil);	MaxApplZone();	if (!IsQuickTimeInstalled())	{		CheckError(-1, "\pPlease install QuickTime and try again.");	}	err = EnterMovies();	if (err)		return;	SetRect(&windowRect, 100, 100, 200, 200);	aWindow = NewCWindow(nil, &windowRect, "\pMovie", false, noGrowDocProc, (WindowPtr) - 1, true, 0);	SetPort(aWindow);	aMovie = GetMovie();	if (aMovie == nil)		return;	GetMovieBox(aMovie, &movieBox);	OffsetRect(&movieBox, -movieBox.left, -movieBox.top);	SetMovieBox(aMovie, &movieBox);	SizeWindow(aWindow, movieBox.right, movieBox.bottom, true);	ShowWindow(aWindow);	SetMovieGWorld(aMovie, (CGrafPtr)aWindow, nil);	StartMovie(aMovie);	while (!IsMovieDone(aMovie) &&!done)	{		if (WaitNextEvent(everyEvent, &theEvent, 0, nil))		{			switch (theEvent.what)			{				case updateEvt:					whichWindow = (WindowPtr)theEvent.message;					if (whichWindow == aWindow)					{						BeginUpdate(whichWindow);						UpdateMovie(aMovie);						SetPort(whichWindow);						EraseRect(&whichWindow->portRect);						EndUpdate(whichWindow);					}					break;				case mouseDown:					part = FindWindow(theEvent.where, &whichWindow);					if (whichWindow == aWindow)					{						switch (part)						{							case inGoAway:								done = TrackGoAway(whichWindow, theEvent.where);								break;							case inDrag:								DragWindow(whichWindow, theEvent.where, &qd.screenBits.bounds);								break;						}					}					break;			}		}		MoviesTask(aMovie, DoTheRightThing);	}	DisposeMovie(aMovie);	DisposeWindow(aWindow);}void MainPlay2(void);void MainPlay2(void){	MovieController aController;	WindowPtr aWindow;	Rect aRect;	Movie aMovie;	Boolean done = false;	OSErr err;	EventRecord theEvent;	WindowPtr whichWindow;	short part;	InitGraf(&qd.thePort);	InitFonts();	InitWindows();	InitMenus();	TEInit();	InitDialogs(nil);	MaxApplZone();	if (!IsQuickTimeInstalled())	{		CheckError(-1, "\pPlease install QuickTime and try again.");	}	err = EnterMovies();	if (err)		return;	SetRect(&aRect, 100, 100, 200, 200);	aWindow = NewCWindow(nil, &aRect, "\pMovie", false, noGrowDocProc, (WindowPtr) - 1, true, 0);	SetPort(aWindow);	aMovie = GetMovie();	if (aMovie == nil)		return;	SetRect(&aRect, 0, 0, 100, 100);	aController = NewMovieController(aMovie, &aRect, mcTopLeftMovie);	if (aController == nil)		return;	err = MCGetControllerBoundsRect(aController, &aRect);	SizeWindow(aWindow, aRect.right, aRect.bottom, true);	ShowWindow(aWindow);	err = MCDoAction(aController, mcActionSetKeysEnabled, (Ptr)true);	while (!done)	{		WaitNextEvent(everyEvent, &theEvent, 0, nil);		if (!MCIsPlayerEvent(aController, &theEvent))		{			switch (theEvent.what)			{				case updateEvt:					whichWindow = (WindowPtr)theEvent.message;					BeginUpdate(whichWindow);					EraseRect(&whichWindow->portRect);					EndUpdate(whichWindow);					break;				case mouseDown:					part = FindWindow(theEvent.where, &whichWindow);					if (whichWindow == aWindow)					{						switch (part)						{							case inGoAway:								done = TrackGoAway(whichWindow, theEvent.where);								break;							case inDrag:								DragWindow(whichWindow, theEvent.where, &qd.screenBits.bounds);								break;						}					}			}		}	}	DisposeMovieController(aController);	DisposeMovie(aMovie);	DisposeWindow(aWindow);}Movie GetMovie(void){	OSErr err;	SFTypeList typeList =	{		MovieFileType,  0, 0, 0	};	StandardFileReply reply;	Movie aMovie = nil;	short movieResFile;	StandardGetFilePreview(nil, 1, typeList, &reply);	if (reply.sfGood)	{		err = OpenMovieFile(&reply.sfFile, &movieResFile, fsRdPerm);		if (err == noErr)		{			short movieResID = 0;				// want first movie			Str255 movieName;			Boolean wasChanged;			err = NewMovieFromFile(&aMovie, movieResFile, &movieResID, movieName, newMovieActive,// flags								   &wasChanged);			CloseMovieFile(movieResFile);		}	}	return aMovie;}void CreateTrackMatte(Track theTrack){	QDErr err;	GWorldPtr aGW;	Rect trackBox;	Fixed trackHeight;	Fixed trackWidth;	CTabHandle grayCTab;	GetTrackDimensions(theTrack, &trackWidth, &trackHeight);	SetRect(&trackBox, 0, 0, FixRound(trackWidth), FixRound(trackHeight));	grayCTab = GetCTable(40);					// 8 bit + 32 = 8 bit gray 	err = NewGWorld(&aGW, 8, &trackBox, grayCTab, (GDHandle)nil, 0);	DisposeCTable(grayCTab);	if (!err && (aGW != nil))	{		SetTrackMatte(theTrack, aGW->portPixMap);		DisposeGWorld(aGW);	}}void UpdateTrackMatte(Track theTrack){	OSErr err;	PixMapHandle trackMatte;	PixMapHandle savePortPix;	Movie theMovie;	GWorldPtr tempGW;	CGrafPtr savePort;	GDHandle saveGDevice;	Rect matteBox;	short i;	theMovie = GetTrackMovie(theTrack);	trackMatte = GetTrackMatte(theTrack);	if (trackMatte == nil)	{		// track doesn't have a matte, so give it one		CreateTrackMatte(theTrack);		trackMatte = GetTrackMatte(theTrack);		if (trackMatte == nil)			return;	}	GetGWorld(&savePort, &saveGDevice);	matteBox = (**trackMatte).bounds;	err = NewGWorld(&tempGW, (**trackMatte).pixelSize, &matteBox, (**trackMatte).pmTable, (GDHandle)nil, 0);	if (err || (tempGW == nil))		return;	SetGWorld(tempGW, nil);	savePortPix = tempGW->portPixMap;	LockPixels(trackMatte);	SetPortPix(trackMatte);	// draw a gray ramp rectangle around the edge of the matte 		for (i = 0; i < 35; i++)	{		RGBColor aColor;		long tempLong;		tempLong = 65536 - ((65536 / 35) * (long)i);		aColor.red = aColor.green = aColor.blue = tempLong;		RGBForeColor(&aColor);		FrameRect(&matteBox);		InsetRect(&matteBox, 1, 1);	}	// fill the center of the matte with black	ForeColor(blackColor);	PaintRect(&matteBox);	SetPortPix(savePortPix);	SetGWorld(savePort, saveGDevice);	DisposeGWorld(tempGW);	UnlockPixels(trackMatte);	SetTrackMatte(theTrack, trackMatte);	DisposeMatte(trackMatte);}pascal OSErr MyCoverProc(Movie aMovie,						 RgnHandle changedRgn,						 long refcon){	#pragma unused(refcon)	CGrafPtr mPort;	GDHandle mGD;	GetMovieGWorld(aMovie, &mPort, &mGD);	DiffRgn(mPort->clipRgn, changedRgn, mPort->clipRgn);	return noErr;}pascal OSErr MyUnCoverProc(Movie aMovie,						   RgnHandle changedRgn,						   long refcon){	#pragma unused(refcon)	CGrafPtr mPort,  curPort;	GDHandle mGD,  curGD;	GetMovieGWorld(aMovie, &mPort, &mGD);	GetGWorld(&curPort, &curGD);	SetGWorld(mPort, mGD);	InvalRgn(changedRgn);	UnionRgn(mPort->clipRgn, changedRgn, mPort->clipRgn);	SetGWorld(curPort, curGD);	return noErr;}	void InitCoverProcs(WindowPtr aWindow,						Movie aMovie)	{		RgnHandle displayBounds;		GrafPtr curPort;		displayBounds = GetMovieDisplayBoundsRgn(aMovie);		if (displayBounds == nil)			return;		GetPort(&curPort);		SetPort(aWindow);		ClipRect(&aWindow->portRect);		DiffRgn(aWindow->clipRgn, displayBounds, aWindow->clipRgn);		DisposeRgn(displayBounds);		SetPort(curPort);		SetMovieCoverProcs(aMovie, NewMovieRgnCoverProc(MyUnCoverProc), NewMovieRgnCoverProc(MyCoverProc), 0);	}void DoUpdate(WindowPtr theWindow,			  Movie theMovie){	BeginUpdate(theWindow);	UpdateMovie(theMovie);	EndUpdate(theWindow);}#pragma options align=mac68ktypedef SndCommand* SndCmdPtr;typedef struct{	short format;	short numSynths;} Snd1Header, * Snd1HdrPtr, ** Snd1HdrHndl;typedef struct{	short format;	short refCount;} Snd2Header, * Snd2HdrPtr, ** Snd2HdrHndl;typedef struct{	short synthID;	long initOption;} SynthInfo, * SynthInfoPtr;#pragma options align=reset// FUNCTIONSlong GetSndHdrOffset(Handle sndHandle){	short howManyCmds;	long sndOffset = 0;	Ptr sndPtr;	if (sndHandle == nil)		return 0;	sndPtr = *sndHandle;	if (sndPtr == nil)		return 0;	if ((*(Snd1HdrPtr)sndPtr).format == firstSoundFormat)	{		short synths = ((Snd1HdrPtr)sndPtr)->numSynths;		sndPtr += sizeof(Snd1Header) + (sizeof(SynthInfo) * synths);	}	else	{		sndPtr += sizeof(Snd2Header);	}	howManyCmds = *(short*)sndPtr;	sndPtr += sizeof(howManyCmds);	// sndPtr is now at the first sound command - cruise all	// 		commands and find the first soundCmd or bufferCmd	while (howManyCmds > 0)	{		switch (((SndCmdPtr)sndPtr)->cmd)		{			case (soundCmd + dataOffsetFlag):			case (bufferCmd + dataOffsetFlag):				sndOffset = ((SndCmdPtr)sndPtr)->param2;				howManyCmds = 0;				/* done, get out of loop */				break;			default:							/* catch any other type of commands */				sndPtr += sizeof(SndCommand);				howManyCmds--;				break;		}	}											/* done with all the commands */	return sndOffset;}#define kMACEBeginningNumberOfBytes  6#define kMACE31MonoPacketSize  2#define kMACE31StereoPacketSize  4#define kMACE61MonoPacketSize  1#define kMACE61StereoPacketSize  2// FUNCTIONSvoid CreateSoundDescription(Handle sndHandle,							SoundDescriptionHandle sndDesc,							long* sndDataOffset,							long* numSamples,							long* sndDataSize){	long sndHdrOffset = 0;	long sampleDataOffset;	SoundHeaderPtr sndHdrPtr = nil;	long numFrames;	long samplesPerFrame;	long bytesPerFrame;	SoundDescriptionPtr sndDescPtr;	*sndDataOffset = 0;	*numSamples = 0;	*sndDataSize = 0;	SetHandleSize((Handle)sndDesc, sizeof(SoundDescription));	CheckError(MemError(), "\pSetHandleSize");	sndHdrOffset = GetSndHdrOffset(sndHandle);	if (sndHdrOffset == 0)		CheckError(-1, "\pGetSndHdrOffset ");	// we can use pointers since we don't move memory	sndHdrPtr = (SoundHeaderPtr)(*sndHandle + sndHdrOffset);	sndDescPtr = *sndDesc;	sndDescPtr->descSize = sizeof(SoundDescription);	// total size of sound description structure	sndDescPtr->resvd1 = 0;	sndDescPtr->resvd2 = 0;	sndDescPtr->dataRefIndex = 1;	sndDescPtr->compressionID = 0;	sndDescPtr->packetSize = 0;	sndDescPtr->version = 0;	sndDescPtr->revlevel = 0;	sndDescPtr->vendor = 0;	switch (sndHdrPtr->encode)	{		case stdSH:			sndDescPtr->dataFormat = 'raw ';			// uncompressed offset-binary data			sndDescPtr->numChannels = 1;			// number of channels of sound			sndDescPtr->sampleSize = 8;			// number of bits per sample			sndDescPtr->sampleRate = sndHdrPtr->sampleRate;			// sample rate					*numSamples = sndHdrPtr->length;			*sndDataSize = *numSamples;			bytesPerFrame = 1;			samplesPerFrame = 1;			sampleDataOffset = (Ptr) & sndHdrPtr->sampleArea - (Ptr)sndHdrPtr;			break;		case extSH:			{				ExtSoundHeaderPtr extSndHdrP;				extSndHdrP = (ExtSoundHeaderPtr)sndHdrPtr;				sndDescPtr->dataFormat = 'raw ';				// uncompressed offset-binary data				sndDescPtr->numChannels = extSndHdrP->numChannels;				// number of channels of sound				sndDescPtr->sampleSize = extSndHdrP->sampleSize;				// number of bits per sample				sndDescPtr->sampleRate = extSndHdrP->sampleRate;				// sample rate							numFrames = extSndHdrP->numFrames;				*numSamples = numFrames;				bytesPerFrame = extSndHdrP->numChannels * (extSndHdrP->sampleSize / 8);				samplesPerFrame = 1;				*sndDataSize = numFrames * bytesPerFrame;				sampleDataOffset = (Ptr)(&extSndHdrP->sampleArea) - (Ptr)extSndHdrP;			}			break;		case cmpSH:			{				CmpSoundHeaderPtr cmpSndHdrP;				cmpSndHdrP = (CmpSoundHeaderPtr)sndHdrPtr;				sndDescPtr->numChannels = cmpSndHdrP->numChannels;				// number of channels of sound				sndDescPtr->sampleSize = cmpSndHdrP->sampleSize;				// number of bits per sample before compression				sndDescPtr->sampleRate = cmpSndHdrP->sampleRate;				// sample rate				numFrames = cmpSndHdrP->numFrames;				sampleDataOffset = (Ptr)(&cmpSndHdrP->sampleArea) - (Ptr)cmpSndHdrP;				switch (cmpSndHdrP->compressionID)				{					case threeToOne:						sndDescPtr->dataFormat = 'MAC3';						// compressed 3:1 data						samplesPerFrame = kMACEBeginningNumberOfBytes;						*numSamples = numFrames * samplesPerFrame;						switch (cmpSndHdrP->numChannels)						{							case 1:								bytesPerFrame = cmpSndHdrP->numChannels * kMACE31MonoPacketSize;								break;							case 2:								bytesPerFrame = cmpSndHdrP->numChannels * kMACE31StereoPacketSize;								break;							default:								CheckError(-1, "\pCorrupt sound data");								break;						}						*sndDataSize = numFrames * bytesPerFrame;						break;					case sixToOne:						sndDescPtr->dataFormat = 'MAC6';						// compressed 6:1 data						samplesPerFrame = kMACEBeginningNumberOfBytes;						*numSamples = numFrames * samplesPerFrame;						switch (cmpSndHdrP->numChannels)						{							case 1:								bytesPerFrame = cmpSndHdrP->numChannels * kMACE61MonoPacketSize;								break;							case 2:								bytesPerFrame = cmpSndHdrP->numChannels * kMACE61StereoPacketSize;								break;							default:								CheckError(-1, "\pCorrupt sound data");								break;						}						*sndDataSize = (*numSamples) * bytesPerFrame;						break;					default:						CheckError(-1, "\pCorrupt sound data");						break;				}			}									// switch cmpSndHdrP->compressionID:*/			break;								// of cmpSH:		default:			CheckError(-1, "\pCorrupt sound data");			break;	}											// switch sndHdrPtr->encode*/	*sndDataOffset = sndHdrOffset + sampleDataOffset;}void CreateMySoundTrack(Movie theMovie){	Track theTrack;	Media theMedia;	Handle sndHandle = nil;	SoundDescriptionHandle sndDesc = nil;	long sndDataOffset;	long sndDataSize;	long numSamples;	OSErr err = noErr;	sndHandle = GetResource('snd ', 128);	CheckError(ResError(), "\pGetResource");	if (sndHandle == nil)		return;	sndDesc = (SoundDescriptionHandle)NewHandle(4);	CheckError(MemError(), "\pNewHandle");	CreateSoundDescription(sndHandle, sndDesc, &sndDataOffset, &numSamples, &sndDataSize);	theTrack = NewMovieTrack(theMovie, 0, 0, kFullVolume);	CheckError(GetMoviesError(), "\pNewMovieTrack");	theMedia = NewTrackMedia(theTrack, SoundMediaType, FixRound((**sndDesc).sampleRate), nil, 0);	CheckError(GetMoviesError(), "\pNewTrackMedia");	err = BeginMediaEdits(theMedia);	CheckError(err, "\pBeginMediaEdits");	err = AddMediaSample(theMedia, sndHandle, sndDataOffset,// offset in data						 sndDataSize, 1,		// duration of each sound sample						 (SampleDescriptionHandle)sndDesc, numSamples, 0,// self-contained samples						 nil);	CheckError(err, "\pAddMediaSample");	err = EndMediaEdits(theMedia);	CheckError(err, "\pEndMediaEdits");	err = InsertMediaIntoTrack(theTrack, 0,		// track start time							   0,				// media start time							   GetMediaDuration(theMedia), fixed1);	CheckError(err, "\pInsertMediaIntoTrack");	if (sndDesc != nil)		DisposeHandle((Handle)sndDesc);}Boolean IsQuickTimeInstalled(void){	short error;	long result;	error = Gestalt(gestaltQuickTime, &result);	return (error == noErr);}