/************************************************************************
* database.cpp                                                          *
* - functions for loading and saving database files                     *
************************************************************************/

#include <windows.h>

#include "dasm.h"
#include "debug.h"
#include "disasm.h"
#include "gname.h"
#include "relocs.h"
#include "xref.h"
#include "data.h"
#include "exeload.h"
#include "schedule.h"
#include "resource.h"
#include "decrypt.h"

/************************************************************************
* forward declarations                                                  *
************************************************************************/
BOOL CALLBACK loadmessbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK savemessbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);

/************************************************************************
* savedecryptitems                                                      *
* - this saves the list of decryptors to the database                   *
************************************************************************/
bool savedecryptitems(savefile *sf)
{
	dword ndecs;
	ndecs=decrypter.numlistitems();
	decrypter.resetiterator();
	if(!sf->swrite(&ndecs,sizeof(dword)))
		return false;
	while(ndecs)
	{
		if(!decrypter.write_item(sf))
			return false;
		ndecs--;
	}
	return true;
}

/************************************************************************
* loaddecryptitems                                                      *
* - here we reload the list of decryptors                               *
************************************************************************/
bool loaddecryptitems(savefile *sf)
{
	dword ndecs,num;
	if(!sf->sread(&ndecs,sizeof(dword),&num))
		return false;
	while(ndecs)
	{
		if(!decrypter.read_item(sf))
			return false;
		ndecs--;
	}
	return true;
}

/************************************************************************
* retstacksavedb                                                        *
* - Borg keeps track of the stack even through saves. It simply saves   *
*   the full stack                                                      *
************************************************************************/
bool retstacksavedb(savefile *sf)
{
	if(!sf->swrite(&dio.retstack.stacktop,sizeof(int)))
		return false;
	if(!sf->swrite(dio.retstack.callstack,sizeof(lptr)*CALLSTACKSIZE))
		return false;
	return true;
}

/************************************************************************
* retstackloaddb                                                        *
* - reloads the stack from the saved database file                      *
************************************************************************/
bool retstackloaddb(savefile *sf)
{
	dword num;
	if(!sf->sread(&dio.retstack.stacktop,sizeof(int),&num))
		return false;
	if(!sf->sread(dio.retstack.callstack,sizeof(lptr)*CALLSTACKSIZE,&num))
		return false;
	return true;
}

/************************************************************************
* dissavedb                                                             *
* - this routine saves the entire disassembly database to the database  *
*   save file. It converts instruction pointers into instruction uids   *
*   and converts data pointers into offsets for saving                  *
************************************************************************/
bool dissavedb(savefile *sf,byte *filebuff)
{
	dword ndsms;
	dsmitemsave structsave;
	dsmitem *currdsm;
	ndsms=g_dsm.numlistitems();
	g_dsm.resetiterator();
	if(!sf->swrite(&ndsms,sizeof(dword)))
		return false;
	while(ndsms)
	{
		currdsm=g_dsm.nextiterator();
		structsave.addr=currdsm->addr;
		structsave.type=currdsm->type;
		structsave.length=currdsm->length;
		structsave.modrm=currdsm->modrm;
		structsave.mode32=currdsm->mode32;
		structsave.override=currdsm->override;
		structsave.flags=currdsm->flags;
		structsave.displayflags=currdsm->displayflags;
		if(structsave.type==dsmxref)
		{
			structsave.fileoffset=0;  // NULL ptrs
			structsave.tptroffset=0;
		}
		else if(structsave.type==dsmcode)
		{
			structsave.fileoffset=currdsm->data-filebuff;
			structsave.tptroffset=((asminstdata *)currdsm->tptr)->uniqueid;
		}
		else
		{
			structsave.fileoffset=strlen((char *)currdsm->data)+1; // strlen
			structsave.tptroffset=0; // points to str as well
		}
		if(!sf->swrite(&structsave,sizeof(dsmitemsave)))
			return false;
		if((structsave.type!=dsmxref)&&(structsave.type!=dsmcode))
		{
			if(!sf->swrite(currdsm->tptr,structsave.fileoffset))
				return false;
		}
		ndsms--;
	}
	// need to save callstack and some other stuff too.
	if(!sf->swrite(&dio.curraddr,sizeof(lptr)))
		return false;
	if(!sf->swrite(&dio.subitem,sizeof(dsmitemtype)))
		return false;
	if(!sf->swrite(&g_dsm.itables,sizeof(int)))
		return false;
	if(!sf->swrite(&g_dsm.jtables,sizeof(int)))
		return false;
	if(!sf->swrite(&g_dsm.irefs,sizeof(int)))
		return false;
	return retstacksavedb(sf);
}

