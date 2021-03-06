/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

#include <precomp.h>
#include "pagemake.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/cp_ce.hxx>
#include <ary/loc/loc_file.hxx>
#include <display/corframe.hxx>
#include "hd_chlst.hxx"
#include "hd_docu.hxx"
#include "hdimpl.hxx"
#include "html_kit.hxx"
#include "navibar.hxx"
#include "opageenv.hxx"
#include "outfile.hxx"
#include "pm_aldef.hxx"
#include "pm_class.hxx"
#include "pm_help.hxx"
#include "pm_index.hxx"
#include "pm_namsp.hxx"
#include "pm_start.hxx"
#include "strconst.hxx"


using namespace csi;
using csi::html::Link;
using csi::html::HorizontalLine;


const int C_nNrOfIndexLetters = 27;


template <class SPECIAL_MAKER>
inline void
Make_SpecialPage( DYN SPECIAL_MAKER * let_dpMaker )
{
    Dyn< SPECIAL_MAKER > pMaker( let_dpMaker );
    pMaker->MakePage();
    pMaker = 0;
}


PageDisplay::PageDisplay( OuputPage_Environment & io_rEnv )
    :   HtmlDisplay_Impl( io_rEnv ),
        pMyFile( new HtmlDocuFile )
{
}

PageDisplay::~PageDisplay()
{

}

void
PageDisplay::Create_OverviewFile()
{
    Env().SetFile_Overview();
    File().SetLocation( Env().CurPath(), 0 );

    SetupFileOnCurEnv( C_sHFTitle_Overview );
    Make_SpecialPage( new PageMaker_Overview(*this) );
    Create_File();
}

void
PageDisplay::Create_AllDefsFile()
{
    // This method is a provisorium, because later this will
    //   be spreaded over the files.

    Env().MoveDir_2Root();
    Env().SetFile_AllDefs();
    File().SetLocation( Env().CurPath(), 0 );

    SetupFileOnCurEnv( "Defines and Macros" );
    Make_SpecialPage( new PageMaker_AllDefs(*this) );
    Create_File();
}

void
PageDisplay::Create_IndexFiles()
{
    Env().MoveDir_2Index();

    for ( int i = 0; i < C_nNrOfIndexLetters; ++i )
        Create_IndexFile(i);
}

void
PageDisplay::Create_HelpFile()
{
    Env().SetFile_Help();
    File().SetLocation( Env().CurPath(), 0 );

    SetupFileOnCurEnv( C_sHFTitle_Help );
    Make_SpecialPage( new PageMaker_Help(*this) );
    Create_File();
}

void
PageDisplay::Create_NamespaceFile()
{
    csv_assert( Env().CurNamespace() != 0 );
    Env().SetFile_CurNamespace();
    File().SetLocation( Env().CurPath(), Env().Depth() );
    if (Env().CurNamespace()->Owner().IsValid())
    {
        StreamLock sNsp(100);
        SetupFileOnCurEnv( sNsp() << C_sHFTypeTitle_Namespace
                                  << " "
                                  << Env().CurNamespace()->LocalName()
                                  << c_str );
    }
    else
    {
        SetupFileOnCurEnv( C_sHFTitle_GlobalNamespaceCpp );
    }

    Make_SpecialPage( new PageMaker_Namespace(*this) );

    Create_File();
}

void
PageDisplay::Setup_OperationsFile_for( const ary::loc::File & i_rFile )
{
    csv_assert( Env().CurNamespace() != 0 );
    Env().SetFile_Operations(&i_rFile);
    File().SetLocation( Env().CurPath(), Env().Depth() );

    StreamLock sOpFile(100);
    SetupFileOnCurEnv( sOpFile() << "Global Functions in Namespace "
                                 << Env().CurNamespace()->LocalName()
                                 << " in Sourcefile "
                                 << i_rFile.LocalName()
                                 << c_str );
    NavigationBar
            aNavi( Env(),
                   NavigationBar::CEGT_operations );
    aNavi.Write( CurOut() );
    CurOut() << new HorizontalLine;

    adcdisp::PageTitle_Std fTitle;
    csi::xml::Element & rTitle = fTitle( CurOut() );
    if (Env().CurNamespace()->Owner().IsValid())
    {
     	rTitle << "Global Functions in Namespace "
               << Env().CurNamespace()->LocalName();
    }
    else
    {
        rTitle << "Global Functions in Global Namespace C++";
    }

    rTitle << new html::LineBreak
           << "in Sourcefile "
           << i_rFile.LocalName();
    CurOut() << new HorizontalLine;
}

