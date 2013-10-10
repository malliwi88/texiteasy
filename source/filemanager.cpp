#include "filemanager.h"
#include "widgettextedit.h"
#include "widgetpdfdocument.h"
#include "widgetpdfviewer.h"
#include "syntaxhighlighter.h"
#include <QDebug>


FileManager FileManager::Instance;

FileManager::FileManager(QObject *parent) :
    QObject(parent),
    _currentWidgetFileId(-1)
{
}

bool FileManager::newFile()
{
    if(this->currentWidgetFile())
    {
        /*if(this->currentWidgetFile()->isEmpty())
        {
            return false;
        }*/
        disconnect(this->currentWidgetFile()->widgetTextEdit(), SIGNAL(cursorPositionChanged(int,int)), this, SLOT(sendCursorPositionChanged(int,int)));
    }
    WidgetFile * newFile = new WidgetFile();
    newFile->initTheme();
    _widgetFiles.append(newFile);
    _currentWidgetFileId = _widgetFiles.count() - 1;
    connect(this->currentWidgetFile()->widgetTextEdit(), SIGNAL(cursorPositionChanged(int,int)), this, SLOT(sendCursorPositionChanged(int,int)));
    return true;
}
bool FileManager::open(QString filename)
{
    bool newWidget = this->newFile();
    this->currentWidgetFile()->open(filename);
    return newWidget;
}
void FileManager::undo()
{
    this->currentWidgetFile()->widgetTextEdit()->undo();
}

void FileManager::redo()
{
    this->currentWidgetFile()->widgetTextEdit()->redo();
}
void FileManager::copy()
{
    this->currentWidgetFile()->widgetTextEdit()->copy();
}
void FileManager::cut()
{
    this->currentWidgetFile()->widgetTextEdit()->cut();
}
void FileManager::paste()
{
    this->currentWidgetFile()->widgetTextEdit()->paste();
}
void FileManager::wrapEnvironment()
{
    this->currentWidgetFile()->widgetTextEdit()->wrapEnvironment();
}
void FileManager::jumpToPdfFromSource()
{
    this->currentWidgetFile()->widgetPdfViewer()->widgetPdfDocument()->jumpToPdfFromSource();
}
void FileManager::rehighlight()
{
    foreach(WidgetFile * widgetFile, _widgetFiles)
    {
        widgetFile->syntaxHighlighter()->rehighlight();
        widgetFile->widgetTextEdit()->onCursorPositionChange();
    }
}
void FileManager::initTheme()
{
    foreach(WidgetFile * widgetFile, _widgetFiles)
    {
        widgetFile->initTheme();
    }
}

void FileManager::close(WidgetFile *widget)
{
    int id = _widgetFiles.indexOf(widget);

    if(_currentWidgetFileId >= id && _currentWidgetFileId != 0)
    {
        --_currentWidgetFileId;
    }
    _widgetFiles.removeOne(widget);
    delete widget;
}