/************************************************************************
* disloaddb                                                             *
* - this routine loads the entire disassembly database from the save    *
*   file It converts instruction uids into the instruction pointers and *
*   converts offsets back into pointers. We have to search the assembly *
*   instructions for the uids in order to find the correct instruction. *
************************************************************************/
bool disloaddb(savefile *sf,byte *filebuff)
{
	dword ndsms,num;
	dsmitemsave structsave;
	dsmitem *currdsm;
	int asminstctr;
	asminstdata *findasm;
	if(!sf->sread(&ndsms,sizeof(dword),&num))
		return false;
	while(ndsms)
	{
		currdsm=new dsmitem;
		if(!sf->sread(&structsave,sizeof(dsmitemsave),&num))
			return false;
		currdsm->addr=structsave.addr;
		currdsm->type=structsave.type;
		currdsm->length=structsave.length;
		currdsm->modrm=structsave.modrm;
		currdsm->mode32=structsave.mode32;
		currdsm->override=structsave.override;
		currdsm->flags=structsave.flags;
		currdsm->displayflags=structsave.displayflags;
		if(structsave.type==dsmxref)
		{
			currdsm->data=NULL;
			currdsm->tptr=NULL;
		}
		else if(structsave.type==dsmcode)
		{
			currdsm->data=structsave.fileoffset+filebuff;
			// now reset the tptr = asminstdata ptr (need to find it from the uniqueid)
			asminstctr=0;
			findasm=reconstruct[asminstctr];
			while((dword)(findasm[0].uniqueid/1000)!=(dword)(structsave.tptroffset/1000))
			{
				asminstctr++;
				findasm=reconstruct[asminstctr];
				if(findasm==NULL)
				{
#ifdef DEBUG
					DebugMessage("File Loader:Failed to find instruction table for %lu",structsave.tptroffset);
#endif
					return false;
				}
			}
			asminstctr=0;
			while(findasm[asminstctr].uniqueid!=structsave.tptroffset)
			{
				asminstctr++;
				if((!findasm[asminstctr].instbyte)&&(!findasm[asminstctr].processor))
				{
#ifdef DEBUG
					DebugMessage("File Loader:Failed to find instruction %lu",structsave.tptroffset);
#endif
					return false;
				}
			}
			currdsm->tptr=(void *)&findasm[asminstctr];
		}
		else
		{
			currdsm->data=new byte[structsave.fileoffset];
			currdsm->tptr=currdsm->data;
		}
		if((structsave.type!=dsmxref)&&(structsave.type!=dsmcode))
		{
			if(!sf->sread(currdsm->tptr,structsave.fileoffset,&num))
				return false;
		}
		g_dsm.addto(currdsm);
		ndsms--;
	}
	// need to save callstack and some other stuff too.
	if(!sf->sread(&dio.curraddr,sizeof(lptr),&num))
		return false;
	if(!sf->sread(&dio.subitem,sizeof(dsmitemtype),&num))
		return false;
	if(!sf->sread(&g_dsm.itables,sizeof(int),&num))
		return false;
	if(!sf->sread(&g_dsm.jtables,sizeof(int),&num))
		return false;
	if(!sf->sread(&g_dsm.irefs,sizeof(int),&num))
		return false;
	g_dsm.dissettable();
	dio.setcuraddr(dio.curraddr);
	return retstackloaddb(sf);
}

