/* main.cpp --- 
 * 
 * Author: Julien Wintz
 * Copyright (C) 2008 - Julien Wintz, Inria.
 * Created: Thu Sep 17 08:29:18 2009 (+0200)
 * Version: $Id$
 * Last-Updated: Wed Jun  9 12:52:00 2010 (+0200)
 *           By: Julien Wintz
 *     Update #: 123
 */

/* Commentary: 
 * 
 */

/* Change log:
 * 
 */

#include <QtGui>
#include <QtOpenGL>

#include "medMainWindow.h"

#include <dtkCore/dtkGlobal.h>

#include <dtkScript/dtkScriptManager.h>

#include <medCore/medPluginManager.h>
#include <medCore/medDataIndex.h>

int main(int argc, char *argv[])
{
    qRegisterMetaType<medDataIndex>("medDataIndex");

    QApplication application(argc, argv);
    application.setApplicationName("medinria");
    application.setApplicationVersion("0.0.1");
    application.setOrganizationName("inria");
    application.setOrganizationDomain("fr");
	application.setWindowIcon(QIcon(":/medinria.ico"));

    medPluginManager::instance()->initialize();
    dtkScriptManager::instance()->initialize();

    medMainWindow mainwindow;
    mainwindow.show();

    if(application.arguments().contains("--wall"))
        mainwindow.setWallScreen(true);

    if(application.arguments().contains("--full"))
        mainwindow.setFullScreen(true);

    if(application.arguments().contains("--stereo")) {
       QGLFormat format;
       format.setAlpha(true);
       format.setDoubleBuffer(true);
       format.setStereo(true);
       format.setDirectRendering(true);
       QGLFormat::setDefaultFormat(format);
    }

    int status = application.exec();
    
    medPluginManager::instance()->uninitialize();
    dtkScriptManager::instance()->uninitialize();
    
    return status;
}