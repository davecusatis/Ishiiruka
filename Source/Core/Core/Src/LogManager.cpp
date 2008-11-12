// Copyright (C) 2003-2008 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#include <stdio.h>
#include <wx/datetime.h> // for the timestamps

#include "Common.h"
#include "StringUtil.h"
#include "LogManager.h"
#include "PowerPC/PowerPC.h"
#include "PowerPC/SymbolDB.h" // for g_symbolDB
#include "Debugger/Debugger_SymbolMap.h"


LogManager::SMessage	(*LogManager::m_Messages)[MAX_MESSAGES];
int						LogManager::m_nextMessages[LogManager::VERBOSITY_LEVELS + 1];

CDebugger_Log*			LogManager::m_Log[LogTypes::NUMBER_OF_LOGS + (LogManager::VERBOSITY_LEVELS * 100)];
int						LogManager::m_activeLog = LogTypes::MASTER_LOG;
bool					LogManager::m_bDirty = true;
bool					LogManager::m_bInitialized = false;


void __Log(int log, const char *format, ...)
{
	char* temp = (char*)alloca(strlen(format)+512);
	va_list args;
	va_start(args, format);
	CharArrayFromFormatV(temp, 512, format, args);
	va_end(args);
	LogManager::Log((LogTypes::LOG_TYPE)log, temp);
}

CDebugger_Log::CDebugger_Log(const char* _szShortName, const char* _szName, int a) :
	m_bLogToFile(true), // write to file or not
	m_bShowInLog(false),
	m_bEnable(false),
	m_pFile(NULL)
{
	strcpy((char*)m_szName, _szName);
	strcpy((char*)m_szShortName_, _szShortName);
	sprintf((char*)m_szShortName, "%s%i", _szShortName, a);
	sprintf((char*)m_szFilename, FULL_LOGS_DIR "%s%i.txt", _szName, a);

	unlink(m_szFilename);
}