/************************************************************************
* saverelocitems                                                        *
* - this saves the relocs list to the database file.                    *
* - we can simply save the number of items followed by each item        *
************************************************************************/
bool saverelocitems(savefile *sf)
{
	dword nrels;
	nrels=reloc.numlistitems();
	reloc.resetiterator();
	// save number of reloc items
	if(!sf->swrite(&nrels,sizeof(dword)))
		return false;
	while(nrels)
	{
		if(!reloc.write_item(sf))
			return false;
		nrels--;
	}
	return true;
}

/************************************************************************
* loadrelocitems                                                        *
* - this reloads the list of relocs from the database file and          *
*   constructs the list again                                           *
************************************************************************/
bool loadrelocitems(savefile *sf)
{
	dword nrels,num;
	// get number of items
	if(!sf->sread(&nrels,sizeof(dword),&num))
		return false;
	while(nrels)
	{
		if(!reloc.read_item(sf))
			return false;
		nrels--;
	}
	return true;
}

/************************************************************************
* gnamesavedb                                                           *
* - saves all the names in the list to the database file being saved.   *
*   this is in a one-pass compatible loading format. ie number of items *
*   followed by each item, and for strings the length of the string     *
*   followed by the string.                                             *
************************************************************************/
bool gnamesavedb(gname *gn,savefile *sf)
{
	dword nexps,nlen;
	gnameitem *currexp;
	nexps=gn->numlistitems();
	gn->resetiterator();
	if(!sf->swrite(&nexps,sizeof(dword)))
		return false;
	while(nexps)
	{
		currexp=gn->nextiterator();
		if(!sf->swrite(&(currexp->addr),sizeof(lptr)))
			return false;
		nlen=strlen(currexp->name)+1;
		if(!sf->swrite(&nlen,sizeof(dword)))
			return false;
		if(!sf->swrite(currexp->name,nlen))
			return false;
		nexps--;
	}
	return true;
}

/************************************************************************
* gnameloaddb                                                           *
* - loads the names from the database file and reconstructs the names   *
*   list                                                                *
************************************************************************/
bool gnameloaddb(gname *gn,savefile *sf)
{
	dword nexps,num,nlen;
	gnameitem *currexp;
	if(!sf->sread(&nexps,sizeof(dword),&num))
		return false;
	while(nexps)
	{
		currexp=new gnameitem;
		if(!sf->sread(&(currexp->addr),sizeof(lptr),&num))
			return false;
		if(!sf->sread(&nlen,sizeof(dword),&num))
			return false;
		currexp->name=new char[nlen];
		if(!sf->sread(currexp->name,nlen,&num))
			return false;
		gn->addto(currexp);
		nexps--;
	}
	return true;
}

/************************************************************************
* savedatasegitems                                                      *
* - we save the data segment data structures to the database file.      *
************************************************************************/
bool savedatasegitems(savefile *sf,byte *filebuff)
{
	dword nsegs;
	nsegs=dta.numlistitems();
	dta.resetiterator();
	if(!sf->swrite(&nsegs,sizeof(dword)))
		return false;
	while(nsegs)
	{
		if(!dta.write_item(sf,filebuff))
			return false;
		nsegs--;
	}
	return true;
}

/************************************************************************
* loaddatasegitems                                                      *
* - loads the data segment data structures in                           *
************************************************************************/
bool loaddatasegitems(savefile *sf,byte *filebuff)
{
	dword nsegs,num;
	if(!sf->sread(&nsegs,sizeof(dword),&num))
		return false;
#ifdef DEBUG
	DebugMessage("Loading %lu datasegs",nsegs);
#endif
	while(nsegs)
	{
		if(!dta.read_item(sf,filebuff))
			return false;
		nsegs--;
	}
	return true;
}

