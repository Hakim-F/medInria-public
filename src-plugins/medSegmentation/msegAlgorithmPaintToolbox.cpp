/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "msegAlgorithmPaintToolbox.h"

#include <medAbstractData.h>
#include <medAbstractDataImage.h>
#include <medAbstractView.h>
#include <medAbstractViewCoordinates.h>
#include <medDataIndex.h>
#include <medImageMaskAnnotationData.h>
#include <medMetaDataKeys.h>
#include <medMessageController.h>
#include <medSegmentationSelectorToolBox.h>
#include <medViewManager.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkLog/dtkLog.h>
#include <dtkCore/dtkSmartPointer.h>
#include <dtkCore/dtkGlobal.h>

#include <vnl/vnl_cross.h>
#include <vnl/vnl_vector.h>

#include <itkImageRegionIterator.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

#include <QtCore>
#include <QColorDialog>

#include <algorithm>
#include <set>

namespace mseg {

    class ClickAndMoveEventFilter : public medViewEventFilter
{
public:
    ClickAndMoveEventFilter(medSegmentationSelectorToolBox * controller, AlgorithmPaintToolbox *cb ) :
    medViewEventFilter(),
        m_cb(cb),
        m_paintState(PaintState::None),
        m_lastPaintState(PaintState::None)
    {}

    virtual bool mousePressEvent( medAbstractView *view, QMouseEvent *mouseEvent )
    {
        if(view->property("Orientation")=="3D")
            return false;
        m_paintState = m_cb->paintState();

        if ( this->m_paintState == PaintState::DeleteStroke )
        {
            m_cb->setPaintState(m_lastPaintState);
            m_paintState = m_lastPaintState;
                m_cb->addToStackIndex(view);
                m_cb->setSeedPlanted(true,posImage);
    }
        return false;
    }

        if(mouseEvent->button() == Qt::RightButton) // right-click for erasing
        {
            m_lastPaintState = m_cb->paintState();
            m_cb->setPaintState(PaintState::DeleteStroke);
            m_paintState = m_cb->paintState(); //update
        }
        m_paintState(m_cb->paintState())

        if (m_paintState == PaintState::Stroke && mouseEvent->button() == Qt::LeftButton)
        {
            m_cb->setPaintState(PaintState::Stroke);
            m_paintState = m_cb->paintState(); //update paintState
        }
            {
                m_cb->forcePaintState(PaintState::DeleteStroke);
            }
            else if(mouseEvent->button() == Qt::LeftButton)
            {
                m_cb->forcePaintState(PaintState::Stroke);
            }
        }
        m_cb->setCurrentView(view);
        medAbstractViewCoordinates * coords = view->coordinates();
        mouseEvent->accept();

        dtkAbstractData * viewData = medSegmentationSelectorToolBox::viewData( view );
        m_cb->setData( viewData ); 

        if (coords->is2D()) {
            
            // Convert mouse click to a 3D point in the image.
            QVector3D posImage = coords->displayToWorld( mouseEvent->posF() );

            if (m_paintState != PaintState::Wand)
            {
                this->m_points.push_back(posImage);
            }
            else
            {
                m_cb->updateWandRegion(view, posImage);
                m_paintState = PaintState::None; //Wand operation is over
            }
        }
        return mouseEvent->isAccepted();
    }

    virtual bool mouseMoveEvent( medAbstractView *view, QMouseEvent *mouseEvent )
    {
        if ( this->m_paintState == PaintState::None )
            return false;

        medAbstractViewCoordinates * coords = view->coordinates();
        m_cb->setCurrentView(view);
        mouseEvent->accept();

        if (coords->is2D())
        {
            QVector3D posImage = coords->displayToWorld( mouseEvent->posF() );
            //Project vector onto plane
            this->m_points.push_back(posImage);
            m_cb->updateStroke( this,view );
        }
        return mouseEvent->isAccepted();
    }

    virtual bool mouseReleaseEvent( medAbstractView *view, QMouseEvent *mouseEvent )
    {
        if ( this->m_paintState == PaintState::None )
            return false;
        m_paintState = PaintState::None; //Painting is done
        m_cb->updateStroke(this,view);
        this->m_points.clear();
            m_cb->addToStackIndex(view);
        return true;
    }

    virtual bool mouseWheelEvent(medAbstractView *view, QWheelEvent * event)
    {
        if ((m_paintState == PaintState::Stroke || m_paintState == PaintState::DeleteStroke) && (event->modifiers()==Qt::ControlModifier))
        { 
            int numDegrees = event->delta() / 8;
            int numSteps = numDegrees / 15;
            if (event->orientation() == Qt::Horizontal)
                m_cb->addBrushSize(-numSteps);
            else 
                m_cb->addBrushSize(numSteps);
            return true;
        }
        return false;
    }


    const std::vector<QVector3D> & points() const { return m_points; }

private :
    AlgorithmPaintToolbox *m_cb;
    std::vector<QVector3D> m_points;
    PaintState::E m_paintState;
    PaintState::E m_lastPaintState;
};

