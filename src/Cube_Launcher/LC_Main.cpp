/***************************************************************
 * Name:      LC_Main.cpp
 * Author:    VuSurTF1
 * Created:   2014-06-23
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "LC_Main.h"

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__WXMAC__)
        wxbuild << _T("-Mac");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

BEGIN_EVENT_TABLE(LC_Frame, wxFrame)
    EVT_BUTTON(BTN_LANCER_CISLATTACK_32, LC_Frame::OnButtonClickedLancerVersionNormale)
    EVT_BUTTON(BTN_LANCER_CISLATTACK_64, LC_Frame::OnButtonClickedLancerVersion64bits)
    EVT_BUTTON(BTN_LANCER_CISLATTACK_S, LC_Frame::OnButtonClickedLancerVersionServeur)

    EVT_BUTTON(BTN_LANCER_INDEV, LC_Frame::OnButtonClickedLancerIndev)
    EVT_BUTTON(BTN_LANCER_SITEWEB, LC_Frame::OnButtonClickedLancerSiteWeb)
    EVT_BUTTON(BTN_LANCER_QUITTER, LC_Frame::OnButtonClickedLancerQuitter)

    EVT_CLOSE(LC_Frame::OnClose)
    EVT_MENU(idMenuQuit, LC_Frame::OnButtonClickedLancerQuitter)
    EVT_MENU(idMenuAbout, LC_Frame::OnAbout)
END_EVENT_TABLE()

LC_Frame::LC_Frame(wxFrame *frame, const wxString& title)
    : wxFrame(frame, -1, title)
{
#if wxUSE_MENUS
    wxMenuBar* mbar = new wxMenuBar();
    wxMenu* fileMenu = new wxMenu(_T(""));

    fileMenu->Append(idMenuQuit, _("&Quitter\tAlt-F4"), _("Quitte l'application."));
    mbar->Append(fileMenu, _("&Launcher"));

    wxMenu* helpMenu = new wxMenu(_T(""));
    helpMenu->Append(idMenuAbout, _("&À propos\tF1"), _("À propos du launcher..."));
    mbar->Append(helpMenu, _("&?"));

    SetMenuBar(mbar);
#endif // wxUSE_MENUS


/*
#if wxUSE_STATUSBAR
     create a status bar with some information about the used wxWidgets version
    CreateStatusBar(1);
    SetStatusText(_("Launcher Cube Conflict v0.3.7"),0);
#endif // wxUSE_STATUSBAR
*/

}

void LC_Frame::OnButtonClickedLancerVersionNormale(wxCommandEvent &event)
{
    system ("start bin/cubeconflict.exe \"-u$HOME/My Games/Cube Conflict\" -glog.txt");
    Destroy();
}

void LC_Frame::OnButtonClickedLancerVersion64bits(wxCommandEvent &event)
{
    system ("start bin64/cubeconflict.exe \"-u$HOME/My Games/Cube Conflict\" -glog.txt");
    Destroy();
}

void LC_Frame::OnButtonClickedLancerVersionServeur(wxCommandEvent &event)
{
    system ("start bin/cubeconflict.exe \"-u$HOME/My Games/Cube Conflict\" -glog_serveur.txt -d");
}

void LC_Frame::OnButtonClickedLancerSiteWeb(wxCommandEvent &event)
{
    system("start www.cube-conflict.com" );
}

void LC_Frame::OnButtonClickedLancerIndev(wxCommandEvent &event)
{
    system("start www.github.com/CubeConflict/Cube-Conflict" );
}

void LC_Frame::OnButtonClickedLancerQuitter(wxCommandEvent &event)
{
    Destroy();
}

void LC_Frame::OnClose(wxCloseEvent &event)
{
    Destroy();
}

void LC_Frame::OnAbout(wxCommandEvent &event)
{
    //wxString msg = wxbuildinfo(long_f);
    wxString msg( wxT("Launcher fait maison uniquement pour Cube Conflict et entièrement réalisé par Jean Onche, no rage de mon codage.") );
    //wxString msg = ;
    wxMessageBox(msg, _("À propos..."));
}

LC_Frame::~LC_Frame()
{
}
