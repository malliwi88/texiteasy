/***************************************************************************
 *   copyright       : (C) 2013 by Quentin BRAMAS                          *
 *   http://texiteasy.com                                                  *
 *                                                                         *
 *   This file is part of texiteasy.                                          *
 *                                                                         *
 *   texiteasy is free software: you can redistribute it and/or modify        *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   texiteasy is distributed in the hope that it will be useful,             *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with texiteasy.  If not, see <http://www.gnu.org/licenses/>.       *                         *
 *                                                                         *
 ***************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "widgetlinenumber.h"
#include "widgetpdfviewer.h"
#include "widgettextedit.h"
#include "widgetscroller.h"
#include "syntaxhighlighter.h"
#include "file.h"
#include "builder.h"
#include "dialogwelcome.h"
#include "dialogconfig.h"
#include "viewer.h"
#include "widgetpdfdocument.h"
#include "dialogclose.h"
#include "widgetfindreplace.h"
#include "minisplitter.h"
#include "widgetsimpleoutput.h"
#include "widgetproject.h"

#include <QMenu>
#include <QAction>
#include <QScrollBar>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QSettings>
#include <QPushButton>
#include <QDebug>
#include <QUrl>
#include <QMimeData>
#include <QString>
#include <QPalette>
#include <QPixmap>
#include <QTableWidget>
#include <QMessageBox>
#include "configmanager.h"
#include "widgetconsole.h"
#include "widgetstatusbar.h"
#include "dialogabout.h"
#include "widgetfile.h"
#include "filemanager.h"
#include "widgettab.h"

#include <QList>

typedef QList<int> IntegerList;
Q_DECLARE_METATYPE(IntegerList)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    dialogConfig(new DialogConfig(this)),
    dialogWelcome(new DialogWelcome(this)),
    _emptyWidget(new WidgetEmpty(0))
{
    ui->setupUi(this);
    ConfigManager::Instance.setMainWindow(this);

    _tabWidget = new WidgetTab();
    connect(_tabWidget, SIGNAL(currentChanged(WidgetFile*)), this, SLOT(onCurrentFileChanged(WidgetFile*)));
    connect(_tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(_tabWidget, SIGNAL(newTabRequested()), this, SLOT(newFile()));
    ui->verticalLayout->setMargin(0);
    ui->verticalLayout->setSpacing(0);
    ui->verticalLayout->setContentsMargins(0,0,0,0);
    ui->verticalLayout->addWidget(_tabWidget);
    ui->verticalLayout->addWidget(_emptyWidget);
    connect(_emptyWidget, SIGNAL(mouseDoubleClick()), this, SLOT(newFile()));



    _widgetStatusBar = new WidgetStatusBar(this);
    ui->actionLinkSync->setChecked(ConfigManager::Instance.isPdfSynchronized());
    _widgetStatusBar->setLinkSyncAction(ui->actionLinkSync);
    this->setStatusBar(_widgetStatusBar);
    connect(&FileManager::Instance, SIGNAL(cursorPositionChanged(int,int)), _widgetStatusBar, SLOT(setPosition(int,int)));
    connect(&FileManager::Instance, SIGNAL(messageFromCurrentFile(QString)), _widgetStatusBar, SLOT(showTemporaryMessage(QString)));
    connect(&FileManager::Instance, SIGNAL(requestOpenFile(QString)), this, SLOT(open(QString)));
    QSettings settings;
    settings.beginGroup("mainwindow");
    if(settings.contains("geometry"))
    {
        this->setGeometry(settings.value("geometry").toRect());
    }

    //define background
    this->initTheme();


    // Connect menubar Actions
    connect(this->ui->actionLinkSync, SIGNAL(toggled(bool)), &FileManager::Instance, SLOT(setPdfSynchronized(bool)));
    connect(this->ui->actionLinkSync, SIGNAL(toggled(bool)), &ConfigManager::Instance, SLOT(setPdfSynchronized(bool)));
    connect(this->ui->actionDeleteLastOpenFiles,SIGNAL(triggered()),this,SLOT(clearLastOpened()));
    connect(this->ui->actionNouveau,SIGNAL(triggered()),this,SLOT(newFile()));
    connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(this->ui->actionAbout, SIGNAL(triggered()), new DialogAbout(this), SLOT(show()));
    connect(this->ui->actionOpen,SIGNAL(triggered()),this,SLOT(open()));
    connect(this->ui->actionOpenLastSession,SIGNAL(triggered()),this,SLOT(openLastSession()));
    connect(this->ui->actionSave,SIGNAL(triggered()),&FileManager::Instance,SLOT(save()));
    connect(this->ui->actionSaveAs,SIGNAL(triggered()),&FileManager::Instance,SLOT(saveAs()));
    connect(this->ui->actionOpenConfigFolder, SIGNAL(triggered()), &ConfigManager::Instance, SLOT(openThemeFolder()));
    connect(this->ui->actionSettings,SIGNAL(triggered()),this->dialogConfig,SLOT(show()));

    connect(this->dialogConfig,SIGNAL(accepted()), &FileManager::Instance,SLOT(rehighlight()));
    connect(this->ui->actionUndo, SIGNAL(triggered()), &FileManager::Instance, SLOT(undo()));
    connect(this->ui->actionRedo, SIGNAL(triggered()), &FileManager::Instance, SLOT(redo()));
    connect(this->ui->actionCopy, SIGNAL(triggered()), &FileManager::Instance, SLOT(copy()));
    connect(this->ui->actionCut, SIGNAL(triggered()), &FileManager::Instance, SLOT(cut()));
    connect(this->ui->actionPaste, SIGNAL(triggered()), &FileManager::Instance, SLOT(paste()));
    connect(this->ui->actionFindReplace, SIGNAL(triggered()), &FileManager::Instance, SLOT(openFindReplaceWidget()));
    connect(this->ui->actionEnvironment, SIGNAL(triggered()), &FileManager::Instance, SLOT(wrapEnvironment()));
    connect(this->ui->actionDefaultCommandLatex,SIGNAL(triggered()), &FileManager::Instance,SLOT(builTex()));
    connect(this->ui->actionBibtex,SIGNAL(triggered()), &FileManager::Instance,SLOT(bibtex()));
    connect(this->ui->actionView, SIGNAL(triggered()), &FileManager::Instance,SLOT(jumpToPdfFromSource()));


    connect(&FileManager::Instance, SIGNAL(filenameChanged(WidgetFile*, QString)), _tabWidget, SLOT(setTabText(WidgetFile*,QString)));
    connect(&FileManager::Instance, SIGNAL(filenameChanged(QString)), this, SLOT(addFilenameToLastOpened(QString)));

    connect(&ConfigManager::Instance, SIGNAL(versionIsOutdated()), this, SLOT(addUpdateMenu()));

    QAction * lastAction = this->ui->menuTh_me->actions().last();
    foreach(const QString& theme, ConfigManager::Instance.themesList())
    {
        QAction * action = new QAction(theme, this->ui->menuTh_me);
        action->setPriority(QAction::LowPriority);
        action->setCheckable(true);
        if(!theme.compare(ConfigManager::Instance.theme()))
        {
            action->setChecked(true);
        }
        this->ui->menuTh_me->insertAction(lastAction,action);
        connect(action, SIGNAL(triggered()), this, SLOT(changeTheme()));
    }
    this->ui->menuTh_me->insertSeparator(lastAction);

    settings.endGroup();
    lastAction = this->ui->menuOuvrir_R_cent->actions().last();
    QStringList lastFiles = settings.value("lastFiles").toStringList();
    foreach(const QString& file, lastFiles)
    {
        QAction * action = new QAction(file, this->ui->menuOuvrir_R_cent);
        action->setPriority(QAction::LowPriority);
        this->ui->menuOuvrir_R_cent->insertAction(lastAction,action);
        connect(action, SIGNAL(triggered()), this, SLOT(openLast()));
    }
    this->ui->menuOuvrir_R_cent->insertSeparator(lastAction);


    initBuildMenu();
    connect(&ConfigManager::Instance, SIGNAL(changed()), this, SLOT(initBuildMenu()));

    {
        QList<QAction *> actionsList = this->findChildren<QAction *>();
        QSettings settings;
        settings.beginGroup("shortcuts");
        //foreach(QAction * action, actionsList) {
        //    if(settings.contains(action->text()))
            {
                //action->setShortcut(QKeySequence(settings.value(action->text()).toString()));
            }
        //}
        dialogConfig->addEditableActions(actionsList);
    }

    setAcceptDrops(true);

    return;
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.beginGroup("mainwindow");
    settings.setValue("geometry", this->geometry());
    delete ui;
}

void MainWindow::focus()
{
    this->activateWindow();
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    QStringList files;
    while(_tabWidget->count())
    {
        files << _tabWidget->widget(0)->file()->getFilename();
        if(!this->closeTab(0))
        {
            event->ignore();
            return;
        }
    }
    ConfigManager::Instance.setOpenFilesWhenClosing(files);
    event->accept();
}
void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    if(!event->mimeData()->hasUrls())
    {
        event->ignore();
        return;
    }
    QList<QUrl> urlList = event->mimeData()->urls();
    foreach(const QUrl & url, urlList)
    {
        if(canBeOpened(url.toLocalFile()) || canBeInserted(url.toLocalFile()))
        {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}
void MainWindow::dragMoveEvent(QDragMoveEvent * event)
{
    event->acceptProposedAction();
}
void MainWindow::dragLeaveEvent(QDragLeaveEvent * event)
{
    event->accept();
}
bool MainWindow::handleMimeData(const QMimeData* mimeData)
{
    // check for our needed mime type, here a file or a list of files
    if (mimeData->hasUrls())
    {
        QStringList openableFiles;
        QStringList insertableFiles;
        // extract the local paths of the files
        foreach (const QUrl & url, mimeData->urls())
        {
            if(canBeOpened(url.toLocalFile()))
            {
                openableFiles.append(url.toLocalFile());
            }
            else if(canBeInserted(url.toLocalFile()))
            {
                insertableFiles.append(url.toLocalFile());
            }
        }
        if(!openableFiles.isEmpty())
        {
            foreach(const QString& file, openableFiles)
            {
                open(file);
            }
            return true;
        }
        if(!insertableFiles.isEmpty() && FileManager::Instance.currentWidgetFile())
        {
            foreach(const QString& file, insertableFiles)
            {
                FileManager::Instance.currentWidgetFile()->widgetTextEdit()->insertFile(file);
            }
            return true;
        }
    }
    return false;
}

void MainWindow::dropEvent(QDropEvent * event)
{
    if(handleMimeData(event->mimeData()))
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}
void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        this->ui->retranslateUi(this);
    }
    else
        QWidget::changeEvent(event);
}
bool MainWindow::canBeOpened(QString filename)
{
    QString ext = QFileInfo(filename).suffix();
    if(!ext.compare("tex")
            || !ext.compare("bib"))
    {
        return true;
    }
    return false;
}
bool MainWindow::canBeInserted(QString filename)
{
    QString ext = QFileInfo(filename).suffix();
    if(!ext.compare("jpeg",Qt::CaseInsensitive)
            || !ext.compare("jpg",Qt::CaseInsensitive)
            || !ext.compare("png",Qt::CaseInsensitive))
    {
        return true;
    }
    return false;
}

void MainWindow::initBuildMenu()
{
    this->ui->menuOtherBuilder->clear();
    this->ui->actionDefaultCommandLatex->setText(ConfigManager::Instance.defaultLatex());
    QStringList commandNameList = ConfigManager::Instance.latexCommandNames();
    foreach(const QString & name, commandNameList)
    {
        QAction * action = new QAction(name, this->ui->menuOtherBuilder);
        action->setPriority(QAction::LowPriority);
        if(!name.compare(ConfigManager::Instance.defaultLatex()))
        {
            delete action;
            continue;
        }
        this->ui->menuOtherBuilder->addAction(action);
        connect(action, SIGNAL(triggered()), &FileManager::Instance,SLOT(builTex()));
    }
}

void MainWindow::newFile()
{
    if(FileManager::Instance.newFile())
    {
        _tabWidget->addTab(FileManager::Instance.currentWidgetFile(), "untitled");
        _tabWidget->setCurrentIndex(_tabWidget->count()-1);
    }
    return;
}
void MainWindow::addFilenameToLastOpened(QString filename)
{
    QSettings settings;
    QFileInfo info(filename);
    settings.setValue("lastFolder",info.path());
    QString basename = info.baseName();
    //window title
    this->setWindowTitle(basename+" - texiteasy");
    //udpate the settings
    {
        QSettings settings;
        QStringList lastFiles = settings.value("lastFiles",QStringList()).toStringList();
        lastFiles.prepend(filename);
        lastFiles.removeDuplicates();
        while(lastFiles.count()>10) { lastFiles.pop_back(); }
        settings.setValue("lastFiles", lastFiles);
    }
}

void MainWindow::openLastSession()
{
    QStringList files = ConfigManager::Instance.openFilesWhenClosing();
    foreach(const QString & file, files)
    {
        open(file);
    }
}

void MainWindow::openLast()
{
    QString filename = dynamic_cast<QAction*>(sender())->text();
    open(filename);
}
void MainWindow::open()
{
    QSettings settings;
    //get the filname

    QString filename = QFileDialog::getOpenFileName(this,tr("Ouvrir un fichier"), ConfigManager::Instance.lastFolder(), ConfigManager::Extensions);
    if(filename.isEmpty())
    {
        return;
    }
    open(filename);
}
void MainWindow::open(QString filename)
{
    QSettings settings;

    //check the filename
    if(filename.isEmpty())
    {
        return;
    }
    this->addFilenameToLastOpened(filename);

    //check if it is already open
    int index = _tabWidget->indexOf(filename);
    if(index != -1)
    {
        _tabWidget->setCurrentIndex(index);
        return;
    }

    //open
    if(FileManager::Instance.open(filename))
    {
        WidgetFile * current = FileManager::Instance.currentWidgetFile();
        QString tabName = FileManager::Instance.currentWidgetFile()->file()->fileInfo().baseName();
        _tabWidget->addTab(current, tabName);
        _tabWidget->setCurrentIndex(_tabWidget->count()-1);
    }
    else
    {
        _tabWidget->setTabText(_tabWidget->currentIndex(), FileManager::Instance.currentWidgetFile()->file()->fileInfo().baseName());
    }

    this->statusBar()->showMessage(filename,4000);
    this->_widgetStatusBar->setEncoding(FileManager::Instance.currentWidgetFile()->widgetTextEdit()->getCurrentFile()->codec());
}
void MainWindow::onCurrentFileChanged(WidgetFile * widget)
{
    this->closeCurrentWidgetFile();
    FileManager::Instance.setCurrent(widget);
    if(!widget)
    {
        ui->verticalLayout->addWidget(_emptyWidget);
        _widgetStatusBar->updateButtons();
        return;
    }
    ui->verticalLayout->addWidget(widget);
    widget->widgetTextEdit()->setFocus();
    _widgetStatusBar->updateButtons();
}
bool MainWindow::closeTab(int index)
{
    WidgetFile * widget = _tabWidget->widget(index);

    if(widget->widgetTextEdit()->getCurrentFile()->isModified())
    {
        int i = QMessageBox::warning(this, trUtf8("Quitter?"),
                    trUtf8("Le fichier %1 n'a pas été enregistré.").arg(widget->file()->fileInfo().baseName()),
                                     trUtf8("Annuler"), trUtf8("Quitter sans sauvegarder"), trUtf8("Sauvegarder et quitter"));

        if(i)
        {
            if(i == 2)
            {
                widget->save();
                if(widget->widgetTextEdit()->getCurrentFile()->isModified())
                {
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
    }

    if(widget == FileManager::Instance.currentWidgetFile())
    {
        this->closeCurrentWidgetFile();
        FileManager::Instance.close(widget);
        _tabWidget->removeTab(index);
    }
    else
    {
        FileManager::Instance.close(widget);
        _tabWidget->removeTab(index);
    }
    if(!_tabWidget->count())
    {
        // The following is also call by the tabWidget but maybe after so we
        // make sure that it is call before closing the widget
        this->onCurrentFileChanged(0);
    }
    return true;
}
void MainWindow::clearLastOpened()
{
    QSettings settings;
    settings.setValue("lastFiles", QStringList());
    this->ui->menuOuvrir_R_cent->clear();
    this->ui->menuOuvrir_R_cent->insertAction(0,this->ui->actionDeleteLastOpenFiles);
}
void MainWindow::changeTheme()
{
    QString text = dynamic_cast<QAction*>(this->sender())->text();
    this->setTheme(text);

}
void MainWindow::setTheme(QString theme)
{
    foreach(QAction * action, this->ui->menuTh_me->actions())
    {
        if(action->text().compare(theme))
            action->setChecked(false);
        else
            action->setChecked(true);

    }
    ConfigManager::Instance.load(theme);
    this->initTheme();
    FileManager::Instance.rehighlight();
}
void MainWindow::initTheme()
{
    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, ConfigManager::Instance.getTextCharFormats("line-number").background().color());
    this->setAutoFillBackground(true);
    this->setPalette(Pal);
    FileManager::Instance.initTheme();
    _widgetStatusBar->initTheme();
    _tabWidget->initTheme();
}
void MainWindow::closeCurrentWidgetFile()
{
    if(ui->verticalLayout->count() > 1)
    {
        QWidget * w = ui->verticalLayout->itemAt(1)->widget();
        ui->verticalLayout->removeWidget(w);
        w->setParent(0);
    }
}
void MainWindow::addUpdateMenu()
{
    QAction * openWebsiteAction = new QAction(trUtf8("Mettre à jour TexitEasy"), this->ui->menuBar);

#ifdef OS_MAC
    /* I don't know why but on mac we cannot add an action directly in the menu Bar
     * So lets add a menu with the action
     */
    QMenu * m = new QMenu(trUtf8("Mettre à jour TexitEasy"), this->ui->menuBar);
    m->addAction(openWebsiteAction);
    this->ui->menuBar->addMenu(m);
#else
    /* On the other platform we can add an action directly in the menu Bar
     */
    this->ui->menuBar->addAction(openWebsiteAction);
#endif
    connect(openWebsiteAction, SIGNAL(triggered()), &ConfigManager::Instance, SLOT(openUpdateWebsite()));

}