/************************************************************************
* xrefsavedb                                                            *
* save xref list to database file, simply writes the list item out      *
* consisting of loc and ref_by, ie two addresses                        *
************************************************************************/
bool xrefsavedb(savefile *sf)
{
	dword nxrefs;
	xrefitem *currxref;
	nxrefs=xrefs.numlistitems();
	xrefs.resetiterator();
	if(!sf->swrite(&nxrefs,sizeof(dword)))
		return false;
	while(nxrefs)
	{
		currxref=xrefs.nextiterator();
		if(!sf->swrite(currxref,sizeof(xrefitem)))
			return false;
		nxrefs--;
	}
	return true;
}

/************************************************************************
* xrefloaddb                                                            *
* load xref list to database file, simply reads the list item in        *
* consisting of loc and ref_by, ie two addresses                        *
* and adds it to the new list                                           *
************************************************************************/
bool xrefloaddb(savefile *sf)
{
	dword nxrefs,num;
	xrefitem *currxref;
	if(!sf->sread(&nxrefs,sizeof(dword),&num))
		return false;
	while(nxrefs)
	{
		currxref=new xrefitem;
		if(!sf->sread(currxref,sizeof(xrefitem),&num))
			return false;
		xrefs.addto(currxref);
		nxrefs--;
	}
	return true;
}