AlgorithmPaintToolbox::AlgorithmPaintToolbox(QWidget *parent ) :
    medSegmentationAbstractToolBox( parent),
    m_MinValueImage(0),
    m_MaxValueImage(500),
    m_strokeRadius(4),
    m_strokeLabel(1),
    m_paintState(PaintState::None)
{
    QWidget *displayWidget = new QWidget(this);
    this->addWidget(displayWidget);

    this->setTitle(this->s_name(this));

    QVBoxLayout * layout = new QVBoxLayout(displayWidget);

    m_strokeButton = new QPushButton( tr("Paint / Erase") , displayWidget);
    m_strokeButton->setToolTip(tr("Left-click: Start painting with specified label.\nRight-click: Erase painted voxels."));

    m_magicWandButton = new QPushButton(tr("Magic Wand"), displayWidget);
    QPixmap pixmap(":medSegmentation/pixmaps/magic_wand.png");
    QIcon buttonIcon(pixmap);
    m_magicWandButton->setIcon(buttonIcon);
    m_magicWandButton->setToolTip(tr("Magic wand to automatically paint similar voxels."));
    m_strokeButton->setCheckable(true);

    m_magicWandButton = new QPushButton(tr("Magic Wand"), displayWidget);
    QPixmap pixmap(":medSegmentation/pixmaps/magic_wand.png");
    QIcon buttonIcon(pixmap);
    m_magicWandButton->setIcon(buttonIcon);
    m_magicWandButton->setToolTip(tr("Magic wand to automatically paint similar voxels."));
    m_magicWandButton->setCheckable(true);

    QHBoxLayout * ButtonLayout = new QHBoxLayout();
    ButtonLayout->addWidget( m_strokeButton );
    ButtonLayout->addWidget( m_magicWandButton );
    addRemoveButtonLayout->addWidget( m_magicWandButton );
    layout->addLayout( ButtonLayout );

    m_strokeLabelSpinBox->hide();
    QHBoxLayout * brushSizeLayout = new QHBoxLayout();
    m_brushSizeSlider = new QSlider(Qt::Horizontal, displayWidget);
    m_brushSizeSlider->setToolTip(tr("Changes the brush radius."));
    m_brushSizeSlider->setValue(this->m_strokeRadius);
    m_brushSizeSlider->setRange(1, 10);
    m_brushSizeSlider->hide();
    m_brushSizeSpinBox = new QSpinBox(displayWidget);
    m_brushSizeSpinBox->setToolTip(tr("Changes the brush radius."));
    m_brushSizeSpinBox->setValue(this->m_strokeRadius);
    m_brushSizeSpinBox->setMinimum(1);
    m_brushSizeSpinBox->setMaximum(10);
    m_brushSizeSpinBox->hide();
    m_brushRadiusLabel = new QLabel(tr("Brush Radius"), displayWidget);
    m_brushRadiusLabel->hide();

    connect(m_brushSizeSpinBox, SIGNAL(valueChanged(int)),m_brushSizeSlider,SLOT(setValue(int)) );
    connect(m_brushSizeSlider,SIGNAL(valueChanged(int)),m_brushSizeSpinBox,SLOT(setValue(int)) );
    m_brushRadiusLabel = new QLabel(tr("Brush Radius"), displayWidget);
    brushSizeLayout->addWidget(m_brushRadiusLabel);
    brushSizeLayout->addWidget( m_brushSizeSlider );
    brushSizeLayout->addWidget( m_brushSizeSpinBox );
    layout->addLayout( brushSizeLayout );


    // magic wand 's  widgets //
    m_wandUpperThresholdSlider = new QSlider(Qt::Horizontal, displayWidget);
    m_wandUpperThresholdSlider->setToolTip(tr("Upper Threshold"));
    m_wandUpperThresholdSlider->setValue(100);
    m_wandUpperThresholdSlider->setMinimum(0);
    m_wandUpperThresholdSlider->setMaximum(10000);
    m_wandUpperThresholdSlider->hide();

    m_wandLowerThresholdSlider = new QSlider(Qt::Horizontal, displayWidget);
    m_wandLowerThresholdSlider->setToolTip(tr("Lower Threshold"));
    m_wandLowerThresholdSlider->setValue(100);
    m_wandLowerThresholdSlider->setMinimum(0);
    m_wandLowerThresholdSlider->setMaximum(10000);
    m_wandLowerThresholdSlider->hide();

    m_wandUpperThresholdSpinBox = new QDoubleSpinBox(displayWidget);
    m_wandUpperThresholdSpinBox->setToolTip(tr("Upper Threshold"));
    m_wandUpperThresholdSpinBox->setMinimum(0);
    m_wandUpperThresholdSpinBox->setMaximum(1000000);
    m_wandUpperThresholdSpinBox->setDecimals(2);
    m_wandUpperThresholdSpinBox->hide();

    m_wandLowerThresholdSpinBox = new QDoubleSpinBox(displayWidget);
    m_wandLowerThresholdSpinBox->setToolTip(tr("Lower Threshold"));
    m_wandLowerThresholdSpinBox->setMinimum(0);
    m_wandLowerThresholdSpinBox->setMaximum(1000000);
    m_wandLowerThresholdSpinBox->setDecimals(2);
    m_wandLowerThresholdSpinBox->hide();

    m_acceptGrowthButton = new QPushButton("Accept growth",this);
    m_acceptGrowthButton->hide();
    m_removeSeedButton = new QPushButton("Remove seed",this);
    m_removeSeedButton->hide();
    seedPlanted = false;
    
    connect(m_acceptGrowthButton,SIGNAL(clicked()),this,SLOT(onAcceptGrowth()));
    connect(m_removeSeedButton,SIGNAL(clicked()),this,SLOT(onRemoveSeed()));

    connect(m_wandUpperThresholdSlider,SIGNAL(valueChanged(int)),this,SLOT(synchronizeWandSpinBoxesAndSliders()));
    connect(m_wandUpperThresholdSpinBox, SIGNAL(editingFinished()),this,SLOT(synchronizeWandSpinBoxesAndSliders()));
    connect(m_wandLowerThresholdSlider,SIGNAL(valueChanged(int)),this,SLOT(synchronizeWandSpinBoxesAndSliders()) );
    connect(m_wandLowerThresholdSpinBox, SIGNAL(editingFinished()),this,SLOT(synchronizeWandSpinBoxesAndSliders()));

    m_wand3DCheckbox = new QCheckBox (tr("Activate 3D mode"), displayWidget);
    m_wand3DCheckbox->setCheckState(Qt::Unchecked);
    m_wand3DCheckbox->hide();

    m_wandInfo = new QLabel("Select a pixel in the image to plant the seed",this);
    m_wandInfo->hide();

    QHBoxLayout * magicWandLayout1 = new QHBoxLayout();
    magicWandLayout1->addWidget( m_wandInfo );
    magicWandLayout1->addWidget( m_wand3DCheckbox );
    QHBoxLayout * magicWandLayout2 = new QHBoxLayout();
    magicWandLayout2->addWidget( m_wandUpperThresholdSlider );
    magicWandLayout2->addWidget( m_wandUpperThresholdSpinBox );
    QHBoxLayout * magicWandLayout3 = new QHBoxLayout();
    magicWandLayout3->addWidget( m_wandLowerThresholdSlider );
    magicWandLayout3->addWidget( m_wandLowerThresholdSpinBox );
    QHBoxLayout * magicWandLayout4 = new QHBoxLayout();
    magicWandLayout4->addWidget( m_acceptGrowthButton );
    magicWandLayout4->addWidget( m_removeSeedButton );

    magicWandLayout = new QFormLayout(this);
    magicWandLayout->addRow(m_wandInfo);
    magicWandLayout->addRow(m_wand3DCheckbox);
    magicWandLayout->addRow(magicWandLayout2);
    magicWandLayout->addRow(magicWandLayout3);
    magicWandLayout->addRow(magicWandLayout4);
    
    layout->addLayout(magicWandLayout);
    this->generateLabelColorMap(24);

    QHBoxLayout * labelSelectionLayout = new QHBoxLayout();

    m_strokeLabelSpinBox = new QSpinBox(displayWidget);
    m_strokeLabelSpinBox->setToolTip(tr("Changes the painted label."));
    m_strokeLabelSpinBox->setValue(this->m_strokeLabel);
    m_strokeLabelSpinBox->setMinimum(1);
    m_strokeLabelSpinBox->setMaximum(24);
    m_strokeLabelSpinBox->hide();
    connect (m_strokeLabelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onLabelChanged(int)));

    m_labelColorWidget = new QPushButton(displayWidget);
    m_labelColorWidget->setToolTip(tr("Current label color"));
    m_labelColorWidget->setStyleSheet("background-color: rgb(255, 0, 0);border:0;border-radius: 0px;width:20px;height:20px;");
    m_labelColorWidget->setCheckable(false);
    m_labelColorWidget->setText("");
    m_labelColorWidget->hide();
    connect(m_labelColorWidget, SIGNAL(clicked()), this, SLOT(onSelectLabelColor()));

    m_colorLabel = new QLabel(tr("Label:"), displayWidget);
    m_colorLabel->hide();

    labelSelectionLayout->addStretch();
    labelSelectionLayout->addWidget(m_colorLabel );
    labelSelectionLayout->addWidget( m_labelColorWidget );
    labelSelectionLayout->addWidget( m_strokeLabelSpinBox );

    layout->addLayout( labelSelectionLayout );

    m_applyButton = new QPushButton( tr("Create Database Item") , displayWidget);
    m_applyButton->setToolTip(tr("Save result to the Temporary Database"));
    m_applyButton->hide();
    //dataButtonsLayout->addWidget( m_applyButton );
    //layout->addWidget( m_applyButton );
    
    m_clearMaskButton = new QPushButton( tr("Clear Mask") , displayWidget);
    m_clearMaskButton->setToolTip(tr("Resets the mask."));
    m_clearMaskButton->hide();
    //dataButtonsLayout->addWidget( m_clearMaskButton );
    //layout->addLayout(dataButtonsLayout);
    QHBoxLayout * dataButtonsLayout = new QHBoxLayout();
    dataButtonsLayout->addWidget(m_applyButton);
    dataButtonsLayout->addWidget(m_clearMaskButton);
    //dataButtonsLayout->addWidget(m_interpolate);
    layout->addLayout(dataButtonsLayout);

    m_clearMaskButton = new QPushButton( tr("Clear Mask") , displayWidget);
    m_clearMaskButton->setToolTip(tr("Resets the mask."));
    QHBoxLayout * dataButtonsLayout = new QHBoxLayout();
    dataButtonsLayout->addWidget(m_applyButton);
    dataButtonsLayout->addWidget(m_clearMaskButton);
    layout->addLayout(dataButtonsLayout);
    connect (m_clearMaskButton,     SIGNAL(clicked()),
        this, SLOT(onClearMaskClicked ()));


    connect (m_applyButton,     SIGNAL(clicked()),
        this, SLOT(onApplyButtonClicked()));

    connect (medViewManager::instance(), SIGNAL(viewOpened()), 
        this, SLOT(updateMouseInteraction()));

    showButtons(false);

    undo_shortcut = new QShortcut(QKeySequence(tr("Ctrl+z","Undo segmentation")),this);
    redo_shortcut = new QShortcut(QKeySequence(tr("Ctrl+y","Redo segmentation")),this);

    undoStacks = new QHash<medAbstractView*,QStack<list_pair*>*>();
    redoStacks = new QHash<medAbstractView*,QStack<list_pair*>*>();
    currentView = NULL;
    listIndexPixel = new list_pair();
    
    connect(undo_shortcut,SIGNAL(activated()),this,SLOT(onUndo()));
    connect(redo_shortcut,SIGNAL(activated()),this,SLOT(onRedo()));
}