void
PageDisplay::Setup_OperationsFile_for( const ary::cpp::Class & i_rClass )
{
    csv_assert( Env().CurNamespace() != 0 );
    Env().SetFile_Operations(0);
    File().SetLocation( Env().CurPath(), Env().Depth() );

    StreamLock sOpFile(100);
    SetupFileOnCurEnv( sOpFile() << "Methods of Class "
                                 << i_rClass.LocalName()
                                 << c_str );
    NavigationBar
            aNavi( Env(),
                   NavigationBar::CEGT_operations );
    aNavi.Write( CurOut() );
    CurOut() << new HorizontalLine;

    adcdisp::PageTitle_Std fTitle;
    fTitle( CurOut(), "Methods of Class", i_rClass.LocalName() );

    CurOut() << new HorizontalLine;
}

void
PageDisplay::Setup_DataFile_for( const ary::loc::File & i_rFile )
{
    csv_assert( Env().CurNamespace() != 0 );
    Env().SetFile_Data(&i_rFile);
    File().SetLocation( Env().CurPath(), Env().Depth() );

    StreamLock sDataFile(100);
    SetupFileOnCurEnv( sDataFile() << "Global Data in Namespace "
                                   << Env().CurNamespace()->LocalName()
                                   << " in Sourcefile "
                                   << i_rFile.LocalName()
                                   << c_str );
    NavigationBar
            aNavi( Env(),
                   NavigationBar::CEGT_data );
    aNavi.Write( CurOut() );
    CurOut() << new HorizontalLine;

    adcdisp::PageTitle_Std fTitle;
    csi::xml::Element & rTitle = fTitle( CurOut() );
    if ( Env().CurNamespace()->Owner().IsValid() )
    {
     	rTitle << "Global Data in Namespace "
               << Env().CurNamespace()->LocalName();
    }
    else
    {
        rTitle << "Global Data in Global Namespace C++";
    }

    rTitle
        << new html::LineBreak
        << "in Sourcefile "
        << i_rFile.LocalName();
    CurOut() << new HorizontalLine;
}

void
PageDisplay::Setup_DataFile_for( const ary::cpp::Class & i_rClass )
{
    csv_assert( Env().CurNamespace() != 0 );
    Env().SetFile_Data(0);
    File().SetLocation( Env().CurPath(), Env().Depth() );

    StreamLock sDataFile(100);
    SetupFileOnCurEnv( sDataFile() << "Data of Class "
                                   << i_rClass.LocalName()
                                   << c_str );

    NavigationBar
            aNavi( Env(),
                   NavigationBar::CEGT_data );
    aNavi.Write( CurOut() );
    CurOut() << new HorizontalLine;

    adcdisp::PageTitle_Std fTitle;
    fTitle( CurOut(), "Data of Class", i_rClass.LocalName() );

    CurOut() << new HorizontalLine;
}

void
PageDisplay::Create_File()
{
    Easy().Leave();
    File().CreateFile();
}

void
PageDisplay::do_Process(const ary::cpp::Class & i_rData)
{
    Env().SetFile_Class(i_rData);
    File().SetLocation( Env().CurPath(), Env().Depth() );

    const char *
            sTypeTitle = i_rData.ClassKey() == ary::cpp::CK_class
                            ?   C_sHFTypeTitle_Class
                            :   i_rData.ClassKey() == ary::cpp::CK_struct
                                    ?   C_sHFTypeTitle_Struct
                                    :   C_sHFTypeTitle_Union;
    StreamLock sClassFile(60);
    SetupFileOnCurEnv( sClassFile() << sTypeTitle
                                    << " "
                                    << i_rData.LocalName()
                                    << c_str );

    Make_SpecialPage( new PageMaker_Class(*this, i_rData) );

    Create_File();
}

