//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id$
// Copyright (C) 2002 - 2005, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// events.cpp - Event handlers for frmMain
//
//
//////////////////////////////////////////////////////////////////////////

#include "pgAdmin3.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/settings.h>
#include <wx/treectrl.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <wx/tipdlg.h>
#include <wx/stc/stc.h>
#include <wx/busyinfo.h>

// App headers
#include "misc.h"
#include "menu.h"
#include "frmMain.h"
//#include "dlgServer.h"
#include "frmOptions.h"
#include "ctl/ctlSQLBox.h"
#include "pgConn.h"
#include "pgDatabase.h"
#include "pgSet.h"
#include "pgServer.h"
#include "pgObject.h"
#include "pgCollection.h"
#include "frmHelp.h"
#include "dlgProperty.h"
#include "frmUpdate.h"
#include "slFunctions.h"

extern wxString loadPath;


// Event table
BEGIN_EVENT_TABLE(frmMain, pgFrame)
    EVT_MENU(MNU_ACTION,                    frmMain::OnAction)
    EVT_MENU(MNU_CONTENTS,                  frmMain::OnContents)
    EVT_MENU(MNU_FAQ,                       frmMain::OnFaq)
    EVT_MENU(MNU_ONLINEUPDATE,              frmMain::OnOnlineUpdate)
    EVT_MENU(MNU_ONLINEUPDATE_NEWDATA,      frmMain::OnOnlineUpdateNewData)
    EVT_MENU(MNU_PGSQLHELP,                 frmMain::OnPgsqlHelp)
    EVT_MENU(MNU_REFRESH,                   frmMain::OnRefresh)
    EVT_MENU(MNU_DELETE,                    frmMain::OnDelete)
    EVT_MENU(MNU_DROP,                      frmMain::OnDrop)
    EVT_MENU(MNU_DROPCASCADED,              frmMain::OnDropCascaded)
    EVT_MENU(MNU_CREATE,                    frmMain::OnCreate)
    EVT_MENU(MNU_PROPERTIES,                frmMain::OnProperties)
    EVT_MENU(MNU_SAVEDEFINITION,            frmMain::OnSaveDefinition)
    EVT_MENU(MNU_SYSTEMOBJECTS,             frmMain::OnShowSystemObjects)
    EVT_MENU(MNU_TIPOFTHEDAY,               frmMain::OnTipOfTheDay)
    EVT_MENU(MNU_NEW+PG_COLUMN,             frmMain::OnNew)
    EVT_MENU(MNU_NEW+PG_PRIMARYKEY,         frmMain::OnNew)
    EVT_MENU(MNU_NEW+PG_FOREIGNKEY,         frmMain::OnNew)
    EVT_MENU(MNU_NEW+PG_UNIQUE,             frmMain::OnNew)
    EVT_MENU(MNU_NEW+PG_CHECK,              frmMain::OnNew)
    EVT_MENU(MNU_NEW+PG_INDEX,              frmMain::OnNew)
    EVT_MENU(MNU_NEW+PG_RULE,               frmMain::OnNew)
    EVT_MENU(MNU_NEW+PG_TRIGGER,            frmMain::OnNew)
    EVT_MENU(MNU_NEW+PGA_JOB,               frmMain::OnNew)
    EVT_MENU(MNU_NEW+PGA_STEP,              frmMain::OnNew)
    EVT_MENU(MNU_NEW+PGA_SCHEDULE,          frmMain::OnNew)
    EVT_MENU(MNU_NEW+SL_NODE,               frmMain::OnNew)
    EVT_MENU(MNU_NEW+SL_PATH,               frmMain::OnNew)
    EVT_MENU(MNU_NEW+SL_LISTEN,             frmMain::OnNew)
    EVT_MENU(MNU_NEW+SL_SET,                frmMain::OnNew)
    EVT_MENU(MNU_NEW+SL_SEQUENCE,           frmMain::OnNew)
    EVT_MENU(MNU_NEW+SL_TABLE,              frmMain::OnNew)
    EVT_MENU(MNU_NEW+SL_SUBSCRIPTION,       frmMain::OnNew)
    EVT_MENU(MNU_CHECKALIVE,                frmMain::OnCheckAlive)
    EVT_MENU(MNU_CONTEXTMENU,               frmMain::OnContextMenu) 
    EVT_MENU(MNU_SLONY_RESTART,             frmMain::OnRestartNode)
    EVT_MENU(MNU_SLONY_UPGRADE,             frmMain::OnUpgradeNode)
    EVT_MENU(MNU_SLONY_FAILOVER,            frmMain::OnFailover)
    EVT_MENU(MNU_SLONY_MERGESET,            frmMain::OnMergeSet)
    EVT_MENU(MNU_SLONY_MOVESET,             frmMain::OnMoveSet)

    EVT_NOTEBOOK_PAGE_CHANGED(CTL_NOTEBOOK, frmMain::OnPageChange)
    EVT_LIST_ITEM_SELECTED(CTL_PROPVIEW,    frmMain::OnPropSelChanged)
    EVT_LIST_ITEM_ACTIVATED(CTL_PROPVIEW,   frmMain::OnPropSelActivated)
    EVT_LIST_ITEM_RIGHT_CLICK(CTL_PROPVIEW, frmMain::OnPropRightClick)
    EVT_TREE_SEL_CHANGED(CTL_BROWSER,       frmMain::OnTreeSelChanged)
    EVT_TREE_ITEM_EXPANDING(CTL_BROWSER,    frmMain::OnExpand)
    EVT_TREE_ITEM_COLLAPSING(CTL_BROWSER,   frmMain::OnCollapse)
    EVT_TREE_ITEM_ACTIVATED(CTL_BROWSER,    frmMain::OnSelActivated)
    EVT_TREE_ITEM_RIGHT_CLICK(CTL_BROWSER,  frmMain::OnSelRightClick) 
    EVT_CLOSE(                              frmMain::OnClose)
