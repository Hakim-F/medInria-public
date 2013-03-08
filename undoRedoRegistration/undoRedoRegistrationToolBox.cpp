// /////////////////////////////////////////////////////////////////
// Generated by medPluginGenerator
// /////////////////////////////////////////////////////////////////


#include "undoRedoRegistration.h"
#include "undoRedoRegistrationToolBox.h"

#include <QtGui>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractProcess.h>
#include <dtkCore/dtkAbstractViewFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medAbstractView.h>
#include <medAbstractDataImage.h>

#include <medToolBoxFactory.h>
#include <medRegistrationSelectorToolbox.h>
#include <registrationFactory/registrationFactory.h>


class undoRedoRegistrationToolBoxPrivate
{
public:
    QPushButton * undoButton;
    QPushButton * redoButton;
    QListWidget * transformationStack;
    QIcon arrowCurrentStep; 
    int currentStep;
    undoRedoRegistration * m_UndoRedo;
};

undoRedoRegistrationToolBox::undoRedoRegistrationToolBox(QWidget *parent) : medRegistrationAbstractToolBox(parent), d(new undoRedoRegistrationToolBoxPrivate)
{
    // Undo/redo Buttons
    d->undoButton = new QPushButton(QIcon(":undoRedoRegistration/icons/ArrowDown.png"),tr("Undo"),this);
    d->redoButton = new QPushButton(QIcon(":undoRedoRegistration/icons/ArrowUp.png"),tr("Redo"),this);
    
    connect(d->undoButton,SIGNAL(clicked()),this,SLOT(onUndo()));
    connect(d->redoButton,SIGNAL(clicked()),this,SLOT(onRedo()));
    
    d->arrowCurrentStep = QIcon(":undoRedoRegistration/icons/BlueArrowRight.png");
    d->currentStep = -1;

    d->m_UndoRedo = new undoRedoRegistration();
    // Transformation Stack
    d->transformationStack = new QListWidget(this);
   
    QHBoxLayout *layoutButtonUndoRedo = new QHBoxLayout;
    layoutButtonUndoRedo->addWidget(d->undoButton);
    layoutButtonUndoRedo->addWidget(d->redoButton);

    QWidget * layoutSection = new QWidget(this);
    layoutSection->setLayout(layoutButtonUndoRedo);
    addWidget(layoutSection);
    addWidget(d->transformationStack);
    this->setTitle(tr("Stack of transformations"));
    connect(registrationFactory::instance(),SIGNAL(transformationAdded(int,QStringList*)),this,SLOT(addTransformationIntoList(int, QStringList*)));
    connect(registrationFactory::instance(),SIGNAL(transformationStackReset()),this,SLOT(onTransformationStackReset()));
}

undoRedoRegistrationToolBox::~undoRedoRegistrationToolBox(void)
{
    delete d;
    
    d = NULL;
}

bool undoRedoRegistrationToolBox::registered(void)
{
    return medToolBoxFactory::instance()->
    registerToolBox<undoRedoRegistrationToolBox>("undoRedoRegistrationToolBox",
                               tr("undoRedoRegistration"),
                               tr("short tooltip description"),
                               QStringList() << "registration");
}

void undoRedoRegistrationToolBox::onUndo()
{
    if(!this->parentToolBox())
            return;
    
    if ((d->currentStep >= 0) && (d->currentStep < d->transformationStack->count()))
    {
        updatePositionArrow(d->currentStep+1);
        d->m_UndoRedo->undo();
        emit this->parentToolBox()->onUndoRedo();
    }
}

void undoRedoRegistrationToolBox::onRedo()
{
    if(!this->parentToolBox())
            return;
    if (d->currentStep>0)
    {
        updatePositionArrow(d->currentStep-1);
        d->m_UndoRedo->redo();
        emit this->parentToolBox()->onUndoRedo();
    }
}

void undoRedoRegistrationToolBox::onTransformationStackReset(void)
{
    d->currentStep=-1;
    while (d->transformationStack->count()!=0)
    {
        QListWidgetItem * tmp = d->transformationStack->takeItem(0);    
        delete tmp;
    }
}

void undoRedoRegistrationToolBox::addTransformationIntoList(int i, QStringList * methodParameters){
    if (i!=-1){
        QString buffer = methodParameters->at(0);
        for(int k = 1;k<methodParameters->size();k++)
            buffer = buffer + QString("\n") + methodParameters->at(k);
        if ((d->currentStep >= 0) && (d->currentStep < d->transformationStack->count()))
            d->transformationStack->item(d->currentStep)->setIcon(QIcon());
        for(int k = d->currentStep-1;k>=0;k--)
        {
            QListWidgetItem * tmp = d->transformationStack->takeItem(k);
            delete tmp;
        }  
        d->currentStep = 0;
        d->transformationStack->insertItem(d->currentStep,methodParameters->at(0)); 
        d->transformationStack->item(d->currentStep)->setToolTip(buffer);
        d->transformationStack->item(d->currentStep)->setIcon(d->arrowCurrentStep);
    }
}

void undoRedoRegistrationToolBox::updatePositionArrow(int newStep){
    
    if ((d->transformationStack->count() != 0) && (newStep >= 0))
    {
        if (!(d->transformationStack->count()==d->currentStep)){
            d->transformationStack->item(d->currentStep)->setIcon(QIcon());
        }
        d->currentStep = newStep;
        if (!(d->transformationStack->count()==d->currentStep))
            d->transformationStack->item(d->currentStep)->setIcon(d->arrowCurrentStep);
    }
}

void undoRedoRegistrationToolBox::setRegistrationToolBox(medRegistrationSelectorToolBox *toolbox)
{
    medRegistrationAbstractToolBox::setRegistrationToolBox(toolbox);
    toolbox->setUndoRedoProcess(d->m_UndoRedo);
}