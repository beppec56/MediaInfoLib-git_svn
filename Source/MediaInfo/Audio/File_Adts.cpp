// File_Adts - Info for AAC (ADTS) files
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

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_ADTS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Adts.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const int32u ADTS_SamplingRate[]=
{96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
 16000, 12000, 11025,  8000,  7350,     0,     0,     0,};

//---------------------------------------------------------------------------
const char* ADTS_ID[]=
{
    "MPEG-4",
    "MPEG-2",
};

//---------------------------------------------------------------------------
const char* ADTS_Format_Profile[]=
{
    "Main",
    "LC",
    "SSR",
    "LTP",
};

//---------------------------------------------------------------------------
const char* ADTS_Profile[]=
{
    "A_AAC/MPEG4/MAIN",
    "A_AAC/MPEG4/LC",
    "A_AAC/MPEG4/SSR",
    "A_AAC/MPEG4/LTP",
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Adts::File_Adts()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;

    //Configuration
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;

    //In
    Frame_Count_Valid=32;

    //Temp
    Frame_Count=0;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Adts::Synchronize()
{
    //Tags
    bool Tag_Found;
    if (!File__Tags_Helper::Synchronize(Tag_Found))
        return false;
    if (Tag_Found)
        return true;

    //Synchronizing
    while (Buffer_Offset+6<=Buffer_Size)
    {
         while (Buffer_Offset+6<=Buffer_Size
             && (CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            Buffer_Offset++;

        if (Buffer_Offset+6<=Buffer_Size)//Testing if size is coherant
        {
            //Retrieving some info
            int16u aac_frame_length =(CC3(Buffer+Buffer_Offset+3)>>5)&0x1FFF;
            //Testing next start, to be sure
            if (1)//File_Offset+Buffer_Offset+aac_frame_length!=File_Size-File_EndTagSize)
            {
                if (/*IsSub && */Buffer_Offset+aac_frame_length==Buffer_Size)
                    break;

                if (Buffer_Offset+aac_frame_length+2>Buffer_Size)
                    return false; //Need more data

                //Testing
                if (aac_frame_length<=7 || (CC2(Buffer+Buffer_Offset+aac_frame_length)&0xFFF6)!=0xFFF0)
                    Buffer_Offset++;
                else
                    break; //while()
            }
            else
                Buffer_Offset++;
        }
    }
    if (Buffer_Offset+6>Buffer_Size)
    {
        //Parsing last bytes
        if (Buffer_Offset+5==Buffer_Size)
        {
            if ((CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            {
                Buffer_Offset++;
                if ((CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
                {
                    Buffer_Offset++;
                    if ((CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
                    {
                        Buffer_Offset++;
                        if ((CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
                        {
                            Buffer_Offset++;
                            if (CC1(Buffer+Buffer_Offset)!=0xFF)
                                Buffer_Offset++;
                        }
                    }
                }
            }
        }

        return false;
    }

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Adts::Synched_Test()
{
    //Tags
    if (!File__Tags_Helper::Synched_Test())
        return false;

    //Must have enough buffer for having header
    if (Buffer_Offset+2>Buffer_Size)
        return false;

    //Quick test of synchro
    if ((CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
        Synched=false;

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Adts::Header_Parse()
{
    //Parsing
    bool protection_absent;
    BS_Begin();
    Skip_BS(12,                                                 "syncword");
    Get_SB (    id,                                             "id"); Param_Info(ADTS_ID[id]);
    Skip_BS( 2,                                                 "layer");
    Get_SB (    protection_absent,                              "protection_absent");
    Get_S1 ( 2, profile_ObjectType,                             "profile_ObjectType"); Param_Info(ADTS_Profile[profile_ObjectType]);
    Get_S1 ( 4, sampling_frequency_index,                       "sampling_frequency_index"); Param_Info(ADTS_SamplingRate[sampling_frequency_index], " Hz");
    Skip_SB(                                                    "private");
    Get_S1 ( 3, channel_configuration,                          "channel_configuration");
    Skip_SB(                                                    "original");
    Skip_SB(                                                    "home");
    Skip_SB(                                                    "copyright_id");
    Skip_SB(                                                    "copyright_id_start");
    Get_S2 (13, aac_frame_length,                               "aac_frame_length");
    Get_S2 (11, adts_buffer_fullness,                           "adts_buffer_fullness"); Param_Info(adts_buffer_fullness==0x7FF?"VBR":"CBR");
    Skip_BS( 2,                                                 "num_raw_data_blocks");
    BS_End();

    //Filling
    Header_Fill_Size(aac_frame_length);
    Header_Fill_Code(0, "Frame");
}

//---------------------------------------------------------------------------
void File_Adts::Data_Parse()
{
    //Counting
    if (File_Offset+Buffer_Offset+Element_Size==File_Size)
        Frame_Count_Valid=Frame_Count; //Finalize frames in case of there are less than Frame_Count_Valid frames
    Frame_Count++;

    //Name
    Element_Info(Ztring::ToZtring(Frame_Count));

    //Parsing
    Skip_XX(Element_Size,                                       "Data");

    //Filling
    if (!IsDetected && Frame_Count>=Frame_Count_Valid)
        Data_Parse_Fill();
}

//---------------------------------------------------------------------------
void File_Adts::Data_Parse_Fill()
{
    //Filling
    int32u BitRate=(ADTS_SamplingRate[sampling_frequency_index]/1024)*aac_frame_length*8;
    File__Tags_Helper::Stream_Prepare(Stream_General);
    Fill(Stream_General, 0, General_Format, "ADTS");
    File__Tags_Helper::Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_Format, "AAC");
    Fill(Stream_Audio, 0, Audio_Format_Version, id?"Version 2":"Version 4");
    Fill(Stream_Audio, 0, Audio_Format_Profile, ADTS_Format_Profile[profile_ObjectType]);
    Fill(Stream_Audio, 0, Audio_Codec, ADTS_Profile[profile_ObjectType]);
    Fill(Stream_Audio, 0, Audio_SamplingRate, ADTS_SamplingRate[sampling_frequency_index]);
    Fill(Stream_Audio, 0, Audio_Channel_s_, channel_configuration);
    Fill(Stream_Audio, 0, Audio_MuxingMode, "ADTS");
    if (adts_buffer_fullness==0x7FF)
        Fill(Stream_Audio, 0, Audio_BitRate_Mode, "VBR");
    else
    {
        Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
        Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
    }
    Fill(Stream_Audio, 0, Audio_Resolution, 16);

    //No more need data
    File__Tags_Helper::Detected("ADTS");
}

} //NameSpace

#endif //MEDIAINFO_ADTS_YES