/************************************************************************
* savedbcoord                                                           *
* - coordinates saving of the databases when save as database file is   *
*   chosen in Borg                                                      *
************************************************************************/
void savedbcoord(char *fname,char *exename)
{
	savefile sf;
	dword flen;
	dword bver;
	HWND sbox;
	// open file
	sbox=CreateDialog(g_hInst,MAKEINTRESOURCE(save_box),g_hMainWnd,(DLGPROC)savemessbox);
	if(!sf.sopen(fname,GENERIC_WRITE,1,CREATE_ALWAYS,0))
	{
		DestroyWindow(sbox);
		return;
	}
	// save header to identify as a database file
	if(!sf.swrite("BORG",4))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Header Info[1]:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// save BORG_VERSION
	bver=BORG_VER;
	if(!sf.swrite(&bver,sizeof(bver)))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Header Info[2]:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// save filename of exe file.
	flen=strlen(exename)+1;
	if(!sf.swrite(&flen,sizeof(dword)))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Header Info[3]:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	if(!sf.swrite(exename,flen))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Header Info[4]:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// save options.
	if(!sf.swrite(&g_options,sizeof(globaloptions)))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Options:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	if(!sf.swrite(&g_floader.exetype,sizeof(int)))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Exetype:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// save segment info
	if(!savedatasegitems(&sf,g_floader.fbuff))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Segments:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// save import info
	if(!gnamesavedb(&import,&sf))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Imports:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// save export info
	if(!gnamesavedb(&expt,&sf))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Exports:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// save names
	if(!gnamesavedb(&name,&sf))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Names:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// save relocs
	if(!saverelocitems(&sf))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Relocs:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// save xrefs
	if(!xrefsavedb(&sf))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Xrefs:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// save asm database
	if(!dissavedb(&sf,g_floader.fbuff))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Database:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// save decrypter list
	if(!savedecryptitems(&sf))
	{
		DestroyWindow(sbox);
		MessageBox(g_hMainWnd,"Decryptors:File write failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	sf.flushfilewrite();
	// close file
	DestroyWindow(sbox);
}

/************************************************************************
* loaddbcoord                                                           *
* - coordinates loading of the databases when load database file is     *
*   chosen in Borg                                                      *
************************************************************************/
bool loaddbcoord(char *fname,char *exename)
{
	savefile sf;
	char tbuff[20];
	dword num;
	dword flen,fsize,gsize;
	dword bver;
	HWND lbox;

	// open file
	lbox=CreateDialog(g_hInst,MAKEINTRESOURCE(load_box),g_hMainWnd,(DLGPROC)loadmessbox);
	if(!sf.sopen(fname,GENERIC_READ,1,OPEN_EXISTING,0))
	{
		DestroyWindow(lbox);
		return false;
	}
	// load header check its a database file
	tbuff[4]=0;
	if(!sf.sread(tbuff,4,&num))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
	if(strcmp(tbuff,"BORG"))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Not A Borg Database File",fname,MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
	// read BORG_VERSION
	if(!sf.sread(&bver,sizeof(bver),&num))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
	if(bver>BORG_VER)
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Savefile from a later version [or very early version]!",fname,MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
	if(bver<BORG_VER)
	{
		MessageBox(g_hMainWnd,"Warning:earlier version savefile [will attempt load]",fname,MB_OK|MB_ICONEXCLAMATION);
#ifdef DEBUG
		DebugMessage("Detected version:%d.%d Savefile",bver/100,bver%100);
#endif
	}
	// load filename of exe file.
	flen=0;
	if(!sf.sread(&flen,sizeof(dword),&num))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
	if(!sf.sread(exename,flen,&num))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
#ifdef DEBUG
	DebugMessage("Filename:%s",exename);
#endif
	// added 226 to allow load of older databases
	gsize=sizeof(globaloptions);
	if(bver<222)
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Older databases are incompatible, please reuse the older version of Borg","Prior to 2.22",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
	// load options.
	if(!sf.sread(&g_options,gsize,&num))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
#ifdef DEBUG
	DebugMessage("Global Options Size:%d",gsize);
#endif
	if(!sf.sread(&g_floader.exetype,sizeof(int),&num))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
	// load exe file.
	// - any errors from here on will now be fatal, and Borg will need to exit
	g_floader.efile=CreateFile(exename,GENERIC_READ|GENERIC_WRITE,1,NULL,OPEN_EXISTING,0,NULL);
	if(g_floader.efile==INVALID_HANDLE_VALUE)
	{
		g_floader.efile=CreateFile(exename,GENERIC_READ,1,NULL,OPEN_EXISTING,0,NULL);
		if(g_floader.efile==INVALID_HANDLE_VALUE)
		{
			DestroyWindow(lbox);
			MessageBox(g_hMainWnd,"File open failed ?",exename,MB_OK|MB_ICONEXCLAMATION);
			return false;
		}
		g_options.readonly=true;
		MessageBox(g_hMainWnd,"Couldn't obtain write permission to file\nFile opened readonly - will not be able to apply any patches",
			"Borg Message",MB_OK);
	}
	if(GetFileType(g_floader.efile)!=FILE_TYPE_DISK)
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"File open failed ?",exename,MB_OK|MB_ICONEXCLAMATION);
		CloseHandle(g_floader.efile);
		return false;
	}
	fsize=GetFileSize(g_floader.efile,NULL);
	g_floader.fbuff=new byte[fsize];
	SetFilePointer(g_floader.efile,0x00,NULL,FILE_BEGIN);
	if(!ReadFile(g_floader.efile,g_floader.fbuff,fsize,&num,NULL))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"File read failed ?",exename,MB_OK|MB_ICONEXCLAMATION);
		CloseHandle(g_floader.efile);
		delete g_floader.fbuff;
		return false;
	}
	// load segment info
	if(!loaddatasegitems(&sf,g_floader.fbuff))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Fatal Error\nSegments:File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		DestroyWindow(g_hMainWnd);
		return false;
	}
#ifdef DEBUG
	DebugMessage("Loading Imports");
#endif
	// load import info
	if(!gnameloaddb(&import,&sf))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Fatal Error\nImports:File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		DestroyWindow(g_hMainWnd);
		return false;
	}
#ifdef DEBUG
	DebugMessage("Loading Exports");
#endif
	// load export info
	if(!gnameloaddb(&expt,&sf))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Fatal Error\nExports:File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		DestroyWindow(g_hMainWnd);
		return false;
	}