#ifdef __WXGTK__
    EVT_TREE_KEY_DOWN(CTL_BROWSER,          frmMain::OnTreeKeyDown)
#endif
END_EVENT_TABLE()




// unfortunately, under GTK we won't get the original wxKeyEvent
// to reset m_metaDown
void frmMain::OnTreeKeyDown(wxTreeEvent& event)
{
    switch (event.GetKeyCode())
    {
	case WXK_F1:
	    OnHelp(event);
	    break;
	case WXK_F5:
	    OnRefresh(event);
	    break;
	case WXK_DELETE:
	    OnDrop(event);
	    break;
	default:
	    event.Skip();
	    break;
    }
}


void frmMain::OnExit(wxCommandEvent& event)
{
    Close(false);   // Allow sub windows to stop us
    event.Skip();
}



void frmMain::OnClose(wxCloseEvent& event)
{
    wxWindow *fr;
    windowList::Node *node;
    while ((node=frames.GetFirst()) != NULL)
    {
        fr=node->GetData();
        if (!fr->Close(!event.CanVeto()))
        {
            if (event.CanVeto())
            {
                event.Veto();
                return;
            }
        }
        delete node;
        fr->Destroy();
    }
    Destroy();
}


void frmMain::OnAction(wxCommandEvent &ev)
{
    actionFactory *af=actionFactory::GetFactory(ev.GetId());
    if (af)
    {
        wxWindow *wnd=af->StartDialog(this, currentObject);
        if (wnd)
            AddFrame(wnd);
    }
}