void
PageDisplay::do_Process(const ary::cpp::Enum & i_rData)
{
    if ( Ce_IsInternal(i_rData) )
        return;

    Env().SetFile_Enum(i_rData);
    File().SetLocation( Env().CurPath(), Env().Depth() );

    StreamLock sEnumFile(100);
    SetupFileOnCurEnv( sEnumFile() << C_sHFTypeTitle_Enum
                                   << " "
                                   << i_rData.LocalName()
                                   << c_str );
    Write_NavBar_Enum(i_rData);
    Write_TopArea_Enum(i_rData);
    Write_DocuArea_Enum(i_rData);
    Write_ChildList_Enum(i_rData);

    Create_File();
}

void
PageDisplay::do_Process(const ary::cpp::Typedef & i_rData)
{
    if ( Ce_IsInternal(i_rData) )
        return;

    Env().SetFile_Typedef(i_rData);
    File().SetLocation( Env().CurPath(), Env().Depth() );

    StreamLock sTypedefFile(100);
    SetupFileOnCurEnv( sTypedefFile() << C_sHFTypeTitle_Typedef
                                      << " "
                                      << i_rData.LocalName()
                                      << c_str );
    Write_NavBar_Typedef(i_rData);
    Write_TopArea_Typedef(i_rData);
    Write_DocuArea_Typedef(i_rData);


    Create_File();
}

void
PageDisplay::Write_NameChainWithLinks( const ary::cpp::CodeEntity & i_rCe )
{
    if ( Env().CurNamespace()->Id() != i_rCe.Id() )
    {
        RecursiveWrite_NamespaceLink( Env().CurNamespace() );
        if ( Env().CurClass() != 0 )
        {
            CurOut() << new html::Sbr;
            RecursiveWrite_ClassLink( Env().CurClass(), 1 );
        }
    }
    else
    {
        RecursiveWrite_NamespaceLink( Env().CurNamespace()->Parent() );
    }
}

const ary::cpp::Gate *
PageDisplay::inq_Get_ReFinder() const
{
 	return &Env().Gate();
}

void
PageDisplay::RecursiveWrite_NamespaceLink( const ary::cpp::Namespace * i_pNamespace )
{
    if ( i_pNamespace == 0 )
    {
        return;
    }
    else if (NOT i_pNamespace->Owner().IsValid())
    {   // Global namespace:
        StreamLock sNspDir(50);
        CurOut()
            >> *new Link( PathPerRoot(Env(),
                                      sNspDir() << C_sDIR_NamespacesCpp
                                                << "/"
                                                << C_sHFN_Namespace
                                                << c_str) )
                    << new xml::AnAttribute( "alt", C_sHFTitle_GlobalNamespaceCpp )
                    >> *new html::Font
                            << new html::SizeAttr("+1")
                            >> *new html::Bold
                                    << "::";
        CurOut()
            << " ";
        return;
    }
    else
    {
        RecursiveWrite_NamespaceLink( i_pNamespace->Parent() );
    }

    uintt nLevelDistance = Env().Depth() - ( i_pNamespace->Depth() + 1 );
    csv_assert( nLevelDistance < 100 );
    CurOut()
        >> *new Link( PathPerLevelsUp(nLevelDistance, C_sHFN_Namespace) )
                << new xml::AnAttribute( "alt", C_sHFTypeTitle_Namespace)
                >> *new html::Font
                        << new html::SizeAttr("+1")
                        >> *new html::Bold
                                << i_pNamespace->LocalName();
    CurOut()
        >> *new html::Font
                << new html::SizeAttr("+1")
                << " :: ";
}

void
PageDisplay::RecursiveWrite_ClassLink( const ary::cpp::Class * i_pClass,
                                       uintt                   i_nLevelDistance )
{
 	if ( i_pClass == 0 )
        return;

    if ( i_pClass->Protection() != ary::cpp::PROTECT_global )
    {
     	RecursiveWrite_ClassLink(
            dynamic_cast< const ary::cpp::Class* >(
                Env().Gate().Ces().Search_Ce(i_pClass->Owner())),
            i_nLevelDistance + 1 );
    }

    CurOut()
        >> *new Link( Path2Class(i_nLevelDistance, i_pClass->LocalName()) )
            << new html::ClassAttr("nqclass")
            << i_pClass->LocalName()
            << " :: ";
}

void
PageDisplay::SetupFileOnCurEnv( const char * i_sTitle )
{
    File().SetLocation( Env().CurPath(), Env().Depth() );
    File().SetTitle( i_sTitle );
    File().SetCopyright( Env().Layout().CopyrightText() );
    File().EmptyBody();

    // This sets CurOut() to the contents of <body></body>
    //   in File() :
    Easy().Enter( File().Body() );
}