CDebugger_Log::~CDebugger_Log(void)
{
	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

// we may need to declare these
CDebugger_LogSettings::CDebugger_LogSettings() {}
CDebugger_LogSettings::~CDebugger_LogSettings(void) {}

void CDebugger_Log::Init()
{
	m_pFile = fopen(m_szFilename, "wtb");
}

void CDebugger_Log::Shutdown() 
{
	if (m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}


void LogManager::Init()
{
	m_Messages = new SMessage[LogManager::VERBOSITY_LEVELS + 1][MAX_MESSAGES];
	m_bDirty = true;

	// create log files
	for(int i = 0; i <= LogManager::VERBOSITY_LEVELS; i++)
	{
		m_Log[LogTypes::MASTER_LOG + i*100]			= new CDebugger_Log("*",   "Master Log", i);
		m_Log[LogTypes::BOOT + i*100]				= new CDebugger_Log("BOOT", "Boot", i);
		m_Log[LogTypes::PIXELENGINE + i*100]		= new CDebugger_Log("PE",  "PixelEngine", i);
		m_Log[LogTypes::COMMANDPROCESSOR + i*100]	= new CDebugger_Log("CP",  "CommandProc", i);
		m_Log[LogTypes::VIDEOINTERFACE + i*100]		= new CDebugger_Log("VI",  "VideoInt", i);
		m_Log[LogTypes::SERIALINTERFACE + i*100]	= new CDebugger_Log("SI",  "SerialInt", i);
		m_Log[LogTypes::PERIPHERALINTERFACE + i*100]= new CDebugger_Log("PI",  "PeripheralInt", i);
		m_Log[LogTypes::MEMMAP + i*100]				= new CDebugger_Log("MI",  "MI & memmap", i);
		m_Log[LogTypes::STREAMINGINTERFACE + i*100] = new CDebugger_Log("Stream", "StreamingInt", i);
		m_Log[LogTypes::DSPINTERFACE + i*100]		= new CDebugger_Log("DSP", "DSPInterface", i);
		m_Log[LogTypes::DVDINTERFACE + i*100]		= new CDebugger_Log("DVD", "DVDInterface", i);
		m_Log[LogTypes::GPFIFO + i*100]				= new CDebugger_Log("GP",  "GPFifo", i);
		m_Log[LogTypes::EXPANSIONINTERFACE + i*100]	= new CDebugger_Log("EXI", "ExpansionInt", i);
		m_Log[LogTypes::AUDIO_INTERFACE + i*100]	= new CDebugger_Log("AI", "AudioInt", i);
		m_Log[LogTypes::GEKKO + i*100]				= new CDebugger_Log("GEKKO", "IBM CPU", i);
		m_Log[LogTypes::HLE + i*100]				= new CDebugger_Log("HLE", "HLE", i);
		m_Log[LogTypes::DSPHLE + i*100]			    = new CDebugger_Log("DSPHLE", "DSP HLE", i);
		m_Log[LogTypes::VIDEO + i*100]			    = new CDebugger_Log("Video", "Video Plugin", i);
		m_Log[LogTypes::AUDIO + i*100]			    = new CDebugger_Log("Audio", "Audio Plugin", i);
		m_Log[LogTypes::DYNA_REC + i*100]			= new CDebugger_Log("DYNA", "Dynamic Recompiler", i);
		m_Log[LogTypes::OSREPORT + i*100]			= new CDebugger_Log("OSREPORT", "Dolphin Console", i);
		m_Log[LogTypes::CONSOLE + i*100]			= new CDebugger_Log("CONSOLE", "OSReport", i);		
		m_Log[LogTypes::WII_IOB + i*100]			= new CDebugger_Log("WII_IOB",			"WII IO Bridge", i);
		m_Log[LogTypes::WII_IPC + i*100]			= new CDebugger_Log("WII_IPC",			"WII IPC", i);
		m_Log[LogTypes::WII_IPC_HLE + i*100]		= new CDebugger_Log("WII_IPC_HLE",		"WII IPC HLE", i);
		m_Log[LogTypes::WII_IPC_DVD + i*100]		= new CDebugger_Log("WII_IPC_DVD",		"WII IPC DVD", i);
		m_Log[LogTypes::WII_IPC_ES + i*100]			= new CDebugger_Log("WII_IPC_ES",		"WII IPC ES", i);
		m_Log[LogTypes::WII_IPC_FILEIO + i*100]		= new CDebugger_Log("WII_IPC_FILEIO",	"WII IPC FILEIO", i);
		m_Log[LogTypes::WII_IPC_NET + i*100]		= new CDebugger_Log("WII_IPC_NET",		"WII IPC NET", i);
		m_Log[LogTypes::WII_IPC_WIIMOTE + i*100]    = new CDebugger_Log("WII_IPC_WIIMOTE",	"WII IPC WIIMOTE", i);

		m_nextMessages[i] = 0; // initiate to zero
	}

	// create the files
	for (int i = 0; i < LogTypes::NUMBER_OF_LOGS; i++)
	{
		for (int j = 0; j <= LogManager::VERBOSITY_LEVELS; j++)
		{
			m_Log[j*100 + i]->Init();
		}
	}	
	m_bInitialized = true;
}


void LogManager::Clear()
{
	for (int v = 0; v <= LogManager::VERBOSITY_LEVELS; v++)
	{
		for (int i = 0; i < MAX_MESSAGES; i++)
		{
			strcpy(m_Messages[v][i].m_szMessage,"");
			m_Messages[v][i].m_dwMsgLen = 0;
			m_Messages[v][i].m_bInUse = false;
		}
		m_nextMessages[v] = 0;
	}
}

// __________________________________________________________________________________________________
// Shutdown
//
void LogManager::Shutdown()
{
	m_bInitialized = false;

	// delete all loggers
	for (int i=0; i<LogTypes::NUMBER_OF_LOGS; i++)
	{
		if (m_Log[i] != NULL)
		{
			m_Log[i]->Shutdown();
			delete m_Log[i];
			m_Log[i] = NULL;
		}
	}

	delete [] m_Messages;
}


// ==========================================================================================
// The function that finally writes the log.
// ---------------
u32 lastPC;
std::string lastSymbol;
void LogManager::Log(LogTypes::LOG_TYPE _type, const char *_fmt, ...)
{
	if (m_LogSettings == NULL)
		return;

	// declarations
	int v; // verbosity level
	int type; // the log type, CONSOLE etc.
	char cvv[20];
	std::string svv;

	// get the current verbosity level and type
	sprintf(cvv, "%03i", (int)_type);
	svv = cvv;
	v = atoi(svv.substr(0, 1).c_str());
	type = atoi(svv.substr(1, 2).c_str());

	// security checks
	if (m_Log[_type] == NULL || !m_Log[_type]->m_bEnable || PC == 0
		|| _type > (LogTypes::NUMBER_OF_LOGS + LogManager::VERBOSITY_LEVELS * 100)
		|| _type < 0)
		return;

	// prepare message
	char Msg[512];
	va_list ap;
	va_start(ap, _fmt);
	vsprintf(Msg, _fmt, ap);
	va_end(ap);

	static u32 count = 0;
	wxDateTime datetime = wxDateTime::UNow(); // get timestamp

	char* Msg2 = (char*)alloca(strlen(_fmt)+512);

	// Here's the old symbol request
	//Debugger::FindSymbol(PC);
	// const Debugger::Symbol& symbol = Debugger::GetSymbol(Index);
	//symbol.GetName().c_str(),

	// Warning: Getting the function name this often is very demanding on the CPU.
	// I have limited it to the two lowest verbosity levels because of that. I've also
	// added a simple caching function so that we don't search again if we get the same
	// question again.
	std::string symbol;
	
	if ((v == 0 || v == 1) && lastPC != PC && LogManager::m_LogSettings->bResolve)
	{
		symbol = g_symbolDB.GetDescription(PC);
		lastSymbol = symbol;
		lastPC = PC;
	}
	else if(lastPC == PC && LogManager::m_LogSettings->bResolve)
	{
		symbol = lastSymbol;
	}
	else
	{
		symbol = "---";
	}

	int Index = 1;
	const char *eol = "\n";
	if (Index > 0)
	{		
		sprintf(Msg2, "%i %02i:%02i:%03i: %x %s (%s, %08x) : %s%s", 
			++count,
			datetime.GetMinute(), datetime.GetSecond(), datetime.GetMillisecond(),
			PowerPC::ppcState.DebugCount, 
			m_Log[_type]->m_szShortName_, // (CONSOLE etc)		
			symbol.c_str(), PC, // current PC location (name, address)
			Msg, eol);
	}

	// ==========================================================================================
	// Level 0 verbosity logs will be written to all verbosity levels. Given that logging is enabled
	// for that level. Level 1 verbosity will only be written to level 1, 2, 3 and so on.
	// ---------------
	int id;
	for (int i = LogManager::VERBOSITY_LEVELS; i >= v ; i--)
	{
		// prepare the right id
		id = i*100 + type;

		// write to memory
		m_Messages[i][m_nextMessages[i]].Set((LogTypes::LOG_TYPE)id, Msg2);

		// ----------------------------------------------------------------------------------------
		// write to file
		// ---------------
		if (m_Log[id]->m_pFile && m_Log[id]->m_bLogToFile)
			fprintf(m_Log[id]->m_pFile, "%s", Msg2);
		if (m_Log[i*100 + LogTypes::MASTER_LOG] && m_Log[i*100 +  LogTypes::MASTER_LOG]->m_pFile
				&& LogManager::m_LogSettings->bWriteMaster)
			fprintf(m_Log[i*100 +  LogTypes::MASTER_LOG]->m_pFile, "%s", Msg2);

		printf("%s", Msg2); // write to console screen

		// this limits the memory space used for the memory logs to MAX_MESSAGES rows
		m_nextMessages[i]++;
		if (m_nextMessages[i] >= MAX_MESSAGES)
			m_nextMessages[i] = 0;		
		// ---------------
	}
	m_bDirty = true; // tell LogWindow that the log has been updated
}

bool IsLoggingActivated()
{
#ifdef LOGGING
	return true;
#else
	return false;
#endif
}
