/***************************************************************
 * Name:      Launcher Cube Conflict (LC_App.cpp)
 * Author:    VuSurTF1
 * Created:   2014-06-23
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "LC_App.h"
#include "LC_Main.h"

#include<wx\statline.h>

IMPLEMENT_APP(LC_App);

bool LC_App::OnInit()
{
    LC_Frame* frame = new LC_Frame(0L, _("Cube Conflict"));

    frame->SetSize(wxSize(300,285));
    frame->SetMinSize(wxSize(300,285));
    frame->SetMaxSize(wxSize(300,285));
    wxPanel *panelAffichage = new wxPanel(frame, -1, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE );

        wxBoxSizer *sizer_intermed = new wxBoxSizer(wxVERTICAL);

            wxBoxSizer *sizer_boutons = new wxBoxSizer(wxVERTICAL);

            wxStaticBoxSizer *cadre1 = new wxStaticBoxSizer(wxVERTICAL, panelAffichage, _T("Démarrer le jeu : "));
                wxButton *btn1=new wxButton(panelAffichage, BTN_LANCER_CISLATTACK_32, _T("Jouer ! (32 bits)"), wxDefaultPosition, wxSize(150, 25), 0);
                cadre1->Add(btn1, 0);
                wxButton *btn2=new wxButton(panelAffichage, BTN_LANCER_CISLATTACK_64, _T("Jouer ! (64 bits)"), wxDefaultPosition, wxSize(150, 25), 0);
                cadre1->Add(btn2, 0);
                wxButton *btn3=new wxButton(panelAffichage, BTN_LANCER_CISLATTACK_S, _T("Héberger un serveur"), wxDefaultPosition, wxSize(150, 25), 0);
                cadre1->Add(btn3, 0);
                cadre1->AddSpacer(2);

            wxStaticBoxSizer *cadre2 = new wxStaticBoxSizer(wxVERTICAL, panelAffichage, _T("Divers : "));
                wxButton *btn4=new wxButton(panelAffichage, BTN_LANCER_INDEV, _T("Version Indev"), wxDefaultPosition, wxSize(150, 25), 0);
                cadre2->Add(btn4, 0);
                cadre2->AddSpacer(2);
                wxButton *btn5=new wxButton(panelAffichage, BTN_LANCER_SITEWEB, _T("Site web"), wxDefaultPosition, wxSize(150, 25), 0);
                cadre2->Add(btn5, 0);
                cadre2->AddSpacer(2);
                wxButton *btn6=new wxButton(panelAffichage, BTN_LANCER_QUITTER, _T("Quitter"), wxDefaultPosition, wxSize(150, 25), 0);
                cadre2->Add(btn6, 0);

            sizer_intermed->Add(cadre1, 0, wxALIGN_CENTER | wxALL, 5);
            sizer_intermed->Add(cadre2, 0, wxALIGN_CENTER | wxALL, 5);

        panelAffichage->SetSizer(sizer_intermed);

    wxStaticLine *ligneHoriz = new wxStaticLine(panelAffichage, -1);
    sizer_intermed->Add(ligneHoriz, 0, wxALL | wxEXPAND, 5);


    wxStaticText *labelVersion = new wxStaticText(panelAffichage, -1, _T("  Launcher Cube Conflict v0.6.2"));
    sizer_intermed->Add(labelVersion, 0);

    frame->SetIcon(wxICON(aaaa));
    frame->Show();

    return true;
}