void frmMain::OnTipOfTheDay(wxCommandEvent& WXUNUSED(event))
{
    extern wxString docPath;
    extern wxLocale *locale;

    wxString file;
    
    file = docPath + wxT("/") + locale->GetCanonicalName() + wxT("/tips.txt");

    if (!wxFile::Exists(file))
        file = docPath + wxT("/en_US/tips.txt");    

    if (!wxFile::Exists(file)) {
        wxLogError(_("Couldn't open a tips.txt file!"));
        return;
    }

    wxTipProvider *tipProvider = wxCreateFileTipProvider(file, settings->GetNextTipOfTheDay());
    settings->SetShowTipOfTheDay(wxShowTip(this, tipProvider));
    settings->SetNextTipOfTheDay(tipProvider->GetCurrentTip());

    delete tipProvider;
}



void frmMain::OnOnlineUpdate(wxCommandEvent &event)
{
    frmUpdate *upd=new frmUpdate(this);
    upd->Show();
}


void frmMain::OnOnlineUpdateNewData(wxCommandEvent &event)
{
    wxLogError(__("Could not contact pgAdmin web site to check for updates.\nMaybe your proxy option setting need adjustment."));
}



void frmMain::OnContents(wxCommandEvent& event)
{
    DisplayHelp(this, wxT("index"));
}


void frmMain::OnPgsqlHelp(wxCommandEvent& event)
{
    DisplaySqlHelp(this, wxT("index"));
}


void frmMain::OnFaq(wxCommandEvent& event)
{
    frmHelp *h=new frmHelp(this);
    h->Show(true);
    if (!h->Load(wxT("http://www.pgadmin.org/faq/")))
        h->Destroy();
}


wxString frmMain::GetHelpPage() const
{
    wxString page;

    if (currentObject)
        page=currentObject->GetHelpPage(true);

    if (page.IsEmpty())
        page = wxT("pg/sql-commands");

    return page;
}


void frmMain::OnCollapse(wxTreeEvent &event)
{
#ifdef WIN32
    // This is weird stuff, but somewhere comes a collapse after we have done
    // connecting the server and expanding the tree.
    // Possibly not necessary 
    if (event.GetItem() == denyCollapseItem)
        event.Veto();
#endif
    denyCollapseItem=wxTreeItemId();
}


void frmMain::OnExpand(wxTreeEvent &event)
{
    wxCookieType cookie;
    wxTreeItemId item=browser->GetFirstChild(event.GetItem(), cookie);
    if (item && !browser->GetItemData(item))
    {
        // the expanding node has a dummy item.
        // delete dummy item, and expand kids.
        execSelChange(event.GetItem(), browser->GetSelection() == item);

        // we don't have any kids, so don't expand
        if (!browser->GetChildrenCount(event.GetItem()))
            event.Veto();
    }
}


void frmMain::OnCheckAlive(wxCommandEvent &event)
{
    checkAlive();
}


void frmMain::OnSaveDefinition(wxCommandEvent& event)
{

    wxLogInfo(wxT("Saving object definition"));

    if (sqlPane->GetText().IsNull()) {
        wxLogError(__("There is nothing in the SQL pane to save!"));
        return;
    }

    wxFileDialog filename(this, _("Select output file"), wxT(""), wxT(""), _("SQL Scripts (*.sql)|*.sql|All files (*.*)|*.*"));
    filename.SetStyle(wxSAVE | wxOVERWRITE_PROMPT);

    // Show the dialogue
    if (filename.ShowModal() == wxID_OK)
    {
        // Write the file
        if (!FileWrite(filename.GetPath(), sqlPane->GetText()))
            wxLogError(__("Failed to write to the output file: %s"), filename.GetPath().c_str());
    }
    else
    {
        wxLogInfo(wxT("User cancelled"));
    }
}

