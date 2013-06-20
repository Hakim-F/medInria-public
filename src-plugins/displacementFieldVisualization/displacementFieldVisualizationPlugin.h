
// /////////////////////////////////////////////////////////////////
// Generated by medPluginGenerator
// /////////////////////////////////////////////////////////////////

#pragma once

#include <dtkCore/dtkPlugin.h>

#include "displacementFieldVisualizationPluginExport.h"

class DISPLACEMENTFIELDVISUALIZATIONPLUGIN_EXPORT displacementFieldVisualizationPluginPrivate;

class DISPLACEMENTFIELDVISUALIZATIONPLUGIN_EXPORT displacementFieldVisualizationPlugin : public dtkPlugin
{
    Q_OBJECT
    Q_INTERFACES(dtkPlugin)
    
public:
    displacementFieldVisualizationPlugin(QObject *parent = 0);
    ~displacementFieldVisualizationPlugin();
    
    virtual bool initialize();
    virtual bool uninitialize();
    
    virtual QString name() const;
    virtual QString identifier() const;
    virtual QString description() const;
    virtual QString contact() const;
    virtual QString version() const;
    virtual QStringList authors() const;
    virtual QStringList contributors() const;
    virtual QStringList dependencies() const;
    
    virtual QStringList tags() const;
    virtual QStringList types() const;
    
private:
    displacementFieldVisualizationPluginPrivate *d;
};


