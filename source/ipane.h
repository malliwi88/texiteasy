#ifndef IPANE
#define IPANE

#include <QString>
#include <QWidget>

class IPane
{
public:
    virtual QString statusbarText() = 0;
    virtual QWidget * paneWidget() = 0;
    virtual QObject * getQObject() = 0;
    virtual QAction * action() = 0;

    virtual void beforeOpen(bool * cancelOpen) {}
    virtual void afterOpen() {}

    virtual void beforeClose(bool * cancelClose) {}
    virtual void afterClose() {}

};

#endif // IPANE