void frmMain::OnShowSystemObjects(wxCommandEvent& event)
{
    // Warn the user
    int rc;

	wxMessageDialog *dlg;
    if (settings->GetShowSystemObjects())
	{
		dlg=new wxMessageDialog(this, 
							  _("System objects will not be removed from the object tree until a refresh is performed.\nClose all connections now?"),
							  _("Hide system objects"),
							  wxYES_NO|wxCANCEL | wxICON_QUESTION);
	}
    else
	{
        dlg=new wxMessageDialog(this,
								_("System objects will not show in the the object tree until a refresh is performed.\nClose all connections now?"),
								_("Show system objects"),
								wxYES_NO|wxCANCEL | wxICON_QUESTION);
	}
	dlg->CenterOnParent();
	rc=dlg->ShowModal();
	delete dlg;

    if (rc == wxID_CANCEL)
    {
        viewMenu->Check(MNU_SYSTEMOBJECTS, settings->GetShowSystemObjects());
        return;
    }

    settings->SetShowSystemObjects(!settings->GetShowSystemObjects());
    viewMenu->Check(MNU_SYSTEMOBJECTS, settings->GetShowSystemObjects());

    if (rc == wxID_YES)
    {
        wxLogInfo(wxT("Clearing treeview to toggle ShowSystemObjects"));

		// Store the servers to prevent dropped ones reappearing in a minute.
		StoreServers();

        // Clear the treeview
        browser->DeleteAllItems();

        // Add the root node
        serversObj = new pgServerCollection(*serverFactory.GetCollectionFactory());
        wxTreeItemId servers = browser->AddRoot(wxGetTranslation(serverFactory.GetCollectionFactory()->GetTypeName()),
            serversObj->GetIconId(), -1, serversObj);

        RetrieveServers();
        browser->Expand(servers);
        browser->SelectItem(servers);
#ifdef __WIN32__
        denyCollapseItem = servers;
#endif
    }
}


void frmMain::OnPropSelChanged(wxListEvent& event)
{
    if (properties->GetSelectedItemCount() == 1)
    {
        wxTreeItemId item=browser->GetSelection();
        pgObject *data=(pgObject*)browser->GetItemData(item);
        if (data && data->IsCollection())
        {
            currentObject=((pgCollection*)data)->FindChild(browser, event.GetIndex());
            if (currentObject)
            {
                setDisplay(currentObject);
                sqlPane->SetReadOnly(false);
                sqlPane->SetText(currentObject->GetSql(browser));
                sqlPane->SetReadOnly(true);
            }
        }
    }
}


void frmMain::OnPropSelActivated(wxListEvent& event)
{
    if (currentObject && currentObject->CanEdit())
    {
        wxCommandEvent nullEvent;
        OnProperties(nullEvent);
    }
}


void frmMain::OnPropRightClick(wxListEvent& event)
{
    OnPropSelChanged(event);

    if (currentObject)
        doPopup(properties, event.GetPoint(), currentObject);
}


void frmMain::OnTreeSelChanged(wxTreeEvent& event)
{
    denyCollapseItem=wxTreeItemId();
	// Reset the listviews/SQL pane
    if (event.GetItem())
        execSelChange(event.GetItem(), true);
}


void frmMain::execSelChange(wxTreeItemId item, bool currentNode)
{
    if (currentNode)
    {
        properties->ClearAll();
        properties->AddColumn(_("Properties"), 500);
        properties->InsertItem(0, _("No properties are available for the current selection"), PGICON_PROPERTY);

        sqlPane->Clear();

        // Reset the toolbar & password menu options
	    // Handle the menus associated with the buttons
        SetButtons(0);
    }

    // Get the item data, and feed it to the relevant handler,
    // cast as required.
    currentObject = (pgObject *)browser->GetItemData(item);

    // If we didn't get an object, then we may have a right click, or 
    // invalid click, so ignore.
    if (!currentObject) return;

    if (currentNode)
    {
        properties->Freeze();
        setDisplay(currentObject, properties, sqlPane);
        properties->Thaw();
        ShowObjStatistics(currentObject, listViews->GetSelection());
    }
    else
        setDisplay(currentObject, 0, 0);
}


