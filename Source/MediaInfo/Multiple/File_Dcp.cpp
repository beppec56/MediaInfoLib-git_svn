// File_Dcp - Info for DCP (XML) files
// Copyright (C) 2011-2011 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_P2_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Dcp.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "ZenLib/Dir.h"
#include "ZenLib/FileName.h"
#include "ZenLib/TinyXml/tinyxml.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Dcp::File_Dcp()
:File__Analyze()
{
    //Temp
    ReferenceFiles=NULL;
}

//---------------------------------------------------------------------------
File_Dcp::~File_Dcp()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dcp::Streams_Finish()
{
    if (ReferenceFiles==NULL)
        return;

    ReferenceFiles->ParseReferences();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_Dcp::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    if (ReferenceFiles==NULL)
        return 0;

    return ReferenceFiles->Read_Buffer_Seek(Method, Value, ID);
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Dcp::FileHeader_Begin()
{
    //Element_Size
    if (File_Size<5 || File_Size>64*1024)
    {
        Reject("Dcp");
        return false; //Dcp XML files are not big
    }

    //Element_Size
    if (Buffer_Size<File_Size)
        return false; //Must wait for more data

    //XML header
    if (Buffer[0]!='<'
     || Buffer[1]!='?'
     || Buffer[2]!='x'
     || Buffer[3]!='m'
     || Buffer[4]!='l')
    {
        Reject("Dcp");
        return false;
    }

    TiXmlDocument document(File_Name.To_Local());
    if (document.LoadFile())
    {
        std::string NameSpace;
        TiXmlElement* AssetMap=document.FirstChildElement("AssetMap");
        if (AssetMap==NULL)
        {
            NameSpace="am:";
            AssetMap=document.FirstChildElement(NameSpace+"AssetMap");
        }
        if (AssetMap)
        {
            Accept("Dcp");
            Fill(Stream_General, 0, General_Format, "DCP");
            Fill(Stream_General, 0, General_Format_Version, NameSpace=="am:"?"SMPTE":"Interop");

            ReferenceFiles=new File__ReferenceFilesHelper(this, Config);

            TiXmlElement* AssetList=AssetMap->FirstChildElement(NameSpace+"AssetList");
            if (AssetList)
            {
                TiXmlElement* Asset=AssetList->FirstChildElement(NameSpace+"Asset");
                while (Asset)
                {
                    TiXmlElement* ChunkList=Asset->FirstChildElement(NameSpace+"ChunkList");
                    if (ChunkList)
                    {
                        TiXmlElement* Chunk=ChunkList->FirstChildElement(NameSpace+"Chunk");
                        if (Chunk)
                        {
                            TiXmlElement* Path=Chunk->FirstChildElement(NameSpace+"Path");
                            if (Path)
                            {
                                File__ReferenceFilesHelper::reference ReferenceFile;
                                ReferenceFile.FileNames.push_back(Path->GetText());
                                ReferenceFile.StreamID=Ztring::ToZtring(ReferenceFiles->References.size()+1);
                                ReferenceFiles->References.push_back(ReferenceFile);
                            }
                        }
                    }

                    Asset=Asset->NextSiblingElement();
                }
            }
        }
        else
        {
            Reject("Dcp");
            return false;
        }
    }
    else
    {
        Reject("Dcp");
        return false;
    }

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_P2_YES
