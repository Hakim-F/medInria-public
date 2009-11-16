// /////////////////////////////////////////////////////////////////
// Generated by dtkPluginGenerator
// /////////////////////////////////////////////////////////////////

#ifndef V3DVIEWSWITCHER_H
#define V3DVIEWSWITCHER_H

#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractView.h>

#include "v3dViewPluginExport.h"

class QMouseEvent;

class v3dViewSwitcherPrivate;

class V3DVIEWPLUGIN_EXPORT v3dViewSwitcher : public dtkAbstractView
{
    Q_OBJECT

public:
             v3dViewSwitcher(void);
    virtual ~v3dViewSwitcher(void);

    virtual QString description(void) const;

    static bool registered(void);

public:
    void reset(void);
    void clear(void);
    void update(void);

    void   link(dtkAbstractView *other);
    void unlink(dtkAbstractView *other);

    void *view(void);

    void setData(dtkAbstractData *data);
    void *data (void);

    QWidget *widget(void);

public slots:
    void onPropertySet         (QString key, QString value);
    void onOrientationPropertySet           (QString value);
    void onScalarBarVisibilityPropertySet   (QString value);
    void onLookupTablePropertySet           (QString value);
    void onBackgroundLookupTablePropertySet (QString value);
    void onOpacityPropertySet               (QString value);
    void onShowAxisPropertySet              (QString value);
    void onInteractionPropertySet           (QString value);
    void onMousePressEvent                  (QMouseEvent *event);

public slots: // Menu interface
    void onMenuAxialTriggered        (void);
    void onMenuSagittalTriggered     (void);
    void onMenuCoronalTriggered      (void);
    void onMenu3DTriggered           (void);
    void onMenuZoomTriggered         (void);
    void onMenuWindowLevelTriggered  (void);

    
private:
    v3dViewSwitcherPrivate *d;
};

dtkAbstractView *createV3dViewSwitcher(void);

#endif