void frmMain::setDisplay(pgObject *data, ctlListView *props, ctlSQLBox *sqlbox)
{
    data->RemoveDummyChild(browser);

    int type = data->GetType();
    pgServer *server=0;


    bool showTree=true;

    switch (type)
    {
//        case PG_SCHEMA:
//        case PG_SCHEMAS:
        case PG_CONSTRAINTS:
        case PG_FOREIGNKEY:
        case PG_CHECK:
        case PG_COLUMNS:
        case PG_COLUMN:
        case PG_PRIMARYKEY:
        case PG_UNIQUE:
        case PG_INDEXES:
        case PG_INDEX:
        case PG_RULES:
        case PG_RULE:
        case PG_TRIGGERS:
        case PG_TRIGGER:

        case PGA_JOBS:
        case PGA_JOB:
        case PGA_STEPS:
        case PGA_STEP:
		case PGA_SCHEDULES:
        case PGA_SCHEDULE:
        case SL_NODE:
        case SL_NODES:
        case SL_PATH:
        case SL_PATHS:
        case SL_LISTEN:
        case SL_LISTENS:
        case SL_SET:
        case SL_SETS:
        case SL_SEQUENCE:
        case SL_SEQUENCES:
        case SL_TABLE:
        case SL_TABLES:
        case SL_SUBSCRIPTION:
        case SL_SUBSCRIPTIONS:
            break;
        default:
        {
            pgaFactory *factory=data->GetFactory();
            if (factory)
            {
                if (factory == &serverFactory)
                {
                    StartMsg(_("Retrieving server properties"));
    
                    server = (pgServer *)data;

                    data->ShowTree(this, browser, props, sqlbox);
                    showTree=false;
                    EndMsg();
                }
                else
                    showTree=true;
            }
            else
                showTree=false;
			break;
        }
    }

    if (showTree)
        data->ShowTree(this, browser, props, sqlbox);

    if (sqlbox)
    {
        sqlbox->SetReadOnly(false);
        sqlbox->SetText(data->GetSql(browser));
        sqlbox->SetReadOnly(true);
    }

    pgConn *conn=data->GetConnection();
    if (conn && conn->GetStatus() == PGCONN_BROKEN)
    {
        checkAlive();
        return;
    }
    unsigned int i;
    wxMenuItem *menuItem;
    i=newMenu->GetMenuItemCount();
    while (i--)
    {
        menuItem=newMenu->GetMenuItems().Item(i)->GetData();
        if (menuItem)
            delete newMenu->Remove(menuItem);
    }

    i=newContextMenu->GetMenuItemCount();
    while (i--)
    {
        menuItem=newContextMenu->GetMenuItems().Item(i)->GetData();
        if (menuItem)
            delete newContextMenu->Remove(menuItem);
    }

    editMenu->Enable(MNU_NEWOBJECT, false);

    wxMenu *indivMenu=data->GetNewMenu();
    if (indivMenu)
    {
        if (indivMenu->GetMenuItemCount())
        {
            editMenu->Enable(MNU_NEWOBJECT, true);

            for (i=0 ; i < indivMenu->GetMenuItemCount() ; i++)
            {
                menuItem=indivMenu->GetMenuItems().Item(i)->GetData();
                newMenu->Append(menuItem->GetId(), menuItem->GetLabel(), menuItem->GetHelp());
                newContextMenu->Append(menuItem->GetId(), menuItem->GetLabel(), menuItem->GetHelp());
            }
        }
        delete indivMenu;
    }
    else
    {
    }
    actionFactory::CheckMenu(data, menuBar, toolBar);

    // !!!!! interate submenus to check if any is enabled
//    toolsMenu->Enable(MNU_CONFIGSUBMENU, checkKids);

}