AlgorithmPaintToolbox::~AlgorithmPaintToolbox(){}

    void AlgorithmPaintToolbox::synchronizeWandSpinBoxesAndSliders()
    {
        QObject * sender = QObject::sender();
        int val;
        if (sender == m_wandUpperThresholdSlider)
        {
            val = m_wandUpperThresholdSlider->value();
            m_wandUpperThresholdSpinBox->blockSignals(true);
            m_wandUpperThresholdSpinBox->setValue(val);
            m_wandUpperThresholdSpinBox->blockSignals(false);
        }
        else if (sender == m_wandUpperThresholdSpinBox)
        {
            val = m_wandUpperThresholdSpinBox->value();
            m_wandUpperThresholdSlider->blockSignals(true);
            m_wandUpperThresholdSlider->setValue(val);
            m_wandUpperThresholdSlider->blockSignals(false);
        }
        else if (sender == m_wandLowerThresholdSlider)
        {
            val = m_wandLowerThresholdSlider->value();
            m_wandLowerThresholdSpinBox->blockSignals(true);
            m_wandLowerThresholdSpinBox->setValue(val);
            m_wandLowerThresholdSpinBox->blockSignals(false);
        }
        else if (sender == m_wandLowerThresholdSpinBox)
        {
            val = m_wandLowerThresholdSpinBox->value();
            m_wandLowerThresholdSlider->blockSignals(true);
            m_wandLowerThresholdSlider->setValue(val);
            m_wandLowerThresholdSlider->blockSignals(false);
        }

        if (seedPlanted)
        {
            onUndo();
            updateWandRegion(currentView,seed);
            addToStackIndex(currentView);
        updateButtons();
        }
    updateButtons();
    this->m_magicWandButton->setChecked(false);
    }

