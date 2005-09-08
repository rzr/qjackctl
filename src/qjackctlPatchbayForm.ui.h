// qjackctlPatchbayForm.ui.h
//
// ui.h extension file, included from the uic-generated form implementation.
/****************************************************************************
   Copyright (C) 2003-2004, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#include "qjackctlAbout.h"
#include "qjackctlPatchbayFile.h"

#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qfileinfo.h>


// Kind of constructor.
void qjackctlPatchbayForm::init (void)
{
    // Create the patchbay view object.
    m_pPatchbay = new qjackctlPatchbay(PatchbayView);
    m_iUntitled = 0;

    m_bActivePatchbay = false;

    // Connect it to some UI feedback slot.
    QObject::connect(PatchbayView->OListView(), SIGNAL(selectionChanged()), this, SLOT(stabilizeForm()));
    QObject::connect(PatchbayView->IListView(), SIGNAL(selectionChanged()), this, SLOT(stabilizeForm()));
    // Dirty patchbay dispatcher (stabilization deferral).
    QObject::connect(PatchbayView, SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));

    newPatchbayFile(false);
}


// Kind of destructor.
void qjackctlPatchbayForm::destroy (void)
{
    // May delete the patchbay view object.
    delete m_pPatchbay;
}


// Notify our parent that we're emerging.
void qjackctlPatchbayForm::showEvent ( QShowEvent *pShowEvent )
{
    qjackctlMainForm *pMainForm = (qjackctlMainForm *) QWidget::parentWidget();
    if (pMainForm)
        pMainForm->stabilizeForm();

    stabilizeForm();

    QWidget::showEvent(pShowEvent);
}


// Notify our parent that we're closing.
void qjackctlPatchbayForm::hideEvent ( QHideEvent *pHideEvent )
{
    QWidget::hideEvent(pHideEvent);

    qjackctlMainForm *pMainForm = (qjackctlMainForm *) QWidget::parentWidget();
    if (pMainForm)
        pMainForm->stabilizeForm();
}


// Window close event handlers.
bool qjackctlPatchbayForm::queryClose (void)
{
    bool bQueryClose = true;

    if (PatchbayView->dirty()) {
        switch (QMessageBox::warning(this,
			QJACKCTL_TITLE ": " + tr("Warning"),
            tr("The patchbay definition has been changed:") + "\n\n" +
            "\"" + m_sPatchbayName +  "\"\n\n" +
            tr("Do you want to save the changes?"),
            tr("Save"), tr("Discard"), tr("Cancel"))) {
        case 0:     // Save...
            savePatchbay();
            // Fall thru....
        case 1:     // Discard
            break;
        default:    // Cancel.
            bQueryClose = false;
        }
    }

    return bQueryClose;
}


// Contents change deferrer slot...
void qjackctlPatchbayForm::contentsChanged (void)
{
    qjackctlMainForm *pMainForm = (qjackctlMainForm *) QWidget::parentWidget();
    if (pMainForm)
        pMainForm->refreshPatchbay();
}


// Refresh complete form.
void qjackctlPatchbayForm::refreshForm ( void )
{
    m_pPatchbay->refresh();
    stabilizeForm();
}


// A helper stabilization slot.
void qjackctlPatchbayForm::stabilizeForm ( void )
{
    SavePatchbayPushButton->setEnabled(PatchbayView->dirty());
    ActivatePatchbayPushButton->setEnabled(QFileInfo(m_sPatchbayPath).exists());

	qjackctlMainForm *pMainForm = (qjackctlMainForm *) QWidget::parentWidget();
	m_bActivePatchbay = (pMainForm && pMainForm->isActivePatchbay(m_sPatchbayPath));
    ActivatePatchbayPushButton->setOn(m_bActivePatchbay);

	QString sPatchbay = m_sPatchbayPath;
	if (PatchbayView->dirty()) {
		PatchbayComboBox->setCurrentText(m_sPatchbayName + " [" + tr("modified") + "]");
		sPatchbay += " *";
	}
	setCaption(QJACKCTL_TITLE ": " + tr("Patchbay - [%1]").arg(sPatchbay));

    qjackctlSocketItem *pSocketItem = (m_pPatchbay->OSocketList())->selectedSocketItem();
    if (pSocketItem) {
        OSocketEditPushButton->setEnabled(true);
        OSocketCopyPushButton->setEnabled(true);
        OSocketRemovePushButton->setEnabled(true);
        OSocketMoveUpPushButton->setEnabled(pSocketItem->itemAbove() != NULL);
        OSocketMoveDownPushButton->setEnabled(pSocketItem->nextSibling() != NULL);
    } else {
        OSocketEditPushButton->setEnabled(false);
        OSocketCopyPushButton->setEnabled(false);
        OSocketRemovePushButton->setEnabled(false);
        OSocketMoveUpPushButton->setEnabled(false);
        OSocketMoveDownPushButton->setEnabled(false);
    }

    pSocketItem = (m_pPatchbay->ISocketList())->selectedSocketItem();
    if (pSocketItem) {
        ISocketEditPushButton->setEnabled(true);
        ISocketCopyPushButton->setEnabled(true);
        ISocketRemovePushButton->setEnabled(true);
        ISocketMoveUpPushButton->setEnabled(pSocketItem->itemAbove() != NULL);
        ISocketMoveDownPushButton->setEnabled(pSocketItem->nextSibling() != NULL);
    } else {
        ISocketEditPushButton->setEnabled(false);
        ISocketCopyPushButton->setEnabled(false);
        ISocketRemovePushButton->setEnabled(false);
        ISocketMoveUpPushButton->setEnabled(false);
        ISocketMoveDownPushButton->setEnabled(false);
    }

    ConnectPushButton->setEnabled(m_pPatchbay->canConnectSelected());
    DisconnectPushButton->setEnabled(m_pPatchbay->canDisconnectSelected());
    DisconnectAllPushButton->setEnabled(m_pPatchbay->canDisconnectAll());
}


// Patchbay path accessor.
QString& qjackctlPatchbayForm::patchbayPath (void)
{
    return m_sPatchbayPath;
}


// Reset patchbay definition from scratch.
void qjackctlPatchbayForm::newPatchbayFile ( bool bSnapshot )
{
    m_pPatchbay->clear();
    m_sPatchbayPath = QString::null;
    m_sPatchbayName = tr("Untitled") + QString::number(m_iUntitled++);
    if (bSnapshot)
        m_pPatchbay->connectionsSnapshot();
    stabilizeForm();
}


// Load patchbay definitions from specific file path.
bool qjackctlPatchbayForm::loadPatchbayFile ( const QString& sFileName )
{
    // Check if we're going to discard safely the current one...
    if (!queryClose())
        return false;

    // We'll have a temporary rack...
    qjackctlPatchbayRack rack;
    // Step 1: load from file...
    if (!qjackctlPatchbayFile::load(&rack, sFileName)) {
        QMessageBox::critical(this,
			QJACKCTL_TITLE ": " + tr("Load error"),
            tr("Could not load patchbay definition file:") + "\n\n\"" + sFileName + "\"",
            tr("Cancel"));
        // Reset/disable further trials.
        m_sPatchbayPath = QString::null;
        return false;
    }
    // Step 2: load from rack...
    m_pPatchbay->loadRack(&rack);

    // Step 3: stabilize form...
    m_sPatchbayPath = sFileName;
    m_sPatchbayName = QFileInfo(sFileName).baseName();
    updateRecentPatchbays(m_sPatchbayPath);
    
    return true;
}


// Save current patchbay definition to specific file path.
bool qjackctlPatchbayForm::savePatchbayFile ( const QString& sFileName )
{
    // We'll have a temporary rack...
    qjackctlPatchbayRack rack;
    // Step 1: save to rack...
    m_pPatchbay->saveRack(&rack);
    // Step 2: save to file...
    if (!qjackctlPatchbayFile::save(&rack, sFileName)) {
        QMessageBox::critical(this,
			QJACKCTL_TITLE ": " + tr("Save error"),
            tr("Could not save patchbay definition file:") + "\n\n\"" + sFileName + "\"",
            tr("Cancel"));
        return false;
    }
    // Step 3: stabilize form...
    m_sPatchbayPath = sFileName;
    m_sPatchbayName = QFileInfo(sFileName).baseName();
    updateRecentPatchbays(m_sPatchbayPath);

    // Step 4: notify main form if applicable ...
	qjackctlMainForm *pMainForm = (qjackctlMainForm *) QWidget::parentWidget();
	m_bActivePatchbay = (pMainForm && pMainForm->isActivePatchbay(m_sPatchbayPath));
	if (m_bActivePatchbay)
		pMainForm->updateActivePatchbay();

	return true;
}


// Create a new patchbay definition from scratch.
void qjackctlPatchbayForm::newPatchbay()
{
    // Assume a snapshot from scratch...
    bool bSnapshot = false;

	// Ask user what he/she wants to do...
	if (m_pPatchbay->jackClient() || m_pPatchbay->alsaSeq()) {
		switch (QMessageBox::information(this,
			QJACKCTL_TITLE ": " + tr("New Patchbay definition"),
			tr("Create patchbay definition as a snapshot") + "\n" +
			tr("of all actual client connections?"),
			tr("Yes"), tr("No"), tr("Cancel"))) {
		case 0:		// Yes.
			bSnapshot = true;
			break;
		case 1:		// No.
			bSnapshot = false;
			break;
		default:	// Cancel.
			return;
		}
	}

    // Check if we can discard safely the current one...
    if (!queryClose())
        return;

    // Reset patchbay editor.
    newPatchbayFile(bSnapshot);
}


// Load patchbay definitions from file.
void qjackctlPatchbayForm::loadPatchbay()
{
    QString sFileName = QFileDialog::getOpenFileName(
		m_sPatchbayPath,                                // Start here.
		tr("Patchbay Definition files") + " (*.xml)",   // Filter (XML files)
		this, 0,                                        // Parent and name (none)
		QJACKCTL_TITLE ": " + tr("Load Patchbay Definition")	// Caption.
    );

    if (sFileName.isEmpty())
        return;

    // Load it right away.
    loadPatchbayFile(sFileName);
}


// Save current patchbay definition to file.
void qjackctlPatchbayForm::savePatchbay()
{
    QString sFileName = QFileDialog::getSaveFileName(
		m_sPatchbayPath,                                // Start here.
		tr("Patchbay Definition files") + " (*.xml)",   // Filter (XML files)
		this, 0,                                        // Parent and name (none)
		QJACKCTL_TITLE ": " + tr("Save Patchbay Definition")	// Caption.
    );

    if (sFileName.isEmpty())
        return;

    // Enforce .xml extension...
    if (QFileInfo(sFileName).extension().isEmpty())
        sFileName += ".xml";

    // Save it right away.
    savePatchbayFile(sFileName);
}


// A new patchbay has been selected
void qjackctlPatchbayForm::selectPatchbay ( int iPatchbay )
{
	// Remember and avoid reloading the previous (first) selected one.
	if (iPatchbay > 0 && iPatchbay < (int) m_recentPatchbays.count()) {
		// If we cannot load the new one, backout...
		if (!loadPatchbayFile(m_recentPatchbays[iPatchbay]))
			PatchbayComboBox->setCurrentItem(0);
	}
}


// Set current active patchbay definition file.
void qjackctlPatchbayForm::toggleActivePatchbay()
{
    // Check if we're going to discard safely the current one...
    if (!queryClose())
        return;

	// Activate it...
	qjackctlMainForm *pMainForm = (qjackctlMainForm *) QWidget::parentWidget();
	if (pMainForm) {
		pMainForm->setActivePatchbay(
			m_bActivePatchbay ? QString::null : m_sPatchbayPath);
	}

	// Need to force/refresh the patchbay list...
	updateRecentPatchbays(m_sPatchbayPath);
}


// Set/initialize the MRU patchbay list.
void qjackctlPatchbayForm::setRecentPatchbays ( const QStringList& patchbays )
{
	m_recentPatchbays = patchbays;
}


// Update patchbay MRU variables and widgets.
void qjackctlPatchbayForm::updateRecentPatchbays ( const QString& sPatchbayPath )
{
	// Remove from list if already there (avoid duplicates)...
	QStringList::Iterator iter = m_recentPatchbays.find(sPatchbayPath);
	if (iter != m_recentPatchbays.end())
		m_recentPatchbays.remove(iter);
	// Put it to front...
	m_recentPatchbays.push_front(sPatchbayPath);

	// Time to keep the list under limits.
	while (m_recentPatchbays.count() > 8)
		m_recentPatchbays.pop_back();

	// Update the main setup list...
	qjackctlMainForm *pMainForm = (qjackctlMainForm *) parentWidget();
	if (pMainForm)
		pMainForm->setRecentPatchbays(m_recentPatchbays);

	// Update the visible combobox...
	const QPixmap pixmap = QPixmap::fromMimeSource("patchbay1.png");
	PatchbayComboBox->clear();
	for (iter = m_recentPatchbays.begin();
			iter != m_recentPatchbays.end(); ++iter) {
		QString sText = QFileInfo(*iter).baseName();
		if (pMainForm && pMainForm->isActivePatchbay(*iter))
			sText += " [" + tr("active") + "]";
		PatchbayComboBox->insertItem(pixmap, sText);
	}

	// Sure this one must be currently selected.
	PatchbayComboBox->setCurrentItem(0);
	stabilizeForm();
}


// (Un)Bind a JACK client to this form.
void qjackctlPatchbayForm::setJackClient ( jack_client_t *pJackClient )
{
    m_pPatchbay->setJackClient(pJackClient);
}


// (Un)Bind a ALSA sequencer to this form.
void qjackctlPatchbayForm::setAlsaSeq ( snd_seq_t *pAlsaSeq )
{
    m_pPatchbay->setAlsaSeq(pAlsaSeq);
}


// Output socket list push button handlers gallore...

void qjackctlPatchbayForm::addOSocket()
{
    (m_pPatchbay->OSocketList())->addSocketItem();
}


void qjackctlPatchbayForm::removeOSocket()
{
    (m_pPatchbay->OSocketList())->removeSocketItem();
}


void qjackctlPatchbayForm::editOSocket()
{
    (m_pPatchbay->OSocketList())->editSocketItem();
}


void qjackctlPatchbayForm::copyOSocket()
{
    (m_pPatchbay->OSocketList())->copySocketItem();
}


void qjackctlPatchbayForm::moveUpOSocket()
{
    (m_pPatchbay->OSocketList())->moveUpSocketItem();
}


void qjackctlPatchbayForm::moveDownOSocket()
{
    (m_pPatchbay->OSocketList())->moveDownSocketItem();
}


// Input socket list push button handlers gallore...

void qjackctlPatchbayForm::addISocket()
{
    (m_pPatchbay->ISocketList())->addSocketItem();
}


void qjackctlPatchbayForm::removeISocket()
{    
    (m_pPatchbay->ISocketList())->removeSocketItem();
}


void qjackctlPatchbayForm::editISocket()
{
    (m_pPatchbay->ISocketList())->editSocketItem();
}


void qjackctlPatchbayForm::copyISocket()
{
    (m_pPatchbay->ISocketList())->copySocketItem();
}


void qjackctlPatchbayForm::moveUpISocket()
{
    (m_pPatchbay->ISocketList())->moveUpSocketItem();
}


void qjackctlPatchbayForm::moveDownISocket()
{
    (m_pPatchbay->ISocketList())->moveDownSocketItem();
}


// Connect current selected ports.
void qjackctlPatchbayForm::connectSelected (void)
{
    m_pPatchbay->connectSelected();
}


// Disconnect current selected ports.
void qjackctlPatchbayForm::disconnectSelected (void)
{
    m_pPatchbay->disconnectSelected();
}


// Disconnect all connected ports.
void qjackctlPatchbayForm::disconnectAll()
{
    m_pPatchbay->disconnectAll();
}


// end of qjackctlPatchbayForm.ui.h