void frmMain::OnSelActivated(wxTreeEvent &event)
{
    // This handler will primarily deal with displaying item
    // properties in seperate windows and 'Add xxx...' clicks

    // Get the item data, and feed it to the relevant handler,
    // cast as required.

    wxTreeItemId item = event.GetItem();
    pgObject *data = (pgObject *)browser->GetItemData(item);
    if (!data)
        return;
    pgServer *server;
    wxCommandEvent nullEvent;

    if (data->IsCreatedBy(serverFactory))
    {
        server = (pgServer *)data;
        if (!server->GetConnected())
        {
            if (ReconnectServer(server) == PGCONN_OK)
            {
                // prevent from being collapsed immediately

                denyCollapseItem=item;
            }
        }
    }
    else
    {
        if (settings->GetDoubleClickProperties())
        {
            if (data->CanEdit())
            {
                OnProperties(nullEvent);
                event.Skip();
                return;
            }
        }
    }

#ifndef __WXMSW__
    browser->Expand(item);
#endif
}


void frmMain::appendIfEnabled(int id)
{
    if (menuBar->IsEnabled(id))
    {
        wxMenuItem *menuItem=menuBar->FindItem(id);
        if (menuItem)
            treeContextMenu->Append(id, menuItem->GetLabel(), menuItem->GetHelp());
    }
}

void frmMain::doPopup(wxWindow *win, wxPoint point, pgObject *object)
{
    if (treeContextMenu)
        delete treeContextMenu;

    treeContextMenu = new wxMenu();

    appendIfEnabled(MNU_REFRESH);

    if (object)
    {
        wxMenu *indivMenu=object->GetNewMenu();
        if (indivMenu)
        {
            if (indivMenu->GetMenuItemCount() > 1)
            {
                wxMenuItem *menuItem = menuBar->FindItem(MNU_NEWOBJECT);
                treeContextMenu->Append(MNU_NEWOBJECT, menuItem->GetLabel(), indivMenu, menuItem->GetHelp());
            }
            else
            {
                if (indivMenu->GetMenuItemCount() == 1)
                {
                    wxMenuItem *menuItem=indivMenu->GetMenuItems().Item(0)->GetData();
                    treeContextMenu->Append(menuItem->GetId(), menuItem->GetLabel(), menuItem->GetHelp());
                }
                delete indivMenu;
            }
        }
    }

    int currentSize = treeContextMenu->GetMenuItemCount();


    actionFactory::AppendEnabledMenus(menuBar, treeContextMenu);

    appendIfEnabled(MNU_SLONY_RESTART);
    appendIfEnabled(MNU_SLONY_UPGRADE);
    appendIfEnabled(MNU_SLONY_FAILOVER);
    appendIfEnabled(MNU_SLONY_MERGESET);
    appendIfEnabled(MNU_SLONY_MOVESET);


    int newSize = treeContextMenu->GetMenuItemCount();
    if (newSize > currentSize)
    {
        treeContextMenu->InsertSeparator(currentSize);
        currentSize = newSize +1;
    }

    appendIfEnabled(MNU_DROP);
    appendIfEnabled(MNU_DROPCASCADED);
    appendIfEnabled(MNU_PROPERTIES);


    newSize = treeContextMenu->GetMenuItemCount();
    if (newSize > currentSize)
    {
        treeContextMenu->InsertSeparator(currentSize);
        currentSize = newSize +1;
    }

    if (currentSize)
        win->PopupMenu(treeContextMenu, point);
}

////////////////////////////////////////////////////////////////////////////////
// This handler will display a popup menu for the currently selected item
////////////////////////////////////////////////////////////////////////////////
void frmMain::OnContextMenu(wxCommandEvent& event)
{
    wxPoint point;

    if (FindFocus() == browser)
    {
        wxRect rect;
        wxTreeItemId item=browser->GetSelection();

        browser->GetBoundingRect(item, rect);
        point = rect.GetPosition();
	    wxPoint origin = GetClientAreaOrigin();

	    // Because this Tree is inside a vertical splitter, we
	    // must compensate for the size of the other elements
	    point.x += origin.x;
	    point.y += origin.y;

        doPopup(this, point, (pgObject*)browser->GetItemData(item));
    }

}


