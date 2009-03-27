
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerTheme.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWEvent.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWTopLevel.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWSplitFrame.h"
#include "vtkKWText.h"
#include "vtkKWTextWithHyperlinksWithScrollbars.h"

#include "vtkSlicerWelcomeGUI.h"
#include "vtkSlicerWelcomeIcons.h"

#include <map>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>



//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkSlicerWelcomeGUI, "$Revision: 1.0 $");


//------------------------------------------------------------------------------
vtkSlicerWelcomeGUI* vtkSlicerWelcomeGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret =
    vtkObjectFactory::CreateInstance("vtkSlicerWelcomeGUI");

  if(ret)
    {
      return (vtkSlicerWelcomeGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSlicerWelcomeGUI;
}



//----------------------------------------------------------------------------
vtkSlicerWelcomeGUI::vtkSlicerWelcomeGUI()
{
  this->Logic = NULL;
  this->SlicerWelcomeIcons = NULL;
  this->SetGUIWidth(-1);
}

//----------------------------------------------------------------------------
vtkSlicerWelcomeGUI::~vtkSlicerWelcomeGUI()
{
//    this->RemoveMRMLNodeObservers ( );
//    this->RemoveLogicObservers ( );
    this->SetGUIWidth(-1);
    
    if ( this->SlicerWelcomeIcons )
      {
      this->SlicerWelcomeIcons->Delete();
      this->SlicerWelcomeIcons = NULL;
      }
    this->Logic = NULL;
//    vtkSetAndObserveMRMLNodeMacro( this->SlicerWelcomeNode, NULL );
}

//----------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::Enter()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  if ( app )
    {
    vtkSlicerGUILayout *geom = app->GetDefaultGeometry ( );
    if ( geom )
      {
      vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
      if ( appGUI )
        {
        if ( appGUI->GetMainSlicerWindow() )
          {
          this->SetGUIWidth (appGUI->GetMainSlicerWindow()->GetMainSplitFrame()->GetFrame1Size ());
          appGUI->GetMainSlicerWindow()->GetMainSplitFrame()->SetFrame1Size (geom->GetDefaultGUIPanelWidth() * 1.75 );
          }
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::Exit ( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  if ( app )
    {
    vtkSlicerGUILayout *geom = app->GetDefaultGeometry ( );
    if ( geom )
      {
      vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
      if ( appGUI )
        {
        if ( appGUI->GetMainSlicerWindow() )
          {
          if ( this->GUIWidth < 0 )
            {
            appGUI->GetMainSlicerWindow()->GetMainSplitFrame()->SetFrame1Size (geom->GetDefaultGUIPanelWidth() );
            }
          else
            {
            // restore.
            appGUI->GetMainSlicerWindow()->GetMainSplitFrame()->SetFrame1Size (this->GUIWidth );
            }
          }
        }
      }
    }
}


//----------------------------------------------------------------------------
vtkIntArray *vtkSlicerWelcomeGUI::NewObservableEvents()
{
 vtkIntArray *events = vtkIntArray::New();
  // Slicer3.cxx calls delete on events
  return events;
}


//----------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::TearDownGUI ( )
{

  this->RemoveGUIObservers ( );
  this->SetLogic ( NULL );
  this->SetAndObserveMRMLScene ( NULL );

}


//----------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::AddGUIObservers ( ) 
{

}



//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::RemoveGUIObservers ( )
{

}






//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  if ( this->Logic == NULL )
    {
    //TODO: error macro
    return;
    }

  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWEntry *e = vtkKWEntry::SafeDownCast ( caller );
  vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::SetStatusText(const char *txt)
{
  if ( this->GetApplicationGUI() )
    {
    if ( this->GetApplicationGUI()->GetMainSlicerWindow() )
      {
      this->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText (txt);
      }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  this->SlicerWelcomeIcons = vtkSlicerWelcomeIcons::New();
  
  this->UIPanel->AddPage ( "SlicerWelcome", "SlicerWelcome", NULL );
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "SlicerWelcome" );    


  // create icons
  this->SlicerWelcomeIcons = vtkSlicerWelcomeIcons::New();

    //---
    // WELCOME & ABOUT FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *welcomeFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    welcomeFrame->SetParent ( page );
    welcomeFrame->Create ( );
    welcomeFrame->SetLabelText ("Welcome & About");
    welcomeFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  welcomeFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildWelcomeAndAboutPanel( welcomeFrame->GetFrame() );
    
    //---
    // OVERVIEW FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *overviewFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    overviewFrame->SetParent ( page );
    overviewFrame->Create ( );
    overviewFrame->SetLabelText ("Overview");
    overviewFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  overviewFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildOverviewPanel( overviewFrame->GetFrame() );
    
    //---
    // LOADING DATA FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *loadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    loadFrame->SetParent ( page );
    loadFrame->Create ( );
    loadFrame->SetLabelText ("Loading Data");
    loadFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  loadFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildLoadDataPanel( loadFrame->GetFrame() );

    //---
    // SAVING DATA FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *saveFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    saveFrame->SetParent ( page );
    saveFrame->Create ( );
    saveFrame->SetLabelText ("Saving Data");
    saveFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  saveFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildSaveDataPanel( saveFrame->GetFrame() );

    //---
    // DISPLAY DATA FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *displayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    displayFrame->SetParent ( page );
    displayFrame->Create ( );
    displayFrame->SetLabelText ("Adjusting Data Display");
    displayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  displayFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildDataDisplayPanel (displayFrame->GetFrame() );

    //---
    // COMMON ACTIVITIES FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *commonFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    commonFrame->SetParent ( page );
    commonFrame->Create ( );
    commonFrame->SetLabelText ("Other Useful Hints");
    commonFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  commonFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildOtherHelpPanel( commonFrame->GetFrame() );

    //---
    // MODULES FRAME
    //---
/*
    vtkSlicerModuleCollapsibleFrame *modulesFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modulesFrame->SetParent ( page );
    modulesFrame->Create ( );
    modulesFrame->SetLabelText ("Adding & Removing Modules");
    modulesFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  modulesFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
     this->BuildConfigureModulesPanel( modulesFrame->GetFrame() );
*/

    //---
    // COMMUNITY FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *communityFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    communityFrame->SetParent ( page );
    communityFrame->Create ( );
    communityFrame->SetLabelText ("Community");
    communityFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  communityFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildCommunityPanel ( communityFrame->GetFrame() );

    //---
    // ACKNOWLEDGMENT FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *acknowledgmentFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    acknowledgmentFrame->SetParent ( page );
    acknowledgmentFrame->Create ( );
    acknowledgmentFrame->SetLabelText ("Acknowledgment");
    acknowledgmentFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4",
                  acknowledgmentFrame->GetWidgetName() );
    vtkKWFrame *f = vtkKWFrame::New();
    f->SetParent ( acknowledgmentFrame->GetFrame() );
    f->Create();
    app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4",
                  f->GetWidgetName() );    

    vtkKWLabel *NACLabel = vtkKWLabel::New();
    NACLabel->SetParent ( f );
    NACLabel->Create();
    NACLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNACLogo() );

    vtkKWLabel *NAMICLabel = vtkKWLabel::New();
    NAMICLabel->SetParent ( f );
    NAMICLabel->Create();
    NAMICLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNAMICLogo() );    

    vtkKWLabel *NCIGTLabel = vtkKWLabel::New();
    NCIGTLabel->SetParent ( f );
    NCIGTLabel->Create();
    NCIGTLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNCIGTLogo() );
    
    vtkKWLabel *BIRNLabel = vtkKWLabel::New();
    BIRNLabel->SetParent ( f );
    BIRNLabel->Create();
    BIRNLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetBIRNLogo() );

    vtkKWTextWithHyperlinksWithScrollbars *txt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
    txt->SetParent ( f ) ;
    txt->Create();
    txt->SetHorizontalScrollbarVisibility ( 0 );
    txt->SetVerticalScrollbarVisibility ( 1 );
    txt->GetWidget()->SetReliefToFlat();
    txt->GetWidget()->SetWrapToWord();
    txt->GetWidget()->QuickFormattingOn();
    txt->GetWidget()->SetHeight ( 4 );
    const char *ack = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, CTSC and the Slicer Community. See <a>http://www.slicer.org</a> for details.\n\n";
    txt->SetText ( ack );
    txt->GetWidget()->ReadOnlyOn();

    app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky e", NAMICLabel->GetWidgetName());
    app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky w", NACLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky e",  BIRNLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky w",  NCIGTLabel->GetWidgetName());                  
    app->Script ( "grid %s -row 2 -column 0 -columnspan 2 -padx 2 -pady 2 -sticky ew",  txt->GetWidgetName());                  
    app->Script ( "grid columnconfigure %s 0 -weight 1", f->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 1 -weight 1", f->GetWidgetName() );


    txt->Delete();
    NACLabel->Delete();
    NAMICLabel->Delete();
    NCIGTLabel->Delete();
    BIRNLabel->Delete();
    f->Delete();

    welcomeFrame->Delete();
    overviewFrame->Delete();
    loadFrame->Delete();
    saveFrame->Delete();
    displayFrame->Delete();
    commonFrame->Delete();
//    modulesFrame->Delete();
    communityFrame->Delete();
    acknowledgmentFrame->Delete();
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildWelcomeAndAboutPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildWelcomeAndAboutPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4",
                f->GetWidgetName() );
  vtkKWLabel *l = vtkKWLabel::New();
  l->SetParent ( f );
  l->Create();
  l->SetImageToIcon ( this->SlicerWelcomeIcons->GetLogoIcon() );
  app->Script ( "pack %s -side top -anchor c -fill x -padx 2 -pady 2",
                l->GetWidgetName() );
    
  vtkKWTextWithHyperlinksWithScrollbars *welcome = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  welcome->SetParent ( parent );
  welcome->Create();
  welcome->SetHorizontalScrollbarVisibility ( 0 );
  welcome->SetVerticalScrollbarVisibility ( 0);
  welcome->GetWidget()->SetReliefToFlat();
  welcome->GetWidget()->SetWrapToWord();
  welcome->GetWidget()->QuickFormattingOn();
  welcome->GetWidget()->SetHeight ( 6 );
  const char *about = "**3D Slicer** is a free open source software platform for medical image processing and 3D visualization of image data. This module contains some basic information and useful links to get you started using Slicer. Please see our website <a>http://www.slicer.org</a> for additional information.";
  welcome->SetText ( about );
  //Important that Read only after SetText otherwise it doesn't work
  welcome->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *disclaimer = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  disclaimer->SetParent ( parent );
  disclaimer->Create();
  disclaimer->SetHorizontalScrollbarVisibility ( 0 );
  disclaimer->SetVerticalScrollbarVisibility ( 0);
  disclaimer->GetWidget()->SetReliefToFlat();
  disclaimer->GetWidget()->SetWrapToWord();
  disclaimer->GetWidget()->QuickFormattingOn();
  disclaimer->GetWidget()->SetForegroundColor ( 0.4, 0.4, 0.4 );
  disclaimer->GetWidget()->SetHeight ( 6 );
  const char *info = "3D Slicer is distributed under a BSD-style license; for details about the contribution and software license agreement, please see <a>http://www.slicer.org/cgi-bin/License/SlicerLicenseForm.pl</a>. The software has been designed for research purposes only and has not been reviewed or approved by the Food and Drug Administration, or by any other agency. ";
  disclaimer->SetText ( info );
  //Important that Read only after SetText otherwise it doesn't work
  disclaimer->GetWidget()->ReadOnlyOn();


  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 6 -pady 2",
                welcome->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 6 -pady 2",
                disclaimer->GetWidgetName() );
  welcome->Delete();
  disclaimer->Delete();
  l->Delete();
  f->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildOverviewPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildOverviewPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();
  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetImageToIcon ( this->SlicerWelcomeIcons->GetMapIcon () );
  app->Script ( "pack %s -side top -anchor c -fill x -padx 2 -pady 2",
                l1->GetWidgetName() );
    
  vtkKWTextWithHyperlinksWithScrollbars *txt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  txt->SetParent ( parent );
  txt->Create();
  txt->SetHorizontalScrollbarVisibility ( 0 );
  txt->SetVerticalScrollbarVisibility ( 1 );
  txt->GetWidget()->SetHeight ( 20 );
  txt->GetWidget()->SetReliefToGroove();
  txt->GetWidget()->SetWrapToWord();
  txt->GetWidget()->QuickFormattingOn();
  txt->SetPadX ( 6 );
  txt->SetPadY ( 6 );
  const char *info = "\nThe basic organization of Slicer's user interface (UI) is shown above. This brief overview will reference these components labeled in the figure:\n\n**File Menu:** contains basic load and save functionality, access to application settings, Tcl and Python interfaces for developers, help and mechanisms for users to provide feedback.\n\n**Toolbar:** contains tools for selecting and navigating among Slicer modules, layout and other utilities.\n\n**GUI Panel:** contains a UI for any selected module.\n\n**Slices Controls:** contains a UI to configure the Slice Viewers. This panel can be collapsed if you need more room for the GUI Panel.\n\n**3D View Controls:** contains a UI to configure the 3D Viewer. This panel can also be collapsed if you need more room for the GUI Panel.\n\n**Error Log:** displays helpful reports when errors are encountered. This information is useful for providing feedback to the developer community.";
  txt->SetText ( info );
  //Important that Read only after SetText otherwise it doesn't work
  txt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4",
                f->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 6 -pady 4",
                txt->GetWidgetName() );

  l1->Delete();
  f->Delete();
  txt->Delete();
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildLoadDataPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildLoadDataPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetImageToIcon ( this->SlicerWelcomeIcons->GetSceneIcon () );

  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( f );
  l2->Create();
  l2->SetImageToIcon ( this->SlicerWelcomeIcons->GetLoadIcon () );

  vtkKWLabel *l3 = vtkKWLabel::New();
  l3->SetParent ( f );
  l3->Create();
  l3->SetImageToIcon ( this->SlicerWelcomeIcons->GetSceneTreeIcon () );
    
  vtkKWTextWithHyperlinksWithScrollbars *scenetxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  scenetxt->SetParent ( f );
  scenetxt->Create();
  scenetxt->SetHorizontalScrollbarVisibility ( 0 );
  scenetxt->SetVerticalScrollbarVisibility ( 1) ;
  scenetxt->GetWidget()->SetReliefToGroove();
  scenetxt->GetWidget()->SetWrapToWord();
  scenetxt->GetWidget()->QuickFormattingOn();
  scenetxt->GetWidget()->SetHeight (14);
  const char *t1 = "**MRML:** The Medical Reality Markup Language (MRML) scene file is an XML-based text file that references all data loaded into Slicer, and records its display and processing parameterization. Loading a MRML scene file will also load its referenced data and display it in the manner it was saved. \n\n**File->Load Scene:** This load command selected from Slicer's File Menu will close the current scene, pop up a File Browser, and load data from the MRML scene file you select. \n\n**File->Import Scene:** This load command selected from Slicer's File Menu will pop up a File Browser, and add all data referenced in the MRML scene file you select into the current scene. \n\n**File->Add Data:** This load command, also selected from Slicer's File Menu, will raise a load panel with a set of options for loading individual datasets. \n\n";
  scenetxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work  
  scenetxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *loadtxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  loadtxt->SetParent ( f );
  loadtxt->Create();
  loadtxt->SetHorizontalScrollbarVisibility ( 0 );
  loadtxt->SetVerticalScrollbarVisibility ( 1);
  loadtxt->GetWidget()->SetReliefToGroove();
  loadtxt->GetWidget()->SetWrapToWord();
  loadtxt->GetWidget()->QuickFormattingOn();
  loadtxt->GetWidget()->SetHeight ( 5 );
  const char *t2 = "**Load any:** Clicking the \"Load\" icon (shown at the left) from Slicer's Toolbar exposes the above three options in a menu. Select the \"Load or Import Scene\" panel to load a scene. Select the \"Load Volumes\" panel to load individual volumes or to use te DICOM browser. Select the \"Load Data\" panel to select entire folders or to load other types of data (such as model data).";
  loadtxt->SetText ( t2 );
  //Important that Read only after SetText otherwise it doesn't work
  loadtxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *treetxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  treetxt->SetParent ( f );
  treetxt->Create();
  treetxt->SetHorizontalScrollbarVisibility ( 0 );
  treetxt->SetVerticalScrollbarVisibility ( 1);
  treetxt->GetWidget()->SetReliefToGroove();
  treetxt->GetWidget()->SetWrapToWord();
  treetxt->GetWidget()->QuickFormattingOn();
  treetxt->GetWidget()->SetHeight ( 5);
  const char *t3 = "**Scene Tree:** All data in the scene is displayed in the \"Data module\". Select the \"Data module\" icon (shown at the left) from Slicer's toolbar, or choose the module from the toolbar's \"Module Selection Menu\".";
  treetxt->SetText ( t3 );
  //Important that Read only after SetText otherwise it doesn't work
  treetxt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", f->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky nw -padx 2 -pady 6", l1->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky new -padx 2 -pady 4", scenetxt->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 0 -sticky nw -padx 4 -pady 6", l2->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 1 -sticky new -padx 2 -pady 4", loadtxt->GetWidgetName() );
  app->Script ( "grid %s -row 2 -column 0 -sticky nw -padx 4 -pady 6", l3->GetWidgetName() );
  app->Script ( "grid %s -row 2 -column 1 -sticky new -padx 2 -pady 4", treetxt->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", f->GetWidgetName() );

  l1->Delete();
  l2->Delete();
  l3->Delete();
  scenetxt->Delete();
  loadtxt->Delete();
  treetxt->Delete();
  f->Delete();

}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildSaveDataPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildSaveDataPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetImageToIcon ( this->SlicerWelcomeIcons->GetSceneIcon () );

  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( f );
  l2->Create();
  l2->SetImageToIcon ( this->SlicerWelcomeIcons->GetSaveIcon () );

  vtkKWTextWithHyperlinksWithScrollbars *scenetxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  scenetxt->SetParent ( f );
  scenetxt->Create();
  scenetxt->SetHorizontalScrollbarVisibility ( 0 );
  scenetxt->SetVerticalScrollbarVisibility ( 1) ;
  scenetxt->GetWidget()->SetReliefToGroove();
  scenetxt->GetWidget()->SetWrapToWord();
  scenetxt->GetWidget()->QuickFormattingOn();
  scenetxt->GetWidget()->SetHeight( 12 );
  const char *t1 = "**MRML:** The Medical Reality Markup Language (MRML) scene file is an XML-based text file that references all data loaded into Slicer, and records its display and processing parameterization. Saving a MRML scene file will NOT save its reference data automatically. If you wish to save another instance of data in the scene, you must do so explicitly. Two ways to access Slicer's save interface are described below. \n\n **File->Save:** This option will raise Slicer's Save interface. This panel gives you the option to save the MRML scene file and all unsaved data by default. You may optionally choose to rename the scene, to change the destination directory for any or all of the data to be written, and to save data that has not been modified during your session too. \n";
  scenetxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  scenetxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *savetxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  savetxt->SetParent ( f );
  savetxt->Create();
  savetxt->SetHorizontalScrollbarVisibility ( 0 );
  savetxt->SetVerticalScrollbarVisibility ( 1);
  savetxt->GetWidget()->SetReliefToGroove();
  savetxt->GetWidget()->SetWrapToWord();
  savetxt->GetWidget()->QuickFormattingOn();
  savetxt->GetWidget()->SetHeight ( 6 );
  const char *t2 = "**Save:** Clicking the icon shown at the left from Slicer's Toolbar raises the same interface described above. \n\n **IMPORTANT:** Once a MRML scene file is saved, deleting or moving any dataset that the scene references on disk will cause the scene description to become invalid. Loading a MRML scene that cannot locate its referenced data can cause unstable results. \n";
  savetxt->SetText ( t2 );
  //Important that Read only after SetText otherwise it doesn't work
  savetxt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", f->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky nw -padx 2 -pady 6", l1->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky new -padx 2 -pady 4", scenetxt->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 0 -sticky nw -padx 4 -pady 6", l2->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 1 -sticky new -padx 2 -pady 4", savetxt->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", f->GetWidgetName() );

  l1->Delete();
  l2->Delete();
  scenetxt->Delete();
  savetxt->Delete();
  f->Delete();
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildDataDisplayPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildDataDisplayPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetImageToIcon ( this->SlicerWelcomeIcons->GetVolumesIcon () );

  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( f );
  l2->Create();
  l2->SetImageToIcon ( this->SlicerWelcomeIcons->GetModelsIcon () );

  vtkKWLabel *l3 = vtkKWLabel::New();
  l3->SetParent ( f );
  l3->Create();
  l3->SetImageToIcon ( this->SlicerWelcomeIcons->GetFiducialsIcon () );
    
  vtkKWLabel *l4 = vtkKWLabel::New();
  l4->SetParent ( f );
  l4->Create();
  l4->SetImageToIcon ( this->SlicerWelcomeIcons->GetSceneSnapshotsIcon () );

  vtkKWTextWithHyperlinksWithScrollbars *voltxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  voltxt->SetParent ( f );
  voltxt->Create();
  voltxt->SetHorizontalScrollbarVisibility ( 0 );
  voltxt->SetVerticalScrollbarVisibility ( 1) ;
  voltxt->GetWidget()->SetReliefToGroove();
  voltxt->GetWidget()->SetWrapToWord();
  voltxt->GetWidget()->QuickFormattingOn();
  voltxt->GetWidget()->SetHeight(6);
  const char *t1 = "**Volumetric Image Data:** The interface for adjusting  Window, Level, Threshold, Color LUT and other parameters that control the display of volume image data in the scene is found in the \"Volumes module\". To access this interface, either select the \"Volumes module icon\" (shown at the left) from Slicer's Toolbar, or choose the module from the Toolbar's \"Module Selection\" menu; then open the module's \"Display\" panel. ";
  voltxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  voltxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *modtxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  modtxt->SetParent ( f );
  modtxt->Create();
  modtxt->SetHorizontalScrollbarVisibility ( 0 );
  modtxt->SetVerticalScrollbarVisibility ( 1 );
  modtxt->GetWidget()->SetReliefToGroove();
  modtxt->GetWidget()->SetWrapToWord();
  modtxt->GetWidget()->QuickFormattingOn();
  modtxt->GetWidget()->SetHeight(6);
  const char *t2 = "**Models:** Display options for geometric data like VTK models and FreeSurfer surfaces are found in the \"Models module\". To adjust parameters such as Color, Transparency, and Clipping, select the \"Models module icon\" (shown at the left) from Slicer's Toolbar, or select the module from the Toolbar's \"Module Selection\" menu; then open the module's \"Display\" panel.";
  modtxt->SetText ( t2 );
  //Important that Read only after SetText otherwise it doesn't work
  modtxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *fidtxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  fidtxt->SetParent ( f );
  fidtxt->Create();
  fidtxt->SetHorizontalScrollbarVisibility ( 0 );
  fidtxt->SetVerticalScrollbarVisibility ( 1);
  fidtxt->GetWidget()->SetReliefToGroove();
  fidtxt->GetWidget()->SetWrapToWord();
  fidtxt->GetWidget()->QuickFormattingOn();
  fidtxt->GetWidget()->SetHeight(6);
  const char *t3 = "**Fiducial Points:** Fiducials are useful tools employed in many Slicer analysis modules. Their visual representation and display can be adjusted in the \"Fiducials module\". Select the \"Fiducials module icon\" (shown at the left) from Slicer's Toolbar, or select the module from the Toolbar's \"Module Selection\" menu; then open the module's \"Display\" panel.";
  fidtxt->SetText ( t3 );
  //Important that Read only after SetText otherwise it doesn't work
  fidtxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *sstxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  sstxt->SetParent ( f );
  sstxt->Create();
  sstxt->SetHorizontalScrollbarVisibility ( 0 );
  sstxt->SetVerticalScrollbarVisibility ( 1);
  sstxt->GetWidget()->SetReliefToGroove();
  sstxt->GetWidget()->SetWrapToWord();
  sstxt->GetWidget()->QuickFormattingOn();
  sstxt->GetWidget()->SetHeight(6);
  const char *t4 = "**Scene Snapshots:** The icons shown at the left are found in the 3D View Controls panel. The top-most icon at the left is used to capture a \"snapshot\" of the scene configuration and save it with a unique name. That \"snapshot\" can be restored at a later time by selecting the \"Scene Snapshot Menubutton\", shown at the lower left. Scene Snapshots will be saved with your scene.";
  sstxt->SetText ( t4 );
  //Important that Read only after SetText otherwise it doesn't work
  sstxt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", f->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky nw -padx 2 -pady 4", l1->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky new -padx 2 -pady 4", voltxt->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 0 -sticky nw -padx 4 -pady 4", l2->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 1 -sticky new -padx 2 -pady 4", modtxt->GetWidgetName() );
  app->Script ( "grid %s -row 2 -column 0 -sticky nw -padx 4 -pady 4", l3->GetWidgetName() );
  app->Script ( "grid %s -row 2 -column 1 -sticky new -padx 2 -pady 4", fidtxt->GetWidgetName() );
  app->Script ( "grid %s -row 3 -column 0 -sticky nw -padx 4 -pady 4", l4->GetWidgetName() );
  app->Script ( "grid %s -row 3 -column 1 -sticky new -padx 2 -pady 4", sstxt->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", f->GetWidgetName() );

  l1->Delete();
  l2->Delete();
  l3->Delete();
  l4->Delete();
  voltxt->Delete();
  modtxt->Delete();
  fidtxt->Delete();
  sstxt->Delete();
  f->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildOtherHelpPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildOtherHelpPanel: got NULL application." );
    return;
    }

  vtkKWTextWithHyperlinksWithScrollbars *basictxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  basictxt->SetParent ( parent );
  basictxt->Create();
  basictxt->SetHorizontalScrollbarVisibility ( 0 );
  basictxt->SetVerticalScrollbarVisibility ( 1);
  basictxt->GetWidget()->SetReliefToGroove();
  basictxt->GetWidget()->SetWrapToWord();
  basictxt->GetWidget()->QuickFormattingOn();
  basictxt->GetWidget()->SetHeight(8);
  const char *t1 = " **Help For Performing Common Tasks:**\n\n* For information on how basic visualization operations are performed, see: <a>link2PPT</a>.\n* For information on merging multiple datasets (and multiple modalities) for a single subject, see: <a>link2PPT</a>. \n* For information about manual and automatic segmentation of image data, see: <a>link2PPT</a>. \n* For more tutorials and accompanying training datasets, please see: <a>link2PPT</a>. \n\n";
  basictxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  basictxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *advtxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  advtxt->SetParent ( parent );
  advtxt->Create();
  advtxt->SetHorizontalScrollbarVisibility ( 0 );
  advtxt->SetVerticalScrollbarVisibility ( 0);
  advtxt->GetWidget()->SetReliefToGroove();
  advtxt->GetWidget()->SetWrapToWord();
  advtxt->GetWidget()->QuickFormattingOn();
  advtxt->GetWidget()->SetHeight(8);
  const char *t2 = " **Information About Specialized Tools:** \n\n* Find a tutorial on automated brain segmentation using the \"EMSegmenter module\" here: <a>link2PPT</a>. \n* Learn about Diffusion MRI processing with Slicer here: <a>link2PPT</a>. \n* Find information about Tool Tracking and Image Guided Therapy Research here: <a>link2PPT</a>. \n* Find additional tutorials and example data here: <a>http://www.na-mic.org/Wiki/index.php/Slicer3.2:Training</a> \n";
  advtxt->SetText ( t2 );
  //Important that Read only after SetText otherwise it doesn't work
  advtxt->GetWidget()->ReadOnlyOn();
  
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", basictxt->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", advtxt->GetWidgetName() );

  basictxt->Delete();
  advtxt->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildConfigureModulesPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildConfigureModulesPanel: got NULL application." );
    return;
    }

  vtkKWTextWithHyperlinksWithScrollbars *txt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  txt->SetParent ( parent );
  txt->Create();
  txt->SetHorizontalScrollbarVisibility ( 0 );
  txt->SetVerticalScrollbarVisibility ( 0);
  txt->GetWidget()->SetReliefToGroove();
  txt->GetWidget()->SetWrapToWord();
  txt->GetWidget()->QuickFormattingOn();
  const char *t = "";
  txt->SetText ( t );
  //Important that Read only after SetText otherwise it doesn't work
  txt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", txt->GetWidgetName() );
  txt->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildCommunityPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildCommunityPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();
  
  vtkKWLabel *l = vtkKWLabel::New();
  l->SetParent ( f );
  l->Create();
  l->SetImageToIcon ( this->SlicerWelcomeIcons->GetCommunityIcon() );
  
  vtkKWTextWithHyperlinksWithScrollbars *txt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  txt->SetParent ( f );
  txt->Create();
  txt->SetHorizontalScrollbarVisibility ( 0 );
  txt->SetVerticalScrollbarVisibility ( 1 );
  txt->GetWidget()->SetReliefToGroove();
  txt->GetWidget()->SetWrapToWord();
  txt->GetWidget()->QuickFormattingOn();
  txt->GetWidget()->SetHeight( 12 );
  const char *t = "We welcome you to participate in or engage with the Slicer user and developer community. Some useful links are listed below:\n\n**Subscribe to User or Developer Mailing Lists:**\n<a>http://www.slicer.org/pages/Mailinglist</a>\n\n**Report Bugs and Request Features:**\n<a>http://www.na-mic.org/Bug/main_page.php</a>\n\n**Track upcoming events:**\n<a>http://www.na-mic.org/Wiki/index.php/Events</a>\n\n";
  txt->SetText ( t );
  //Important that Read only after SetText otherwise it doesn't work
  txt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", f->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor c -fill x -expand y -padx 2 -pady 4", l->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", txt->GetWidgetName() );
  
  l->Delete();
  txt->Delete();
  f->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::Init ( )
{
}