#ifdef DEBUG
	DebugMessage("Loading Names");
#endif
	// load names
	if(!gnameloaddb(&name,&sf))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Fatal Error\nNames:File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		DestroyWindow(g_hMainWnd);
		return false;
	}
#ifdef DEBUG
	DebugMessage("Loading Relocs");
#endif
	// load relocs
	if(!loadrelocitems(&sf))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Fatal Error\nRelocs:File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		DestroyWindow(g_hMainWnd);
		return false;
	}
#ifdef DEBUG
	DebugMessage("Loading Xrefs");
#endif
	// load xrefs
	if(!xrefloaddb(&sf))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Fatal Error\nXrefs:File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		DestroyWindow(g_hMainWnd);
		return false;
	}
#ifdef DEBUG
	DebugMessage("Loading Asm database");
#endif
	// load asm database
	if(!disloaddb(&sf,g_floader.fbuff))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Fatal Error\nDatabase:File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		DestroyWindow(g_hMainWnd);
		return false;
	}
#ifdef DEBUG
	DebugMessage("Relocating File");
#endif
	if(!reloc.relocfile())
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Fatal Error\nRelocating File",fname,MB_OK|MB_ICONEXCLAMATION);
		DestroyWindow(g_hMainWnd);
		return false;
	}
#ifdef DEBUG
	DebugMessage("Loading Decryptors");
#endif
	if(!loaddecryptitems(&sf))
	{
		DestroyWindow(lbox);
		MessageBox(g_hMainWnd,"Fatal Error\nDecryptors:File read failed ?",fname,MB_OK|MB_ICONEXCLAMATION);
		DestroyWindow(g_hMainWnd);
		return false;
	}
	DestroyWindow(lbox);
	return true;
}

/************************************************************************
* savedb                                                                *
* - the first place of call when save as database is selected.          *
* - asks the user to select a file before calling the fileloader savedb *
*   which is where the save to database is controlled from              *
************************************************************************/
void savedb(void)
{
	char szFile[MAX_PATH*2];
	if(g_scheduler.sizelist())
	{
		MessageBox(g_hMainWnd,"There are still items to process yet","Borg Warning",MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	getfiletosave(szFile);
	if(szFile[0])
	{
		savedbcoord(szFile,current_exe_name);
	}
}

/************************************************************************
* loaddb                                                                *
* - the first place of call when load from database is selected.        *
* - asks the user to select a file before calling the fileloader loaddb *
*   which is where the load from database is controlled from            *
* - starts up the secondary thread when the file is loaded              *
************************************************************************/
void loaddb(void)
{
	char szFile[MAX_PATH*2];
	getfiletoload(szFile);
	if(szFile[0])
	{
		if(loaddbcoord(szFile,current_exe_name))
		{
			StatusMessage("File Opened");
			strcat(winname," : ");
			strcat(winname,current_exe_name);
			SetWindowText(g_hMainWnd,winname);
			InThread=true;
			ThreadHandle=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Thread,0,0,&ThreadId);
			changemenus();
			g_scheduler.addtask(scrolling,priority_userrequest,nlptr,NULL);
		}
		else
			MessageBox(g_hMainWnd,"File open failed ?",program_name,MB_OK|MB_ICONEXCLAMATION);
	}
}

/************************************************************************
* savemessbox                                                           *
* - A small dialog box which contains the message 'saving' to be shown  *
*   as a database file is saved                                         *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK savemessbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		CenterWindow(hdwnd);
		return false;
	default:
		break;
	}
	return false;
}
#ifdef __BORLANDC__
#pragma warn +par
#endif

/************************************************************************
* loadmessbox                                                           *
* - A small dialog box which contains the message 'loading' to be shown *
*   as a database file is loaded                                        *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK loadmessbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		CenterWindow(hdwnd);
		return false;
	default:
		break;
	}
	return false;
}
#ifdef __BORLANDC__
#pragma warn +par
#endif