////////////////////////////////////////////////////////////////////////////////
// This handler will display a popup menu for the item at the mouse position
////////////////////////////////////////////////////////////////////////////////
void frmMain::OnSelRightClick(wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    if (item != browser->GetSelection())
    {
        browser->SelectItem(item);
        currentObject = (pgObject*)browser->GetItemData(item);
    }

    if (currentObject)
        doPopup(browser, event.GetPoint(), currentObject);
}


void frmMain::OnDelete(wxCommandEvent &ev)
{
    OnDrop(ev);
}


void frmMain::OnDropCascaded(wxCommandEvent &ev)
{
    execDrop(ev, true);
}


void frmMain::OnDrop(wxCommandEvent &ev)
{
    execDrop(ev, false);
}


void frmMain::execDrop(wxCommandEvent &ev, bool cascaded)
{
    wxTreeItemId item=browser->GetSelection();
    pgCollection *collection = (pgCollection*)browser->GetItemData(item);

    if (collection == currentObject)
        dropSingleObject(currentObject, true, cascaded);
    else
    {
        if (collection && collection->IsCollection())
        {
            long index=properties->GetFirstSelected();

            if (index >= 0)
            {
                pgObject *data=collection->FindChild(browser, index);

                if (!data || !data->CanDrop())
                    return;

                if (properties->GetSelectedItemCount() == 1)
                {
                    dropSingleObject(data, true, cascaded);
                }
                else
                {
                    if (cascaded || data->RequireDropConfirm() || settings->GetConfirmDelete())
                    {
                        wxString text, caption;
                        if (cascaded)
                        {
                            text = _("Are you sure you wish to drop multiple objects including all objects that depend on them?");
                            caption = _("Drop multiple objects cascaded?");
                        }
                        else
                        {
                            text = _("Are you sure you wish to drop multiple objects?");
                            caption = _("Drop multiple objects?");
                        }
                        wxMessageDialog msg(this, text, caption, wxYES_NO | wxICON_QUESTION);
                        if (msg.ShowModal() != wxID_YES)
                        {
                            return;
                        }
                    }

                    bool done=true;
                    long count=0;
                    while (done && data && index >= 0)
                    {
                        if (data->GetSystemObject())
                        {
                            wxMessageDialog msg(this, wxString::Format(_("Cannot drop system %s"), 
                                data->GetTranslatedTypeName().c_str(), ""), 
                                _("Trying to drop system object"), wxICON_EXCLAMATION);
                            msg.ShowModal();
                            return;
                        }

                        done = dropSingleObject(data, false, cascaded);

                        if (done)
                        {
                            properties->DeleteItem(index);
                            count++;
                            index = properties->GetFirstSelected();

                            if (index >= 0)
                                data=collection->FindChild(browser, index);
                        }
                    }
                }
                Refresh(collection);
            }
        }
    }
}