void AlgorithmPaintToolbox::onStrokeToggled(bool checked)
    if (!checked ) {
        m_paintState=(PaintState::None);
        m_brushSizeSpinBox->hide();
        m_brushSizeSlider->hide();
        m_brushRadiusLabel->hide();
    setPaintState(PaintState::Stroke);
    m_brushSizeSpinBox->show();
    m_brushSizeSlider->show();
    m_brushRadiusLabel->show();
void AlgorithmPaintToolbox::onMagicWandToggled(bool checked)
{
    if (!checked ) {
        this->m_viewFilter->removeFromAllViews();
        m_paintState = (PaintState::None);
        onAcceptGrowth(); // to uncheck the button will automatically accept the current growth 
        m_wand3DCheckbox->hide();
        updateButtons();
        m_wandUpperThresholdSlider->hide();
        m_wandLowerThresholdSlider->hide();
        m_wandUpperThresholdSpinBox->hide();
        m_wandLowerThresholdSpinBox->hide();
        m_acceptGrowthButton->hide();
        m_removeSeedButton->hide();
        m_wandInfo->hide();
        return;
    }
    setPaintState(PaintState::Wand);
    updateButtons();
    this->m_strokeButton->setChecked(false);
    m_viewFilter = ( new ClickAndMoveEventFilter(this->segmentationToolBox(), this) );
    m_wand3DCheckbox->show();
    m_wandUpperThresholdSlider->show();
    m_wandLowerThresholdSlider->show();
    m_wandUpperThresholdSpinBox->show();
    m_wandLowerThresholdSpinBox->show();
    m_wandInfo->show();
    m_viewFilter = ( new ClickEventFilter(this->segmentationToolBox(), this) );
    this->segmentationToolBox()->addViewEventFilter( m_viewFilter );
}

void AlgorithmPaintToolbox::onApplyButtonClicked()
{
    dtkAbstractProcessFactory *factory = dtkAbstractProcessFactory::instance();

    dtkSmartPointer <medProcessPaintSegm> alg =
            factory->createSmartPointer( medProcessPaintSegm::s_identifier() );

    alg->setInput( this->m_maskData, medProcessPaintSegm::MaskChannel );
    alg->setInput( this->m_imageData, medProcessPaintSegm::ImageChannel );

    this->segmentationToolBox()->run( alg );
}

void AlgorithmPaintToolbox::onClearMaskClicked()
void AlgorithmPaintToolbox::onLabelChanged(int newVal)
{
    QColor labelColor = m_labelColorMap[newVal-1].second;
    m_labelColorWidget->setStyleSheet("background-color: " + labelColor.name() + ";border:0;border-radius: 0px;width:20px;height:20px;");
}

void AlgorithmPaintToolbox::onSelectLabelColor()
{
    QColor currentColor = m_labelColorMap[m_strokeLabelSpinBox->value() - 1].second;
    QColor newColor = QColorDialog::getColor(currentColor,this);

    if (newColor.isValid())
    {
        m_labelColorMap[m_strokeLabelSpinBox->value() - 1].second = newColor;
        if (m_maskAnnotationData)
        {
            m_maskAnnotationData->setColorMap(m_labelColorMap);
            m_maskAnnotationData->invokeModified();
        }

        this->onLabelChanged(m_strokeLabelSpinBox->value());
    }
}

void AlgorithmPaintToolbox::onClearMaskPressed()
{
    if ( m_maskData && m_itkMask ){
        m_itkMask->FillBuffer( medSegmentationSelectorToolBox::MaskPixelValues::Unset );
        m_itkMask->Modified();
        m_itkMask->GetPixelContainer()->Modified();
        m_itkMask->SetPipelineMTime(m_itkMask->GetMTime());

        m_maskAnnotationData->invokeModified();
    }
}

void AlgorithmPaintToolbox::setData( dtkAbstractData *dtkdata )
{
    if (!dtkdata)
        return;

    // disconnect existing
    if ( m_imageData ) {
        // TODO?
    }

    m_lastVup = QVector3D();
    m_lastVpn = QVector3D();

    m_imageData = dtkSmartPointer<dtkAbstractData>(dtkdata);
    
    // Update values of slider

    GenerateMinMaxValuesFromImage < itk::Image <char,3> > ();
    GenerateMinMaxValuesFromImage < itk::Image <unsigned char,3> > ();
    GenerateMinMaxValuesFromImage < itk::Image <short,3> > ();
    GenerateMinMaxValuesFromImage < itk::Image <unsigned short,3> > ();
    GenerateMinMaxValuesFromImage < itk::Image <int,3> > ();
    GenerateMinMaxValuesFromImage < itk::Image <unsigned int,3> > ();
    GenerateMinMaxValuesFromImage < itk::Image <long,3> > ();
    GenerateMinMaxValuesFromImage < itk::Image <unsigned long,3> > ();
    GenerateMinMaxValuesFromImage < itk::Image <float,3> > ();
    GenerateMinMaxValuesFromImage < itk::Image <double,3> > ();

    if ( m_imageData ) {
        medImageMaskAnnotationData * existingMaskAnnData = NULL;
        foreach( medAttachedData * data, m_imageData->attachedData() ) {

            if ( qobject_cast<medImageMaskAnnotationData*>(data) ) {
                existingMaskAnnData =  qobject_cast<medImageMaskAnnotationData*>(data);
                break;
            }
        }

        if ( existingMaskAnnData ) {

            m_maskAnnotationData = existingMaskAnnData;
            m_maskData = existingMaskAnnData->maskData();

        } else {

            m_maskData =
                    dtkAbstractDataFactory::instance()->createSmartPointer( medProcessPaintSegm::MaskImageTypeIdentifier() );

            if ( !m_maskData ) {
                dtkDebug() << DTK_PRETTY_FUNCTION << "Failed to create " << medProcessPaintSegm::MaskImageTypeIdentifier();
                return;
            }

        //    if ( this->m_maskAnnotationData ) {
        //        m_maskAnnotationData->parentData()->removeAttachedData(m_maskAnnotationData);
        //    }

            m_maskAnnotationData = new medImageMaskAnnotationData;
            this->initializeMaskData( m_imageData, m_maskData );
            m_maskAnnotationData->setMaskData(qobject_cast<medAbstractDataImage*>(m_maskData));

            m_maskAnnotationData->setColorMap( m_labelColorMap );

            m_imageData->addAttachedData(m_maskAnnotationData);
        }
    }

    if ( m_imageData ) {
        m_itkMask = dynamic_cast<MaskType*>( reinterpret_cast<itk::Object*>(m_maskData->data()) );
        this->showButtons(true);
    } else {
        m_itkMask = NULL;
        this->showButtons(false);
    }
}

void AlgorithmPaintToolbox::generateLabelColorMap(unsigned int numLabels)
{
    medImageMaskAnnotationData::ColorMapType colorMap;
    typedef medImageMaskAnnotationData::ColorMapType::value_type PairType;

    QColor tmpColor;
    double realHueValue = 0;
    double factor = (1.0 + sqrt(5.0)) / 2.0;
    for (unsigned int i = 0;i < numLabels;++i)
    {
        tmpColor.setHsvF(realHueValue,1.0,1.0);
        colorMap.push_back(PairType(i+1 , tmpColor));

        realHueValue += 1.0 / factor;
        if (realHueValue > 1.0)
            realHueValue -= 1.0;
    }

    m_labelColorMap = colorMap;
}

//static
medSegmentationAbstractToolBox *
    AlgorithmPaintToolbox::createInstance(QWidget *parent )
{
    return new AlgorithmPaintToolbox( parent );
}

QString AlgorithmPaintToolbox::s_description()
{
    static const QString desc = "Paint Tool";
    return desc;
}

QString AlgorithmPaintToolbox::s_identifier()
{
    static const QString id = "mseg::AlgorithmPaintToolbox";
    return id;
}

QString AlgorithmPaintToolbox::s_name(const QObject * trObj)
{
    if (!trObj)
        trObj = qApp;

    return trObj->tr( "Paint Segmentation" );
}


void AlgorithmPaintToolbox::initializeMaskData( medAbstractData * imageData, medAbstractData * maskData )
{
    MaskType::Pointer mask = MaskType::New();

    Q_ASSERT(mask->GetImageDimension() == 3);

    medAbstractDataImage * mImage = qobject_cast<medAbstractDataImage*>(imageData);
    Q_ASSERT(mImage);
    //Q_ASSERT(mask->GetImageDimension() >= mImage->Dimension());

    MaskType::RegionType region;
    region.SetSize(0, ( mImage->Dimension() > 0 ? mImage->xDimension() : 1 ) );
    region.SetSize(1, ( mImage->Dimension() > 1 ? mImage->yDimension() : 1 ) );
    region.SetSize(2, ( mImage->Dimension() > 2 ? mImage->zDimension() : 1 ) );

    MaskType::DirectionType direction;
    MaskType::SpacingType spacing;
    MaskType::PointType origin;

    direction.Fill(0);
    spacing.Fill(0);
    origin.Fill(0);
    for (unsigned int i = 0;i < mask->GetImageDimension();++i)
        direction(i,i) = 1;

    unsigned int maxIndex = std::min<unsigned int>(mask->GetImageDimension(),mImage->Dimension());

    switch (mImage->Dimension())
    {
        case 2:
        {
            itk::ImageBase <2> * imageDataOb = dynamic_cast<itk::ImageBase <2> *>( reinterpret_cast<itk::Object*>(imageData->data()) );

            for (unsigned int i = 0;i < maxIndex;++i)
            {
                for (unsigned int j = 0;j < maxIndex;++j)
                    direction(i,j) = imageDataOb->GetDirection()(i,j);

                spacing[i] = imageDataOb->GetSpacing()[i];
                origin[i] = imageDataOb->GetOrigin()[i];
            }

            break;
        }

        case 4:
        {
            itk::ImageBase <4> * imageDataOb = dynamic_cast<itk::ImageBase <4> *>( reinterpret_cast<itk::Object*>(imageData->data()) );

            for (unsigned int i = 0;i < maxIndex;++i)
            {
                for (unsigned int j = 0;j < maxIndex;++j)
                    direction(i,j) = imageDataOb->GetDirection()(i,j);

                spacing[i] = imageDataOb->GetSpacing()[i];
                origin[i] = imageDataOb->GetOrigin()[i];
            }

            break;
        }

        case 3:
        default:
        {
            itk::ImageBase <3> * imageDataOb = dynamic_cast<itk::ImageBase <3> *>( reinterpret_cast<itk::Object*>(imageData->data()) );

            for (unsigned int i = 0;i < maxIndex;++i)
            {
                for (unsigned int j = 0;j < maxIndex;++j)
                    direction(i,j) = imageDataOb->GetDirection()(i,j);

                spacing[i] = imageDataOb->GetSpacing()[i];
                origin[i] = imageDataOb->GetOrigin()[i];
            }

            break;
        }
    }

    mask->SetOrigin(origin);
    mask->SetDirection(direction);
    mask->SetSpacing(spacing);
    mask->SetLargestPossibleRegion(region);
    mask->SetBufferedRegion(region);
    mask->Allocate();
    mask->FillBuffer( medSegmentationSelectorToolBox::MaskPixelValues::Unset );

    maskData->setData((QObject*)(mask.GetPointer()));
}

    void AlgorithmPaintToolbox::updateWandRegion(medAbstractView * view, QVector3D &vec)
    {
        this->updateFromGuiItems();

        if ( !m_imageData ) {
            this->setData(this->segmentationToolBox()->viewData(view));
        }
        if (!m_imageData) {
            dtkWarn() << "Could not set data";
            return;
        }

        if ((m_imageData->identifier().contains("4"))||
            (m_imageData->identifier().contains("RGB"))||
            (m_imageData->identifier().contains("Vector"))||
            (m_imageData->identifier().contains("2")))
        {
            medMessageController::instance()->showError(tr("Magic wand option is only available for 3D images"),3000);
            return;
        }

        const medAbstractViewCoordinates * coords = view->coordinates();
        const QVector3D vpn = coords->viewPlaneNormal();

        const MaskType::DirectionType & direction = m_itkMask->GetDirection();

        typedef  MaskType::DirectionType::InternalMatrixType::element_type ElemType;
        vnl_vector_fixed<ElemType, 3> vecVpn(vpn.x(), vpn.y(), vpn.z() );

        double absDotProductMax = 0;
        unsigned int planeIndex = 0;
        for (unsigned int i = 0;i < 3;++i)
        {
            double dotProduct = 0;
            for (unsigned int j = 0;j < 3;++j)
                dotProduct += direction(j,i) * vecVpn[j];

            if (fabs(dotProduct) > absDotProductMax)
            {
                planeIndex = i;
                absDotProductMax = fabs(dotProduct);
            }
        }

        MaskType::PointType point;
        MaskType::IndexType index;

        point[0] = vec.x();
        point[1] = vec.y();
        point[2] = vec.z();

        bool isInside = m_itkMask->TransformPhysicalPointToIndex (point, index);

        if (isInside)
        {
            RunConnectedFilter < itk::Image <char,3> > (index,planeIndex);
            RunConnectedFilter < itk::Image <unsigned char,3> > (index,planeIndex);
            RunConnectedFilter < itk::Image <short,3> > (index,planeIndex);
            RunConnectedFilter < itk::Image <unsigned short,3> > (index,planeIndex);
            RunConnectedFilter < itk::Image <int,3> > (index,planeIndex);
            RunConnectedFilter < itk::Image <unsigned int,3> > (index,planeIndex);
            RunConnectedFilter < itk::Image <long,3> > (index,planeIndex);
            RunConnectedFilter < itk::Image <unsigned long,3> > (index,planeIndex);
            RunConnectedFilter < itk::Image <float,3> > (index,planeIndex);
            RunConnectedFilter < itk::Image <double,3> > (index,planeIndex);


        }
    }

    template <typename IMAGE>
    void
    AlgorithmPaintToolbox::RunConnectedFilter (MaskType::IndexType &index, unsigned int planeIndex)
    {
        IMAGE *tmpPtr = dynamic_cast<IMAGE *> ((itk::Object*)(m_imageData->data()));

        MaskType::PixelType pxValue = m_strokeLabel;

        if (!tmpPtr)
            return;

        typedef itk::ConnectedThresholdImageFilter<IMAGE, MaskType> ConnectedThresholdImageFilterType;
        typename ConnectedThresholdImageFilterType::Pointer ctiFilter = ConnectedThresholdImageFilterType::New();

        double value = tmpPtr->GetPixel(index);

        ctiFilter->SetUpper( m_wandUpperThreshold );
        ctiFilter->SetLower( m_wandLowerThreshold );

        MaskType::RegionType regionRequested = tmpPtr->GetLargestPossibleRegion();
        regionRequested.SetIndex(planeIndex, index[planeIndex]);
        regionRequested.SetSize(planeIndex, 1);
        MaskType::RegionType outRegion = regionRequested;
        outRegion.SetIndex(planeIndex,0);

        if (m_wand3DCheckbox->checkState() == Qt::Unchecked)
        {
            typename IMAGE::Pointer workPtr = IMAGE::New();
            workPtr->Initialize();
            workPtr->SetDirection(tmpPtr->GetDirection());
            workPtr->SetSpacing(tmpPtr->GetSpacing());
            workPtr->SetOrigin(tmpPtr->GetOrigin());
            workPtr->SetRegions(outRegion);
            workPtr->Allocate();

            itk::ImageRegionConstIterator < IMAGE > inputItr (tmpPtr, regionRequested);
            itk::ImageRegionIterator < IMAGE > workItr (workPtr, outRegion);

            while (!workItr.IsAtEnd())
            {
                workItr.Set(inputItr.Get());

                ++workItr;
                ++inputItr;
            }

            ctiFilter->SetInput( workPtr );
            index[planeIndex] = 0;
            ctiFilter->AddSeed( index );

            ctiFilter->Update();

            itk::ImageRegionConstIterator <MaskType> outFilterItr (ctiFilter->GetOutput(), outRegion);
            itk::ImageRegionIterator <MaskType> maskFilterItr (m_itkMask, regionRequested);
            while (!maskFilterItr.IsAtEnd())
            {
                if (outFilterItr.Get() != 0)
                {
                    maskFilterItr.Set(pxValue);
                    listIndexPixel->append(pair(maskFilterItr.GetIndex(),pxValue));
                }

                ++outFilterItr;
                ++maskFilterItr;
            }
        }
        else
        {
            ctiFilter->SetInput( tmpPtr );
            ctiFilter->AddSeed( index );

            ctiFilter->Update();

            itk::ImageRegionConstIterator <MaskType> outFilterItr (ctiFilter->GetOutput(), tmpPtr->GetLargestPossibleRegion());
            itk::ImageRegionIterator <MaskType> maskFilterItr (m_itkMask, tmpPtr->GetLargestPossibleRegion());
            while (!maskFilterItr.IsAtEnd())
            {
                if (outFilterItr.Get() != 0){
                    maskFilterItr.Set(pxValue);
                    //listIndexPixel->append(pair(maskFilterItr.GetIndex(),pxValue)); // too expensive in 3D => TO IMPROVE
                }

                ++outFilterItr;
                ++maskFilterItr;
            }
        }

        m_itkMask->Modified();
        m_itkMask->GetPixelContainer()->Modified();
        m_itkMask->SetPipelineMTime(m_itkMask->GetMTime());

        m_maskAnnotationData->invokeModified();
    }

    template <typename IMAGE>
    void
    AlgorithmPaintToolbox::GenerateMinMaxValuesFromImage ()
    {
        IMAGE *tmpPtr = dynamic_cast<IMAGE *> ((itk::Object*)(m_imageData->data()));

        if (!tmpPtr)
            return;

        typedef typename itk::MinimumMaximumImageCalculator< IMAGE > MinMaxCalculatorType;

        typename MinMaxCalculatorType::Pointer minMaxFilter = MinMaxCalculatorType::New();

        minMaxFilter->SetImage(tmpPtr);
        minMaxFilter->Compute();

        m_MinValueImage = minMaxFilter->GetMinimum();
        m_MaxValueImage = minMaxFilter->GetMaximum();
        m_wandLowerThresholdSlider->setMaximum(m_MaxValueImage);
        m_wandUpperThresholdSlider->setMaximum(m_MaxValueImage);
        m_wandLowerThresholdSlider->setMinimum(m_MinValueImage);
        m_wandUpperThresholdSlider->setMinimum(m_MinValueImage);
    }

void AlgorithmPaintToolbox::updateStroke( ClickAndMoveEventFilter * filter, medAbstractView * view)
{
    this->updateFromGuiItems();

    const double radius = m_strokeRadius; // in image units.

    if ( !m_imageData ) {
        this->setData(this->segmentationToolBox()->viewData(view));
    }
    if (!m_imageData) {
        dtkWarn() << "Could not set data";
        return;
    }

    QVector3D newPoint = filter->points().back();

    typedef  MaskType::DirectionType::InternalMatrixType::element_type ElemType;
    itk::Point< ElemType > centerPoint;
    centerPoint.SetElement(0, newPoint.x());
    centerPoint.SetElement(1, newPoint.y());
    centerPoint.SetElement(2, newPoint.z());

    const medAbstractViewCoordinates * coords = view->coordinates();
    const QVector3D vup = coords->viewUp();
    const QVector3D vpn = coords->viewPlaneNormal();

    vnl_vector_fixed<ElemType, 3> vecVup(vup.x(), vup.y(), vup.z() );
    vnl_vector_fixed<ElemType, 3> vecVpn(vpn.x(), vpn.y(), vpn.z() );
    vnl_vector_fixed<ElemType, 3> vecRight = vnl_cross_3d(vecVup,vecVpn);

    if ( vup != m_lastVup || vpn != m_lastVpn ) {
        const MaskType::SpacingType & spacing = m_itkMask->GetSpacing();

        // Rows of the direction matrix are the directions of the image i,j,k pixel directions.
        const MaskType::DirectionType & direction = m_itkMask->GetDirection();

        // project spacing onto view.

        vnl_matrix_fixed<ElemType,2,3> projMatrix;
        projMatrix.set_row(0, vecRight );
        projMatrix.set_row(1, vecVup );

        projMatrix = projMatrix * direction.GetVnlMatrix(); // (direction.GetTranspose());

        double sampleSpacing[2];
        // Compute the total projection of each of the spacings onto the view plane x & y.
        for (int i = 0; i < 2; i++) //output axis
        {
            double s = 0;  // sum squares
            double r = 0;
            for (int j = 0; j < 3; j++)
            {
                const double elem = projMatrix.get(i,j);
                const double elem2 = elem*elem;
                s += elem2*(spacing[j] >= 0 ? spacing[j] : -spacing[j]);
                r += elem2;
            }
            s /= r;
            sampleSpacing[i] = s;
        }

        // Store result.
        std::copy( sampleSpacing, sampleSpacing + 2, m_sampleSpacing);

        //Oversample
        m_sampleSpacing[0] *= 0.5;
        m_sampleSpacing[1] *= 0.5;
        m_lastVup = vup;
        m_lastVpn = vpn;
    }

    const double radius2 = radius*radius;

    const int Nx = std::max( 1, (int)std::ceil(radius/m_sampleSpacing[0]) );
    const int Ny = std::max( 1, (int)std::ceil(radius/m_sampleSpacing[1]) );

    MaskType::PixelType pxValue;
    switch ( m_paintState ) {
    case PaintState::Stroke :
        pxValue = m_strokeLabel;
        break;
    default:
        pxValue = medSegmentationSelectorToolBox::MaskPixelValues::Unset;
        break;
    }

    MaskType::IndexType index;
    itk::Point<ElemType,3> testPt;

    for ( int y(-Ny); y < Ny; ++y ) {
        double dy = y*m_sampleSpacing[1];
        for ( int x(-Nx); x < Nx; ++x ) {
            double dx = x*m_sampleSpacing[0];
            if ( dx*dx + dy*dy > radius2 )
                continue;

            for ( int ic(0); ic<3; ++ic) {
                testPt[ic] = centerPoint[ic] + dx * vecRight[ic] + dy * vecVup[ic];
            }

            bool isInside = m_itkMask->TransformPhysicalPointToIndex( testPt, index );
            if ( isInside ) {
                m_itkMask->SetPixel( index, pxValue );
                listIndexPixel->append(pair(index,pxValue));
            }
        }
    }
    m_itkMask->Modified();
    m_itkMask->GetPixelContainer()->Modified();
    m_itkMask->SetPipelineMTime(m_itkMask->GetMTime());
    m_maskAnnotationData->invokeModified();
}

void AlgorithmPaintToolbox::updateFromGuiItems()
{
    this->m_strokeRadius = m_brushSizeSlider->value();
    this->m_strokeLabel = m_strokeLabelSpinBox->value();
    this->m_wandLowerThreshold = m_wandLowerThresholdSpinBox->value();
    this->m_wandUpperThreshold = m_wandUpperThresholdSpinBox->value();
}

void AlgorithmPaintToolbox::showButtons( bool value )
{
    if (value)
    {
        m_applyButton->show();
        m_clearMaskButton->show();
    }
    else
    {
        m_applyButton->hide();
        m_clearMaskButton->hide();
    }
}

void AlgorithmPaintToolbox::updateButtons()
{
    if ( m_paintState == PaintState::None ) {
        m_wandThresholdSizeSlider->hide();
        m_wandThresholdSizeSpinBox->hide();
        m_wand3DCheckbox->hide();
            if (value==PaintState::Stroke)
                m_magicWandButton->setChecked(false);
            else 
        m_brushSizeSlider->hide();
                m_strokeButton->setChecked(false);
        m_brushSizeSpinBox->hide();
        m_brushRadiusLabel->hide();
        m_labelColorWidget->hide();
        m_strokeLabelSpinBox->hide();
        m_colorLabel->hide();
        }
    else
    {
        m_labelColorWidget->show();
        m_strokeLabelSpinBox->show();
        m_colorLabel->show();

        if ( m_paintState == PaintState::Wand ) {
            m_brushSizeSpinBox->hide();
            m_wandThresholdSizeSlider->show();
            m_wandThresholdSizeSpinBox->show();
            m_wand3DCheckbox->show();
        }
        else if ( m_paintState == PaintState::Stroke ) {
            m_brushSizeSlider->show();
            m_brushSizeSpinBox->show();
            m_brushRadiusLabel->show();
            m_wand3DCheckbox->hide();
        }
    }
}


void AlgorithmPaintToolbox::updateMouseInteraction() //Apply the current interaction (paint, ...) to a new view
{
    if (m_paintState != PaintState::None)
    {
        m_viewFilter = ( new ClickAndMoveEventFilter(this->segmentationToolBox(), this) );
        this->segmentationToolBox()->addViewEventFilter( m_viewFilter );
    }
}

void AlgorithmPaintToolbox::onUndo()
{
    if (currentView==NULL)
        return;

    if (undoStacks==NULL)
        return;

    if (!undoStacks->contains(currentView))
        return;

    QStack<list_pair*> * undo_stack = undoStacks->value(currentView);

    if (undo_stack->isEmpty())
        return;

    if (!redoStacks->contains(currentView))
        redoStacks->insert(currentView,new QStack<list_pair*>());

    QStack<list_pair*> * redo_stack = redoStacks->value(currentView);
    redo_stack->append(undo_stack->pop());

    m_itkMask->FillBuffer( medSegmentationSelectorToolBox::MaskPixelValues::Unset ); // clear mask
    for(int k =0;k<undo_stack->size();k++)
    {
        list_pair * list = undo_stack->at(k);
        for(int i = 0;i<list->size();i++)
        {
            m_itkMask->SetPixel( list->at(i).first, list->at(i).second);    
        }
    }
    m_itkMask->Modified();
    m_itkMask->GetPixelContainer()->Modified();
    m_itkMask->SetPipelineMTime(m_itkMask->GetMTime());
    m_maskAnnotationData->invokeModified();
}

void AlgorithmPaintToolbox::onRedo()
{
    if (currentView==NULL)
        return;

    if (redoStacks==NULL)
        return;

    if (!redoStacks->contains(currentView))
        return;

    QStack<list_pair*> * redo_stack = redoStacks->value(currentView);
    QStack<list_pair*> * undo_stack = undoStacks->value(currentView);

    if (redo_stack->isEmpty())
        return;

    undo_stack->append(redo_stack->pop());

    m_itkMask->FillBuffer( medSegmentationSelectorToolBox::MaskPixelValues::Unset ); // clear mask

    for(int k =0;k<undo_stack->size();k++)
    {
        list_pair * list = undo_stack->at(k);
        for(int i = 0;i<list->size();i++)
        {
            m_itkMask->SetPixel( list->at(i).first, list->at(i).second);    
        }
    }
    m_itkMask->Modified();
    m_itkMask->GetPixelContainer()->Modified();
    m_itkMask->SetPipelineMTime(m_itkMask->GetMTime());
    m_maskAnnotationData->invokeModified();
}

void AlgorithmPaintToolbox::addToStackIndex(medAbstractView * view)
{
    //TODO : We need to remove the view from the QHash when it is closed

    if (currentView==NULL)
        return;

    qDebug() << "view dans table hash : " << undoStacks->contains(view);
    if (undoStacks->contains(view))
        undoStacks->value(view)->append(new list_pair(*listIndexPixel));
    else
    {
        undoStacks->insert(view,new QStack<list_pair*>());
        undoStacks->value(view)->append(new list_pair(*listIndexPixel));
    }
    listIndexPixel->clear();
    if (redoStacks->contains(view))
            redoStacks->value(view)->clear();
}

void AlgorithmPaintToolbox::setCurrentView(medAbstractView * view){
    currentView = view;   
}

void AlgorithmPaintToolbox::addBrushSize(int size)
{
    if (m_paintState==PaintState::Stroke || m_paintState==PaintState::DeleteStroke)
        m_brushSizeSlider->setValue(m_brushSizeSlider->value()+size);
}

void AlgorithmPaintToolbox::wheelEvent(QWheelEvent * event)
{
    if (m_paintState==PaintState::Stroke || m_paintState==PaintState::DeleteStroke)
    {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        if (event->orientation() == Qt::Horizontal)
            addBrushSize(-numSteps);
        else 
            addBrushSize(numSteps);

    }
}

bool AlgorithmPaintToolbox::getSeedPlanted()
{
    return seedPlanted;
}
void AlgorithmPaintToolbox::setSeedPlanted(bool val,QVector3D seed)
{
    seedPlanted = val;
    this->seed=seed;
    if (val)
    {
        m_wandInfo->setText("Seed x : " + QString::number(seed.x()) + " mm y : " + QString::number(seed.y()) + " mm location : " + QString::number(seed.z()) + " mm"); 
        m_acceptGrowthButton->show();
        m_removeSeedButton->show();
    }
}

void AlgorithmPaintToolbox::onAcceptGrowth()
{
    seedPlanted = false;
    m_wandInfo->setText("Select a pixel in the image to plant the seed");
    m_acceptGrowthButton->hide();
    m_removeSeedButton->hide();
}

void AlgorithmPaintToolbox::onRemoveSeed()
{
    onAcceptGrowth(); // Accepting the growth will remove the seed.
    onUndo();
}

} // namespace mseg



