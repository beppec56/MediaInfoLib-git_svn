// MediaInfo_Internal - All information about media files
// Copyright (C) 2002-2009 Jerome Martinez, Zen@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Give information about a lot of media files
// Dispatch the file to be tested by all containers
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_InternalH
#define MediaInfo_InternalH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal_Const.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "ZenLib/Thread.h"
#include "ZenLib/CriticalSection.h"
#include <bitset>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class File__Analyze;
class Internet__Base;

//***************************************************************************
/// @brief MediaInfo_Internal
/// @version 0.7
//***************************************************************************

class MediaInfo_Internal : public ZenLib::Thread
{
public :
    //Constructor/Destructor
    MediaInfo_Internal ();
    ~MediaInfo_Internal ();

    //File
    size_t Open (const String &File_Name);
    size_t Open (const ZenLib::int8u* Begin, size_t Begin_Size, const ZenLib::int8u* End=NULL, size_t End_Size=0, ZenLib::int64u File_Size=0);
    size_t Open_Buffer_Init (ZenLib::int64u File_Size=(ZenLib::int64u)-1, const String &File_Name=String());
    size_t Open_Buffer_Init (ZenLib::int64u File_Size, ZenLib::int64u File_Offset);
    std::bitset<32> Open_Buffer_Continue (const ZenLib::int8u* Buffer, size_t Buffer_Size);
    ZenLib::int64u Open_Buffer_Continue_GoTo_Get ();
    bool   Open_Buffer_Position_Set(int64u File_Offset);
    size_t Open_Buffer_Finalize ();
    void Close ();

    //General information
    String  Inform ();
    String  Inform (stream_t StreamKind, size_t StreamNumber=0); //All about only a specific stream

    //Get
    String Get (stream_t StreamKind, size_t StreamNumber, size_t Parameter, info_t InfoKind=Info_Text);
    String Get (stream_t StreamKind, size_t StreamNumber, const String &Parameter, info_t InfoKind=Info_Text, info_t SearchKind=Info_Name);

    //Set
    size_t Set (const String &ToSet, stream_t StreamKind, size_t StreamNumber, size_t Parameter, const String &OldValue=_T(""));
    size_t Set (const String &ToSet, stream_t StreamKind, size_t StreamNumber, const String &Parameter, const String &OldValue=_T(""));

    //Output_Buffered
    size_t Output_Buffer_Get (const String &Value);
    size_t Output_Buffer_Get (size_t Pos);

    //Info
    String Option (const String &Option, const String &Value=_T(""));
    size_t State_Get ();
    size_t Count_Get (stream_t StreamKind, size_t StreamNumber=(size_t)-1);

    //Internal
    static bool LibraryIsModified(); //Is the library has been modified? (#defines...)

private :
    friend class File_Bdmv;  //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_Cdxa;  //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_Mpeg4; //Theses classes need access to internal structure for optimization. There is recursivity with theses formats

    //Parsing handles
    File__Analyze*  Info;
    Internet__Base* Internet;
    Ztring          File_Name;

    //Helpers
    void CreateDummy (const String& Value); //Create dummy Information
    MediaInfo_Internal(const MediaInfo_Internal&); // Copy Constructor

    //Open Buffer
    bool Info_IsMultipleParsing;

    //Config
    std::vector<std::vector<ZtringList> > Stream;
    std::vector<std::vector<ZtringListList> > Stream_More;
    Ztring Details;
    void Traiter(Ztring &C); //enleve les $if...

public :
    bool SelectFromExtension (const String &Parser); //Select File_* from the parser name
    int  ListFormats(const String &File_Name=String());
    MediaInfo_Config_MediaInfo Config;

private :
    //Threading
    size_t  BlockMethod; //Open() return: 0=immedialtly, 1=after local info, 2=when user interaction is needed
    bool    IsInThread;
    void    Entry();
    ZenLib::CriticalSection CS;
};

} //NameSpace
#endif
