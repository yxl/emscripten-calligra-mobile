/*
 * This file is part of Maemo 5 Office UI for Calligra
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Pendurthi Kaushik <kaushiksjce@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "Accelerator.h"
#include "MainWindow.h"
#include "Common.h"
#include <QDialog>

enum  scrollingAttributes  { stateFix,scrollUp,scrollDown,scrollLeft,scrollRight,toggleScrollTransition };
enum  slidingAttributes  { nextEnabled,previousEnabled,toggleSlideTransition,crossedRange,vibrationIsSet };

AcceleratorScrollSlide::AcceleratorScrollSlide(QObject *parent) :QObject(parent)
{

    qDebug()<<"Accelerator starts";

    // ALL THE SCROLLING ATTRIBUTES SET TO RESPECTIVE STATES

    scrollingAttributes.resize(6);
    scrollingAttributes.setBit(stateFix,true);
    scrollingAttributes.setBit(toggleScrollTransition,true);
    scrollingAttributes.setBit(scrollUp,true);
    scrollingAttributes.setBit(scrollLeft,true);
    scrollingAttributes.setBit(scrollRight,true);
    scrollingAttributes.setBit(scrollDown,true);

    // ALL THE SLIDING ATTRIBUTES SET TO RESPECTIVE STATES

    slidingAttributes.resize(5);
    slidingAttributes.fill(false);

    stateFixValueX=0;
    stateFixValueY=0;
    countSteppedInRange=5;


}


AcceleratorScrollSlide::~AcceleratorScrollSlide()
{
    qDebug()<<"Accelertor ends";

}


/////////////////////////////////////////////
//////////       SLIDING       //////////////
//////////                     //////////////
/////////////////////////////////////////////

void AcceleratorScrollSlide::startSlideSettings()
{

    vibrationDialog = new QDialog;
    Q_CHECK_PTR(vibrationDialog);
    slideSetting=new QLabel("do you want vibration ?");
    Q_CHECK_PTR(slideSetting);
    spin = new QSpinBox;
    Q_CHECK_PTR(spin);
    spin->setRange(1,10);
    spin->setSingleStep(1);
    yesToVibration=new QPushButton("yes");
    Q_CHECK_PTR(yesToVibration);
    noToVibration=new QPushButton("no");
    Q_CHECK_PTR(noToVibration);
    vibrationLayout=new QVBoxLayout();
    Q_CHECK_PTR(vibrationLayout);
    vibrationLayout->addWidget(slideSetting);
    vibrationLayout->addWidget(spin);
    vibrationLayout->addWidget(yesToVibration);
    vibrationLayout->addWidget(noToVibration);
    vibrationDialog->setLayout(vibrationLayout);
    vibrationDialog->show();
    vibrationDialog->raise();

    disconnect(yesToVibration,SIGNAL(clicked()),this,SLOT(ifYesVibration()));
    connect(yesToVibration,SIGNAL(clicked()),this,SLOT(ifYesVibration()));

    disconnect(noToVibration,SIGNAL(clicked()),this,SLOT(ifNoVibration()));
    connect(noToVibration,SIGNAL(clicked()),this,SLOT(ifNoVibration()));
}

void AcceleratorScrollSlide::startRecognitionSlide()
{
    vibrationLevelPatterns
            <<VIBRATIONLEVEL1
            <<VIBRATIONLEVEL2
            <<VIBRATIONLEVEL3
            <<VIBRATIONLEVEL4
            <<VIBRATIONLEVEL5
            <<VIBRATIONLEVEL6
            <<VIBRATIONLEVEL7
            <<VIBRATIONLEVEL8
            <<VIBRATIONLEVEL9
            <<VIBRATIONLEVEL10;

    slidingAttributes.setBit(toggleSlideTransition,true);

    QTimer *timerAcceleratorSliding=new QTimer(this);
    Q_CHECK_PTR(timerAcceleratorSliding);
    QObject::connect(timerAcceleratorSliding, SIGNAL(timeout()),this,SLOT(beginSliding()));
    timerAcceleratorSliding->start(1);


}


void AcceleratorScrollSlide::beginSliding()
{
    if(slidingAttributes.at(toggleSlideTransition))
    {
    }
}

void AcceleratorScrollSlide::ifYesVibration()
{
    slidingAttributes.setBit(vibrationIsSet,true);
    vibrationValueLevel = spin->value();
    vibrationDialog->close();
    closeSlideDialog();
}


void AcceleratorScrollSlide::ifNoVibration()
{
    slidingAttributes.setBit(vibrationIsSet,false);
    vibrationDialog->close();
    closeSlideDialog();
}

void AcceleratorScrollSlide::closeSlideDialog()
{

    delete slideSetting;
    slideSetting=0;

    delete spin;
    spin=0;

    delete yesToVibration;
    yesToVibration=0;

    delete noToVibration;
    noToVibration=0;

    delete vibrationLayout;
    vibrationLayout=0;

    delete vibrationDialog;
    vibrationDialog=0;

}



void AcceleratorScrollSlide::stopRecognition()
{


    slidingAttributes.setBit(toggleSlideTransition,false);
    slidingAttributes.setBit(vibrationIsSet,false);
}


///////////////////////////////////////////////////////
/////          ACCELERATOR SCROLLING          /////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

void AcceleratorScrollSlide::startScrollSettings()
{

    scrollSettingsDialog =new QDialog;
    Q_CHECK_PTR(scrollSettingsDialog);
    addScroll=new QLabel(" enable Accerelator scrolling?   (Ctrl+G)");
    Q_CHECK_PTR(addScroll);
    yesToScroll=new QPushButton("yes");
    Q_CHECK_PTR(yesToScroll);
    noToScroll=new QPushButton("no");
    Q_CHECK_PTR(noToScroll);
    scrollLayout=new QVBoxLayout();
    Q_CHECK_PTR(scrollLayout);
    scrollLayout->addWidget(addScroll);
    scrollLayout->addWidget(yesToScroll);
    scrollLayout->addWidget(noToScroll);
    scrollSettingsDialog->setLayout(scrollLayout);
    scrollSettingsDialog->show();
    scrollSettingsDialog->raise();

    disconnect(yesToScroll,SIGNAL(clicked()),this,SLOT(ifYesScroll()));
    connect(yesToScroll,SIGNAL(clicked()),this,SLOT(ifYesScroll()));

    disconnect(noToScroll,SIGNAL(clicked()),this,SLOT(ifNoScroll()));
    connect(noToScroll,SIGNAL(clicked()),this,SLOT(ifNoScroll()));

}

void AcceleratorScrollSlide::ifNoScroll()
{
#ifdef Q_WS_MAEMO_5
    MainWindow::stopAcceleratorScrolling=true;
#endif
    scrollSettingsDialog->close();
    closeScrollDialog();
#ifdef Q_WS_MAEMO_5
    if (MainWindow::enableScrolling) {

        emit stopTheAccelerator();
    }
#endif
}

void AcceleratorScrollSlide::ifYesScroll()
{
#ifdef Q_WS_MAEMO_5
    MainWindow::stopAcceleratorScrolling=false;
#endif
    scrollSettingsDialog->close();
    closeScrollDialog();
}

void AcceleratorScrollSlide::closeScrollDialog()
{

    delete addScroll;
    addScroll=0;

    delete yesToScroll;
    yesToScroll=0;

    delete noToScroll;
    noToScroll=0;

    delete scrollLayout;
    scrollLayout=0;


    delete scrollSettingsDialog;
    scrollSettingsDialog=0;

}



void AcceleratorScrollSlide::startRecognitionScroll()
{
    scrollingAttributes.setBit(toggleScrollTransition,true);
    scrollingAttributes.setBit(stateFix,true);
    verticalScrollValue=0;
    horizontalScrollValue=0;

    QTimer *timerAcceleratorScrolling=new QTimer(this);
    Q_CHECK_PTR(timerAcceleratorScrolling);
    QObject::connect(timerAcceleratorScrolling, SIGNAL(timeout()),this,SLOT(beginScrolling()));
    timerAcceleratorScrolling->start(1);

}


void AcceleratorScrollSlide::beginScrolling()
{
}

int AcceleratorScrollSlide::getHorizontalScrollValue()
{
    return horizontalScrollValue;
}

int AcceleratorScrollSlide::getVerticalScrollValue()
{
    return verticalScrollValue;
}

void AcceleratorScrollSlide::resetScrollValues()
{
    horizontalScrollValue=verticalScrollValue=0;
}

void AcceleratorScrollSlide::stopRecognitionScroll()
{
    //distroyed vibrator scrolling
}