void
PageDisplay::Write_NavBar_Enum( const ary::cpp::Enum & i_rData )
{
    NavigationBar   aNavi( Env(), i_rData );
    aNavi.MakeSubRow("List of");
    aNavi.AddItem( C_sTitle_EnumValues, C_sLabel_EnumValues, true );
    aNavi.Write( CurOut(), true );

    CurOut() << new HorizontalLine;
}

void
PageDisplay::Write_TopArea_Enum( const ary::cpp::Enum & i_rData )
{
    Write_NameChainWithLinks( i_rData );

    adcdisp::PageTitle_Std fTitle;
    fTitle( CurOut(), C_sHFTypeTitle_Enum, i_rData.LocalName() );

    CurOut() << new HorizontalLine;
}

void
PageDisplay::Write_DocuArea_Enum( const ary::cpp::Enum & i_rData )
{
    Docu_Display aDocuShow( Env() );

    aDocuShow.Assign_Out(CurOut());
    aDocuShow.Process(i_rData.Docu());
    aDocuShow.Unassign_Out();

    CurOut() << new HorizontalLine;
}

void
PageDisplay::Write_ChildList_Enum( const ary::cpp::Enum & i_rData )
{
    bool    bChildrenExist = false;
    ChildList_Display::Area_Result
            aResult( bChildrenExist, CurOut() );

    ChildList_Display aDisplay(Env(), i_rData);
    aDisplay.Run_Simple( aResult,
                         ary::cpp::Enum::SLOT_Values,
                         C_sLabel_EnumValues,
                         C_sTitle_EnumValues );

    if (NOT bChildrenExist)
        CurOut() >> *new html::Headline(4) << "This enum has no values.";
    CurOut() << new HorizontalLine;
}

void
PageDisplay::Write_NavBar_Typedef( const ary::cpp::Typedef & i_rData )
{
    NavigationBar   aNavi( Env(), i_rData );
    aNavi.Write( CurOut(), true );

    CurOut() << new HorizontalLine;
}

void
PageDisplay::Write_TopArea_Typedef( const ary::cpp::Typedef & i_rData )
{
    Write_NameChainWithLinks( i_rData );

    adcdisp::PageTitle_Std fTitle;
    fTitle( CurOut(), C_sHFTypeTitle_Typedef, i_rData.LocalName() );

    CurOut() << new HorizontalLine;
}

void
PageDisplay::Write_DocuArea_Typedef( const ary::cpp::Typedef & i_rData )
{
    adcdisp::ExplanationList aDef( CurOut() );
    aDef.AddEntry("Definition:");
    xml::Element & rDef = aDef.Def();

    ary::cpp::Type_id
        nDefiningType = i_rData.DescribingType();

    const ary::cpp::CodeEntity *
        pRelatedCe = Env().Gate().Search_RelatedCe(nDefiningType);
    if ( pRelatedCe != 0 )
    {
        const char * sTypeKey = Get_TypeKey(*pRelatedCe);
        if ( NOT csv::no_str(sTypeKey) )
            rDef << sTypeKey << " ";
    }

    dshelp::Get_LinkedTypeText( rDef, Env(), nDefiningType );

    Docu_Display aDocuShow( Env() );

    aDocuShow.Assign_Out(CurOut());
    aDocuShow.Process(i_rData.Docu());
    aDocuShow.Unassign_Out();

    CurOut() << new HorizontalLine;
}

void
PageDisplay::Create_IndexFile( int i_nLetter )
{
    static char aLetters[C_nNrOfIndexLetters+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
    csv_assert( 0 <= i_nLetter AND i_nLetter < C_nNrOfIndexLetters );

    char cCurLetter = aLetters[i_nLetter];
    Env().SetFile_Index( cCurLetter );

    static char sIndexFileTitle[] = "Global Index X";
    const int nPositionOfLetterInTitle = 13;
    sIndexFileTitle[nPositionOfLetterInTitle] = cCurLetter;
    SetupFileOnCurEnv( sIndexFileTitle );

    Make_SpecialPage( new PageMaker_Index(*this, cCurLetter ) );

    Create_File();
}