bool frmMain::dropSingleObject(pgObject *data, bool updateFinal, bool cascaded)
{
    if (updateFinal)
    {
        // accelerator can bypass disabled menu, so we need to check
        if (!data || !data->CanDrop())
            return false;

        if (data->GetSystemObject())
        {
            wxMessageDialog msg(this, wxString::Format(_("Cannot drop system %s %s."), 
                data->GetTranslatedTypeName().c_str(), data->GetFullIdentifier().c_str()), 
                _("Trying to drop system object"), wxICON_EXCLAMATION);
            msg.ShowModal();
            return false;
        }

        if (cascaded || data->RequireDropConfirm() || settings->GetConfirmDelete())
        {
            wxString text, caption;
            if (cascaded)
            {
                text = wxString::Format(_("Are you sure you wish to drop %s %s including all objects that depend on it?"),
                    data->GetTranslatedTypeName().c_str(), data->GetFullIdentifier().c_str());
                caption = wxString::Format(_("Drop %s cascaded?"), data->GetTranslatedTypeName().c_str());
            }
            else
            {
                text = wxString::Format(_("Are you sure you wish to drop %s %s?"),
                    data->GetTranslatedTypeName().c_str(), data->GetFullIdentifier().c_str());
                caption = wxString::Format(_("Drop %s?"), data->GetTranslatedTypeName().c_str());
            }
            wxMessageDialog msg(this, text, caption, wxYES_NO | wxICON_QUESTION);
            if (msg.ShowModal() != wxID_YES)
            {
                return false;
            }
        }
    }
    bool done=data->DropObject(this, browser, cascaded);

    if (done)
    {
        wxLogInfo(wxT("Dropping %s %s"), data->GetTypeName().c_str(), data->GetIdentifier().c_str());

        wxTreeItemId parentItem=browser->GetItemParent(data->GetId());

        if (updateFinal)
        {
            wxTreeItemId nextItem=browser->GetNextVisible(data->GetId());
            if (nextItem)
            {
                pgObject *nextData=(pgObject*)browser->GetItemData(nextItem);
                if (!nextData || nextData->GetType() != data->GetType())
                    nextItem=browser->GetPrevSibling(data->GetId());
            }
            else
                nextItem=browser->GetPrevSibling(data->GetId());

            if (nextItem)
                browser->SelectItem(nextItem);
        }
        int droppedType = data->GetType();
        browser->Delete(data->GetId());
        // data is invalid now


        if (updateFinal)
        {
            pgCollection *collection=0;

            while (parentItem)
            {
                collection = (pgCollection*)browser->GetItemData(parentItem);
                if (collection && collection->IsCollection() && collection->IsCollectionForType(droppedType))
                {
                    collection->UpdateChildCount(browser);
                    break;
                }
                parentItem=browser->GetItemParent(parentItem);
            }
        }
    }
    return done;
}


void frmMain::OnRefresh(wxCommandEvent &ev)
{
    // Refresh - Clear the treeview below the current selection
    // this doesn't use currentObject deliberately!

    wxTreeItemId item=browser->GetSelection();
    pgObject *data = (pgObject*)browser->GetItemData(item);
    if (!data)
        return;

    Refresh(data);
}


void frmMain::OnCreate(wxCommandEvent &ev)
{
    if (currentObject)
    {
        if (!dlgProperty::CreateObjectDialog(this, currentObject, -1))
            checkAlive();
    }
}


void frmMain::OnNew(wxCommandEvent &ev)
{
    int type=ev.GetId() - MNU_NEW;
    if (pgaFactory::GetFactory(type) == &serverFactory)
    {
        if (currentObject && currentObject->IsCreatedBy(serverFactory))
        {
            pgServer *server=(pgServer*)currentObject;
            if (!server->GetConnected())
                ReconnectServer(server);
        }
        return;
    }

    if (currentObject)
    {
        if (!dlgProperty::CreateObjectDialog(this, currentObject, type))
            checkAlive();
    }
}


void frmMain::OnProperties(wxCommandEvent &ev)
{
    if (currentObject)
    {
        if (!dlgProperty::EditObjectDialog(this, sqlPane, currentObject))
            checkAlive();
    }
}


void frmMain::OnMergeSet(wxCommandEvent& event)
{
    if (currentObject)
    {
        if (!slFunctions::MergeSet(this, currentObject))
            checkAlive();
    }
}


void frmMain::OnMoveSet(wxCommandEvent& event)
{
    if (currentObject)
    {
        if (!slFunctions::MoveSet(this, currentObject))
            checkAlive();
    }
}


void frmMain::OnFailover(wxCommandEvent& event)
{
    if (currentObject)
    {
        if (!slFunctions::Failover(this, currentObject))
            checkAlive();
    }
}


void frmMain::OnUpgradeNode(wxCommandEvent& event)
{
    if (currentObject)
    {
        if (!slFunctions::UpgradeNode(this, currentObject))
            checkAlive();
    }
}


void frmMain::OnRestartNode(wxCommandEvent& event)
{
    if (currentObject)
    {
        if (!slFunctions::RestartNode(this, currentObject))
            checkAlive();
    }
}
